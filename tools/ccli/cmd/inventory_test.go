package cmd

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
)

func TestProblemRecordFormats(t *testing.T) {
	records := []statement.ProblemRecord{{
		Key: "hkoi/D1", Title: "Title", TimeMS: int64Pointer(1000), MemoryMiB: int64Pointer(256),
		CapturedAt: time.Date(2026, 9, 14, 0, 0, 0, 0, time.UTC), CaptureCount: 2,
	}}
	for _, format := range []string{"table", "json", "yaml"} {
		t.Run(format, func(t *testing.T) {
			var output bytes.Buffer
			if err := writeProblemRecords(&output, records, format); err != nil {
				t.Fatal(err)
			}
			for _, wanted := range []string{"hkoi/D1", "Title", "1000", "256"} {
				if !strings.Contains(output.String(), wanted) {
					t.Errorf("%s output missing %q: %s", format, wanted, output.String())
				}
			}
		})
	}
}

func TestUsageErrorClassification(t *testing.T) {
	if !isUsageError(usageError("bad format")) {
		t.Fatal("usage error was not classified")
	}
	if isUsageError(assertionError("runtime failure")) {
		t.Fatal("runtime error was classified as usage")
	}
}

func TestShowWritesRawMarkdownForPipedOutput(t *testing.T) {
	root := t.TempDir()
	if err := statement.EnsureRoot(root); err != nil {
		t.Fatal(err)
	}
	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Piped","url":"https://example.invalid/piped","html":"<html><body><div class=\"task\"><h2>Body</h2><p>A <strong>bold</strong> statement.</p></div></body></html>"}`)
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	if _, err := statement.StoreCapture(root, capture, result, time.Now().UTC()); err != nil {
		t.Fatal(err)
	}
	records := statement.Summarize(mustScan(t, root))
	if len(records) != 1 {
		t.Fatalf("unexpected records: %#v", records)
	}
	artifact, err := os.ReadFile(filepath.Join(root, filepath.FromSlash(records[0].Statement)))
	if err != nil {
		t.Fatal(err)
	}

	var stdout, stderr bytes.Buffer
	rootCmd.SetOut(&stdout)
	rootCmd.SetErr(&stderr)
	rootCmd.SetArgs([]string{"statement", "--output-dir", root, "show", records[0].Key})
	defer func() {
		rootCmd.SetOut(nil)
		rootCmd.SetErr(nil)
		rootCmd.SetArgs(nil)
	}()
	if err := rootCmd.Execute(); err != nil {
		t.Fatalf("show failed: %v (stderr=%s)", err, stderr.String())
	}
	if stdout.String() != string(artifact) {
		t.Fatalf("piped show output is not the stored artifact:\n%q", stdout.String())
	}
}

func mustScan(t *testing.T, root string) []statement.CaptureRecord {
	t.Helper()
	records, warnings := statement.Scan(root)
	if len(warnings) != 0 {
		t.Fatalf("unexpected scan warnings: %v", warnings)
	}
	return records
}

type assertionError string

func (e assertionError) Error() string { return string(e) }

func int64Pointer(value int64) *int64 { return &value }
