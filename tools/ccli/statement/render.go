package statement

import (
	"bytes"
	"encoding/base64"
	"fmt"
	"net/url"
	"regexp"
	"sort"
	"strings"

	nethtml "golang.org/x/net/html"
)

var numericSampleID = regexp.MustCompile(`^\s*[0-9]+\s*$`)

// HasClass reports whether an element contains one exact CSS class token.
func HasClass(node *nethtml.Node, name string) bool {
	if node == nil || node.Type != nethtml.ElementNode {
		return false
	}
	for _, attr := range node.Attr {
		if attr.Key != "class" {
			continue
		}
		for _, className := range strings.Fields(attr.Val) {
			if className == name {
				return true
			}
		}
	}
	return false
}

// FindFirstClass returns the first element carrying a class token.
func FindFirstClass(root *nethtml.Node, className string) *nethtml.Node {
	var found *nethtml.Node
	Walk(root, func(node *nethtml.Node) bool {
		if found == nil && HasClass(node, className) {
			found = node
		}
		return found == nil
	})
	return found
}

// Walk visits a DOM subtree depth-first until the visitor returns false.
func Walk(root *nethtml.Node, visit func(*nethtml.Node) bool) {
	if root == nil || !visit(root) {
		return
	}
	for child := root.FirstChild; child != nil; child = child.NextSibling {
		Walk(child, visit)
	}
}

// TextContent returns concatenated text-node content from a subtree.
func TextContent(root *nethtml.Node) string {
	if root == nil {
		return ""
	}
	var builder strings.Builder
	Walk(root, func(node *nethtml.Node) bool {
		if node.Type == nethtml.TextNode {
			builder.WriteString(node.Data)
		}
		return true
	})
	return builder.String()
}

// Attribute returns one exact attribute value.
func Attribute(node *nethtml.Node, key string) (string, bool) {
	if node == nil {
		return "", false
	}
	for _, attr := range node.Attr {
		if attr.Key == key {
			return attr.Val, true
		}
	}
	return "", false
}

// FindTag returns the first element with the requested tag name.
func FindTag(root *nethtml.Node, tag string) *nethtml.Node {
	var found *nethtml.Node
	Walk(root, func(node *nethtml.Node) bool {
		if found == nil && node.Type == nethtml.ElementNode && strings.EqualFold(node.Data, tag) {
			found = node
		}
		return found == nil
	})
	return found
}

// FindTags returns all elements with the requested tag name in document order.
func FindTags(root *nethtml.Node, tag string) []*nethtml.Node {
	var found []*nethtml.Node
	Walk(root, func(node *nethtml.Node) bool {
		if node.Type == nethtml.ElementNode && strings.EqualFold(node.Data, tag) {
			found = append(found, node)
		}
		return true
	})
	return found
}

type sampleTableResult struct {
	Samples  []Sample
	Excluded []*nethtml.Node
	Found    bool
	Complete bool
}

// ExtractSampleTable finds and extracts one source-independent Input/Output table.
func ExtractSampleTable(root *nethtml.Node) sampleTableResult {
	if root == nil {
		return sampleTableResult{}
	}
	var candidate *nethtml.Node
	for _, table := range FindTags(root, "table") {
		if !HasClass(table, "samples") && FindFirstClass(table, "samples-wrapper") == nil && !hasClassAncestor(table, "samples-wrapper") {
			continue
		}
		if !tableHasInputOutputHeader(table) {
			continue
		}
		candidate = table
		break
	}
	if candidate == nil {
		return sampleTableResult{}
	}

	result := sampleTableResult{Found: true}
	var current *Sample
	allRowsHandled := true
	for _, row := range FindTags(candidate, "tr") {
		if row == candidate {
			continue
		}
		if tableHeaderRow(row) {
			continue
		}
		cells := elementsWithClass(row, "io")
		if len(cells) >= 2 && rowHasNumericIdentifier(row) {
			input, ok := sampleCellValue(cells[0], true)
			if !ok {
				allRowsHandled = false
				continue
			}
			output, _ := sampleCellValue(cells[1], false)
			result.Samples = append(result.Samples, Sample{Input: input, Output: output})
			current = &result.Samples[len(result.Samples)-1]
			continue
		}
		if current != nil && meaningfulExplanation(row) {
			if current.Explanation == nil {
				current.Explanation = explanationNode(row)
			} else {
				allRowsHandled = false
			}
			continue
		}
		if rowHasVisibleText(row) && !isTableControl(row) {
			allRowsHandled = false
		}
	}
	result.Complete = len(result.Samples) > 0 && allRowsHandled
	if !result.Complete {
		return result
	}
	if wrapper := nearestAncestor(candidate, "samples-wrapper", root); wrapper != nil {
		result.Excluded = []*nethtml.Node{wrapper}
	} else {
		result.Excluded = []*nethtml.Node{candidate}
	}
	return result
}

