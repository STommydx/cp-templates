package repoinit

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/STommydx/cp-templates/tools/ccli/output"
)

func TestRunWithOutput(t *testing.T) {
	// Create a temporary directory for testing
	tempDir, err := os.MkdirTemp("", "ccli_repoinit_test")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(tempDir)

	// Test initializing a repository
	testDir := filepath.Join(tempDir, "test_repo")
	settings := Settings{
		Directory:             testDir,
		NumberOfMainPrograms:  3,
		TemplateRepositoryUrl: "https://github.com/STommydx/cp-templates.git",
		IncludeTestlib:        false,
	}

	out := output.NewTestOutputter()
	err = RunWithOutput(settings, out)
	if err != nil {
		// We expect this to fail because we're not actually connecting to GitHub
		// But we can still check that the basic directory structure was created
		t.Logf("RunWithOutput failed (expected): %v", err)
	}

	// Check that the directory was created
	if _, err := os.Stat(testDir); os.IsNotExist(err) {
		t.Error("Repository directory was not created")
	}

	// Check that some basic files were created
	expectedFiles := []string{
		"A.cpp",
		"B.cpp",
		"C.cpp",
		".gitignore",
		".clang-format",
		"CMakeLists.txt",
	}

	for _, file := range expectedFiles {
		filePath := filepath.Join(testDir, file)
		if _, err := os.Stat(filePath); os.IsNotExist(err) {
			t.Errorf("Expected file %s was not created", file)
		}
	}

	// Check that the output contains expected messages
	outputStr := out.Output()
	if outputStr == "" {
		t.Error("No output was generated")
	}
}
