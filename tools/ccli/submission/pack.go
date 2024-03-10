package submission

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"

	"github.com/STommydx/cp-templates/tools/ccli/spinner"
)

type PackSettings struct {
	SourceFiles []string
	OutputPath  string
}

func Pack(settings PackSettings) error {
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
	if spinner.Run("Pack source files", func() error {
		outputFile, err := os.Create(settings.OutputPath)
		if err != nil {
			return fmt.Errorf("failed to create output file: %w", err)
		}
		defer outputFile.Close()
		writer := bufio.NewWriter(outputFile)
		defer writer.Flush()
		for _, sourceFilePath := range settings.SourceFiles {
			sourceFile, err := os.Open(sourceFilePath)
			if err != nil {
				return fmt.Errorf("failed to open source file: %w", err)
			}
			defer sourceFile.Close()
			sourceFileDir := filepath.Dir(sourceFilePath)
			includeRegex := regexp.MustCompile(`^#include\s+"([^"]+)"$`)
			scanner := bufio.NewScanner(sourceFile)
			for scanner.Scan() {
				if matches := includeRegex.FindStringSubmatch(scanner.Text()); matches != nil {
					matchedFilePath := matches[1]
					includedFile, err := os.Open(filepath.Join(sourceFileDir, matchedFilePath))
					if err != nil {
						return fmt.Errorf("failed to open included file %s: %w", matchedFilePath, err)
					}
					if _, err := io.Copy(writer, includedFile); err != nil {
						return fmt.Errorf("failed to write included file %s: %w", matchedFilePath, err)
					}
				} else {
					if _, err := writer.WriteString(scanner.Text()); err != nil {
						return fmt.Errorf("failed to write to output file: %w", err)
					}
					if _, err := writer.WriteRune('\n'); err != nil {
						return fmt.Errorf("failed to write to output file: %w", err)
					}
				}
			}
		}
		return nil
	}) != nil {
		return nil
	}
	return nil
}
