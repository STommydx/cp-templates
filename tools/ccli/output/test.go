package output

import (
	"fmt"
	"io"
	"strings"
	"sync"
)

// TestOutputter implements Outputter for testing purposes
type TestOutputter struct {
	mu     sync.Mutex
	output strings.Builder
}

// NewTestOutputter creates a new TestOutputter
func NewTestOutputter() *TestOutputter {
	return &TestOutputter{}
}

// Info prints informational messages
func (t *TestOutputter) Info(message string) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("INFO: " + message + "\n")
}

// Infof prints formatted informational messages
func (t *TestOutputter) Infof(format string, args ...interface{}) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("INFO: " + fmt.Sprintf(format, args...) + "\n")
}

// Warn prints warning messages
func (t *TestOutputter) Warn(message string) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("WARN: " + message + "\n")
}

// Warnf prints formatted warning messages
func (t *TestOutputter) Warnf(format string, args ...interface{}) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("WARN: " + fmt.Sprintf(format, args...) + "\n")
}

// Error prints error messages
func (t *TestOutputter) Error(message string) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("ERROR: " + message + "\n")
}

// Errorf prints formatted error messages
func (t *TestOutputter) Errorf(format string, args ...interface{}) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("ERROR: " + fmt.Sprintf(format, args...) + "\n")
}

// Success prints success messages
func (t *TestOutputter) Success(message string) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("SUCCESS: " + message + "\n")
}

// Successf prints formatted success messages
func (t *TestOutputter) Successf(format string, args ...interface{}) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.WriteString("SUCCESS: " + fmt.Sprintf(format, args...) + "\n")
}

// WithSpinner creates a new test spinner
func (t *TestOutputter) WithSpinner(message string) Spinner {
	return &testSpinner{message: message, outputter: t}
}

// Writer returns the underlying writer for direct output
func (t *TestOutputter) Writer() io.Writer {
	return &testWriter{outputter: t}
}

// Output returns the accumulated output
func (t *TestOutputter) Output() string {
	t.mu.Lock()
	defer t.mu.Unlock()
	return t.output.String()
}

// Clear clears the accumulated output
func (t *TestOutputter) Clear() {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.output.Reset()
}

// testSpinner is a test implementation of Spinner
type testSpinner struct {
	message   string
	outputter *TestOutputter
	isStarted bool
	isStopped bool
	isFailed  bool
}

// Start starts the spinner
func (t *testSpinner) Start() error {
	t.isStarted = true
	t.outputter.mu.Lock()
	defer t.outputter.mu.Unlock()
	t.outputter.output.WriteString("SPINNER START: " + t.message + "\n")
	return nil
}

// Stop stops the spinner
func (t *testSpinner) Stop() error {
	t.isStopped = true
	t.outputter.mu.Lock()
	defer t.outputter.mu.Unlock()
	t.outputter.output.WriteString("SPINNER STOP: " + t.message + "\n")
	return nil
}

// StopFail stops the spinner with a failure indication
func (t *testSpinner) StopFail() error {
	t.isFailed = true
	t.outputter.mu.Lock()
	defer t.outputter.mu.Unlock()
	t.outputter.output.WriteString("SPINNER STOP FAIL: " + t.message + "\n")
	return nil
}

// testWriter is a test implementation of io.Writer
type testWriter struct {
	outputter *TestOutputter
}

// Write writes to the test output
func (t *testWriter) Write(p []byte) (n int, err error) {
	t.outputter.mu.Lock()
	defer t.outputter.mu.Unlock()
	return t.outputter.output.Write(p)
}
