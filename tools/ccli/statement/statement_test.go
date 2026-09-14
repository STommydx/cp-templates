package statement_test

import (
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	nethtml "golang.org/x/net/html"
)

// neutralAdapter is a test double for the adapter contract. The shared package
// is source-independent, so its tests must not depend on a real adapter.
type neutralAdapter struct{}

func (neutralAdapter) ID() string { return "neutral" }

func (neutralAdapter) URLPatterns() []statement.URLPattern {
	return []statement.URLPattern{{Host: "example.invalid", PathPrefix: "/tasks/"}}
}

func (neutralAdapter) Match(_ *statement.CaptureEnvelope, document *nethtml.Node) bool {
	return statement.FindFirstClass(document, "statement") != nil
}

func (neutralAdapter) Extract(_ *statement.CaptureEnvelope, document *nethtml.Node) (statement.Extraction, error) {
	root := statement.FindFirstClass(document, "statement")
	if root == nil {
		return statement.Extraction{}, errors.New("statement root not found")
	}
	extracted := statement.ExtractSampleTable(root)
	extraction := statement.Extraction{
		Root:          root,
		Samples:       extracted.Samples,
		SamplesAnchor: extracted.Anchor,
		Excluded:      extracted.Excluded,
		Metadata: statement.ProblemMetadata{
			Identity: statement.ProblemIdentity{Code: "NEUTRAL", Title: "Neutral Statement"},
		},
	}
	// A source may only replace its sample table when every row was extracted.
	if !extracted.Complete {
		extraction.Samples = nil
		extraction.SamplesAnchor = nil
		extraction.Excluded = nil
		extraction.Warnings = []string{"sample table was not extracted completely; table preserved"}
	}
	return extraction, nil
}

func TestParseStatementFixtureThroughAdapter(t *testing.T) {
	result := parse(t, neutralCapture(t, "https://example.invalid/tasks/neutral", string(mustRead(t, "testdata/statement.html"))), statement.ParseOptions{})

	if result.Mode != "task-page" || result.Metadata.Source.Adapter != "neutral" {
		t.Fatalf("unexpected parser provenance: mode=%s adapter=%s", result.Mode, result.Metadata.Source.Adapter)
	}
	if result.Metadata.Identity.Code != "NEUTRAL" {
		t.Fatalf("unexpected code: %q", result.Metadata.Identity.Code)
	}
	if len(result.Warnings) != 0 {
		t.Fatalf("unexpected warnings: %v", result.Warnings)
	}
	if len(result.Metadata.Samples) != 3 || result.Metadata.Samples[0].Input != "8 17\r\n2 7 1 8 2 4 5 1" {
		t.Fatalf("unexpected samples: %#v", result.Metadata.Samples)
	}

	for _, wanted := range []string{
		"## Description",
		"## Sample Tests",
		"### Sample 1",
		"```text\n4 1 4\n```",
		"| Situation | Required behavior |",
		"| Multiple segments have the same length | Choose the smallest left endpoint |",
		"has brightness 18 and the smallest left endpoint.",
		"![The window never reaches the threshold](https://example.invalid/figures/window.png)",
		"1 + 2 + 6 = 9",
		"2×10^{5}",
		"R_{S}",
		"  - outer item\n    - inner item",
		"a\\`b",
		"Optional hint",
		"<math",
	} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
	if samples, scoring := strings.Index(result.Markdown, "### Sample 1"), strings.Index(result.Markdown, "## Scoring"); samples < 0 || scoring < 0 || samples > scoring {
		t.Errorf("samples must render where the source table was, before later sections:\n%s", result.Markdown)
	}
	for _, unwanted := range []string{"Neutral Statement\n\n## Neutral Statement", "this content must not appear", "class=\"samples\"", "Run samples", "\nExplanation\n"} {
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
	result, err := statement.ParseCapture(capture, []statement.Adapter{neutralAdapter{}}, statement.ParseOptions{})
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
	raw := neutralCapture(t, "https://example.invalid/tasks/neutral", string(mustRead(t, "testdata/statement.html")))
	capture, err := statement.DecodeCapture(raw)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{neutralAdapter{}}, statement.ParseOptions{})
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
	html := `<html><body><div class="statement"><p>before&#27;[31mred&#27;[0m after</p><p>c1&#155;31mred</p><p><a href="javascript:alert(1)">bad link</a></p><p><a href="https://example.invalid/ok">good link</a></p><svg onload="alert(1)"><text>diagram label</text><script>alert(1)</script></svg></div></body></html>`
	result := parse(t, neutralCapture(t, "https://example.invalid/tasks/neutral", html), statement.ParseOptions{})

	for _, r := range result.Markdown {
		if r == 0x1b || (r >= 0x80 && r <= 0x9f) {
			t.Fatalf("Markdown kept control character %U:\n%q", r, result.Markdown)
		}
	}
	if strings.Contains(result.Markdown, "javascript:") {
		t.Fatalf("Markdown kept an unsafe link destination:\n%s", result.Markdown)
	}
	if strings.Contains(result.Markdown, "onload") {
		t.Fatalf("Markdown kept an event handler:\n%s", result.Markdown)
	}
	for _, wanted := range []string{"https://example.invalid/ok", "diagram label"} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown dropped preserved content %q:\n%s", wanted, result.Markdown)
		}
	}
}

