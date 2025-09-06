package output

import (
	"strings"
	"testing"
)

func TestTerminalOutputter(t *testing.T) {
	out := NewTerminalOutputter(&strings.Builder{})

	// Test basic output methods
	out.Info("Info message")
	out.Warn("Warning message")
	out.Error("Error message")
	out.Success("Success message")

	// These methods don't write to the builder directly, so we can't easily test them
	// but we can at least verify they don't panic
	out.Infof("Info %s", "formatted")
	out.Warnf("Warning %s", "formatted")
	out.Errorf("Error %s", "formatted")
	out.Successf("Success %s", "formatted")

	// Test spinner creation
	spinner := out.WithSpinner("Test message")
	if spinner == nil {
		t.Error("WithSpinner returned nil")
	}

	// Test that spinner methods don't panic
	spinner.Start()
	spinner.Stop()
	spinner.StopFail()
}

func TestTestOutputter(t *testing.T) {
	out := NewTestOutputter()

	// Test basic output methods
	out.Info("Info message")
	out.Warn("Warning message")
	out.Error("Error message")
	out.Success("Success message")

	// Check output
	output := out.Output()
	if !strings.Contains(output, "INFO: Info message") {
		t.Error("Info message not found in output")
	}
	if !strings.Contains(output, "WARN: Warning message") {
		t.Error("Warning message not found in output")
	}
	if !strings.Contains(output, "ERROR: Error message") {
		t.Error("Error message not found in output")
	}
	if !strings.Contains(output, "SUCCESS: Success message") {
		t.Error("Success message not found in output")
	}

	// Clear output
	out.Clear()
	if out.Output() != "" {
		t.Error("Output was not cleared")
	}

	// Test formatted output methods
	out.Infof("Info %s", "formatted")
	out.Warnf("Warning %s", "formatted")
	out.Errorf("Error %s", "formatted")
	out.Successf("Success %s", "formatted")

	// Check formatted output
	output = out.Output()
	if !strings.Contains(output, "INFO: Info formatted") {
		t.Error("Formatted info message not found in output")
	}
	if !strings.Contains(output, "WARN: Warning formatted") {
		t.Error("Formatted warning message not found in output")
	}
	if !strings.Contains(output, "ERROR: Error formatted") {
		t.Error("Formatted error message not found in output")
	}
	if !strings.Contains(output, "SUCCESS: Success formatted") {
		t.Error("Formatted success message not found in output")
	}

	// Test spinner
	spinner := out.WithSpinner("Test message")
	if spinner == nil {
		t.Error("WithSpinner returned nil")
	}

	// Test spinner methods
	spinner.Start()
	spinner.Stop()
	spinner.StopFail()

	// Check spinner output
	output = out.Output()
	if !strings.Contains(output, "SPINNER START: Test message") {
		t.Error("Spinner start message not found in output")
	}
	if !strings.Contains(output, "SPINNER STOP: Test message") {
		t.Error("Spinner stop message not found in output")
	}
	if !strings.Contains(output, "SPINNER STOP FAIL: Test message") {
		t.Error("Spinner stop fail message not found in output")
	}
}
