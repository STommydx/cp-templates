package statement

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

// ProblemRecord is the newest logical-problem summary emitted by inventory commands.
type ProblemRecord struct {
	Key           string    `json:"key" yaml:"key"`
	Adapter       string    `json:"adapter" yaml:"adapter"`
	Code          string    `json:"code,omitempty" yaml:"code,omitempty"`
	Title         string    `json:"title" yaml:"title"`
	TimeMS        *int64    `json:"time_ms,omitempty" yaml:"time_ms,omitempty"`
	MemoryMiB     *int64    `json:"memory_mib,omitempty" yaml:"memory_mib,omitempty"`
	CapturedAt    time.Time `json:"captured_at" yaml:"captured_at"`
	ReceivedAt    time.Time `json:"received_at" yaml:"received_at"`
	CaptureCount  int       `json:"capture_count" yaml:"capture_count"`
	WarningsCount int       `json:"warnings_count" yaml:"warnings_count"`
	SamplesCount  int       `json:"samples_count" yaml:"samples_count"`
	Directory     string    `json:"directory" yaml:"directory"`
	Statement     string    `json:"statement" yaml:"statement"`
}

// CaptureRecord describes one durable capture before logical grouping.
type CaptureRecord struct {
	ProblemRecord
	CaptureName string
}

// Scan recursively reads valid capture records and returns non-fatal warnings separately.
func Scan(root string) ([]CaptureRecord, []error) {
	var records []CaptureRecord
	var warnings []error
	if _, err := os.Stat(root); errors.Is(err, os.ErrNotExist) {
		return records, warnings
	} else if err != nil {
		return records, []error{err}
	}
	err := filepath.WalkDir(root, func(path string, entry fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			warnings = append(warnings, fmt.Errorf("scan %s: %w", path, walkErr))
			return nil
		}
		if entry.IsDir() {
			if path != root && strings.HasPrefix(entry.Name(), ".") {
				return fs.SkipDir
			}
			return nil
		}
		if entry.Name() != "problem.json" {
			return nil
		}
		captureDir := filepath.Dir(path)
		metadataBytes, err := os.ReadFile(path)
		if err != nil {
			warnings = append(warnings, fmt.Errorf("read %s: %w", path, err))
			return nil
		}
		var metadata ProblemMetadata
		if err := json.Unmarshal(metadataBytes, &metadata); err != nil {
			warnings = append(warnings, fmt.Errorf("decode %s: %w", path, err))
			return nil
		}
		if metadata.Source.Adapter == "" || metadata.Source.URL == "" {
			warnings = append(warnings, fmt.Errorf("incomplete metadata %s", path))
			return nil
		}
		relativeDir, err := filepath.Rel(root, captureDir)
		if err != nil {
			warnings = append(warnings, fmt.Errorf("relative path %s: %w", path, err))
			return nil
		}
		adapterID := metadata.Source.Adapter
		if metadata.Mode == "fallback" {
			adapterID = FallbackAdapterID
		}
		logicalIdentity := metadata.Identity.Code
		if logicalIdentity == "" {
			logicalIdentity = "url-" + urlHash(metadata.Source.URL)
		}
		statementPath := filepath.Join(captureDir, "statement.md")
		capturePath := filepath.Join(captureDir, "capture.json")
		if _, err := os.Stat(statementPath); err != nil {
			warnings = append(warnings, fmt.Errorf("missing statement for %s: %w", path, err))
			return nil
		}
		if _, err := os.Stat(capturePath); err != nil {
			warnings = append(warnings, fmt.Errorf("missing capture for %s: %w", path, err))
			return nil
		}
		captureName := filepath.Base(captureDir)
		receivedAt := metadata.Source.ReceivedAt
		if receivedAt.IsZero() {
			receivedAt = receiptTime(captureName)
		}
		records = append(records, CaptureRecord{
			CaptureName: captureName,
			ProblemRecord: ProblemRecord{
				Key:           adapterID + "/" + logicalIdentity,
				Adapter:       adapterID,
				Code:          metadata.Identity.Code,
				Title:         metadata.Identity.Title,
				TimeMS:        metadata.Limits.TimeMS,
				MemoryMiB:     metadata.Limits.MemoryMiB,
				CapturedAt:    metadata.Source.CapturedAt,
				ReceivedAt:    receivedAt,
				WarningsCount: len(metadata.Warnings),
				SamplesCount:  len(metadata.Samples),
				Directory:     filepath.ToSlash(relativeDir),
				Statement:     filepath.ToSlash(filepath.Join(relativeDir, "statement.md")),
			},
		})
		return nil
	})
	if err != nil {
		warnings = append(warnings, err)
	}
	return records, warnings
}

// Summarize groups captures by logical key and selects the newest record per key.
func Summarize(records []CaptureRecord) []ProblemRecord {
	groups := make(map[string][]CaptureRecord)
	for _, record := range records {
		groups[record.Key] = append(groups[record.Key], record)
	}
	result := make([]ProblemRecord, 0, len(groups))
	for key, captures := range groups {
		sort.SliceStable(captures, func(i, j int) bool {
			if captures[i].ReceivedAt.Equal(captures[j].ReceivedAt) {
				return captures[i].CaptureName > captures[j].CaptureName
			}
			return captures[i].ReceivedAt.After(captures[j].ReceivedAt)
		})
		newest := captures[0].ProblemRecord
		newest.Key = key
		newest.CaptureCount = len(captures)
		result = append(result, newest)
	}
	sort.Slice(result, func(i, j int) bool { return result[i].Key < result[j].Key })
	return result
}

// FindCapture resolves a canonical key, unique code, or URL-hash identity.
func FindCapture(records []CaptureRecord, identifier, captureName string) (CaptureRecord, error) {
	matches := make([]CaptureRecord, 0)
	for _, record := range records {
		if record.Key == identifier || record.Code == identifier || strings.HasSuffix(record.Key, "/"+identifier) {
			matches = append(matches, record)
		}
	}
	keys := make(map[string]struct{})
	for _, record := range matches {
		keys[record.Key] = struct{}{}
	}
	if len(keys) > 1 {
		var canonical []string
		for key := range keys {
			canonical = append(canonical, key)
		}
		sort.Strings(canonical)
		return CaptureRecord{}, fmt.Errorf("ambiguous problem %q; use one of: %s", identifier, strings.Join(canonical, ", "))
	}
	if len(matches) == 0 {
		return CaptureRecord{}, fmt.Errorf("problem %q not found", identifier)
	}
	if captureName != "" {
		for _, record := range matches {
			if record.CaptureName == captureName {
				return record, nil
			}
		}
		return CaptureRecord{}, fmt.Errorf("capture %q not found for %s", captureName, identifier)
	}
	sort.SliceStable(matches, func(i, j int) bool {
		if matches[i].ReceivedAt.Equal(matches[j].ReceivedAt) {
			return matches[i].CaptureName > matches[j].CaptureName
		}
		return matches[i].ReceivedAt.After(matches[j].ReceivedAt)
	})
	return matches[0], nil
}

func receiptTime(name string) time.Time {
	base := name
	if len(base) > len(receiptLayout) {
		base = base[:len(receiptLayout)]
	}
	parsed, err := time.Parse(receiptLayout, base)
	if err != nil {
		return time.Time{}
	}
	return parsed
}
