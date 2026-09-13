package cmd

import (
	"bytes"
	"encoding/json"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"

	huma "github.com/danielgtaylor/huma/v2"
	"github.com/danielgtaylor/huma/v2/adapters/humago"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
)

func TestStatementAPIContract(t *testing.T) {
	root := t.TempDir()
	if err := statement.EnsureRoot(root); err != nil {
		t.Fatal(err)
	}
	mux := http.NewServeMux()
	api := humago.New(mux, huma.DefaultConfig("ccli statement server", "1.0.0"))
	registerStatementAPI(api, root, []statement.Adapter{hkoi.New()}, "hkoi")
	server := httptest.NewServer(statementServerHandler{next: mux})
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
