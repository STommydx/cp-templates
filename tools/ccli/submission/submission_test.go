package submission

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/STommydx/cp-templates/tools/ccli/output"
)

func TestCompileWithOutput(t *testing.T) {
	// Create a temporary directory for testing
	tempDir, err := os.MkdirTemp("", "ccli_test")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(tempDir)

	// Create a simple C++ file for testing
	sourceFile := filepath.Join(tempDir, "test.cpp")
	err = os.WriteFile(sourceFile, []byte("#include <iostream>\nint main() { std::cout << \"Hello, World!\" << std::endl; return 0; }"), 0644)
	if err != nil {
		t.Fatal(err)
	}

	// Test compiling the file
	outputPath := filepath.Join(tempDir, "test.out")
	settings := CompileSettings{
		SourceFiles:      []string{sourceFile},
		OutputPath:       outputPath,
		CompilationFlags: []string{"-std=c++17"},
	}

	out := output.NewTestOutputter()
	err = CompileWithOutput(settings, out)
	if err != nil {
		t.Errorf("CompileWithOutput failed: %v", err)
	}

	// Check that the output file was created
	if _, err := os.Stat(outputPath); os.IsNotExist(err) {
		t.Error("Output file was not created")
	}

	// Check that the output contains expected messages
	outputStr := out.Output()
	if outputStr == "" {
		t.Error("No output was generated")
	}
}

func TestPackWithOutput(t *testing.T) {
	// Create a temporary directory for testing
	tempDir, err := os.MkdirTemp("", "ccli_test")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(tempDir)

	// Create a simple C++ file for testing
	sourceFile := filepath.Join(tempDir, "test.cpp")
	err = os.WriteFile(sourceFile, []byte("#include <iostream>\nint main() { std::cout << \"Hello, World!\" << std::endl; return 0; }"), 0644)
	if err != nil {
		t.Fatal(err)
	}

	// Test packing the file
	outputPath := filepath.Join(tempDir, "test.pack.cpp")
	settings := PackSettings{
		SourceFiles: []string{sourceFile},
		OutputPath:  outputPath,
	}

	out := output.NewTestOutputter()
	err = PackWithOutput(settings, out)
	if err != nil {
		t.Errorf("PackWithOutput failed: %v", err)
	}

	// Check that the output file was created
	if _, err := os.Stat(outputPath); os.IsNotExist(err) {
		t.Error("Output file was not created")
	}

	// Check that the output contains expected messages
	outputStr := out.Output()
	if outputStr == "" {
		t.Error("No output was generated")
	}
}

func TestRunWithOutput(t *testing.T) {
	// Create a temporary directory for testing
	tempDir, err := os.MkdirTemp("", "ccli_test")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(tempDir)

	// Create a simple C++ file for testing
	sourceFile := filepath.Join(tempDir, "test.cpp")
	err = os.WriteFile(sourceFile, []byte("#include <iostream>\nint main() { std::cout << \"Hello, World!\" << std::endl; return 0; }"), 0644)
	if err != nil {
		t.Fatal(err)
	}

	// Compile the file first
	outputPath := filepath.Join(tempDir, "test.out")
	compileSettings := CompileSettings{
		SourceFiles:      []string{sourceFile},
		OutputPath:       outputPath,
		CompilationFlags: []string{"-std=c++17"},
	}

	compileOut := output.NewTestOutputter()
	err = CompileWithOutput(compileSettings, compileOut)
	if err != nil {
		t.Errorf("CompileWithOutput failed: %v", err)
	}

	// Test running the compiled file
	runSettings := RunSettings{
		ExecutablePath: outputPath,
	}

	out := output.NewTestOutputter()
	err = RunWithOutput(runSettings, out)
	if err != nil {
		t.Errorf("RunWithOutput failed: %v", err)
	}

	// Check that the output contains expected messages
	outputStr := out.Output()
	if outputStr == "" {
		t.Error("No output was generated")
	}
}
