package statement_test

import (
	"strings"
	"testing"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
)

func TestURLMappingAndForcedAdapter(t *testing.T) {
	body := `{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Editor","url":"https://JUDGE.HKOI.ORG/task/LN-1?view=full#section","html":"<html><body><div class=\"task\"><div class=\"task-displayid\">LN-1</div></div></body></html>"}`
	capture, err := statement.DecodeCapture([]byte(body))
	if err != nil {
		t.Fatal(err)
	}
	result, err := statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{})
	if err != nil {
		t.Fatal(err)
	}
	if result.Mode != "task-page" || result.Metadata.Source.Adapter != "hkoi" {
		t.Fatalf("URL mapping failed: %#v", result)
	}
	capture.URL = "https://judge.hkoi.org/schooladmin/hosted/LN-1/edit"
	result, err = statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{})
	if err != nil {
		t.Fatal(err)
	}
	if result.Mode != "fallback" || !strings.Contains(result.Markdown, "fallback") {
		t.Fatalf("editor URL did not fall back: %#v", result)
	}
	_, err = statement.ParseCapture(capture, []statement.Adapter{hkoi.New()}, statement.ParseOptions{ForcedAdapterID: "missing"})
	if err == nil || !strings.Contains(err.Error(), "unknown adapter") {
		t.Fatalf("unknown forced adapter error: %v", err)
	}
}
