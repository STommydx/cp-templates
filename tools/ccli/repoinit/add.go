package repoinit

import (
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/output"
)

// CreateSourceFileFromTemplate creates a new source file from the main.cpp template
func CreateSourceFileFromTemplate(filename string, out output.Outputter) error {
	spinner := out.WithSpinner("Create source file from template")
	if err := spinner.Start(); err != nil {
		return err
	}

	// Check if file already exists
	if _, err := os.Stat(filename); err == nil {
		spinner.StopFail()
		return fmt.Errorf("file %s already exists", filename)
	}

	// Open the template file
	templateFile, err := templatesFS.Open("templates/main.cpp")
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to open template file: %w", err)
	}
	defer templateFile.Close()

	// Create the new file
	file, err := os.Create(filename)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer file.Close()

	// Copy template content to new file
	if _, err := io.Copy(file, templateFile); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to copy template file: %w", err)
	}

	return spinner.Stop()
}

// UpdateCMakeLists updates the CMakeLists.txt file with a new add_executable line
func UpdateCMakeLists(filename string, out output.Outputter) error {
	spinner := out.WithSpinner("Update CMakeLists.txt")
	if err := spinner.Start(); err != nil {
		return err
	}

	// Check if filename ends with .cpp
	if !strings.HasSuffix(filename, ".cpp") {
		spinner.StopFail()
		return fmt.Errorf("filename must end with .cpp")
	}

	// Open CMakeLists.txt in append mode
	cmakePath := "CMakeLists.txt"
	file, err := os.OpenFile(cmakePath, os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to open CMakeLists.txt: %w", err)
	}
	defer file.Close()

	// Create the add_executable line
	outputName := strings.ReplaceAll(filename, ".cpp", ".out")
	line := fmt.Sprintf("add_executable(%s %s)\n", outputName, filename)

	// Append the line to the file
	if _, err := file.WriteString(line); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to write to CMakeLists.txt: %w", err)
	}

	return spinner.Stop()
}