func tableHasInputOutputHeader(table *nethtml.Node) bool {
	for _, row := range FindTags(table, "tr") {
		if tableHeaderRow(row) {
			return true
		}
	}
	return false
}

func tableHeaderRow(row *nethtml.Node) bool {
	var input, output bool
	for _, cell := range directCells(row) {
		text := strings.ToLower(strings.TrimSpace(TextContent(cell)))
		input = input || text == "input"
		output = output || text == "output"
	}
	return input && output
}

func directCells(row *nethtml.Node) []*nethtml.Node {
	var cells []*nethtml.Node
	for child := row.FirstChild; child != nil; child = child.NextSibling {
		if child.Type == nethtml.ElementNode && (child.Data == "td" || child.Data == "th") {
			cells = append(cells, child)
		}
	}
	return cells
}

func elementsWithClass(root *nethtml.Node, className string) []*nethtml.Node {
	var nodes []*nethtml.Node
	Walk(root, func(node *nethtml.Node) bool {
		if HasClass(node, className) {
			nodes = append(nodes, node)
		}
		return true
	})
	return nodes
}

func rowHasNumericIdentifier(row *nethtml.Node) bool {
	cells := directCells(row)
	return len(cells) > 0 && numericSampleID.MatchString(TextContent(cells[0]))
}

func sampleCellValue(cell *nethtml.Node, decodeRaw bool) (string, bool) {
	if decodeRaw {
		if raw, ok := Attribute(cell, "data-raw"); ok {
			decoded, err := base64.StdEncoding.DecodeString(raw)
			if err == nil {
				return string(decoded), true
			}
		}
	}
	if pre := FindTag(cell, "pre"); pre != nil {
		return TextContent(pre), true
	}
	return TextContent(cell), true
}

func meaningfulExplanation(row *nethtml.Node) bool {
	if isTableControl(row) || !rowHasVisibleText(row) {
		return false
	}
	return true
}

func isTableControl(row *nethtml.Node) bool {
	if HasClass(row, "run") || HasClass(row, "control") || HasClass(row, "controls") {
		return true
	}
	var control bool
	Walk(row, func(node *nethtml.Node) bool {
		if node.Type == nethtml.ElementNode && (node.Data == "button" || node.Data == "input" || node.Data == "form") {
			control = true
			return false
		}
		return true
	})
	return control
}

func rowHasVisibleText(row *nethtml.Node) bool {
	return strings.TrimSpace(TextContent(row)) != ""
}

func explanationNode(row *nethtml.Node) *nethtml.Node {
	cells := directCells(row)
	if len(cells) > 0 {
		return cells[0]
	}
	return row
}

func hasClassAncestor(node *nethtml.Node, className string) bool {
	for parent := node.Parent; parent != nil; parent = parent.Parent {
		if HasClass(parent, className) {
			return true
		}
	}
	return false
}

