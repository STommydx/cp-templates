/*
Copyright © 2024 NAME HERE <EMAIL ADDRESS>
*/
package cmd

import (
	"fmt"
	"path"

	"github.com/STommydx/cp-templates/tools/ccli/submission"
	"github.com/spf13/cobra"
)

var outputPath string

var packCmd = &cobra.Command{
	Use:   "pack source...",
	Short: "Pack source files into a single file for online judge submissions",
	Long:  `Pack source files into a single file for online judge submissions.`,
	Args:  cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		sourceFileName := path.Base(args[0])
		if outputPath == "" {
			outputPath = path.Join("./submissions/", sourceFileName)
		}
		if err := submission.Pack(submission.PackSettings{
			SourceFiles: args,
			OutputPath:  outputPath,
		}); err != nil {
			fmt.Println(err)
		}
	},
}

func init() {
	rootCmd.AddCommand(packCmd)
	packCmd.Flags().StringVarP(&outputPath, "output", "o", "", "Output file path")
}
