package cmd

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"

	"charm.land/log/v2"
	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
	huma "github.com/danielgtaylor/huma/v2"
	"github.com/danielgtaylor/huma/v2/adapters/humachi"
	"github.com/go-chi/chi/v5"
	"github.com/go-chi/httplog/v3"
)

func TestStatementAPIContract(t *testing.T) {
	root := t.TempDir()
	if err := statement.EnsureRoot(root); err != nil {
		t.Fatal(err)
	}
	router := chi.NewRouter()
	api := humachi.New(router, huma.DefaultConfig("ccli statement server", "1.0.0"))
	registerStatementAPI(api, root, []statement.Adapter{hkoi.New()}, "hkoi", log.NewWithOptions(io.Discard, log.Options{}))
	server := httptest.NewServer(router)
	defer server.Close()

	html := `<html><body><div class="task"><div class="task-displayid">T-1</div><div class="task-info">Time limit: 1.000 s Memory limit: 256 MB</div><p>Neutral text.</p></div></body></html>`
	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Neutral","url":"https://example.invalid/tasks/neutral","html":` + mustJSONForCommandTest(html) + `,"future":true}`)

	response := doRequest(t, server.URL+"/", http.MethodPost, "application/json", body)
	if response.StatusCode != http.StatusCreated {
		t.Fatalf("valid capture status: %d body=%s", response.StatusCode, readBody(response))
	}
	var payload struct {
		Mode      string `json:"mode"`
		Directory string `json:"directory"`
		Capture   string `json:"capture"`
	}
	decodeResponse(t, response, &payload)
	if payload.Mode != "task-page" || payload.Directory == "" || payload.Capture == "" {
		t.Fatalf("unexpected capture response: %#v", payload)
	}
	stored, err := os.ReadFile(filepath.Join(root, filepath.FromSlash(payload.Capture)))
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(stored, body) {
		t.Fatal("capture.json did not preserve request bytes")
	}
	exactBody := captureBodyOfSize(statement.MaxCaptureBytes)
	exactResponse := doRequest(t, server.URL+"/", http.MethodPost, "application/json", exactBody)
	if exactResponse.StatusCode != http.StatusCreated {
		t.Fatalf("exact-size capture status=%d body=%s", exactResponse.StatusCode, readBody(exactResponse))
	}
	exactResponse.Body.Close()
	invalidURLBody := bytes.Replace(body, []byte("https://example.invalid/tasks/neutral"), []byte("mailto:local@example.invalid"), 1)

	checks := []struct {
		name        string
		method      string
		path        string
		contentType string
		body        []byte
		status      int
	}{
		{name: "malformed", method: http.MethodPost, path: "/", contentType: "application/json", body: []byte("{"), status: http.StatusBadRequest},
		{name: "schema", method: http.MethodPost, path: "/", contentType: "application/json", body: []byte(`{"type":"wrong"}`), status: http.StatusUnprocessableEntity},
		{name: "content type", method: http.MethodPost, path: "/", contentType: "text/plain", body: []byte("{}"), status: http.StatusUnsupportedMediaType},
		{name: "wrong path", method: http.MethodPost, path: "/other", contentType: "application/json", body: body, status: http.StatusNotFound},
		{name: "oversize", method: http.MethodPost, path: "/", contentType: "application/json", body: bytes.Repeat([]byte{'x'}, statement.MaxCaptureBytes+1), status: http.StatusRequestEntityTooLarge},
		{name: "semantic url", method: http.MethodPost, path: "/", contentType: "application/json", body: invalidURLBody, status: http.StatusUnprocessableEntity},
	}
	for _, check := range checks {
		t.Run(check.name, func(t *testing.T) {
			response := doRequest(t, server.URL+check.path, check.method, check.contentType, check.body)
			if response.StatusCode != check.status {
				t.Fatalf("status=%d want=%d body=%s", response.StatusCode, check.status, readBody(response))
			}
			response.Body.Close()
		})
	}

	health := doRequest(t, server.URL+"/health", http.MethodGet, "", nil)
	if health.StatusCode != http.StatusNoContent {
		t.Fatalf("health status=%d", health.StatusCode)
	}
	health.Body.Close()

	openapi := doRequest(t, server.URL+"/openapi.json", http.MethodGet, "", nil)
	if openapi.StatusCode != http.StatusOK {
		t.Fatalf("openapi status=%d", openapi.StatusCode)
	}
	var spec struct {
		Paths map[string]map[string]struct {
			Responses map[string]json.RawMessage `json:"responses"`
		} `json:"paths"`
	}
	decodeResponse(t, openapi, &spec)
	for _, status := range []string{"201", "400", "413", "415", "422", "500"} {
		if _, ok := spec.Paths["/"]["post"].Responses[status]; !ok {
			t.Errorf("OpenAPI missing POST response %s", status)
		}
	}
	if _, ok := spec.Paths["/health"]["get"].Responses["204"]; !ok {
		t.Error("OpenAPI missing health 204 response")
	}
}
func doRequest(t *testing.T, url, method, contentType string, body []byte) *http.Response {
	t.Helper()
	request, err := http.NewRequest(method, url, bytes.NewReader(body))
	if err != nil {
		t.Fatal(err)
	}
	if contentType != "" {
		request.Header.Set("Content-Type", contentType)
	}
	response, err := http.DefaultClient.Do(request)
	if err != nil {
		t.Fatal(err)
	}
	return response
}

