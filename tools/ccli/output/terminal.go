package output

import (
	"fmt"
	"io"
	"os"
	"time"

	"github.com/fatih/color"
	"github.com/mattn/go-isatty"
	"github.com/theckman/yacspin"
)

// TerminalOutputter implements Outputter for terminal output
type TerminalOutputter struct {
	writer io.Writer
}

// NewTerminalOutputter creates a new TerminalOutputter
func NewTerminalOutputter(writer io.Writer) *TerminalOutputter {
	return &TerminalOutputter{
		writer: writer,
	}
}

// Info prints informational messages
func (t *TerminalOutputter) Info(message string) {
	fmt.Fprintln(t.writer, message)
}

// Infof prints formatted informational messages
func (t *TerminalOutputter) Infof(format string, args ...interface{}) {
	fmt.Fprintf(t.writer, format+"\n", args...)
}

// Warn prints warning messages
func (t *TerminalOutputter) Warn(message string) {
	color.New(color.FgYellow).Fprintln(t.writer, message)
}

// Warnf prints formatted warning messages
func (t *TerminalOutputter) Warnf(format string, args ...interface{}) {
	color.New(color.FgYellow).Fprintf(t.writer, format+"\n", args...)
}

// Error prints error messages
func (t *TerminalOutputter) Error(message string) {
	color.New(color.FgRed).Fprintln(t.writer, message)
}

// Errorf prints formatted error messages
func (t *TerminalOutputter) Errorf(format string, args ...interface{}) {
	color.New(color.FgRed).Fprintf(t.writer, format+"\n", args...)
}

// Success prints success messages
func (t *TerminalOutputter) Success(message string) {
	color.New(color.FgGreen).Fprintln(t.writer, message)
}

// Successf prints formatted success messages
func (t *TerminalOutputter) Successf(format string, args ...interface{}) {
	color.New(color.FgGreen).Fprintf(t.writer, format+"\n", args...)
}

// WithSpinner creates a new spinner with the given message
func (t *TerminalOutputter) WithSpinner(message string) Spinner {
	if !isatty.IsTerminal(os.Stderr.Fd()) {
		return &noopSpinner{}
	}

	spinnerConfig := yacspin.Config{
		Frequency:         100 * time.Millisecond,
		CharSet:           yacspin.CharSets[11],
		Suffix:            " ",
		Message:           message,
		SuffixAutoColon:   true,
		ColorAll:          true,
		Colors:            []string{"fgYellow"},
		StopCharacter:     "✓",
		StopColors:        []string{"fgGreen"},
		StopMessage:       message,
		StopFailCharacter: "✗",
		StopFailColors:    []string{"fgRed"},
		StopFailMessage:   message,
		Writer:            os.Stderr,
	}

	spinner, err := yacspin.New(spinnerConfig)
	if err != nil {
		// Fallback to noop spinner if we can't create a real one
		return &noopSpinner{}
	}

	return &terminalSpinner{spinner: spinner}
}

// Writer returns the underlying writer for direct output
func (t *TerminalOutputter) Writer() io.Writer {
	return t.writer
}

// terminalSpinner wraps yacspin.Spinner
type terminalSpinner struct {
	spinner *yacspin.Spinner
}

// Start starts the spinner
func (t *terminalSpinner) Start() error {
	return t.spinner.Start()
}

// Stop stops the spinner
func (t *terminalSpinner) Stop() error {
	return t.spinner.Stop()
}

// StopFail stops the spinner with a failure indication
func (t *terminalSpinner) StopFail() error {
	return t.spinner.StopFail()
}

// noopSpinner is a no-op implementation of Spinner for non-terminal environments
type noopSpinner struct{}

// Start is a no-op
func (n *noopSpinner) Start() error {
	return nil
}

// Stop is a no-op
func (n *noopSpinner) Stop() error {
	return nil
}

// StopFail is a no-op
func (n *noopSpinner) StopFail() error {
	return nil
}

// DefaultTerminalOutputter returns a TerminalOutputter that writes to stderr
func DefaultTerminalOutputter() *TerminalOutputter {
	return NewTerminalOutputter(os.Stderr)
}
