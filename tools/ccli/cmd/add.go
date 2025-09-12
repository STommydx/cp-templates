package cmd

import (
	"fmt"
	"os"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/output"
	"github.com/STommydx/cp-templates/tools/ccli/repoinit"
	"github.com/spf13/cobra"
)

var addCmd = &cobra.Command{
	Use:   "add [filename]",
	Short: "Add a new source file from template and update CMakeLists.txt",
	Long:  `Create a new source file from the main.cpp template and add an add_executable line in the CMakeLists.txt file.`,
	Args:  cobra.ExactArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		out := output.NewTerminalOutputter(os.Stderr)
		filename := args[0]

		// Ensure the filename ends with .cpp
		if !strings.HasSuffix(filename, ".cpp") {
			filename += ".cpp"
		}

		// Create the new source file from template
		if err := repoinit.CreateSourceFileFromTemplate(filename, out); err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}

		// Update CMakeLists.txt
		if err := repoinit.UpdateCMakeLists(filename, out); err != nil {
			out.Error(err.Error())
			os.Exit(1)
		}

		out.Info(fmt.Sprintf("Successfully added %s", filename))
	},
}

func init() {
	rootCmd.AddCommand(addCmd)
}
