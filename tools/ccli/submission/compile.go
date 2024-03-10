package submission

import (
	"bytes"
	"fmt"
	"github.com/STommydx/cp-templates/tools/ccli/spinner"
	"github.com/mattn/go-isatty"
	"io"
	"os"
	"os/exec"
	"path/filepath"
)

type CompileSettings struct {
	SourceFiles      []string
	OutputPath       string
	CompilationFlags []string
}

func Compile(settings CompileSettings) error {
	if err := spinner.Run("Create output directory", func() error {
		outputDir := filepath.Dir(settings.OutputPath)
		if _, err := os.Stat(outputDir); os.IsNotExist(err) {
			err := os.Mkdir(outputDir, os.ModePerm)
			if err != nil {
				return fmt.Errorf("failed to create directory: %w", err)
			}
		}
		return nil
	}); err != nil {
		return err
	}
	var stderrBuf bytes.Buffer
	defer io.Copy(os.Stderr, &stderrBuf)
	return spinner.Run("Compile source files", func() error {
		compileArgs := append(settings.SourceFiles, "-o", settings.OutputPath)
		if isatty.IsTerminal(os.Stderr.Fd()) {
			compileArgs = append(compileArgs, "-fdiagnostics-color=always")
		}
		compileArgs = append(compileArgs, settings.CompilationFlags...)
		compileCommand := exec.Command("g++", compileArgs...)
		compileCommand.Stderr = &stderrBuf
		if err := compileCommand.Run(); err != nil {
			return fmt.Errorf("failed to start compile command: %w", err)
		}
		return nil
	})
}
