package submission

import (
	"bufio"
	"os"
	"os/exec"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/output"
	"github.com/pkg/errors"
)

type RunSettings struct {
	ExecutablePath string
	Args           []string
}

// RunWithOutput runs an executable with the given outputter for messaging
func RunWithOutput(settings RunSettings, out output.Outputter) error {
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

	if cmd.ProcessState.ExitCode() != 0 {
		out.Errorf("Program exited with code %d", cmd.ProcessState.ExitCode())
	} else {
		out.Successf("Program exited with code %d", cmd.ProcessState.ExitCode())
	}

	out.Infof("Elapsed time: %s", elapsedTime)

	extraTokens := 0
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(bufio.ScanWords)
	for scanner.Scan() {
		extraTokens++
	}

	if extraTokens > 0 {
		out.Warnf("⚠ %d extra tokens found", extraTokens)
	}

	return nil
}

// Run runs an executable using the default terminal outputter (backward compatibility)
func Run(settings RunSettings) error {
	return RunWithOutput(settings, output.DefaultTerminalOutputter())
}
