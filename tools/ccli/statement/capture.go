package statement

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"net/url"
	"time"
)

const MaxCaptureBytes = 8 << 20

type ValidationKind string

const (
	ValidationMalformed ValidationKind = "malformed"
	ValidationSchema    ValidationKind = "schema"
)

type CaptureValidationError struct {
	Kind ValidationKind
	Err  error
}

func (e *CaptureValidationError) Error() string { return e.Err.Error() }
func (e *CaptureValidationError) Unwrap() error { return e.Err }

type CaptureEnvelope struct {
	Type       string   `json:"type" required:"true" enum:"statement-html" doc:"Capture payload type"`
	Version    int      `json:"version" required:"true" minimum:"1" maximum:"1" doc:"Capture payload version"`
	CapturedAt string   `json:"capturedAt" required:"true" format:"date-time" doc:"Browser capture time"`
	Title      string   `json:"title" required:"true" doc:"Browser document title"`
	URL        string   `json:"url" required:"true" minLength:"1" format:"uri" doc:"Rendered page URL"`
	HTML       string   `json:"html" required:"true" minLength:"1" doc:"Rendered page HTML"`
	_          struct{} `json:"-" additionalProperties:"true"`

	raw []byte
}

func (c *CaptureEnvelope) RawBytes() []byte {
	if c == nil || c.raw == nil {
		return nil
	}
	return bytes.Clone(c.raw)
}

func (c *CaptureEnvelope) CaptureTime() (time.Time, error) {
	if c == nil {
		return time.Time{}, errors.New("capture is nil")
	}
	return time.Parse(time.RFC3339, c.CapturedAt)
}

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

	var fields map[string]json.RawMessage
	if err := json.Unmarshal(body, &fields); err != nil {
		return nil, &CaptureValidationError{Kind: ValidationMalformed, Err: fmt.Errorf("decode capture object: %w", err)}
	}
	for _, field := range []string{"type", "version", "capturedAt", "title", "url", "html"} {
		if _, ok := fields[field]; !ok {
			return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("capture field %q is required", field)}
		}
	}
	if capture.Type != "statement-html" {
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("unsupported capture type %q", capture.Type)}
	}
	if capture.Version != 1 {
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("unsupported capture version %d", capture.Version)}
	}
	if capture.HTML == "" {
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: errors.New("capture html must not be empty")}
	}
	if _, err := capture.CaptureTime(); err != nil {
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("invalid capturedAt: %w", err)}
	}
	u, err := capture.ParsedURL()
	if err != nil || u.Scheme == "" || u.Host == "" {
		if err == nil {
			err = errors.New("URL must be absolute")
		}
		return nil, &CaptureValidationError{Kind: ValidationSchema, Err: fmt.Errorf("invalid url: %w", err)}
	}
	capture.raw = bytes.Clone(body)
	return &capture, nil
}
