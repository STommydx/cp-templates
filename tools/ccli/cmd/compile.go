package cmd

import (
	"fmt"
	"os"
	"path"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/submission"
	"github.com/spf13/cobra"
)

var compilationFlags string
var submissionPath string
var packSubmission bool

var compileCmd = &cobra.Command{
	Use:   "compile [-f flags] [-s submission] [-o output] source...",
	Short: "Compile source files to binary",
	Long: `Compile source files to binary.

This command compiles source files to binary. This also packs source files into a single file for online judge submissions.`,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		sourceFileName := path.Base(args[0])
		if packSubmission {
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
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
	compileCmd.Flags().BoolVarP(&packSubmission, "pack", "p", true, "Pack source files into a single file for online judge submissions")
	compileCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
	compileCmd.Flags().StringVarP(&submissionPath, "submission", "s", "", "Submission file path")
	compileCmd.Flags().StringVarP(&compilationFlags, "flag", "f", "-std=c++20 -Wall -Wextra -g -fsanitize=address -fsanitize=undefined", "Compilation flags")
}
