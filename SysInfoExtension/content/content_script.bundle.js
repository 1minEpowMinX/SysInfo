(() => {
  // content/lib/compat.js
  if (typeof browser === "undefined") {
    globalThis.browser = chrome;
  }
  var t = (key, substitutions) => browser.i18n.getMessage(key, substitutions) || key;
  var SYSINFO_LOG_TAG = "[SysInfo]";
  var slog = (...args) => console.log(SYSINFO_LOG_TAG, ...args);
  var swarn = (...args) => console.warn(SYSINFO_LOG_TAG, ...args);
  var _sysinfoRoot = document.documentElement;
  _sysinfoRoot.dataset.sysinfoLoaded = String(Date.now());
  _sysinfoRoot.dataset.sysinfoVersion = browser.runtime && browser.runtime.getManifest && browser.runtime.getManifest().version || "?";
  try {
    browser.runtime.sendMessage(
      {
        action: "diagPing",
        source: "content-script-load",
        path: location.pathname,
        time: Date.now()
      },
      () => {
        if (browser.runtime.lastError) {
        }
      }
    );
  } catch (e) {
  }
  slog("content script loaded", { href: location.href });

  // shared/constants.js
  var STORAGE_KEY = "sysinfo_settings_v1";
  var HISTORY_KEY = "sysinfo_history_v1";
  var DEFAULT_PORTAL_IDS = ["41", "141"];
  var DEFAULT_TYPE_IDS = [
    "217",
    "218",
    "219",
    "220",
    "213",
    "216",
    "212",
    "181",
    "182",
    "183",
    "184",
    "185",
    "187",
    "192"
  ];

  // content/lib/constants.js
  var SUBMIT_TTL_MS = 2 * 60 * 1e3;
  var SYSINFO_REQUEST_RETRIES = 5;
  var SYSINFO_REQUEST_RETRY_MS = 2e3;
  var INSERTION_TICK_MS = 2e3;
  var URL_TICK_MS = 500;
  var EDITOR_WAIT_MS = 3e4;
  var EDITOR_ROOT_GRACE_MS = 3e3;
  var FORM_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/create\/(\d+)/;
  var TICKET_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/([A-Z][A-Z0-9]+-\d+)(?:\/|$)/;
  var EDITOR_ROOT_SELECTOR = "#ak-editor-textarea";
  var EDITOR_SELECTOR = `${EDITOR_ROOT_SELECTOR} > p`;
  var SUBMIT_CONTROL_SELECTOR = 'button[type="submit"], input[type="submit"], form .buttons-container button.aui-button.aui-button-primary';
  var TITLE_SELECTOR = ".cv-page-title-main h1 span";
  var TITLE_WAIT_MS = 5e3;

  // content/lib/portals.js
  var userPortals = null;
  var userTypes = null;
  var loaded = null;
  function applyStored(stored) {
    userPortals = isStringArray(stored && stored.portals) ? stored.portals : null;
    userTypes = isStringArray(stored && stored.types) ? stored.types : null;
    slog("portals: lists applied", {
      portals: userPortals ? userPortals.length : "default",
      types: userTypes ? userTypes.length : "default"
    });
  }
  function watchStorage() {
    try {
      browser.storage.onChanged.addListener((changes, area) => {
        if (area !== "local" || !changes[STORAGE_KEY]) return;
        applyStored(changes[STORAGE_KEY].newValue);
      });
    } catch (e) {
      swarn("portals: onChanged unavailable", e && e.message);
    }
  }
  function loadPortals() {
    if (loaded) return loaded;
    loaded = new Promise((resolve) => {
      try {
        browser.storage.local.get([STORAGE_KEY], (r) => {
          applyStored(r && r[STORAGE_KEY]);
          watchStorage();
          resolve();
        });
      } catch (e) {
        swarn("portals: storage exception", e && e.message);
        resolve();
      }
    });
    return loaded;
  }
  function isStringArray(value) {
    return Array.isArray(value) && value.length > 0 && value.every((v) => typeof v === "string");
  }
  function getAllowedPortals() {
    return userPortals || DEFAULT_PORTAL_IDS;
  }
  function getAllowedTypes() {
    return userTypes || DEFAULT_TYPE_IDS;
  }
  function isTicketAllowed(pathname) {
    const match = pathname.match(FORM_PATH_RE);
    if (!match) return false;
    const [, portalId, ticketId] = match;
    return getAllowedPortals().includes(portalId) && getAllowedTypes().includes(ticketId);
  }

  // content/lib/history.js
  var pendingInsertion = null;
  function markPendingInsertion(portalId, typeId) {
    pendingInsertion = { portalId, typeId, formPath: location.pathname, submittedAt: null };
  }
  function markFormSubmitted() {
    if (!pendingInsertion) return;
    if (location.pathname !== pendingInsertion.formPath) return;
    pendingInsertion.submittedAt = Date.now();
    slog("history: form submitted", { formPath: pendingInsertion.formPath });
  }
  function detectCreatedTicket(pathname) {
    const m = pathname.match(TICKET_PATH_RE);
    return m ? { portalId: m[1], ticketKey: m[2] } : null;
  }
  function waitForHeading() {
    return new Promise((resolve) => {
      const existing = document.querySelector(TITLE_SELECTOR);
      if (existing) {
        resolve(existing.textContent.trim());
        return;
      }
      const timer = setTimeout(() => {
        observer.disconnect();
        swarn("history: no heading on a ticket page", { selector: TITLE_SELECTOR });
        resolve(null);
      }, TITLE_WAIT_MS);
      const observer = new MutationObserver(() => {
        const el = document.querySelector(TITLE_SELECTOR);
        if (!el) return;
        clearTimeout(timer);
        observer.disconnect();
        resolve(el.textContent.trim());
      });
      observer.observe(document.body, { childList: true, subtree: true });
    });
  }
  function finalizeHistoryIfCreated() {
    if (!pendingInsertion) return;
    if (!pendingInsertion.submittedAt) {
      pendingInsertion = null;
      return;
    }
    if (Date.now() - pendingInsertion.submittedAt > SUBMIT_TTL_MS) {
      pendingInsertion = null;
      return;
    }
    if (!isTicketAllowed(pendingInsertion.formPath)) {
      pendingInsertion = null;
      return;
    }
    const created = detectCreatedTicket(location.pathname);
    if (!created || created.portalId !== pendingInsertion.portalId) {
      pendingInsertion = null;
      return;
    }
    slog("history: ticket created", created);
    const pending = pendingInsertion;
    pendingInsertion = null;
    waitForHeading().then((headingText) => {
      const cleanTitle = (headingText || document.title.replace(/\s*-\s*Jira.*$/i, "").trim()).slice(0, 120);
      browser.storage.local.get([HISTORY_KEY], (r) => {
        const list = r && r[HISTORY_KEY] || [];
        if (list.some((it) => it.id === created.ticketKey)) return;
        const entry = {
          id: created.ticketKey,
          portalId: pending.portalId,
          typeId: pending.typeId,
          title: cleanTitle || created.ticketKey,
          url: location.href,
          when: Date.now()
        };
        const updated = [entry, ...list].slice(0, 20);
        browser.storage.local.set({ [HISTORY_KEY]: updated });
        slog("history: saved", entry);
      });
    });
  }

  // content/lib/url_watcher.js
  var lastPathname = location.pathname;
  function checkUrlChange() {
    if (location.pathname === lastPathname) return;
    const prev = lastPathname;
    lastPathname = location.pathname;
    slog("url change", { from: prev, to: lastPathname });
    finalizeHistoryIfCreated();
  }
  function setupUrlWatcher() {
    setInterval(checkUrlChange, URL_TICK_MS);
    window.addEventListener("popstate", checkUrlChange);
  }

  // content/lib/submit_watcher.js
  function requestForm() {
    const editor = document.querySelector(EDITOR_SELECTOR);
    return editor ? editor.closest("form") : null;
  }
  function sendControlFor(node) {
    if (!(node instanceof Element)) return null;
    const control = node.closest(SUBMIT_CONTROL_SELECTOR);
    if (!control) return null;
    const form = requestForm();
    return form && form.contains(control) ? control : null;
  }
  function checkSubmitEvent(event) {
    if (!isTicketAllowed(location.pathname)) return;
    if (event.target !== requestForm()) return;
    if (event.submitter && !sendControlFor(event.submitter)) return;
    slog("form submit event");
    markFormSubmitted();
  }
  function checkSubmitClick(event) {
    if (!isTicketAllowed(location.pathname)) return;
    if (!sendControlFor(event.target)) return;
    slog("submit control clicked");
    markFormSubmitted();
  }
  function setupSubmitWatcher() {
    document.addEventListener("submit", checkSubmitEvent, true);
    document.addEventListener("click", checkSubmitClick, true);
  }

  // shared/sysinfo_payload.js
  var BOOT_TIME_KEY = "lastBootTime";
  var LEGACY_BOOT_TIME_KEY = "uptime";
  function normalizeSysInfo(data) {
    const raw = data || {};
    return {
      hostname: raw.hostname,
      username: raw.username,
      ip: raw.ip,
      // ?? and not ||: the legacy name stands in for an absent key alone, so an
      // agent that sends the new key empty is not read as one predating it.
      lastBootTime: raw[BOOT_TIME_KEY] ?? raw[LEGACY_BOOT_TIME_KEY]
    };
  }

  // content/lib/sysinfo.js
  var orFallback = (value, fallbackKey) => value || t(fallbackKey);
  function buildSysInfoLines(data) {
    const raw = [
      `${t("sysinfoHostname")}: ${orFallback(data.hostname, "sysinfoUnavailable")}`,
      `${t("sysinfoUsername")}: ${orFallback(data.username, "sysinfoUnavailable")}`,
      `${t("sysinfoIP")}: ${orFallback(data.ip, "sysinfoNoIp")}`,
      `${t("sysinfoLastBootTime")}: ${orFallback(data.lastBootTime, "sysinfoUnavailable")}`
    ];
    const result = [];
    for (let i = 0; i < raw.length; i += 2) {
      result.push(raw.slice(i, i + 2).join(", "));
    }
    return result;
  }
  function makeDivider(lines, char = "\u2500", percent = 0.45) {
    const maxLen = Math.max(...lines.map((l) => l.length));
    return char.repeat(Math.floor(maxLen * percent));
  }
  function alreadyInserted(target, divider) {
    return (target.innerText || "").includes(divider);
  }
  function requestSysInfo(callback, retriesLeft = SYSINFO_REQUEST_RETRIES) {
    const attempt = SYSINFO_REQUEST_RETRIES - retriesLeft + 1;
    const tStart = Date.now();
    const retry = (reason) => {
      swarn(
        "sysinfo fetch failed",
        `(attempt ${attempt}/${SYSINFO_REQUEST_RETRIES})`,
        "reason=",
        reason,
        "elapsed=",
        Date.now() - tStart,
        "ms"
      );
      if (retriesLeft > 0) {
        setTimeout(() => requestSysInfo(callback, retriesLeft - 1), SYSINFO_REQUEST_RETRY_MS);
      } else {
        swarn("sysinfo fetch: giving up after all retries");
        callback(null);
      }
    };
    try {
      browser.runtime.sendMessage({ action: "getSystemInfo" }, (res) => {
        if (browser.runtime.lastError) {
          retry(browser.runtime.lastError.message);
          return;
        }
        if (!res || !res.success) {
          retry(res && res.error || "no response");
          return;
        }
        slog("sysinfo received", "(elapsed=", Date.now() - tStart, "ms)", res.data);
        callback(res.data ? normalizeSysInfo(res.data) : null);
      });
    } catch (e) {
      retry(e && e.message);
    }
  }

  // content/lib/toast.js
  var SVG_NS = "http://www.w3.org/2000/svg";
  function svgEl(name, attrs) {
    const el = document.createElementNS(SVG_NS, name);
    for (const k in attrs) el.setAttribute(k, attrs[k]);
    return el;
  }
  function buildCheckIcon() {
    const svg = svgEl("svg", {
      width: "11",
      height: "11",
      viewBox: "0 0 24 24",
      fill: "none",
      stroke: "currentColor",
      "stroke-width": "3",
      "stroke-linecap": "round",
      "stroke-linejoin": "round"
    });
    svg.appendChild(svgEl("polyline", { points: "20 6 9 17 4 12" }));
    return svg;
  }
  function buildAlertIcon() {
    const svg = svgEl("svg", {
      width: "11",
      height: "11",
      viewBox: "0 0 24 24",
      fill: "none",
      stroke: "currentColor",
      "stroke-width": "3",
      "stroke-linecap": "round",
      "stroke-linejoin": "round"
    });
    svg.appendChild(svgEl("line", { x1: "12", y1: "6", x2: "12", y2: "13" }));
    svg.appendChild(svgEl("line", { x1: "12", y1: "18", x2: "12", y2: "18" }));
    return svg;
  }
  function buildCloseIcon() {
    const svg = svgEl("svg", {
      width: "14",
      height: "14",
      viewBox: "0 0 24 24",
      fill: "none",
      stroke: "currentColor",
      "stroke-width": "2",
      "stroke-linecap": "round",
      "stroke-linejoin": "round"
    });
    svg.appendChild(svgEl("line", { x1: "18", y1: "6", x2: "6", y2: "18" }));
    svg.appendChild(svgEl("line", { x1: "6", y1: "6", x2: "18", y2: "18" }));
    return svg;
  }
  function ensureToastContainer() {
    let container = document.getElementById("sysinfo-toast-container");
    if (container) return container;
    container = document.createElement("div");
    container.id = "sysinfo-toast-container";
    document.documentElement.appendChild(container);
    return container;
  }
  var KINDS = {
    success: { titleKey: "bannerTitle", buildIcon: buildCheckIcon },
    error: { titleKey: "toastErrorTitle", buildIcon: buildAlertIcon }
  };
  function showToast(message, duration = 3e3, kind = "success") {
    const container = ensureToastContainer();
    const kindName = KINDS[kind] ? kind : "success";
    const spec = KINDS[kindName];
    const toast = document.createElement("div");
    toast.className = `sysinfo-toast sysinfo-toast--${kindName}`;
    const iconWrap = document.createElement("div");
    iconWrap.className = "sysinfo-toast__icon";
    iconWrap.setAttribute("aria-hidden", "true");
    iconWrap.appendChild(spec.buildIcon());
    const textWrap = document.createElement("div");
    textWrap.className = "sysinfo-toast__text";
    const title = document.createElement("div");
    title.className = "sysinfo-toast__title";
    title.textContent = t(spec.titleKey);
    const body = document.createElement("div");
    body.className = "sysinfo-toast__body";
    body.textContent = message;
    textWrap.appendChild(title);
    textWrap.appendChild(body);
    const close = document.createElement("button");
    close.className = "sysinfo-toast__close";
    close.type = "button";
    close.setAttribute("aria-label", "Close");
    close.appendChild(buildCloseIcon());
    close.addEventListener("click", () => removeToast(toast));
    toast.appendChild(iconWrap);
    toast.appendChild(textWrap);
    toast.appendChild(close);
    container.appendChild(toast);
    requestAnimationFrame(() => toast.classList.add("show"));
    if (duration > 0) setTimeout(() => removeToast(toast), duration);
    return () => removeToast(toast);
  }
  function removeToast(toast) {
    if (!toast.isConnected) return;
    toast.classList.remove("show");
    toast.classList.add("sysinfo-toast--fading");
    const cleanup = () => toast.remove();
    toast.addEventListener("transitionend", cleanup, { once: true });
    setTimeout(cleanup, 600);
  }

  // content/lib/insertion.js
  function insertSysInfoInto(target, data) {
    const path = location.pathname;
    const formMatch = path.match(FORM_PATH_RE);
    if (!formMatch || !isTicketAllowed(path)) return;
    const lines = buildSysInfoLines(data);
    const divider = makeDivider(lines);
    const indents = "\n\u200B\n\u200B\n\u200B\n";
    const text = `${indents}${divider}
${lines.join("\n")}`;
    if (alreadyInserted(target, divider)) return;
    slog("inserting sysinfo block", { path, target: target.tagName });
    target.innerText = text;
    showToast(t("toastReceived"), 1e4);
    markPendingInsertion(formMatch[1], formMatch[2]);
  }
  function whitelistedFormPath() {
    const path = location.pathname;
    return FORM_PATH_RE.test(path) && isTicketAllowed(path) ? path : null;
  }
  function watchEditor(data) {
    let lastElement = null;
    let armedForm = null;
    let deadline = 0;
    let rootSeenAt = 0;
    let dismissWaitReport = null;
    const reported = /* @__PURE__ */ new Set();
    const check = () => {
      const formPath = whitelistedFormPath();
      if (formPath !== armedForm) {
        armedForm = formPath;
        deadline = Date.now() + EDITOR_WAIT_MS;
        rootSeenAt = 0;
      }
      const el = document.querySelector(EDITOR_SELECTOR);
      if (el) {
        if (el === lastElement) return;
        lastElement = el;
        if (dismissWaitReport) {
          dismissWaitReport();
          dismissWaitReport = null;
        }
        if (data) {
          insertSysInfoInto(el, data);
        } else if (formPath && !reported.has(formPath)) {
          reported.add(formPath);
          swarn("insertion: agent unreachable on a whitelisted form", { path: formPath });
          showToast(t("toastAgentUnreachable"), 0, "error");
        }
        return;
      }
      if (!formPath || reported.has(formPath)) return;
      const rootPresent = !!document.querySelector(EDITOR_ROOT_SELECTOR);
      if (!rootPresent) rootSeenAt = 0;
      else if (!rootSeenAt) rootSeenAt = Date.now();
      const due = rootSeenAt ? Math.min(deadline, rootSeenAt + EDITOR_ROOT_GRACE_MS) : deadline;
      if (Date.now() < due) return;
      reported.add(formPath);
      swarn(
        "insertion: no editor on a whitelisted form",
        { path: formPath, selector: EDITOR_SELECTOR, rootPresent, waitedMs: Date.now() - (deadline - EDITOR_WAIT_MS) }
      );
      dismissWaitReport = showToast(t("toastEditorMissing"), 0, "error");
    };
    slog("editor watcher armed", { selector: EDITOR_SELECTOR, intervalMs: INSERTION_TICK_MS });
    setInterval(check, INSERTION_TICK_MS);
    new MutationObserver(check).observe(document.body, { childList: true, subtree: true });
    check();
  }
  function startInsertion() {
    requestSysInfo((data) => {
      if (!data) swarn("insertion: no data \u2014 watcher reports instead of inserting");
      watchEditor(data);
    });
  }

  // content/content_script.js
  slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });
  loadPortals().then(() => {
    setupUrlWatcher();
    setupSubmitWatcher();
    startInsertion();
  });
})();
