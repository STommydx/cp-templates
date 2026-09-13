package statement

import (
	"fmt"

	"github.com/yuin/goldmark"
	"github.com/yuin/goldmark/ast"
	"github.com/yuin/goldmark/extension"
	"github.com/yuin/goldmark/text"
)

var statementMarkdown = goldmark.New(goldmark.WithExtensions(extension.GFM))

// blockCounts records the headings and fenced code blocks the renderer emits.
type blockCounts struct {
	headings int
	fences   int
}

func (c *blockCounts) heading() { c.headings++ }

func (c *blockCounts) fence() { c.fences++ }

// BuildMarkdown returns the generated Markdown unchanged after checking that
// Goldmark's GFM parser reads it back as the structure the renderer emitted.
// The parser accepts any input, so validation compares block counts instead of
// looking for parse errors; a fence that closes early or a marker that stops
// being a block changes those counts. The source is preserved unchanged so raw
// MathML and unknown HTML survive for later consumers.
func BuildMarkdown(source string, counts blockCounts) (string, error) {
	source = SanitizeTerminalText(source)
	document := statementMarkdown.Parser().Parse(text.NewReader([]byte(source)))
	parsed := blockCounts{}
	if err := ast.Walk(document, func(node ast.Node, entering bool) (ast.WalkStatus, error) {
		if !entering {
			return ast.WalkContinue, nil
		}
		switch node.(type) {
		case *ast.Heading:
			parsed.headings++
		case *ast.FencedCodeBlock:
			parsed.fences++
		}
		return ast.WalkContinue, nil
	}); err != nil {
		return "", fmt.Errorf("parse generated Markdown: %w", err)
	}
	if parsed != counts {
		return "", fmt.Errorf("generated Markdown parsed as %d headings and %d code fences, want %d and %d",
			parsed.headings, parsed.fences, counts.headings, counts.fences)
	}
	return source, nil
}
