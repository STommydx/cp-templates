package cmd

import (
	"errors"
	"fmt"
	"os"

	"github.com/spf13/cobra"
)

type UsageError struct {
	Err error
}

func (e *UsageError) Error() string { return e.Err.Error() }
func (e *UsageError) Unwrap() error { return e.Err }

func usageError(format string, args ...any) error {
	return &UsageError{Err: fmt.Errorf(format, args...)}
}

var rootCmd = &cobra.Command{
	Use:           "ccli",
	Short:         "Competitive Programming CLI",
	SilenceUsage:  true,
	SilenceErrors: true,
	Long: `ccli is a CLI tool for competitive programming.

It is useful for consolidating templates into single file for online judge submissions. It also contains utilities that aids running program against downloaded testcases and submitting to online judges.`,
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		if isUsageError(err) {
			os.Exit(2)
		}
		os.Exit(1)
	}
}

func isUsageError(err error) bool {
	var usageErr *UsageError
	return errors.As(err, &usageErr)
}

func init() {
}