func decodeResponse(t *testing.T, response *http.Response, value any) {
	t.Helper()
	defer response.Body.Close()
	if err := json.NewDecoder(response.Body).Decode(value); err != nil {
		t.Fatal(err)
	}
}

func readBody(response *http.Response) string {
	defer response.Body.Close()
	body, _ := io.ReadAll(response.Body)
	return strings.TrimSpace(string(body))
}

func mustJSONForCommandTest(value string) string {
	encoded, _ := json.Marshal(value)
	return string(encoded)
}
func captureBodyOfSize(size int) []byte {
	prefix := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Exact","url":"https://example.invalid/exact","html":"`)
	suffix := []byte(`"}`)
	return append(append(prefix, []byte(strings.Repeat("x", size-len(prefix)-len(suffix)))...), suffix...)
}
func TestRenderStatementMarkdown(t *testing.T) {
	raw := "# Neutral title\n\nA **bold** statement."
	rendered, err := renderStatementMarkdown(raw, defaultStatementWidth)
	if err != nil {
		t.Fatal(err)
	}
	plain := stripANSI(rendered)
	if plain == raw {
		t.Fatalf("Markdown was not rendered: %q", rendered)
	}
	if !strings.Contains(plain, "Neutral title") || !strings.Contains(plain, "bold statement") {
		t.Fatalf("rendered output lost content: %q", plain)
	}
	if !strings.Contains(rendered, "\x1b[") {
		t.Fatalf("terminal rendering emitted no styling: %q", rendered)
	}
}

func TestRenderStatementMarkdownWrapsToWidth(t *testing.T) {
	long := strings.Repeat("word ", 40) + "end."
	narrow := stripANSI(renderStatementMarkdownForWidth(t, long, 40))
	wide := stripANSI(renderStatementMarkdownForWidth(t, long, 200))
	if narrow == wide {
		t.Fatal("wrapping width was ignored")
	}
	for _, line := range strings.Split(narrow, "\n") {
		if len(line) > 60 {
			t.Fatalf("line exceeded the requested width: %q", line)
		}
	}
}

var ansiPattern = regexp.MustCompile("\x1b\\[[0-9;?]*[a-zA-Z]")

func stripANSI(value string) string {
	return ansiPattern.ReplaceAllString(value, "")
}

func renderStatementMarkdownForWidth(t *testing.T, markdown string, width int) string {
	t.Helper()
	rendered, err := renderStatementMarkdown(markdown, width)
	if err != nil {
		t.Fatal(err)
	}
	return rendered
}

