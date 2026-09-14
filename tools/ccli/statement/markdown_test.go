package statement

import "testing"

func TestBuildMarkdownRejectsStructureDrift(t *testing.T) {
	source := "# Title\n\n```text\nfirst line\n```\n"
	validated, err := BuildMarkdown(source, blockCounts{headings: 1, fences: 1})
	if err != nil {
		t.Fatalf("valid structure rejected: %v", err)
	}
	if validated != source {
		t.Fatalf("source changed:\n%q", validated)
	}

	// A fence that closes early leaves the remaining lines as prose instead of
	// one code block, which is the drift the renderer must not emit.
	drifted := "# Title\n\n```text\nfirst\n```\nsecond\n```\n"
	if _, err := BuildMarkdown(drifted, blockCounts{headings: 1, fences: 1}); err == nil {
		t.Fatal("fence that closes early was accepted")
	}

	// A marker that stops being a block changes the heading count too.
	if _, err := BuildMarkdown(source, blockCounts{headings: 2, fences: 1}); err == nil {
		t.Fatal("heading drift was accepted")
	}
	if _, err := BuildMarkdown("\n\n", blockCounts{headings: 1}); err == nil {
		t.Fatal("empty document accepted")
	}
}
