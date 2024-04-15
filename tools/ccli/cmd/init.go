package cmd

import (
	"fmt"
	"os"

	"github.com/STommydx/cp-templates/tools/ccli/repoinit"
	"github.com/spf13/cobra"
)

var mainProgramCount int
var templateRepository string
var includeTestlib bool

var initCmd = &cobra.Command{
	Use:   "init [directory]",
	Short: "Initialize a new repository for programming contests",
	Long:  `Initialize a new repository for programming contests. Creates configurable number of starting main program files and download templates to template folder.`,
	Args:  cobra.MaximumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		directory := "./"
		if len(args) > 0 {
			directory = args[0]
		}
		if err := repoinit.Run(repoinit.Settings{
			Directory:             directory,
			NumberOfMainPrograms:  mainProgramCount,
			TemplateRepositoryUrl: templateRepository,
			IncludeTestlib:        includeTestlib,
		}); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	},
}

func init() {
	rootCmd.AddCommand(initCmd)
	initCmd.Flags().IntVarP(&mainProgramCount, "count", "c", 7, "Number of main program files to create")
	initCmd.Flags().StringVarP(&templateRepository, "repository", "r", "https://github.com/STommydx/cp-templates.git", "URL of template repository")
	initCmd.Flags().BoolVarP(&includeTestlib, "testlib", "t", false, "Include testlib library for problem preparation")
}
