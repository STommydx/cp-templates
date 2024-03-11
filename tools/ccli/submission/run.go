package submission

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"time"

	"github.com/fatih/color"
	"github.com/pkg/errors"
)

type RunSettings struct {
	ExecutablePath string
	Args           []string
}

func Run(settings RunSettings) error {
	cmd := exec.Command(settings.ExecutablePath, settings.Args...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	startTime := time.Now()
	var execErr *exec.ExitError
	err := cmd.Run()
	if err != nil && !errors.As(err, &execErr) {
		return err
	}
	elapsedTime := time.Since(startTime)
	printColor := color.FgGreen
	if cmd.ProcessState.ExitCode() != 0 {
		printColor = color.FgRed
	}
	color.New(printColor, color.Bold).Fprintf(os.Stderr, "Program exited with code %d\n", cmd.ProcessState.ExitCode())
	fmt.Fprintf(os.Stderr, "Elapsed time: %s\n", elapsedTime)
	extraTokens := 0
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(bufio.ScanWords)
	for scanner.Scan() {
		extraTokens++
	}
	if extraTokens > 0 {
		color.New(color.FgYellow).Fprintf(os.Stderr, "⚠ %d extra tokens found\n", extraTokens)
	}
	return nil
}
