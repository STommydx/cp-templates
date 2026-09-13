# PageMole

PageMole is a minimal Chrome Manifest V3 extension for capturing the rendered HTML of the active page and sending it to a local problem tool. Judge-specific parsing, metadata extraction, and statement storage belong to `ccli`.

## Install locally

1. Open `chrome://extensions` in Chrome or Chromium.
2. Enable **Developer mode**.
3. Select **Load unpacked**.
4. Choose this directory: `templates/tools/pagemole`.
5. Pin **PageMole** to the browser toolbar.

Chrome does not hot-reload unpacked extensions. After changing a file, click **Reload** for PageMole on `chrome://extensions`.

## Capture a page

1. Start `ccli statement serve` on `http://127.0.0.1:27121/`.
2. Open a fully rendered problem page.
3. Click the PageMole toolbar action.
4. An `OK` badge indicates that the receiver returned a successful response.

The extension captures `document.documentElement.outerHTML`, the document title, and the page URL. The page must be scriptable and fully rendered before capture.

## Request payload

PageMole sends a JSON `POST /` request with `Content-Type: application/json`:

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

The receiver preserves the request body and returns a successful HTTP status after storing the capture. The endpoint is fixed at `http://127.0.0.1:27121/` for the first version.

## Output

`ccli statement serve` stores durable data under the XDG data directory:

```text
<ccli-data>/statements/<adapter>/<code-or-urlhash>-<slug>/<receipt-timestamp>/
    capture.json
    problem.json
    statement.md
```

See [`../ccli/README.md`](../ccli/README.md) for the API, storage, and inventory contracts.

## Scope

PageMole captures and transports page content. Parsing, metadata extraction, structured sample extraction, and Markdown rendering run in `ccli`.

## Writing

Follow [`../../WRITING.md`](../../WRITING.md) for comments, Markdown, commit messages, and pull request descriptions.
