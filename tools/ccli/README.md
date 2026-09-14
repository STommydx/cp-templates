# ccli

`ccli` manages competitive programming source files, compiles solutions, packs template includes, and serves captured problem statements.

## Source workflow

```bash
ccli init project-name
ccli add A.cpp
ccli run A.cpp
```

`ccli add` creates a source file from the repository scaffold and updates the project `CMakeLists.txt`. `ccli run` packs the included templates, compiles the source, and runs it against the configured input. Manual `#include` use remains supported.

## Statement workflow

Start the foreground receiver before clicking the PageMole browser action:

```bash
ccli statement serve
```

The receiver binds to `127.0.0.1:27121`. PageMole sends a versioned JSON envelope containing the rendered page HTML. The receiver validates the envelope, parses the page, and writes three files for each capture:

```text
<storage-root>/<adapter>/<code-or-urlhash>-<slug>/<receipt-timestamp>/
    capture.json
    problem.json
    statement.md
```

`capture.json` preserves the request body byte-for-byte. `problem.json` records the parser mode, source URL, problem identity, limits, execution metadata, structured samples, warnings, and capture provenance. `statement.md` is the readable statement.

The default storage root is `$XDG_DATA_HOME/ccli/statements` on systems that define `XDG_DATA_HOME`. Platform defaults come from `github.com/adrg/xdg`. Pass `--output-dir DIR` on the `statement` parent command to use another root for both serving and lookup commands.
The receiver is loopback-only but has no authentication; any local process can submit captures. v1 retains captures indefinitely, so use a private output root and remove unwanted data manually when needed.

The first adapter is `hkoi`. It maps `judge.hkoi.org/task/` pages. Use `--adapter hkoi` when a neutral local fixture should use the HKOI page structure without matching that public URL.
The HKOI selector and extraction contract is documented in [`statement/adapters/hkoi/README.md`](statement/adapters/hkoi/README.md), including the page-level `.task-info`/`.task` split, sample-table rules, and fallback behavior.
Generic adapter authoring, lifecycle, extraction, and test requirements are documented in [`statement/README.md`](statement/README.md).

`serve` logs to standard error through `charm.land/log/v2` and takes `--log-level` (`debug`, `info`, `warn`, or `error`, default `info`) and `--log-format` (`text` or `json`, default `text`). It records startup (listen address and storage root), each stored capture with its key, mode, and sample count, one warning record per parser warning, and the cause of storage or parse failures. Huma and HTTP failures are reported by `github.com/go-chi/httplog/v3`: every request at `debug`, and rejected requests at `info` and above.

## Statement commands

```bash
ccli statement root
ccli statement list --format table
ccli statement list --format json
ccli statement list --format yaml
ccli statement path hkoi/UDEV
ccli statement path hkoi/UDEV --file metadata
ccli statement path hkoi/UDEV --file capture --capture 20260913T132107.102282Z
ccli statement show hkoi/UDEV
```

`list` returns one row per logical problem and includes the newest capture. It reports `capture_count`, limits, warnings, and sample counts. `root` prints one absolute storage-root path. `path` prints a storage-root-relative capture directory or artifact path; `path --file statement` identifies the raw Markdown artifact. `show` renders the selected Markdown for a terminal with Glamour v2, which uses the dark theme by default and honors `GLAMOUR_STYLE`. Rendered lines wrap to the terminal width, falling back to 80 columns. Piped or redirected output writes the stored Markdown artifact unchanged. Pager integration is not part of this command contract.

Structured output uses `table`, `json`, or `yaml`. The JSON and YAML record fields are `key`, `adapter`, `code`, `title`, `time_ms`, `memory_mib`, `captured_at`, `received_at`, `capture_count`, `warnings_count`, `samples_count`, `directory`, and `statement`.

## API contract

The local server exposes:

- `POST /` for PageMole captures, returning `201 Created` with relative artifact paths;
- `GET /health`, returning `204 No Content` when the server is ready;
- Huma-generated `/openapi.json`, `/openapi.yaml`, `/docs`, and `/schemas` endpoints.

The capture request uses `Content-Type: application/json` and the following required fields:

```json
{
  "type": "statement-html",
  "version": 1,
  "capturedAt": "2026-09-13T00:00:00.000Z",
  "title": "Example problem",
  "url": "https://example.invalid/tasks/example",
  "html": "<html>...</html>"
}
```

The server accepts unknown JSON fields for forward compatibility. It returns `400` for malformed JSON, `415` for unsupported content types, `413` for bodies larger than 8 MiB, `422` for schema or resolver validation errors, and `500` for parser or storage failures.

## Writing

Follow [`../../WRITING.md`](../../WRITING.md) for comments, Markdown, commit messages, and pull request descriptions. Keep API contracts and rationale in this README; keep source comments short and focused on non-obvious constraints.
