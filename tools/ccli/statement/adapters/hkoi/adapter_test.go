package hkoi_test

import (
	"encoding/json"
	"os"
	"slices"
	"strings"
	"testing"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
)

func TestAdapterIdentityAndURLPatterns(t *testing.T) {
	adapter := hkoi.New()
	if adapter.ID() != "hkoi" {
		t.Fatalf("unexpected adapter id: %q", adapter.ID())
	}
	// The published mapping selects real captures, so it is asserted literally.
	want := []statement.URLPattern{{Host: "judge.hkoi.org", PathPrefix: "/task/"}}
	if !slices.Equal(adapter.URLPatterns(), want) {
		t.Fatalf("URL patterns = %#v, want %#v", adapter.URLPatterns(), want)
	}
}

// TestURLSelectionUsesAdapterPatterns proves the adapter only claims its own pages.
func TestURLSelectionUsesAdapterPatterns(t *testing.T) {
	html := string(mustRead(t, "testdata/lantern.html"))
	for _, check := range []struct {
		name     string
		url      string
		wantMode string
	}{
		{name: "matching host and prefix", url: "https://judge.hkoi.org/task/UDEV", wantMode: "task-page"},
		{name: "matching host without the statement prefix", url: "https://judge.hkoi.org/schooladmin/hosted/UDEV", wantMode: "fallback"},
		{name: "unrelated host", url: "https://example.invalid/task/UDEV", wantMode: "fallback"},
	} {
		t.Run(check.name, func(t *testing.T) {
			result := parse(t, captureBody(t, check.url, html), statement.ParseOptions{})
			if result.Mode != check.wantMode {
				t.Fatalf("mode=%s want=%s", result.Mode, check.wantMode)
			}
		})
	}
}

// TestStructuralMatchFailureFallsBack proves a matching URL with an unexpected
// body uses the fallback namespace instead of parsing the wrong structure.
func TestStructuralMatchFailureFallsBack(t *testing.T) {
	result := parse(t, captureBody(t, "https://judge.hkoi.org/task/NOEX", `<html><body><p>plain page</p></body></html>`), statement.ParseOptions{})

	if result.Mode != "fallback" || result.Metadata.Source.Adapter != statement.FallbackAdapterID {
		t.Fatalf("unexpected fallback: mode=%s adapter=%s", result.Mode, result.Metadata.Source.Adapter)
	}
	if len(result.Warnings) == 0 {
		t.Fatal("structural mismatch did not produce a warning")
	}
}

