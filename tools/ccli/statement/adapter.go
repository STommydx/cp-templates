package statement

import "golang.org/x/net/html"

const FallbackAdapterID = "unknown"

type URLPattern struct {
	Host       string
	PathPrefix string
}

type Adapter interface {
	ID() string
	URLPatterns() []URLPattern
	Match(capture *CaptureEnvelope, document *html.Node) bool
	Extract(capture *CaptureEnvelope, document *html.Node) (Extraction, error)
}

type Extraction struct {
	Root     *html.Node
	Samples  []Sample
	Metadata ProblemMetadata
	Excluded []*html.Node
	Warnings []string
}

type Sample struct {
	Input       string     `json:"input" yaml:"input"`
	Output      string     `json:"output" yaml:"output"`
	Explanation *html.Node `json:"-" yaml:"-"`
}

type ParseOptions struct {
	ForcedAdapterID string
}

type ParseResult struct {
	Markdown string
	Mode     string
	Metadata ProblemMetadata
	Warnings []string
}
