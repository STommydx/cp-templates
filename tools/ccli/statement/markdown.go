package statement

import (
	"errors"
	"strings"

	"github.com/yuin/goldmark"
	"github.com/yuin/goldmark/extension"
	"github.com/yuin/goldmark/text"
)

var statementMarkdown = goldmark.New(goldmark.WithExtensions(extension.GFM))

// BuildMarkdown builds a Goldmark AST before returning the source unchanged.
// The DOM renderer owns HTML-to-Markdown policy; preserving the source keeps
// raw MathML and unknown HTML structures loss-minimizing for later consumers.
func BuildMarkdown(source string) (string, error) {
	source = SanitizeTerminalText(source)
	if strings.TrimSpace(source) == "" {
		return "", errors.New("generated Markdown is empty")
	}
	if document := statementMarkdown.Parser().Parse(text.NewReader([]byte(source))); document == nil {
		return "", errors.New("Goldmark could not build the Markdown document")
	}
	return source, nil
}