func TestRenderStatementMarkdownHonorsGlamourStyle(t *testing.T) {
	t.Setenv("GLAMOUR_STYLE", "notty")
	if rendered := renderStatementMarkdownForWidth(t, "# Title\n\nBody text.", 40); strings.Contains(rendered, "\x1b[") {
		t.Fatalf("GLAMOUR_STYLE was ignored: %q", rendered)
	}
}

func TestStatementServerLogsCapturesAndFailures(t *testing.T) {
	var logged bytes.Buffer
	logger := log.NewWithOptions(&logged, log.Options{Formatter: log.JSONFormatter})
	server := newLoggingTestServer(t, logger)
	defer server.Close()

	body := []byte(`{"type":"statement-html","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"Neutral","url":"https://example.invalid/tasks/neutral","html":"<html><body><div class=\"task\"><h2>Body</h2><p>Neutral text.</p></div></body></html>"}`)
	doRequest(t, server.URL+"/", http.MethodPost, "application/json", body).Body.Close()
	doRequest(t, server.URL+"/other", http.MethodPost, "application/json", body).Body.Close()
	doRequest(t, server.URL+"/health", http.MethodGet, "", nil).Body.Close()

	records := decodeLogRecords(t, logged.String())
	stored := findLogRecord(records, "capture stored")
	if stored == nil {
		t.Fatalf("capture was not logged: %v", records)
	}
	if stored["mode"] != "task-page" || !strings.HasPrefix(fmt.Sprint(stored["key"]), "hkoi/") {
		t.Fatalf("unexpected capture record: %#v", stored)
	}
	if findLogRecord(records, "parser warning") == nil {
		t.Errorf("parser warning was not logged: %v", records)
	}
	if failed := findLogRecordContaining(records, "HTTP 404"); failed == nil {
		t.Errorf("rejected request was not logged: %v", records)
	} else if failed["level"] != "warn" {
		t.Errorf("rejected request was logged at %v", failed["level"])
	}
	if findLogRecordContaining(records, "204") != nil {
		t.Errorf("successful health request was logged at info level: %v", records)
	}
}

func TestStatementServerLogsAllRequestsAtDebug(t *testing.T) {
	var logged bytes.Buffer
	logger := log.NewWithOptions(&logged, log.Options{Level: log.DebugLevel, Formatter: log.JSONFormatter})
	server := newLoggingTestServer(t, logger)
	defer server.Close()

	doRequest(t, server.URL+"/health", http.MethodGet, "", nil).Body.Close()

	if findLogRecordContaining(decodeLogRecords(t, logged.String()), "204") == nil {
		t.Errorf("debug did not log a successful request: %q", logged.String())
	}
}

// newLoggingTestServer installs the request logging wiring the serve command uses.
func newLoggingTestServer(t *testing.T, logger *log.Logger) *httptest.Server {
	t.Helper()
	root := t.TempDir()
	if err := statement.EnsureRoot(root); err != nil {
		t.Fatal(err)
	}
	router := chi.NewRouter()
	router.Use(httplog.RequestLogger(slog.New(logger), &httplog.Options{Level: requestLogLevel(logger)}))
	api := humachi.New(router, huma.DefaultConfig("ccli statement server", "1.0.0"))
	registerStatementAPI(api, root, []statement.Adapter{hkoi.New()}, "hkoi", logger)
	return httptest.NewServer(router)
}

func decodeLogRecords(t *testing.T, output string) []map[string]any {
	t.Helper()
	var records []map[string]any
	for _, line := range strings.Split(strings.TrimSpace(output), "\n") {
		if line == "" {
			continue
		}
		var record map[string]any
		if err := json.Unmarshal([]byte(line), &record); err != nil {
			t.Fatalf("log line is not JSON: %v (%q)", err, line)
		}
		records = append(records, record)
	}
	return records
}

func findLogRecord(records []map[string]any, message string) map[string]any {
	for _, record := range records {
		if record["msg"] == message {
			return record
		}
	}
	return nil
}

func findLogRecordContaining(records []map[string]any, fragment string) map[string]any {
	for _, record := range records {
		if message, ok := record["msg"].(string); ok && strings.Contains(message, fragment) {
			return record
		}
	}
	return nil
}
