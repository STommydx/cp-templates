# PageMole

PageMole is a minimal Chrome Manifest V3 extension for capturing the rendered
HTML of the active page and sending it to a local problem tool. It does not
parse judges and does not depend on Competitive Companion.

## Install locally

1. Open `chrome://extensions` in Chrome or Chromium.
2. Enable **Developer mode**.
3. Select **Load unpacked**.
4. Choose this directory: `templates/tools/pagemole`.
5. Pin **PageMole** to the browser toolbar.

Chrome does not hot-reload unpacked extensions. After changing a file, click
**Reload** for PageMole on `chrome://extensions`.

## Capture a page

1. Start a local receiver on `http://127.0.0.1:27121/`.
2. Open a supported problem page.
3. Click the PageMole toolbar action.
4. An `OK` badge indicates that the receiver returned a successful response.

The extension captures `document.documentElement.outerHTML`, so the page must
be scriptable and fully rendered before capture.

## Request payload

PageMole sends a JSON `POST /` request with `Content-Type: application/json`:

```json
{
  "type": "statement-html",
  "version": 1,
  "capturedAt": "2026-09-13T00:00:00.000Z",
  "title": "Example problem",
  "url": "https://judge.example/problem/123",
  "html": "<!doctype html>..."
}
```

The receiver should bind to loopback, preserve the raw HTML, and return any
successful HTTP status. The endpoint and port are fixed for this first version;
the future `ccli statement serve` command will own the receiver.

## Scope

PageMole only captures and transports page content. Judge-specific parsing,
statement storage, and normalized output belong in `ccli`.
