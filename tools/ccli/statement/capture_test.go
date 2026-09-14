package statement_test

import (
	"errors"
	"testing"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
)

func TestDecodeCaptureValidationKinds(t *testing.T) {
	cases := []struct {
		name string
		body string
		kind statement.ValidationKind
	}{
		{name: "malformed", body: "{", kind: statement.ValidationMalformed},
		{name: "wrong type", body: `{"type":"other","version":1,"capturedAt":"2026-09-13T00:00:00Z","title":"","url":"https://example.invalid/a","html":"<p>x</p>"}`, kind: statement.ValidationSchema},
		{name: "wrong version", body: `{"type":"statement-html","version":2,"capturedAt":"2026-09-13T00:00:00Z","title":"","url":"https://example.invalid/a","html":"<p>x</p>"}`, kind: statement.ValidationSchema},
	}
	for _, testCase := range cases {
		t.Run(testCase.name, func(t *testing.T) {
			_, err := statement.DecodeCapture([]byte(testCase.body))
			if err == nil {
				t.Fatal("expected validation error")
			}
			var validationErr *statement.CaptureValidationError
			if !errors.As(err, &validationErr) || validationErr.Kind != testCase.kind {
				t.Fatalf("unexpected error: %v", err)
			}
		})
	}
}
