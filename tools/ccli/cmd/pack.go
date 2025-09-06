package cmd

import (
	"os"
	"path"

	"github.com/STommydx/cp-templates/tools/ccli/output"
	"github.com/STommydx/cp-templates/tools/ccli/submission"
	"github.com/spf13/cobra"
)

var outputPath string

var packCmd = &cobra.Command{
	Use:   "pack [-o output] source...",
	Short: "Pack source files into a single file for online judge submissions",
	Long:  `Pack source files into a single file for online judge submissions.`,
	Args:  cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		out := output.NewTerminalOutputter(os.Stderr)
		sourceFileName := path.Base(args[0])
		if outputPath == "" {
			outputPath = path.Join("./submissions/", sourceFileName)
		}
		if err := submission.PackWithOutput(submission.PackSettings{
			SourceFiles: args,
			OutputPath:  outputPath,
		}, out); err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}
	},
}

func init() {
	rootCmd.AddCommand(packCmd)
	packCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
}
