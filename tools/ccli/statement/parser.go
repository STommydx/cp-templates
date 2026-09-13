package statement

import (
	"errors"
	"fmt"
	"net/url"
	"strings"

	nethtml "golang.org/x/net/html"
)

var ErrUnknownAdapter = errors.New("unknown adapter")

func ParseCapture(capture *CaptureEnvelope, adapters []Adapter, options ParseOptions) (ParseResult, error) {
	if capture == nil {
		return ParseResult{}, errors.New("capture is nil")
	}
	document, err := parseHTML(capture.HTML)
	if err != nil {
		return ParseResult{}, fmt.Errorf("parse capture html: %w", err)
	}
	if options.ForcedAdapterID != "" {
		var selected Adapter
		for _, adapter := range adapters {
			if adapter.ID() == options.ForcedAdapterID {
				selected = adapter
				break
			}
		}
		if selected == nil {
			return ParseResult{}, fmt.Errorf("%w %q", ErrUnknownAdapter, options.ForcedAdapterID)
		}
		return parseWithAdapter(capture, document, selected)
	}

	candidates := matchingAdapters(capture, adapters)
	if len(candidates) == 1 {
		return parseWithAdapter(capture, document, candidates[0])
	}
	if len(candidates) == 0 {
		return fallbackResult(capture, document, "no adapter URL pattern matched")
	}
	ids := make([]string, 0, len(candidates))
	for _, candidate := range candidates {
		ids = append(ids, candidate.ID())
	}
	return fallbackResult(capture, document, "multiple adapter URL patterns matched: "+strings.Join(SortStrings(ids), ", "))
}

func parseWithAdapter(capture *CaptureEnvelope, document *nethtml.Node, adapter Adapter) (ParseResult, error) {
	if !adapter.Match(capture, document) {
		fallbackDocument, err := parseHTML(capture.HTML)
		if err != nil {
			return ParseResult{}, fmt.Errorf("reparse fallback html: %w", err)
		}
		return fallbackResult(capture, fallbackDocument, fmt.Sprintf("adapter %q structural match failed", adapter.ID()))
	}
	extraction, err := adapter.Extract(capture, document)
	if err != nil || extraction.Root == nil {
		fallbackDocument, parseErr := parseHTML(capture.HTML)
		if parseErr != nil {
			return ParseResult{}, fmt.Errorf("reparse fallback html: %w", parseErr)
		}
		reason := fmt.Sprintf("adapter %q extraction failed", adapter.ID())
		if err != nil {
			reason += ": " + err.Error()
		}
		return fallbackResult(capture, fallbackDocument, reason)
	}

	capturedAt, _ := capture.CaptureTime()
	metadata := extraction.Metadata
	metadata.SchemaVersion = MetadataSchemaVersion
	metadata.Mode = "task-page"
	metadata.Source = SourceMetadata{
		Adapter:       adapter.ID(),
		ParserVersion: ParserVersion,
		URL:           capture.URL,
		CapturedAt:    capturedAt,
	}
	if metadata.Identity.Title == "" {
		metadata.Identity.Title = capture.Title
	}
	if metadata.Identity.Slug == "" {
		metadata.Identity.Slug = slugify(metadata.Identity.Title)
	}
	metadata.Samples = extraction.Samples
	metadata.Warnings = append([]string(nil), extraction.Warnings...)
	markdown, err := RenderMarkdown(metadata.Identity.Title, capture.URL, capture.CapturedAt, extraction.Root, extraction.Excluded, extraction.Samples, false)
	if err != nil {
		return ParseResult{}, err
	}
	return ParseResult{Markdown: markdown, Mode: "task-page", Metadata: metadata, Warnings: append([]string(nil), metadata.Warnings...)}, nil
}

func fallbackResult(capture *CaptureEnvelope, document *nethtml.Node, reason string) (ParseResult, error) {
	capturedAt, _ := capture.CaptureTime()
	metadata := ProblemMetadata{
		SchemaVersion: MetadataSchemaVersion,
		Mode:          "fallback",
		Source: SourceMetadata{
			Adapter:       FallbackAdapterID,
			ParserVersion: ParserVersion,
			URL:           capture.URL,
			CapturedAt:    capturedAt,
		},
		Identity: ProblemIdentity{Title: capture.Title, Slug: slugify(capture.Title)},
		Warnings: []string{reason},
	}
	root := FindTag(document, "body")
	if root == nil {
		root = document
	}
	markdown, err := RenderMarkdown(capture.Title, capture.URL, capture.CapturedAt, root, nil, nil, true)
	if err != nil {
		return ParseResult{}, err
	}
	return ParseResult{
		Markdown: markdown,
		Mode:     "fallback",
		Metadata: metadata,
		Warnings: append([]string(nil), metadata.Warnings...),
	}, nil
}

func matchingAdapters(capture *CaptureEnvelope, adapters []Adapter) []Adapter {
	u, err := capture.ParsedURL()
	if err != nil {
		return nil
	}
	var matches []Adapter
	for _, adapter := range adapters {
		for _, pattern := range adapter.URLPatterns() {
			if matchesURLPattern(u, pattern) {
				matches = append(matches, adapter)
				break
			}
		}
	}
	return matches
}

func matchesURLPattern(u *url.URL, pattern URLPattern) bool {
	if u == nil || !strings.EqualFold(u.Hostname(), pattern.Host) {
		return false
	}
	return strings.HasPrefix(u.EscapedPath(), pattern.PathPrefix) || strings.HasPrefix(u.Path, pattern.PathPrefix)
}

func parseHTML(source string) (*nethtml.Node, error) {
	return nethtml.Parse(strings.NewReader(source))
}
