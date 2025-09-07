package submission

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"regexp"

	"github.com/STommydx/cp-templates/tools/ccli/output"
)

type PackSettings struct {
	SourceFiles []string
	OutputPath  string
}

// PackWithOutput packs source files with the given outputter for messaging
func PackWithOutput(settings PackSettings, out output.Outputter) error {
	spinner := out.WithSpinner("Create submission directory")
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
		out.Warn("⚠ Submission file is up-to-date, skipping packing")
		return nil
	}

	spinner = out.WithSpinner("Pack source files")
	if err := spinner.Start(); err != nil {
		return err
	}

	outputFile, err := os.Create(settings.OutputPath)
	if err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to create output file: %w", err)
	}
	defer outputFile.Close()

	writer := bufio.NewWriter(outputFile)
	defer writer.Flush()

	writeCount := make(map[string]int)
	for _, sourceFilePath := range settings.SourceFiles {
		var parseAndWriteSourceFile func(sourceFilePath string) error
		parseAndWriteSourceFile = func(sourceFilePath string) error {
			if _, ok := writeCount[sourceFilePath]; ok {
				return nil
			}
			writeCount[sourceFilePath] = 1

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
					if matchedFilePath != "testlib.h" {
						includedFilePath := filepath.Join(sourceFileDir, matchedFilePath)
						if err := parseAndWriteSourceFile(includedFilePath); err != nil {
							return fmt.Errorf("failed to write included file %s: %w", matchedFilePath, err)
						}
					} else {
						if _, err := writer.WriteString(scanner.Text()); err != nil {
							return fmt.Errorf("failed to write to output file: %w", err)
						}
						if _, err := writer.WriteRune('\n'); err != nil {
							return fmt.Errorf("failed to write to output file: %w", err)
						}

						outputDir := filepath.Dir(settings.OutputPath)
						testlibOutputPath := filepath.Join(outputDir, "testlib.h")
						if _, err := os.Stat(testlibOutputPath); os.IsNotExist(err) {
							testlibRelPath, err := filepath.Rel(outputDir, filepath.Join(sourceFileDir, "testlib.h"))
							if err != nil {
								return fmt.Errorf("failed to get relative path for testlib: %w", err)
							}
							if err := os.Symlink(testlibRelPath, testlibOutputPath); err != nil {
								return fmt.Errorf("failed to create symlink for testlib: %w", err)
							}
						}
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
			return nil
		}

		if err := parseAndWriteSourceFile(sourceFilePath); err != nil {
			spinner.StopFail()
			return err
		}
	}

	return spinner.Stop()
}

// Pack packs source files using the default terminal outputter (backward compatibility)
func Pack(settings PackSettings) error {
	return PackWithOutput(settings, output.DefaultTerminalOutputter())
}

func isUpToDate(outputPath string, sourceFiles []string) (bool, error) {
	outputStat, err := os.Stat(outputPath)
	if err != nil {
		if os.IsNotExist(err) {
			return false, nil
		}
		return false, fmt.Errorf("failed to stat output file: %w", err)
	}
	for _, sourceFilePath := range sourceFiles {
		sourceFileStat, err := os.Stat(sourceFilePath)
		if err != nil {
			return false, fmt.Errorf("failed to stat source file: %w", err)
		}
		if sourceFileStat.ModTime().After(outputStat.ModTime()) {
			return false, nil
		}
	}
	return true, nil
}
