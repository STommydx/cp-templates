package statement

import "golang.org/x/net/html"

// FallbackAdapterID is the fixed inventory namespace for conservative fallback parses.

const FallbackAdapterID = "unknown"

// URLPattern identifies a source by normalized host and path prefix.
type URLPattern struct {
	Host       string
	PathPrefix string
}

// Adapter maps one source's rendered DOM into the shared statement model.
// URL selection happens before Match; Extract runs only after Match succeeds.
type Adapter interface {
	ID() string
	URLPatterns() []URLPattern
	Match(capture *CaptureEnvelope, document *html.Node) bool
	Extract(capture *CaptureEnvelope, document *html.Node) (Extraction, error)
}

// Extraction is the adapter-to-renderer and adapter-to-metadata handoff.
// Root points into the request-local DOM and Excluded contains exact subtrees
// already represented by structured values such as Samples. SamplesAnchor is
// the subtree that Samples replaces in place; it must also appear in Excluded,
// and a nil anchor appends the rendered samples after Root.
type Extraction struct {
	Root          *html.Node
	Samples       []Sample
	SamplesAnchor *html.Node
	Metadata      ProblemMetadata
	Excluded      []*html.Node
	Warnings      []string
}

// Sample is one ordered input/output example from a source page.
type Sample struct {
	Input       string     `json:"input" yaml:"input"`
	Output      string     `json:"output" yaml:"output"`
	Explanation *html.Node `json:"-" yaml:"-"`
}

// ParseOptions controls parser selection for one capture.
type ParseOptions struct {
	ForcedAdapterID string
}

// ParseResult contains rendered Markdown and normalized metadata for a capture.
type ParseResult struct {
	Markdown string
	Mode     string
	Metadata ProblemMetadata
	Warnings []string
}
