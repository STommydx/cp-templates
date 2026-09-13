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

// Adapter extracts the page-level metadata and statement content used by HKOI task pages.
type Adapter struct{}

// New returns the registered HKOI adapter.
func New() statement.Adapter { return Adapter{} }

// ID returns the stable storage and diagnostics identifier.
func (Adapter) ID() string { return "hkoi" }

// URLPatterns limits automatic selection to public HKOI task pages.
func (Adapter) URLPatterns() []statement.URLPattern {
	return []statement.URLPattern{{Host: "judge.hkoi.org", PathPrefix: "/task/"}}
}

// Match verifies that the rendered document contains the HKOI statement root.
func (Adapter) Match(_ *statement.CaptureEnvelope, document *nethtml.Node) bool {
	return statementRoot(document) != nil
}

// statementRoot returns the statement container, which is always a div. The
// site toggles classes on the body element, so a bare class lookup can select
// page chrome instead of the statement.
func statementRoot(document *nethtml.Node) *nethtml.Node {
	var root *nethtml.Node
	statement.Walk(document, func(node *nethtml.Node) bool {
		if node.Type == nethtml.ElementNode && node.Data == "div" && statement.HasClass(node, "task") {
			root = node
			return false
		}
		return true
	})
	return root
}

var (
	timeLimitPattern   = regexp.MustCompile(`(?i)\btime[^0-9]*([0-9]+(?:\.[0-9]+)?)\s*([a-z]+)`)
	memoryLimitPattern = regexp.MustCompile(`(?i)\bmemory[^0-9]*([0-9]+(?:\.[0-9]+)?)\s*([a-z]+)`)
)

// Canonical multipliers for the units HKOI labels render. An unrecognized unit
// leaves the limit absent instead of guessing a conversion.
var (
	timeUnits   = map[string]float64{"ms": 1, "millisecond": 1, "milliseconds": 1, "s": 1000, "sec": 1000, "secs": 1000, "second": 1000, "seconds": 1000, "min": 60000, "mins": 60000, "minute": 60000, "minutes": 60000}
	memoryUnits = map[string]float64{"mb": 1, "mib": 1, "gb": 1024, "gib": 1024}
)

// Extract reads metadata outside `.task` and statement content inside `.task`.
// The page-level split is required by the rendered HKOI DOM.
func (Adapter) Extract(capture *statement.CaptureEnvelope, document *nethtml.Node) (statement.Extraction, error) {
	root := statementRoot(document)
	if root == nil {
		return statement.Extraction{}, fmt.Errorf(".task root not found")
	}
	metadata := statement.ProblemMetadata{
		Identity: statement.ProblemIdentity{Title: capture.Title},
	}
	if displayID := statement.FindFirstClass(document, "task-displayid"); displayID != nil {
		metadata.Identity.Code = strings.TrimSpace(statement.TextContent(displayID))
	}
	if info := statement.FindFirstClass(document, "task-info"); info != nil {
		parseLimits(statement.TextContent(info), &metadata.Limits)
	}

	extracted := statement.ExtractSampleTable(root)
	var warnings []string
	if extracted.Found && !extracted.Complete {
		warnings = append(warnings, "sample table found but expected data rows or explanations were incomplete; table preserved")
	}
	if !extracted.Found {
		warnings = append(warnings, "sample table with Input and Output headers was not found")
	}
	return statement.Extraction{
		Root:          root,
		Samples:       extracted.Samples,
		SamplesAnchor: extracted.Anchor,
		Metadata:      metadata,
		Excluded:      extracted.Excluded,
		Warnings:      warnings,
	}, nil
}

// parseLimits records canonical values for the limits a task page displays and
// keeps the displayed text beside them.
func parseLimits(text string, limits *statement.ProblemLimits) {
	if match := timeLimitPattern.FindStringSubmatch(text); len(match) == 3 {
		if value, err := strconv.ParseFloat(match[1], 64); err == nil {
			if multiplier, ok := timeUnits[strings.ToLower(match[2])]; ok {
				canonical := int64(math.Round(value * multiplier))
				limits.TimeMS = &canonical
				limits.TimeRaw = strings.TrimSpace(match[0])
			}
		}
	}
	if match := memoryLimitPattern.FindStringSubmatch(text); len(match) == 3 {
		if value, err := strconv.ParseFloat(match[1], 64); err == nil {
			if multiplier, ok := memoryUnits[strings.ToLower(match[2])]; ok {
				canonical := int64(math.Round(value * multiplier))
				limits.MemoryMiB = &canonical
				limits.MemoryRaw = strings.TrimSpace(match[0])
			}
		}
	}
}
