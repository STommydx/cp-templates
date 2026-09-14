package statement_test

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
)

func TestInventoryRejectsSymlinksAndSkipsStaging(t *testing.T) {
	root := t.TempDir()
	captureDir := filepath.Join(root, "hkoi", "D1-title", "20260914T000000.000000Z")
	if err := os.MkdirAll(captureDir, 0o700); err != nil {
		t.Fatal(err)
	}
	metadata := statement.ProblemMetadata{
		SchemaVersion: 1,
		Mode:          "task-page",
		Source: statement.SourceMetadata{
			Adapter:    "hkoi",
			URL:        "https://judge.hkoi.org/task/D1",
			CapturedAt: time.Unix(0, 0).UTC(),
			ReceivedAt: time.Unix(1, 0).UTC(),
		},
		Identity: statement.ProblemIdentity{Code: "D1", Title: "Title"},
	}
	metadataBytes, _ := json.Marshal(metadata)
	for name, content := range map[string][]byte{
		"problem.json": metadataBytes,
		"capture.json": []byte(`{}`),
		"statement.md": []byte("# Title\n"),
	} {
		if err := os.WriteFile(filepath.Join(captureDir, name), content, 0o600); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.MkdirAll(filepath.Join(root, ".staging", "partial"), 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, ".staging", "partial", "problem.json"), []byte("not json"), 0o600); err != nil {
		t.Fatal(err)
	}
	symlinkDir := filepath.Join(root, "hkoi", "symlink")
	if err := os.MkdirAll(symlinkDir, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Symlink(filepath.Join(captureDir, "problem.json"), filepath.Join(symlinkDir, "problem.json")); err != nil {
		t.Fatal(err)
	}

	records, warnings := statement.Scan(root)
	if len(records) != 1 || records[0].Key != "hkoi/D1" {
		t.Fatalf("unexpected records: %#v", records)
	}
	foundSymlinkWarning := false
	for _, warning := range warnings {
		foundSymlinkWarning = foundSymlinkWarning || strings.Contains(warning.Error(), "symlink")
	}
	if !foundSymlinkWarning {
		t.Fatalf("inventory did not warn about symlink: %v", warnings)
	}
}