func nearestAncestor(node *nethtml.Node, className string, stop *nethtml.Node) *nethtml.Node {
	for current := node; current != nil && current != stop; current = current.Parent {
		if HasClass(current, className) {
			return current
		}
	}
	return nil
}

// RenderMarkdown builds a readable statement and validates its Markdown AST with Goldmark.
func RenderMarkdown(title, sourceURL string, capturedAt string, root *nethtml.Node, excluded []*nethtml.Node, samples []Sample, fallback bool) (string, error) {
	var builder strings.Builder
	builder.WriteString("# ")
	builder.WriteString(escapeMarkdownText(strings.TrimSpace(title)))
	builder.WriteString("\n\nSource: ")
	builder.WriteString(SanitizeTerminalText(sourceURL))
	builder.WriteString("\nCaptured: ")
	builder.WriteString(SanitizeTerminalText(capturedAt))
	builder.WriteString("\n\n")
	if fallback {
		builder.WriteString("<!-- fallback: task-page root was absent or unusable; rendered document body -->\n\n")
	}
	state := renderState{excluded: nodeSet(excluded)}
	builder.WriteString(renderNode(root, state))
	for index, sample := range samples {
		builder.WriteString("\n\n### Sample ")
		builder.WriteString(fmt.Sprint(index + 1))
		builder.WriteString("\n\nInput\n\n")
		builder.WriteString(fencedText(sample.Input))
		builder.WriteString("\n\nOutput\n\n")
		builder.WriteString(fencedText(sample.Output))
		if sample.Explanation != nil {
			builder.WriteString("\n\n")
			builder.WriteString(renderNode(sample.Explanation, state))
		}
	}
	return BuildMarkdown(normalizeMarkdown(builder.String()))
}

type renderState struct {
	excluded  map[*nethtml.Node]bool
	listDepth int
}

func nodeSet(nodes []*nethtml.Node) map[*nethtml.Node]bool {
	set := make(map[*nethtml.Node]bool, len(nodes))
	for _, node := range nodes {
		set[node] = true
	}
	return set
}

func renderNode(node *nethtml.Node, state renderState) string {
	if node == nil || state.excluded[node] {
		return ""
	}
	switch node.Type {
	case nethtml.TextNode:
		return escapeMarkdownText(node.Data)
	case nethtml.CommentNode:
		return ""
	case nethtml.DocumentNode, nethtml.ElementNode:
	default:
		return ""
	}
	if node.Type == nethtml.DocumentNode {
		return renderChildren(node, state)
	}
	tag := strings.ToLower(node.Data)
	if tag == "script" || tag == "style" || tag == "noscript" || tag == "iframe" {
		return ""
	}
	if (tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "h5" || tag == "h6") && (HasClass(node, "sr-only") || HasClass(node, "hidden-print")) {
		return ""
	}
	switch tag {
	case "h1", "h2", "h3", "h4", "h5", "h6":
		level := min(int(tag[1]-'0')+1, 6)
		return "\n\n" + strings.Repeat("#", level) + " " + strings.TrimSpace(renderInlineChildren(node, state)) + "\n\n"
	case "p":
		return "\n\n" + strings.TrimSpace(renderInlineChildren(node, state)) + "\n\n"
	case "br":
		return "\n"
	case "strong", "b":
		return "**" + strings.TrimSpace(renderInlineChildren(node, state)) + "**"
	case "em", "i":
		return "*" + strings.TrimSpace(renderInlineChildren(node, state)) + "*"
	case "code":
		if node.Parent != nil && node.Parent.Type == nethtml.ElementNode && node.Parent.Data == "pre" {
			return TextContent(node)
		}
		return renderInlineCode(renderInlineChildren(node, state))
	case "pre":
		return "\n\n" + fencedText(TextContent(node)) + "\n\n"
	case "ul", "ol":
		return "\n\n" + renderList(node, state, tag == "ol") + "\n\n"
	case "li":
		return strings.TrimSpace(renderInlineChildren(node, state))
	case "blockquote":
		text := strings.TrimSpace(normalizeInline(renderChildren(node, state)))
		lines := strings.Split(text, "\n")
		for index := range lines {
			lines[index] = "> " + lines[index]
		}
		return "\n\n" + strings.Join(lines, "\n") + "\n\n"
	case "a":
		text := strings.TrimSpace(renderInlineChildren(node, state))
		if href, ok := Attribute(node, "href"); ok {
			if destination := safeLinkDestination(href); destination != "" {
				return "[" + text + "](" + destination + ")"
			}
		}
		return text
	case "img":
		alt, _ := Attribute(node, "alt")
		src, _ := Attribute(node, "src")
		if destination := safeLinkDestination(src); destination != "" {
			return fmt.Sprintf("![%s](%s)", escapeMarkdownText(alt), destination)
		}
		return escapeMarkdownText(alt)
	case "table":
		return "\n\n" + serializeHTML(node) + "\n\n"
	case "math", "svg":
		return serializeHTML(node)
	case "details":
		return "\n\n" + renderChildren(node, state) + "\n\n"
	case "summary":
		return "\n\n" + strings.TrimSpace(renderInlineChildren(node, state)) + "\n\n"
	}
	return renderChildren(node, state)
}

