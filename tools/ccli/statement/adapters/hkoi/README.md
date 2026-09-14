# HKOI statement adapter

The `hkoi` adapter parses rendered task pages served from `judge.hkoi.org/task/`. It does not fetch pages. PageMole supplies the rendered HTML, and the adapter reads that document locally.

## URL selection

The adapter advertises this pattern:

```text
host: judge.hkoi.org
path prefix: /task/
```

Host comparison is case-insensitive. Fragments and query parameters do not affect selection. Editor pages under `/schooladmin/hosted/` are not part of this mapping.

## Captured page structure

The page-level metadata block is a sibling of the statement root:

```text
document
├── .task-info
│   ├── .task-displayid
│   ├── Time Limit: ...
│   └── Memory Limit: ...
└── .task
    └── rendered statement content
        └── .samples-wrapper
            └── table.samples
```

The sibling relationship is significant. `.task-displayid` and `.task-info` must be searched from the document, not from `.task`.

## Match contract

`Match` returns true when the document contains a `.task` element. It is a structural check only. It performs no I/O, does not inspect another URL, and does not mutate the DOM.

A URL match with no `.task` root falls through to the parser's `unknown` fallback. The fallback preserves visible body content and records a warning instead of trying another adapter.

## Extraction contract

`Extract` runs after a successful `Match`:

| Source structure | Extracted value |
| --- | --- |
| `.task` | Statement root passed to the shared renderer |
| `.task-displayid` | Problem code, trimmed as visible text |
| `.task-info` | Displayed time and memory limits, converted to milliseconds and MiB while retaining raw text |
| `.samples-wrapper` table with `Input` and `Output` headers | Ordered structured samples |
| Numeric sample row with two `.io` cells | Input/output pair |
| Valid `data-raw` on input cell | Exact decoded input bytes |
| Explanation row after a sample row | Explanation attached to that sample, taken from its `.explanation` content cell rather than the leading label cell |

The adapter selects the first table whose headers contain both `Input` and `Output`. Subtask and score tables are not sample tables. Run controls are ignored. The sample wrapper is excluded from normal rendering only after all data and explanation rows are handled; otherwise the table remains visible and a warning is recorded. The rendered sample section replaces the wrapper in place, so samples stay under the section that introduced them.

The shared renderer preserves headings, lists, code blocks, links, images, MathML, details, blockquotes, and unknown structures, and converts simple uniform tables into Markdown tables; a table that would lose cells or formatting stays as HTML. The adapter does not produce Markdown or write files.

Execution metadata is populated only when the rendered page states an explicit value. Missing values remain absent.

## Tests and maintenance

`testdata/lantern.html` is the adapter fixture. It mirrors the shapes observed in captured HKOI pages: the full live DOM PageMole sends (`documentElement.outerHTML`, so navbar, sidebar, code-editor switcher, modal, scripts, and the code iframe are present), `.task-info` with `hidden-xs` limit labels and the created-on dropdown, sr-only duplicate headings, MathJax containers whose CHTML half is empty while `mjx-assistive-mml` carries the real `<math>`, `pre` blocks both standalone and inside `td.explanation`, a bordered details table, and a `table.samples` with `data-raw` input cells, empty `run-sample` cells, label-plus-`.explanation` rows, and separator rows. It keeps original statement text, neutral branding, and `example.invalid` URLs.

Update the fixture when a new selector or content invariant is verified against a captured page, and add an assertion for the observable metadata or Markdown behavior that the change protects. The statement root is the first `div` carrying `task`: the site toggles classes on the body element, so a bare class lookup can select page chrome instead of the statement.
