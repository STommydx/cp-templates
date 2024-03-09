package cmd

import (
	"os"

	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "ccli",
	Short: "Competitive Programming CLI",
	Long: `ccli is a CLI tool for competitive programming.

It is useful for consolidating templates into single file for online judge submissions. It also contains utilities that aids running program against downloaded testcases and submitting to online judges.`,
}

func Execute() {
	err := rootCmd.Execute()
	if err != nil {
		os.Exit(1)
	}
}

func init() {
}
