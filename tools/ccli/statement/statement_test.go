package statement_test

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
)

func TestDecodeAndParseTaskPage(t *testing.T) {
	htmlBytes := mustRead(t, "testdata/task.html")
	raw := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00.000Z","title":"Lantern Network","url":"https://example.invalid/tasks/lantern","html":` + mustJSON(string(htmlBytes)) + `,"futureField":"ignored"}`)
	capture, err := statement.DecodeCapture(raw)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	if result.Mode != "task-page" || result.Metadata.Source.Adapter != "hkoi" {
		t.Fatalf("unexpected parser provenance: %#v", result.Metadata.Source)
	}
	if result.Metadata.Identity.Code != "LN-1" {
		t.Fatalf("unexpected code: %q", result.Metadata.Identity.Code)
	}
	if result.Metadata.Limits.TimeMS == nil || *result.Metadata.Limits.TimeMS != 1000 {
		t.Fatalf("unexpected time limit: %#v", result.Metadata.Limits.TimeMS)
	}
	if result.Metadata.Limits.MemoryMiB == nil || *result.Metadata.Limits.MemoryMiB != 256 {
		t.Fatalf("unexpected memory limit: %#v", result.Metadata.Limits.MemoryMiB)
	}
	if len(result.Metadata.Samples) != 2 || result.Metadata.Samples[0].Input != "3\n" {
		t.Fatalf("unexpected samples: %#v", result.Metadata.Samples)
	}
	for _, wanted := range []string{"## Task", "### Sample 1", "```text\n3\n```", "The first lantern reaches itself.", "<math>", "<table class=\"score\">", "breadth first search"} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
	for _, unwanted := range []string{"Duplicate Lantern Network heading", "this content must not appear", "class=\"samples\"", "Run samples"} {
		if strings.Contains(result.Markdown, unwanted) {
			t.Errorf("Markdown contains excluded content %q:\n%s", unwanted, result.Markdown)
		}
	}
}

func TestFallbackRetainsBodyWithoutSamples(t *testing.T) {
	capture, err := statement.DecodeCapture([]byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Unknown page","url":"https://example.invalid/unknown","html":"<html><body><h1>Visible heading</h1><p>Visible text.</p></body></html>"}`))
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{})
	if err != nil {
		t.Fatal(err)
	}
	if result.Mode != "fallback" || result.Metadata.Source.Adapter != statement.FallbackAdapterID {
		t.Fatalf("unexpected fallback result: %#v", result)
	}
	if !strings.Contains(result.Markdown, "Visible text.") || strings.Contains(result.Markdown, "### Sample") {
		t.Fatalf("unexpected fallback Markdown: %s", result.Markdown)
	}
	if len(result.Warnings) == 0 {
		t.Fatal("fallback did not record a warning")
	}
}

func TestStorePreservesRawAndCollisions(t *testing.T) {
	htmlBytes := mustRead(t, "testdata/task.html")
	raw := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Lantern Network","url":"https://example.invalid/tasks/lantern","html":` + mustJSON(string(htmlBytes)) + `}`)
	capture, err := statement.DecodeCapture(raw)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	root := t.TempDir()
	received := time.Date(2026, 9, 13, 1, 2, 3, 456789000, time.UTC)
	first, err := statement.StoreCapture(root, capture, result, received)
	if err != nil {
		t.Fatal(err)
	}
	second, err := statement.StoreCapture(root, capture, result, received)
	if err != nil {
		t.Fatal(err)
	}
	if first == second || !strings.HasSuffix(second, "-01") {
		t.Fatalf("collision was not suffixed: %q, %q", first, second)
	}
	stored, err := os.ReadFile(filepath.Join(root, filepath.FromSlash(first), "capture.json"))
	if err != nil {
		t.Fatal(err)
	}
	if string(stored) != string(raw) {
		t.Fatalf("raw capture changed: %q != %q", stored, raw)
	}
	captures, warnings := statement.Scan(root)
	if len(warnings) != 0 || len(captures) != 2 {
		t.Fatalf("inventory scan: captures=%d warnings=%v", len(captures), warnings)
	}
	records := statement.Summarize(captures)
	if len(records) != 1 || records[0].CaptureCount != 2 || records[0].SamplesCount != 2 {
		t.Fatalf("unexpected summary: %#v", records)
	}
}

func mustRead(t *testing.T, name string) []byte {
	t.Helper()
	content, err := os.ReadFile(name)
	if err != nil {
		t.Fatal(err)
	}
	return content
}

func mustJSON(value string) string {
	encoded, _ := json.Marshal(value)
	return string(encoded)
}
