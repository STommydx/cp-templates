package statement

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"net/url"
	"time"
)

// MaxCaptureBytes is the maximum accepted PageMole request size.
const MaxCaptureBytes = 8 << 20

// ValidationKind distinguishes malformed JSON from valid JSON that violates the envelope schema.
type ValidationKind string

const (
	ValidationMalformed ValidationKind = "malformed"
	ValidationSchema    ValidationKind = "schema"
)

// CaptureValidationError reports a malformed or schema-invalid PageMole envelope.
type CaptureValidationError struct {
	Kind ValidationKind
	Err  error
}

// Error implements error for CaptureValidationError.
func (e *CaptureValidationError) Error() string { return e.Err.Error() }

// Unwrap exposes the underlying decode or validation error.
func (e *CaptureValidationError) Unwrap() error { return e.Err }

// CaptureEnvelope is the versioned wire payload sent by PageMole.
type CaptureEnvelope struct {
	Type       string   `json:"type" required:"true" const:"statement-html" doc:"Capture payload type"`
	Version    int      `json:"version" required:"true" const:"1" doc:"Capture payload version"`
	CapturedAt string   `json:"capturedAt" required:"true" format:"date-time" doc:"Browser capture time"`
	Title      string   `json:"title" required:"true" doc:"Browser document title"`
	URL        string   `json:"url" required:"true" minLength:"1" format:"uri" doc:"Rendered page URL"`
	HTML       string   `json:"html" required:"true" minLength:"1" doc:"Rendered page HTML"`
	_          struct{} `json:"-" additionalProperties:"true"`

	raw []byte
}

// RawBytes returns a copy of the exact decoded request body when available.
func (c *CaptureEnvelope) RawBytes() []byte {
	if c == nil || c.raw == nil {
		return nil
	}
	return bytes.Clone(c.raw)
}

// CaptureTime parses the browser-provided RFC3339 timestamp.
func (c *CaptureEnvelope) CaptureTime() (time.Time, error) {
	if c == nil {
		return time.Time{}, errors.New("capture is nil")
	}
	return time.Parse(time.RFC3339, c.CapturedAt)
}

// ParsedURL parses the absolute source URL from the capture.
func (c *CaptureEnvelope) ParsedURL() (*url.URL, error) {
	if c == nil {
		return nil, errors.New("capture is nil")
	}
	u, err := url.Parse(c.URL)
	if err != nil {
		return nil, err
	}
	return u, nil
}

// WithRawBytes returns a value carrying the exact original request bytes.
// Huma supplies the typed fields; its RawBody field supplies these bytes.
func (c CaptureEnvelope) WithRawBytes(body []byte) CaptureEnvelope {
	c.raw = bytes.Clone(body)
	return c
}

// ValidateCapture checks semantic envelope constraints after JSON decoding.
// Wire-shape validation remains Huma's responsibility for HTTP requests.
func ValidateCapture(capture *CaptureEnvelope) error {
	if capture == nil {
		return &CaptureValidationError{Kind: ValidationSchema, Err: errors.New("capture is nil")}
	}
	if capture.Type != "statement-html" {
		return &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("unsupported capture type %q", capture.Type)}
	}
	if capture.Version != 1 {
		return &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("unsupported capture version %d", capture.Version)}
	}
	if capture.HTML == "" {
		return &CaptureValidationError{Kind: ValidationSchema, Err: errors.New("capture html must not be empty")}
	}
	if _, err := capture.CaptureTime(); err != nil {
		return &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("invalid capturedAt: %w", err)}
	}
	u, err := capture.ParsedURL()
	if err != nil || u.Scheme == "" || u.Host == "" {
		if err == nil {
			err = errors.New("URL must be absolute")
		}
		return &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("invalid url: %w", err)}
	}
	return nil
}

// DecodeCapture validates one PageMole body and retains its exact bytes.
func DecodeCapture(body []byte) (*CaptureEnvelope, error) {
	if len(body) > MaxCaptureBytes {
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("capture body exceeds %d bytes", MaxCaptureBytes)}
	}

	var capture CaptureEnvelope
	if err := json.Unmarshal(body, &capture); err != nil {
		kind := ValidationSchema
		var syntaxErr *json.SyntaxError
		if errors.As(err, &syntaxErr) {
			kind = ValidationMalformed
		}
		return nil, &CaptureValidationError{Kind: kind, Err: fmt.Errorf("decode capture: %w", err)}
	}
	if err := ValidateCapture(&capture); err != nil {
		return nil, err
	}
	capture.raw = bytes.Clone(body)
	return &capture, nil
}
