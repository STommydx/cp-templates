package cmd

import (
	"bytes"
	"strings"
	"testing"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
)

func TestProblemRecordFormats(t *testing.T) {
	records := []statement.ProblemRecord{{
		Key: "hkoi/D1", Title: "Title", TimeMS: int64Pointer(1000), MemoryMiB: int64Pointer(256),
		CapturedAt: time.Date(2026, 9, 14, 0, 0, 0, 0, time.UTC), CaptureCount: 2,
	}}
	for _, format := range []string{"table", "json", "yaml"} {
		t.Run(format, func(t *testing.T) {
			var output bytes.Buffer
			if err := writeProblemRecords(&output, records, format); err != nil {
				t.Fatal(err)
			}
			for _, wanted := range []string{"hkoi/D1", "Title", "1000", "256"} {
				if !strings.Contains(output.String(), wanted) {
					t.Errorf("%s output missing %q: %s", format, wanted, output.String())
				}
			}
		})
	}
}

func TestUsageErrorClassification(t *testing.T) {
	if !isUsageError(usageError("bad format")) {
		t.Fatal("usage error was not classified")
	}
	if isUsageError(assertionError("runtime failure")) {
		t.Fatal("runtime error was classified as usage")
	}
}

type assertionError string

func (e assertionError) Error() string { return string(e) }

func int64Pointer(value int64) *int64 { return &value }
