package cmd

import (
	"fmt"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/submission"
	"github.com/spf13/cobra"
)

var runCmd = &cobra.Command{
	Use:   "run [-f flags] [-s submission] [-o output] source...",
	Short: "Compile and run source files",
	Long: `Compile source files to binary and execute it.

Compile source files to binary and execute it. This also packs source files into a single file for online judge submissions.
The exit code and time elapsed are printed after successful execution.`,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		sourceFileName := path.Base(args[0])
		if submissionPath == "" {
			submissionPath = path.Join("./submissions/", sourceFileName)
		}
		if err := submission.Pack(submission.PackSettings{
			SourceFiles: args,
			OutputPath:  submissionPath,
		}); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		if outputPath == "" {
			outputFileName := strings.ReplaceAll(sourceFileName, ".cpp", ".out")
			outputPath = path.Join("./build/", outputFileName)
		}
		if err := submission.Compile(submission.CompileSettings{
			SourceFiles:      []string{submissionPath},
			OutputPath:       outputPath,
			CompilationFlags: strings.Split(compilationFlags, " "),
		}); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		absoluteOutputPath, err := filepath.Abs(outputPath)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		if err := submission.Run(submission.RunSettings{
			ExecutablePath: absoluteOutputPath,
		}); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	},
}

func init() {
	rootCmd.AddCommand(runCmd)
	runCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
	runCmd.Flags().StringVarP(&submissionPath, "submission", "s", "", "Submission file path")
	runCmd.Flags().StringVarP(&compilationFlags, "flag", "f", "-std=c++20 -Wall -Wextra -g -fsanitize=address -fsanitize=undefined", "Compilation flags")
}