func TestParseRejectsPathologicalDOMDepth(t *testing.T) {
	deep := strings.Repeat("<div>", 2000) + "text" + strings.Repeat("</div>", 2000)
	if _, err := statement.ParseCapture(mustDecode(t, neutralCapture(t, "https://example.invalid/tasks/neutral", deep)), []statement.Adapter{neutralAdapter{}}, statement.ParseOptions{}); err == nil {
		t.Fatal("deeply nested document was accepted")
	}
}

func TestStoreBoundsLongProblemCode(t *testing.T) {
	capture := mustDecode(t, neutralCapture(t, "https://example.invalid/tasks/neutral", string(mustRead(t, "testdata/statement.html"))))
	result, err := statement.ParseCapture(capture, []statement.Adapter{neutralAdapter{}}, statement.ParseOptions{})
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

func parse(t *testing.T, body []byte, options statement.ParseOptions) statement.ParseResult {
	t.Helper()
	result, err := statement.ParseCapture(mustDecode(t, body), []statement.Adapter{neutralAdapter{}}, options)
	if err != nil {
		t.Fatal(err)
	}
	return result
}

func mustDecode(t *testing.T, body []byte) *statement.CaptureEnvelope {
	t.Helper()
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	return capture
}

func neutralCapture(t *testing.T, url, html string) []byte {
	t.Helper()
	body, err := json.Marshal(map[string]any{
		"type":       "statement-html",
		"version":    1,
		"capturedAt": "2026-09-13T00:00:00Z",
		"title":      "Neutral Statement",
		"url":        url,
		"html":       html,
	})
	if err != nil {
		t.Fatal(err)
	}
	return body
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

func TestRendererPreservesNestedAndQuotedContent(t *testing.T) {
	html := "<html><body><div class=\"statement\"><ul><li>outer<ul><li>inner</li></ul></li></ul><p><code>a`b</code></p><pre>first\n```\nsecond</pre><table class=\"samples\"><tr><th>#</th><th>Input</th><th>Output</th></tr><tr class=\"sample\"><td>1</td><td class=\"io\"><pre>x</pre></td><td class=\"io\"><pre>y</pre></td></tr><tr><td colspan=\"3\">first explanation</td></tr><tr><td colspan=\"3\">second explanation</td></tr></table></div></body></html>"
	result := parse(t, neutralCapture(t, "https://example.invalid/tasks/neutral", html), statement.ParseOptions{})

	for _, wanted := range []string{"- outer\n  - inner", "a\\`b", "first explanation", "second explanation"} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
}
