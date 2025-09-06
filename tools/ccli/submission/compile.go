package submission

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"

	"github.com/STommydx/cp-templates/tools/ccli/output"
	"github.com/mattn/go-isatty"
)

type CompileSettings struct {
	SourceFiles      []string
	OutputPath       string
	CompilationFlags []string
}

// CompileWithOutput compiles source files with the given outputter for messaging
func CompileWithOutput(settings CompileSettings, out output.Outputter) error {
	spinner := out.WithSpinner("Create output directory")
	if err := spinner.Start(); err != nil {
		return err
	}

	outputDir := filepath.Dir(settings.OutputPath)
	if _, err := os.Stat(outputDir); os.IsNotExist(err) {
		err := os.Mkdir(outputDir, os.ModePerm)
		if err != nil {
			spinner.StopFail()
			return fmt.Errorf("failed to create directory: %w", err)
		}
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	isUpdateToDate, err := isUpToDate(settings.OutputPath, settings.SourceFiles)
	if err != nil {
		return err
	}

	if isUpdateToDate {
		out.Warn("⚠ Executable is up-to-date, skipping compilation")
		return nil
	}

	spinner = out.WithSpinner("Compile source files")
	if err := spinner.Start(); err != nil {
		return err
	}

	compileArgs := append(settings.SourceFiles, "-o", settings.OutputPath)
	if isatty.IsTerminal(os.Stderr.Fd()) {
		compileArgs = append(compileArgs, "-fdiagnostics-color=always")
	}
	compileArgs = append(compileArgs, settings.CompilationFlags...)

	compileCommand := exec.Command("g++", compileArgs...)
	var stderrBuf bytes.Buffer
	compileCommand.Stderr = &stderrBuf

	if err := compileCommand.Run(); err != nil {
		spinner.StopFail()
		// Copy stderr to output
		io.Copy(out.Writer(), &stderrBuf)
		return fmt.Errorf("failed to start compile command: %w", err)
	}

	if err := spinner.Stop(); err != nil {
		return err
	}

	// Copy stderr to output
	io.Copy(out.Writer(), &stderrBuf)
	return nil
}

// Compile compiles source files using the default terminal outputter (backward compatibility)
func Compile(settings CompileSettings) error {
	return CompileWithOutput(settings, output.DefaultTerminalOutputter())
}
