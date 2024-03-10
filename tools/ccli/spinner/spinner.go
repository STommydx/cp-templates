package spinner

import (
	"fmt"
	"github.com/mattn/go-isatty"
	"os"
	"time"

	"github.com/theckman/yacspin"
)

func Run(message string, f func() error) error {
	if !isatty.IsTerminal(os.Stderr.Fd()) {
		return f()
	}

	spinnerConfig := yacspin.Config{
		Frequency:         100 * time.Millisecond,
		CharSet:           yacspin.CharSets[11],
		Suffix:            " ",
		Message:           message,
		SuffixAutoColon:   true,
		ColorAll:          true,
		Colors:            []string{"fgYellow"},
		StopCharacter:     "✓",
		StopColors:        []string{"fgGreen"},
		StopMessage:       message,
		StopFailCharacter: "✗",
		StopFailColors:    []string{"fgRed"},
		StopFailMessage:   message,
		Writer:            os.Stderr,
	}

	spinner, err := yacspin.New(spinnerConfig)
	if err != nil {
		return fmt.Errorf("failed to create spinner: %w", err)
	}

	if err := spinner.Start(); err != nil {
		return fmt.Errorf("failed to start spinner: %w", err)
	}

	if err := f(); err != nil {
		spinner.StopFail()
		return fmt.Errorf("failed to %s: %w", message, err)
	}

	if err := spinner.Stop(); err != nil {
		return fmt.Errorf("failed to stop spinner: %w", err)
	}
	return nil
}
