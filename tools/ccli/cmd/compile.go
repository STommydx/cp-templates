/*
Copyright © 2024 NAME HERE <EMAIL ADDRESS>
*/
package cmd

import (
	"fmt"
	"github.com/STommydx/cp-templates/tools/ccli/submission"
	"github.com/spf13/cobra"
	"os"
	"path"
	"strings"
)

var compilationFlags string
var submissionPath string

var compileCmd = &cobra.Command{
	Use:   "compile [-f flags] [-s submission] [-o output] source...",
	Short: "Compile source files to binary",
	Long: `Compile source files to binary.

This command compiles source files to binary. This also packs source files into a single file for online judge submissions.`,
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
			fmt.Println(err)
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
			fmt.Println(err)
			os.Exit(1)
		}
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
	compileCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
	compileCmd.Flags().StringVarP(&submissionPath, "submission", "s", "", "Submission file path")
	compileCmd.Flags().StringVarP(&compilationFlags, "flag", "f", "-std=c++20 -Wall -Wextra -g", "Compilation flags")
}