func TestExtractReadsRealPageStructure(t *testing.T) {
	result := parse(t, captureBody(t, "https://example.invalid/task/UDEV", string(mustRead(t, "testdata/lantern.html"))), statement.ParseOptions{ForcedAdapterID: "hkoi"})

	if result.Mode != "task-page" || result.Metadata.Source.Adapter != "hkoi" {
		t.Fatalf("unexpected provenance: mode=%s adapter=%s", result.Mode, result.Metadata.Source.Adapter)
	}
	if result.Metadata.Identity.Code != "UDEV" || result.Metadata.Identity.Title != "UDEV Lantern Network" {
		t.Fatalf("unexpected identity: %#v", result.Metadata.Identity)
	}
	limits := result.Metadata.Limits
	if limits.TimeMS == nil || *limits.TimeMS != 1000 || limits.TimeRaw != "Time Limit: 1.000 s" {
		t.Fatalf("unexpected time limit: %#v", limits)
	}
	if limits.MemoryMiB == nil || *limits.MemoryMiB != 256 || limits.MemoryRaw != "Memory Limit: 256 MB" {
		t.Fatalf("unexpected memory limit: %#v", limits)
	}
	if len(result.Warnings) != 0 {
		t.Fatalf("unexpected warnings: %v", result.Warnings)
	}

	samples := result.Metadata.Samples
	if len(samples) != 3 {
		t.Fatalf("expected 3 samples, got %d: %#v", len(samples), samples)
	}
	// data-raw carries the exact input bytes, including the source CRLF endings.
	if samples[0].Input != "8 17\r\n2 7 1 8 2 4 5 1" || samples[0].Output != "4 1 4" {
		t.Fatalf("unexpected first sample: %#v", samples[0])
	}
	if samples[1].Output != "-1 -1" || samples[2].Output != "3 2 4" {
		t.Fatalf("unexpected sample outputs: %#v", samples)
	}

	for _, wanted := range []string{
		"### Sample 1",
		"### Sample 3",
		"```text\n4 1 4\n```",
		"2 + 7 + 1 + 8 = 18",
		"No segment of length 1 or 2 is sufficient.",
		"![The running window never reaches the threshold](https://example.invalid/figures/lantern-threshold.png)",
		"| Situation | Required behavior |",
		"Optional hint",
		"<math",
	} {
		if !strings.Contains(result.Markdown, wanted) {
			t.Errorf("Markdown missing %q:\n%s", wanted, result.Markdown)
		}
	}
	for _, unwanted := range []string{
		"Online Judge",                // navbar chrome
		"Switch to Problem Statement", // code editor chrome
		"Streams",                     // modal dialog
		"class=\"samples\"",           // extracted sample table
		"run-sample",                  // sample run cells
		"UDEV Lantern Network\n\n# Lantern Network\n\nUDEV", // duplicate sr-only heading outside the title
	} {
		if strings.Contains(result.Markdown, unwanted) {
			t.Errorf("Markdown kept excluded content %q:\n%s", unwanted, result.Markdown)
		}
	}
	if strings.Count(result.Markdown, "# Lantern Network") != 1 {
		t.Errorf("statement heading was duplicated or lost:\n%s", result.Markdown)
	}
}

// TestStatementRootIgnoresBodyClass covers the site toggling classes on the body
// element, which must not select the page chrome as the statement.
func TestStatementRootIgnoresBodyClass(t *testing.T) {
	html := `<html><body class="task dark"><nav>Navigation chrome</nav><div class="task-info"><div class="task-displayid">SHAD</div></div><div class="task"><h2>Body</h2><p>Statement content.</p></div></body></html>`
	result := parse(t, captureBody(t, "https://example.invalid/task/SHAD", html), statement.ParseOptions{ForcedAdapterID: "hkoi"})

	if !strings.Contains(result.Markdown, "Statement content.") {
		t.Fatalf("statement content was lost:\n%s", result.Markdown)
	}
	if strings.Contains(result.Markdown, "Navigation chrome") {
		t.Fatalf("body class selected the page chrome:\n%s", result.Markdown)
	}
}

func TestExtractWarnsWithoutSampleTable(t *testing.T) {
	html := `<html><body><div class="task-info"><div class="task-displayid">NOEX</div></div><div class="task"><h2>Body</h2><p>No examples here.</p></div></body></html>`
	result := parse(t, captureBody(t, "https://example.invalid/task/NOEX", html), statement.ParseOptions{ForcedAdapterID: "hkoi"})

	if len(result.Warnings) == 0 {
		t.Fatal("missing sample table did not produce a warning")
	}
	if !strings.Contains(result.Markdown, "No examples here.") {
		t.Fatalf("fallback statement content was lost:\n%s", result.Markdown)
	}
}

func parse(t *testing.T, body []byte, options statement.ParseOptions) statement.ParseResult {
	t.Helper()
	capture, err := statement.DecodeCapture(body)
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, options)
	if err != nil {
		t.Fatal(err)
	}
	return result
}

func captureBody(t *testing.T, url, html string) []byte {
	t.Helper()
	encoded, err := json.Marshal(map[string]any{
		"type":       "statement-html",
		"version":    1,
		"capturedAt": "2026-09-13T00:00:00Z",
		"title":      "UDEV Lantern Network",
		"url":        url,
		"html":       html,
	})
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}

func mustRead(t *testing.T, name string) []byte {
	t.Helper()
	content, err := os.ReadFile(name)
	if err != nil {
		t.Fatal(err)
	}
	return content
}
