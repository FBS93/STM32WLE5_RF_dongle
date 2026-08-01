// Enhance generated documentation with syntax highlighting, Mermaid diagrams, and persistent navigation controls.
(() => {
  "use strict";

  const treeKey = "docgen.navigation.open.";
  const sidebarScrollKey = "docgen.sidebar.scroll";
  const sidebar = document.querySelector("#sidebar");
  const resizer = document.querySelector("#sidebar-resizer");
  const root = document.documentElement;
  const widthKey = root.dataset.sidebarWidthKey;
  const minimum = Number(root.dataset.sidebarWidthMinimum);
  const maximum = Number(root.dataset.sidebarWidthMaximum);
  const session = storage("sessionStorage");
  const local = storage("localStorage");

  const highlight = globalThis.hljs;
  if (highlight) {
    document.querySelectorAll(
      ".raw-source > code, .markdown-content pre > code[class*='language-']:not(.language-mermaid)"
    ).forEach((code) => {
      highlight.highlightElement(code);
    });
  }

  function storage(name) {
    try { return window[name]; } catch (_) { return null; }
  }

  function saveSidebarScroll() {
    if (!sidebar) return;
    try { session?.setItem(sidebarScrollKey, String(sidebar.scrollTop)); } catch (_) { /* Storage is optional. */ }
  }

  document.querySelectorAll(".directory-tree[data-nav-path]").forEach((directory) => {
    const toggle = directory.querySelector(".directory-toggle");
    const children = document.getElementById(toggle?.getAttribute("aria-controls"));
    if (!toggle || !children) return;
    const key = treeKey + directory.dataset.navPath;
    toggle.addEventListener("click", () => {
      const open = toggle.getAttribute("aria-expanded") !== "true";
      toggle.setAttribute("aria-expanded", String(open));
      children.hidden = !open;
      try { session?.setItem(key, open ? "open" : "closed"); } catch (_) { /* Storage is optional. */ }
    });
  });

  if (sidebar) {
    sidebar.addEventListener("scroll", saveSidebarScroll, { passive: true });
    window.addEventListener("pagehide", saveSidebarScroll);
  }

  if (resizer) {
    function clamp(value) { return Math.min(maximum, Math.max(minimum, value)); }
    function setWidth(value) {
      const width = clamp(value);
      root.style.setProperty("--sidebar-width", `${width}px`);
      resizer.setAttribute("aria-valuenow", width);
      try { local?.setItem(widthKey, width); } catch (_) { /* Storage is optional. */ }
    }
    const initialWidth = Number.parseFloat(
      getComputedStyle(root).getPropertyValue("--sidebar-width")
    );
    resizer.setAttribute("aria-valuenow", clamp(initialWidth));
    resizer.addEventListener("pointerdown", (event) => {
      if (matchMedia("(max-width: 720px)").matches) return;
      resizer.setPointerCapture(event.pointerId);
      const move = (pointer) => setWidth(pointer.clientX);
      const finish = () => {
        resizer.removeEventListener("pointermove", move);
        resizer.removeEventListener("pointerup", finish);
        resizer.removeEventListener("pointercancel", finish);
        resizer.releasePointerCapture(event.pointerId);
      };
      resizer.addEventListener("pointermove", move);
      resizer.addEventListener("pointerup", finish, { once: true });
      resizer.addEventListener("pointercancel", finish, { once: true });
    });
    resizer.addEventListener("keydown", (event) => {
      const current = Number(resizer.getAttribute("aria-valuenow"));
      const delta = event.shiftKey ? 25 : 10;
      if (event.key === "ArrowLeft") setWidth(current - delta);
      else if (event.key === "ArrowRight") setWidth(current + delta);
      else if (event.key === "Home") setWidth(minimum);
      else if (event.key === "End") setWidth(maximum);
      else return;
      event.preventDefault();
    });
  }

  const mermaid = globalThis.mermaid;
  if (!mermaid) return;
  mermaid.initialize({ startOnLoad: false, securityLevel: "strict" });
  document.querySelectorAll(".markdown-content pre > code.language-mermaid")
    .forEach((code, index) => {
      const source = code.parentElement;
      if (!(source instanceof HTMLPreElement)) return;
      mermaid.render(`docgen-mermaid-${index}`, code.textContent || "")
        .then(({ svg }) => {
          const diagram = document.createElement("div");
          diagram.className = "mermaid-diagram";
          diagram.setAttribute("role", "region");
          diagram.setAttribute("aria-label", "Mermaid diagram");
          diagram.innerHTML = svg;
          const svgElement = diagram.querySelector("svg");
          if (!svgElement) throw new Error("Mermaid did not return an SVG");
          svgElement.setAttribute("role", "img");
          svgElement.setAttribute("aria-label", "Mermaid diagram");
          source.replaceWith(diagram);
        })
        .catch(() => { /* Keep the source block visible as the fallback. */ });
    });
})();
