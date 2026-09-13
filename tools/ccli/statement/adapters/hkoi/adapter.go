package hkoi

import (
	"fmt"
	"math"
	"regexp"
	"strconv"
	"strings"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	nethtml "golang.org/x/net/html"
)

type Adapter struct{}

func New() statement.Adapter { return Adapter{} }

func (Adapter) ID() string { return "hkoi" }

func (Adapter) URLPatterns() []statement.URLPattern {
	return []statement.URLPattern{{Host: "judge.hkoi.org", PathPrefix: "/task/"}}
}

func (Adapter) Match(_ *statement.CaptureEnvelope, document *nethtml.Node) bool {
	return statement.FindFirstClass(document, "task") != nil
}

var (
	timeLimitPattern   = regexp.MustCompile(`(?i)time[^0-9]*([0-9]+(?:\.[0-9]+)?)\s*(milliseconds?|ms|seconds?|secs?|minutes?|mins?|s|m)`)
	memoryLimitPattern = regexp.MustCompile(`(?i)memory[^0-9]*([0-9]+(?:\.[0-9]+)?)\s*(mib|mb|gib|gb)`)
	interactivePattern = regexp.MustCompile(`(?i)interactive\s*:\s*(yes|no|true|false)`)
)

func (Adapter) Extract(capture *statement.CaptureEnvelope, document *nethtml.Node) (statement.Extraction, error) {
	root := statement.FindFirstClass(document, "task")
	if root == nil {
		return statement.Extraction{}, fmt.Errorf(".task root not found")
	}
	metadata := statement.ProblemMetadata{
		Identity: statement.ProblemIdentity{Title: capture.Title},
	}
	if displayID := statement.FindFirstClass(root, "task-displayid"); displayID != nil {
		metadata.Identity.Code = strings.TrimSpace(statement.TextContent(displayID))
	}
	if info := statement.FindFirstClass(root, "task-info"); info != nil {
		infoText := statement.TextContent(info)
		parseLimits(infoText, &metadata.Limits)
		if match := interactivePattern.FindStringSubmatch(infoText); len(match) == 2 {
			interactive := strings.EqualFold(match[1], "yes") || strings.EqualFold(match[1], "true")
			metadata.Execution.Interactive = &interactive
		}
	}

	extracted := statement.ExtractSampleTable(root)
	var warnings []string
	if extracted.Found && !extracted.Complete {
		warnings = append(warnings, "sample table found but expected data rows or explanations were incomplete; table preserved")
	}
	if !extracted.Found {
		warnings = append(warnings, "sample table with Input and Output headers was not found")
	}
	if extracted.Complete {
		metadata.Samples = extracted.Samples
	}
	return statement.Extraction{
		Root:     root,
		Samples:  metadata.Samples,
		Metadata: metadata,
		Excluded: extracted.Excluded,
		Warnings: warnings,
	}, nil
}

func parseLimits(text string, limits *statement.ProblemLimits) {
	if match := timeLimitPattern.FindStringSubmatch(text); len(match) == 3 {
		if value, err := strconv.ParseFloat(match[1], 64); err == nil {
			unit := strings.ToLower(match[2])
			multiplier := 1000.0
			if strings.HasPrefix(unit, "ms") || strings.HasPrefix(unit, "millisecond") {
				multiplier = 1
			} else if strings.HasPrefix(unit, "m") {
				multiplier = 60_000
			}
			canonical := int64(math.Round(value * multiplier))
			limits.TimeMS = &canonical
			limits.TimeRaw = strings.TrimSpace(match[0])
		}
	}
	if match := memoryLimitPattern.FindStringSubmatch(text); len(match) == 3 {
		if value, err := strconv.ParseFloat(match[1], 64); err == nil {
			multiplier := 1.0
			if strings.HasPrefix(strings.ToLower(match[2]), "g") {
				multiplier = 1024
			}
			canonical := int64(math.Round(value * multiplier))
			limits.MemoryMiB = &canonical
			limits.MemoryRaw = strings.TrimSpace(match[0])
		}
	}
}
