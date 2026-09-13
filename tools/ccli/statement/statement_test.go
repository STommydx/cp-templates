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
	if result.Metadata.Identity.Code != "M17" {
		t.Fatalf("unexpected code: %q", result.Metadata.Identity.Code)
	}
	if result.Metadata.Limits.TimeMS == nil || *result.Metadata.Limits.TimeMS != 2000 {
		t.Fatalf("unexpected time limit: %#v", result.Metadata.Limits.TimeMS)
	}
	if result.Metadata.Limits.MemoryMiB == nil || *result.Metadata.Limits.MemoryMiB != 256 {
		t.Fatalf("unexpected memory limit: %#v", result.Metadata.Limits.MemoryMiB)
	}
	if result.Metadata.Limits.TimeRaw != "Time limit: 2 seconds" {
		t.Fatalf("unexpected raw time limit: %q", result.Metadata.Limits.TimeRaw)
	}
	if result.Metadata.Execution.Interactive == nil || *result.Metadata.Execution.Interactive {
		t.Fatalf("unexpected interactive metadata: %#v", result.Metadata.Execution.Interactive)
	}
	if len(result.Metadata.Samples) != 3 || result.Metadata.Samples[0].Input != "8 17\n2 7 1 8 2 4 5 1\n" {
		t.Fatalf("unexpected samples: %#v", result.Metadata.Samples)
	}
	for _, wanted := range []string{"## Description", "## Constraints", "### Sample 1", "```text\n4 1 4\n```", "smallest left endpoint", "<math>", "<table class=\"details\">", "Optional hint"} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
	for _, unwanted := range []string{"M17 — Lantern Network\n\n## Lantern Network", "this content must not appear", "class=\"samples\"", "Run samples"} {
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
	if len(records) != 1 || records[0].CaptureCount != 2 || records[0].SamplesCount != 3 {
		t.Fatalf("unexpected summary: %#v", records)
	}
}

func TestRendererStripsActiveContent(t *testing.T) {
	htmlBytes := `<html><body><div class="task-info"><div class="task-displayid">SEC</div><span>Time limit: 1.000 s</span><span>Memory limit: 256 MB</span></div><div class="task"><p>before&#27;[31mred&#27;[0m after</p><p><a href="javascript:alert(1)">bad link</a></p><p><a href="https://example.invalid/ok">good link</a></p><svg onload="alert(1)"></svg></div></body></html>`
	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Security","url":"https://example.invalid/security","html":` + mustJSON(htmlBytes) + `}`)
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	if strings.ContainsRune(result.Markdown, 0x1b) {
		t.Fatalf("Markdown kept a terminal escape byte:\n%q", result.Markdown)
	}
	if strings.Contains(result.Markdown, "javascript:") {
		t.Fatalf("Markdown kept an unsafe link destination:\n%s", result.Markdown)
	}
	if strings.Contains(result.Markdown, "onload") {
		t.Fatalf("Markdown kept an event handler:\n%s", result.Markdown)
	}
	if !strings.Contains(result.Markdown, "https://example.invalid/ok") {
		t.Fatalf("Markdown dropped a safe link:\n%s", result.Markdown)
	}
}

func TestParseRejectsPathologicalDOMDepth(t *testing.T) {
	deep := strings.Repeat("<div>", 2000) + "text" + strings.Repeat("</div>", 2000)
	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Deep","url":"https://example.invalid/deep","html":` + mustJSON(deep) + `}`)
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"}); err == nil {
		t.Fatal("deeply nested document was accepted")
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
func TestStoreBoundsLongProblemCode(t *testing.T) {
	htmlBytes := mustRead(t, "testdata/task.html")
	raw := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Lantern Network","url":"https://example.invalid/tasks/long","html":` + mustJSON(string(htmlBytes)) + `}`)
	capture, err := statement.DecodeCapture(raw)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	result.Metadata.Identity.Code = strings.Repeat("D", 1000)
	directory, err := statement.StoreCapture(t.TempDir(), capture, result, time.Now().UTC())
	if err != nil {
		t.Fatal(err)
	}
	logicalDirectory := filepath.Base(filepath.Dir(filepath.FromSlash(directory)))
	if len(logicalDirectory) > 255 {
		t.Fatalf("logical storage path too long: %d", len(logicalDirectory))
	}
}
func TestRendererPreservesNestedAndQuotedContent(t *testing.T) {
	html := "<html><body><div class=\"task-info\"><div class=\"task-displayid\">EDGE</div><span>Time limit: 1.000 s</span><span>Memory limit: 256 MB</span></div><div class=\"task\"><ul><li>outer<ul><li>inner</li></ul></li></ul><p><code>a`b</code></p><pre>first\n```\nsecond</pre><div class=\"samples-wrapper\"><table class=\"samples\"><tr><th>#</th><th>Input</th><th>Output</th></tr><tr class=\"sample\"><td>1</td><td class=\"io\"><pre>x</pre></td><td class=\"io\"><pre>y</pre></td></tr><tr><td colspan=\"3\">first explanation</td></tr><tr><td colspan=\"3\">second explanation</td></tr></table></div></div></body></html>"
	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Edge","url":"https://example.invalid/edge","html":` + mustJSON(html) + `}`)
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "hkoi"})
	if err != nil {
		t.Fatal(err)
	}
	for _, wanted := range []string{"- outer\n  - inner", "a\\`b", "first explanation", "second explanation"} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
}