func renderChildren(node *nethtml.Node, state renderState) string {
	var builder strings.Builder
	for child := node.FirstChild; child != nil; child = child.NextSibling {
		builder.WriteString(renderNode(child, state))
	}
	return builder.String()
}

func renderInlineChildren(node *nethtml.Node, state renderState) string {
	return normalizeInline(renderChildren(node, state))
}

func renderInlineCode(value string) string {
	value = strings.TrimSpace(value)
	longest := 1
	for _, run := range regexp.MustCompile("`+").FindAllString(value, -1) {
		if len(run) >= longest {
			longest = len(run) + 1
		}
	}
	delimiter := strings.Repeat("`", longest)
	if longest > 1 {
		value = " " + value + " "
	}
	return delimiter + value + delimiter
}

func renderList(node *nethtml.Node, state renderState, ordered bool) string {
	var lines []string
	index := 1
	for child := node.FirstChild; child != nil; child = child.NextSibling {
		if child.Type != nethtml.ElementNode || child.Data != "li" || state.excluded[child] {
			continue
		}
		prefix := "- "
		if ordered {
			prefix = fmt.Sprintf("%d. ", index)
		}
		lines = append(lines, prefix+renderListItem(child, state))
		index++
	}
	return strings.Join(lines, "\n")
}

func renderListItem(node *nethtml.Node, state renderState) string {
	var inline strings.Builder
	var nested []string
	for child := node.FirstChild; child != nil; child = child.NextSibling {
		if child.Type == nethtml.ElementNode && (child.Data == "ul" || child.Data == "ol") {
			nestedText := renderList(child, state, child.Data == "ol")
			for _, line := range strings.Split(nestedText, "\n") {
				nested = append(nested, "  "+line)
			}
			continue
		}
		inline.WriteString(renderNode(child, state))
	}
	result := strings.TrimSpace(normalizeInline(inline.String()))
	if len(nested) > 0 {
		result += "\n" + strings.Join(nested, "\n")
	}
	return result
}

func normalizeInline(value string) string {
	return strings.Join(strings.Fields(value), " ")
}

// SanitizeTerminalText removes control characters that can alter terminal output.
func SanitizeTerminalText(value string) string {
	return strings.Map(func(r rune) rune {
		if r == '\n' || r == '\r' || r == '\t' {
			return r
		}
		if r < 0x20 || r == 0x7f {
			return -1
		}
		return r
	}, value)
}

func escapeMarkdownText(value string) string {
	value = SanitizeTerminalText(value)
	return strings.NewReplacer("\\", "\\\\", "`", "\\`", "*", "\\*", "_", "\\_", "[", "\\[", "]", "\\]", "<", "\\<", ">", "\\>").Replace(value)
}

