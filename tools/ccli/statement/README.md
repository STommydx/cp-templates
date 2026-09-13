# Statement parser and adapter contract

The `statement` package owns the PageMole capture model, parser selection, DOM renderer, normalized metadata, storage, and inventory. A source adapter supplies only source-specific URL patterns, structural matching, and extraction.

## Add an adapter

Create `statement/adapters/<stable-id>/adapter.go` and register its constructor in `cmd/statement.go`:

```go
type Adapter interface {
    ID() string
    URLPatterns() []URLPattern
    Match(capture *CaptureEnvelope, document *html.Node) bool
    Extract(capture *CaptureEnvelope, document *html.Node) (Extraction, error)
}
```

Satisfy these contracts:

1. `ID` returns a stable lowercase identifier. It is used in metadata, storage namespaces, inventory keys, and diagnostics. Do not change it after captures are published.
2. `URLPatterns` advertises exact hosts and path prefixes. Do not use a broad host pattern when the source has multiple page families. Hosts are matched case-insensitively; fragments and queries do not affect selection.
3. `Match` is a pure, cheap structural check over the already parsed DOM. It must not perform I/O, fetch another page, mutate the DOM, or guess from the title.
4. `Extract` runs only after `Match` succeeds. Return a non-nil `Root` on success. Return source-specific metadata, ordered samples, exact subtrees that are safe to exclude, and warnings for non-fatal observations.
5. Preserve unknown content. Leave unfamiliar sections in `Root`; do not remove a subtree unless its content is represented elsewhere in `Samples` or normalized metadata.
6. Keep adapter-specific selectors, URL rules, code extraction, limit parsing, and sample-table identification in the adapter package. Put reusable DOM traversal, safe text, sample fences, and Markdown behavior in the shared package.
7. Never invent absent limits, execution flags, codes, or samples. Missing values stay absent. A selected adapter that fails matching or extraction falls directly to the `unknown` fallback; the parser does not try another adapter.

## Extraction checklist

Before registering an adapter, answer these questions in its README and tests:

- Which URL host and path prefix select the adapter?
- Which DOM root proves that the page belongs to this source?
- Where are source identity and displayed limits located relative to the statement root?
- Which elements are content, controls, editor chrome, or run controls?
- Which table identifies structured samples without selecting score or subtask tables?
- How are exact input bytes recovered when a source exposes a raw-data attribute?
- How are explanation rows associated with samples?
- Which unknown structures remain visible in rendered Markdown?
- Which warnings are emitted when the expected structure is incomplete?

## Tests

Use a neutral, original fixture that mirrors the source structure without copying a real statement or using a real URL. Test observable behavior:

- URL selection and editor-page exclusion;
- structural match failure and extraction failure fallback;
- identity, limits, execution metadata, and provenance;
- sample boundaries, raw input bytes, explanations, and run controls;
- preserved unknown content and warnings;
- rendered Markdown without duplicate sample tables;
- a fixture shape copied from captured DOM evidence, not a simplified shape invented for the test.

The HKOI implementation is the reference adapter: [`adapters/hkoi/README.md`](adapters/hkoi/README.md).
