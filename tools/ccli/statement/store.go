package statement

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/adrg/xdg"
)

const (
	// DefaultDataSubdir is the durable XDG-relative storage location.
	DefaultDataSubdir = "ccli/statements"
	receiptLayout     = "20060102T150405.000000Z"
)

// ResolveRoot returns the explicit absolute override or the platform XDG data root.
func ResolveRoot(override string) (string, error) {
	if override != "" {
		absolute, err := filepath.Abs(override)
		if err != nil {
			return "", err
		}
		return absolute, nil
	}
	return filepath.Join(xdg.DataHome, DefaultDataSubdir), nil
}

// EnsureRoot creates the durable storage root with private permissions.
func EnsureRoot(root string) error {
	if root == "" {
		return errors.New("storage root is empty")
	}
	if err := os.MkdirAll(root, 0o700); err != nil {
		return err
	}
	return os.Chmod(root, 0o700)
}

// CleanupStaging removes interrupted writes without touching completed captures.
func CleanupStaging(root string) error {
	staging := filepath.Join(root, ".staging")
	if err := os.MkdirAll(staging, 0o700); err != nil {
		return err
	}
	entries, err := os.ReadDir(staging)
	if err != nil {
		return err
	}
	for _, entry := range entries {
		if err := os.RemoveAll(filepath.Join(staging, entry.Name())); err != nil {
			return err
		}
	}
	return nil
}

// StoreCapture atomically writes one immutable capture and returns its root-relative directory.
func StoreCapture(root string, capture *CaptureEnvelope, result ParseResult, receivedAt time.Time) (string, error) {
	if capture == nil {
		return "", errors.New("capture is nil")
	}
	if err := EnsureRoot(root); err != nil {
		return "", err
	}
	adapterID := result.Metadata.Source.Adapter
	if result.Mode == "fallback" || adapterID == "" {
		adapterID = FallbackAdapterID
	}
	identity := safeComponent(result.Metadata.Identity.Code)
	if identity == "" {
		identity = "url-" + urlHash(capture.URL)
	}
	slug := result.Metadata.Identity.Slug
	if slug == "" {
		slug = slugify(result.Metadata.Identity.Title)
	}
	if slug == "" {
		slug = "problem"
	}
	logicalDir := identity + "-" + slug
	parent := filepath.Join(root, adapterID, logicalDir)
	if err := os.MkdirAll(parent, 0o700); err != nil {
		return "", err
	}
	if err := os.Chmod(parent, 0o700); err != nil {
		return "", err
	}
	stamp := receivedAt.UTC().Format(receiptLayout)

	stagingRoot := filepath.Join(root, ".staging")
	if err := os.MkdirAll(stagingRoot, 0o700); err != nil {
		return "", err
	}
	stage, err := os.MkdirTemp(stagingRoot, "capture-")
	if err != nil {
		return "", err
	}
	defer os.RemoveAll(stage)
	if err := os.Chmod(stage, 0o700); err != nil {
		return "", err
	}

	raw := capture.RawBytes()
	if len(raw) == 0 {
		raw, err = json.Marshal(capture)
		if err != nil {
			return "", fmt.Errorf("marshal capture: %w", err)
		}
	}
	metadata := result.Metadata
	metadata.SchemaVersion = MetadataSchemaVersion
	metadata.Mode = result.Mode
	metadata.Source.ReceivedAt = receivedAt.UTC()
	metadata.Source.URL = capture.URL
	metadata.Source.CapturedAt, _ = capture.CaptureTime()
	metadata.Identity.Title = result.Metadata.Identity.Title
	if metadata.Identity.Title == "" {
		metadata.Identity.Title = capture.Title
	}
	metadata.Identity.Slug = slug
	metadata.Warnings = append([]string(nil), result.Warnings...)
	metadataJSON, err := json.MarshalIndent(metadata, "", "  ")
	if err != nil {
		return "", fmt.Errorf("marshal metadata: %w", err)
	}
	metadataJSON = append(metadataJSON, '\n')

	files := map[string][]byte{
		"capture.json": raw,
		"problem.json": metadataJSON,
		"statement.md": []byte(result.Markdown),
	}
	for name, content := range files {
		if err := writePrivateFile(filepath.Join(stage, name), content); err != nil {
			return "", err
		}
	}

	for suffix := 0; ; suffix++ {
		name := stamp
		if suffix > 0 {
			name += "-" + fmt.Sprintf("%02d", suffix)
		}
		finalDir := filepath.Join(parent, name)
		err := os.Rename(stage, finalDir)
		if err == nil {
			relative, relErr := filepath.Rel(root, finalDir)
			if relErr != nil {
				return "", relErr
			}
			return filepath.ToSlash(relative), nil
		}
		if !errors.Is(err, os.ErrExist) {
			return "", err
		}
		// A collision leaves the stage intact for the next suffix.
	}
}

func writePrivateFile(path string, content []byte) error {
	file, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0o600)
	if err != nil {
		return err
	}
	if _, err := file.Write(content); err != nil {
		_ = file.Close()
		return err
	}
	return file.Close()
}

func urlHash(rawURL string) string {
	u, err := normalizeURL(rawURL)
	if err != nil {
		u = rawURL
	}
	sum := sha256.Sum256([]byte(u))
	return hex.EncodeToString(sum[:])[:12]
}

func normalizeURL(rawURL string) (string, error) {
	parsed, err := parseURL(rawURL)
	if err != nil {
		return "", err
	}
	parsed.Scheme = strings.ToLower(parsed.Scheme)
	parsed.Host = strings.ToLower(parsed.Host)
	parsed.Fragment = ""
	return parsed.String(), nil
}

func parseURL(rawURL string) (*url.URL, error) {
	return url.Parse(rawURL)
}

func safeComponent(value string) string {
	var builder strings.Builder
	for _, runeValue := range value {
		allowed := (runeValue >= 'a' && runeValue <= 'z') || (runeValue >= 'A' && runeValue <= 'Z') || (runeValue >= '0' && runeValue <= '9') || runeValue == '.' || runeValue == '_' || runeValue == '-'
		if allowed {
			builder.WriteRune(runeValue)
		} else if builder.Len() > 0 && !strings.HasSuffix(builder.String(), "-") {
			builder.WriteByte('-')
		}
	}
	component := strings.Trim(builder.String(), ".-")
	if len(component) <= 80 {
		return component
	}
	sum := sha256.Sum256([]byte(value))
	suffix := "-" + hex.EncodeToString(sum[:])[:12]
	return strings.TrimRight(component[:80-len(suffix)], ".-") + suffix
}

func slugify(value string) string {
	value = strings.ToLower(strings.TrimSpace(value))
	var builder strings.Builder
	lastDash := false
	for _, runeValue := range value {
		allowed := (runeValue >= 'a' && runeValue <= 'z') || (runeValue >= '0' && runeValue <= '9') || runeValue == '.' || runeValue == '_' || runeValue == '-'
		if allowed {
			builder.WriteRune(runeValue)
			lastDash = runeValue == '-'
			continue
		}
		if !lastDash && builder.Len() > 0 {
			builder.WriteByte('-')
			lastDash = true
		}
	}
	return strings.Trim(strings.TrimSuffix(builder.String(), "-"), ".")[:min(80, len(strings.Trim(strings.TrimSuffix(builder.String(), "-"), ".")))]
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
