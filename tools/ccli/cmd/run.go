package cmd

import (
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/output"
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
		out := output.NewTerminalOutputter(os.Stderr)
		sourceFileName := path.Base(args[0])
		if packSubmission {
			if submissionPath == "" {
				submissionPath = path.Join("./submissions/", sourceFileName)
			}
			if err := submission.PackWithOutput(submission.PackSettings{
				SourceFiles: args,
				OutputPath:  submissionPath,
			}, out); err != nil {
				out.Error(err.Error())
				os.Exit(1)
			}
		}
		if outputPath == "" {
			outputFileName := strings.ReplaceAll(sourceFileName, ".cpp", ".out")
			outputPath = path.Join("./build/", outputFileName)
		}
		if err := submission.CompileWithOutput(submission.CompileSettings{
			SourceFiles:      args,
			OutputPath:       outputPath,
			CompilationFlags: strings.Split(compilationFlags, " "),
		}, out); err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}
		absoluteOutputPath, err := filepath.Abs(outputPath)
		if err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}
		if err := submission.RunWithOutput(submission.RunSettings{
			ExecutablePath: absoluteOutputPath,
		}, out); err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}
	},
}

func init() {
	rootCmd.AddCommand(runCmd)
	runCmd.Flags().BoolVarP(&packSubmission, "pack", "p", true, "Pack source files into a single file for online judge submissions")
	runCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
	runCmd.Flags().StringVarP(&submissionPath, "submission", "s", "", "Submission file path")
	runCmd.Flags().StringVarP(&compilationFlags, "flag", "f", "-std=c++20 -Wall -Wextra -g -fsanitize=address -fsanitize=undefined", "Compilation flags")
}