func safeLinkDestination(value string) string {
	value = strings.TrimSpace(SanitizeTerminalText(value))
	parsed, err := url.Parse(value)
	if err != nil {
		return ""
	}
	switch strings.ToLower(parsed.Scheme) {
	case "", "http", "https", "mailto":
	default:
		return ""
	}
	if strings.ContainsAny(value, "\r\n()<>\"") {
		return ""
	}
	return value
}

func fencedText(value string) string {
	value = SanitizeTerminalText(strings.TrimRight(value, "\n"))
	longest := 3
	for _, run := range regexp.MustCompile("`+").FindAllString(value, -1) {
		if len(run)+1 > longest {
			longest = len(run) + 1
		}
	}
	fence := strings.Repeat("`", longest)
	return fence + "text\n" + value + "\n" + fence
}

func serializeHTML(node *nethtml.Node) string {
	safe := cloneSafeHTML(node)
	if safe == nil {
		return escapeMarkdownText(TextContent(node))
	}
	var buffer bytes.Buffer
	if err := nethtml.Render(&buffer, safe); err != nil {
		return escapeMarkdownText(TextContent(node))
	}
	return SanitizeTerminalText(buffer.String())
}

// cloneSafeHTML copies a subtree while dropping active content, event handlers,
// inline styles, and unsafe URI attributes. Unknown markup is preserved.
func cloneSafeHTML(node *nethtml.Node) *nethtml.Node {
	if node == nil {
		return nil
	}
	if node.Type == nethtml.TextNode {
		return &nethtml.Node{Type: nethtml.TextNode, Data: SanitizeTerminalText(node.Data)}
	}
	if node.Type != nethtml.ElementNode {
		return nil
	}
	tag := strings.ToLower(node.Data)
	if tag == "script" || tag == "style" || tag == "noscript" || tag == "iframe" || tag == "svg" {
		return nil
	}
	safe := &nethtml.Node{Type: nethtml.ElementNode, Data: node.Data}
	for _, attr := range node.Attr {
		key := strings.ToLower(attr.Key)
		if strings.HasPrefix(key, "on") || key == "style" || key == "srcdoc" {
			continue
		}
		value := SanitizeTerminalText(attr.Val)
		if key == "href" || key == "src" {
			value = safeLinkDestination(value)
			if value == "" {
				continue
			}
		}
		safe.Attr = append(safe.Attr, nethtml.Attribute{Key: attr.Key, Val: value})
	}
	for child := node.FirstChild; child != nil; child = child.NextSibling {
		if cloned := cloneSafeHTML(child); cloned != nil {
			safe.AppendChild(cloned)
		}
	}
	return safe
}

func normalizeMarkdown(value string) string {
	lines := strings.Split(strings.Trim(value, " \t\n"), "\n")
	var output []string
	blank := false
	fenceLength := 0
	for _, line := range lines {
		trimmed := strings.TrimSpace(line)
		backticks := leadingBackticks(trimmed)
		if fenceLength == 0 && backticks >= 3 {
			fenceLength = backticks
			blank = false
			output = append(output, line)
			continue
		}
		if fenceLength > 0 {
			output = append(output, line)
			if backticks >= fenceLength && strings.TrimSpace(trimmed[backticks:]) == "" {
				fenceLength = 0
			}
			continue
		}
		line = strings.TrimRight(line, " \t")
		if line == "" {
			if blank {
				continue
			}
			blank = true
		} else {
			blank = false
		}
		output = append(output, line)
	}
	return strings.TrimSpace(strings.Join(output, "\n")) + "\n"
}

func leadingBackticks(value string) int {
	count := 0
	for count < len(value) && value[count] == '`' {
		count++
	}
	return count
}

// SortStrings returns a sorted copy without modifying the input slice.
func SortStrings(values []string) []string {
	result := append([]string(nil), values...)
	sort.Strings(result)
	return result
}
