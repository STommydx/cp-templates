package output

import (
	"io"
)

// Outputter defines the interface for output operations
type Outputter interface {
	// Info prints informational messages
	Info(message string)

	// Infof prints formatted informational messages
	Infof(format string, args ...interface{})

	// Warn prints warning messages
	Warn(message string)

	// Warnf prints formatted warning messages
	Warnf(format string, args ...interface{})

	// Error prints error messages
	Error(message string)

	// Errorf prints formatted error messages
	Errorf(format string, args ...interface{})

	// Success prints success messages
	Success(message string)

	// Successf prints formatted success messages
	Successf(format string, args ...interface{})

	// WithSpinner creates a new spinner with the given message
	WithSpinner(message string) Spinner

	// Writer returns the underlying writer for direct output
	Writer() io.Writer
}

// Spinner defines the interface for spinner operations
type Spinner interface {
	// Start starts the spinner
	Start() error

	// Stop stops the spinner
	Stop() error

	// StopFail stops the spinner with a failure indication
	StopFail() error
}
