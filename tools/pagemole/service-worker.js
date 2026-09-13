const CAPTURE_ENDPOINT = 'http://127.0.0.1:27121/';
const REQUEST_TIMEOUT_MS = 10000;

async function setStatus(tabId, text, color) {
  await chrome.action.setBadgeText({ tabId, text });
  await chrome.action.setBadgeBackgroundColor({ tabId, color });
}

async function capture(tab) {
  if (typeof tab.id !== 'number') {
    throw new Error('The active tab has no usable id');
  }

  const [result] = await chrome.scripting.executeScript({
    target: { tabId: tab.id },
    func: () => ({
      html: document.documentElement?.outerHTML ?? '',
      title: document.title,
      url: window.location.href,
    }),
  });

  const page = result?.result;
  if (!page?.html) {
    throw new Error('The active page did not provide HTML');
  }

  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);

  try {
    const response = await fetch(CAPTURE_ENDPOINT, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        type: 'statement-html',
        version: 1,
        capturedAt: new Date().toISOString(),
        title: page.title,
        url: page.url,
        html: page.html,
      }),
      signal: controller.signal,
    });

    if (!response.ok) {
      throw new Error(`Capture server returned HTTP ${response.status}`);
    }
  } finally {
    clearTimeout(timeout);
  }
}

chrome.action.onClicked.addListener(async tab => {
  if (typeof tab.id !== 'number') {
    return;
  }

  try {
    await setStatus(tab.id, '...', '#6e7781');
    await capture(tab);
    await setStatus(tab.id, 'OK', '#1a7f37');
  } catch (error) {
    console.error('PageMole capture failed', error);
    await setStatus(tab.id, 'ERR', '#cf222e');
  }
});
