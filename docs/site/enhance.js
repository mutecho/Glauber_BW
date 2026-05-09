const pages = [
  { href: "index.html", label: "首页" },
  { href: "overview.html", label: "总览" },
  { href: "project.html", label: "项目说明" },
  { href: "formulas.html", label: "公式链路" },
  { href: "notes.html", label: "手记" },
];

const themes = [
  { value: "sage", label: "Sage" },
  { value: "mist", label: "Mist" },
  { value: "dawn", label: "Dawn" },
  { value: "graphite", label: "Graphite" },
  { value: "gruv", label: "Gruv" },
  { value: "dusk", label: "Dusk" },
  { value: "night", label: "Night" },
];

const pageSummaries = new Map([
  ["Blast-Wave Docs", "把运行契约、配置说明、公式链路、ROOT payload 和 QA 约束整理成一个可跳读的技术文档入口。"],
  ["Blast-Wave Overview", "仓库结构、当前文档分工、本地产物目录和协作台账入口。"],
  ["Blast-Wave 项目说明", "编译、运行、配置契约、ROOT 输出结构、QA 行为和当前能力边界。"],
  ["Blast-Wave 数学物理公式流程说明", "participant 几何、密度演化、发射抽样、热动量、流场 boost 和 flow 观测量。"],
  ["Blast-Wave 简明手记", "当前主链、默认路径、容易混淆的概念和运行提醒。"],
]);

const currentFile = location.pathname.split("/").pop() || "index.html";
const normalizeText = (value = "") => value.replace(/\s+/g, " ").trim();

// The generated pages share a small product-style nav with consistent theme controls.
(() => {
  if (document.querySelector(".site-nav")) {
    return;
  }

  const nav = document.createElement("nav");
  nav.className = "site-nav";
  nav.setAttribute("aria-label", "Site navigation");
  nav.innerHTML = `
    <a class="site-mark" href="index.html">Blast-Wave Docs</a>
    <div class="site-links">
      ${pages
        .map(
          (page) =>
            `<a href="${page.href}"${page.href === currentFile ? ' aria-current="page"' : ""}>${page.label}</a>`,
        )
        .join("")}
    </div>
    <div class="theme-switcher" aria-label="Theme">
      ${themes
        .map((theme) => `<button type="button" data-theme-choice="${theme.value}">${theme.label}</button>`)
        .join("")}
    </div>
  `;
  document.body.prepend(nav);
})();

// Theme choices keep the same restrained system while changing the reading mood.
(() => {
  const key = "blast-wave-doc-theme";
  const allowedThemes = new Set(themes.map((theme) => theme.value));
  const saved = allowedThemes.has(localStorage.getItem(key)) ? localStorage.getItem(key) : "sage";
  const applyTheme = (theme) => {
    document.documentElement.dataset.theme = theme;
    localStorage.setItem(key, theme);
    document.querySelectorAll("[data-theme-choice]").forEach((button) => {
      button.toggleAttribute("aria-pressed", button.dataset.themeChoice === theme);
    });
  };

  applyTheme(saved);
  document.querySelectorAll("[data-theme-choice]").forEach((button) => {
    button.addEventListener("click", () => applyTheme(button.dataset.themeChoice));
  });
})();

// Pandoc emits a linear body; this creates the stable main reading pane.
(() => {
  const header = document.querySelector("#title-block-header");
  const toc = document.querySelector("#TOC");
  if (!header || document.querySelector("#doc-content")) {
    return;
  }

  const title = normalizeText(header.querySelector(".title")?.textContent || document.title);
  const isHome = title === "Blast-Wave Docs";
  document.body.classList.add(isHome ? "page-home" : "page-doc");

  const main = document.createElement("main");
  main.id = "doc-content";
  main.setAttribute("aria-label", `${title} content`);

  let node = toc ? toc.nextSibling : header.nextSibling;
  while (node) {
    const next = node.nextSibling;
    main.appendChild(node);
    node = next;
  }
  document.body.appendChild(main);
})();

// The home page is a routing surface; document pages get a quieter compact hero.
(() => {
  const header = document.querySelector("#title-block-header");
  const title = header?.querySelector(".title");
  if (!header || !title || header.querySelector(".hero-layout, .doc-hero-layout")) {
    return;
  }

  const pageTitle = title.textContent.trim();
  const summary = pageSummaries.get(pageTitle) || pageSummaries.get("Blast-Wave Docs");

  if (document.body.classList.contains("page-home")) {
    title.textContent = "Blast-Wave Docs";
    header.classList.add("home-hero");
    header.innerHTML = `
      <div class="hero-layout">
        <div class="hero-copy-column">
          ${title.outerHTML}
          <p class="hero-copy">${summary}</p>
          <nav class="hero-links" aria-label="High value documentation links">
            <a class="hero-link" href="project.html">
              <strong>项目说明</strong>
              <span>编译、运行、配置、输出和能力边界。</span>
            </a>
            <a class="hero-link" href="formulas.html">
              <strong>公式链路</strong>
              <span>participant 几何、密度、发射、热动量和 flow。</span>
            </a>
            <a class="hero-link" href="project.html#root-输出文件里有什么">
              <strong>ROOT 输出</strong>
              <span>events、particles、histograms 与 analysis payload。</span>
            </a>
            <a class="hero-link" href="notes.html#最容易混淆的点">
              <strong>概念速查</strong>
              <span>当前主链、常见误解和运行提醒。</span>
            </a>
          </nav>
        </div>
        <aside class="hero-panel" aria-label="Generator pipeline overview">
          <div class="hero-panel-title">
            <span>Generator pipeline</span>
            <span>current</span>
          </div>
          <div class="pipeline-step">
            <b>01</b>
            <p><strong>Participants</strong><span>Monte Carlo Glauber or response-test point cloud.</span></p>
          </div>
          <div class="pipeline-step">
            <b>02</b>
            <p><strong>Event medium</strong><span>Density evolution, emission geometry, and flow field inputs.</span></p>
          </div>
          <div class="pipeline-step">
            <b>03</b>
            <p><strong>Emission chain</strong><span>Sites, eta_s, local momentum, velocity, and Lorentz boost.</span></p>
          </div>
          <div class="pipeline-step">
            <b>04</b>
            <p><strong>ROOT payload</strong><span>Events, particles, diagnostics, QA, and optional flow analysis.</span></p>
          </div>
        </aside>
      </div>
    `;
    return;
  }

  header.classList.add("doc-hero");
  header.innerHTML = `
    <div class="doc-hero-layout">
      ${title.outerHTML}
      <p>${summary}</p>
    </div>
  `;
})();

// Avoid repeating the same document title immediately below the compact hero.
(() => {
  const content = document.querySelector("#doc-content");
  const pageTitle = normalizeText(document.querySelector("#title-block-header .title")?.textContent);
  const first = content?.firstElementChild;
  if (first?.matches("h1") && normalizeText(first.textContent) === pageTitle) {
    first.remove();
  }
})();

// Consecutive display equations read better as one mathematical block.
(() => {
  const content = document.querySelector("#doc-content");
  if (!content) {
    return;
  }

  const isMathOnlyParagraph = (node) =>
    node?.nodeType === Node.ELEMENT_NODE &&
    node.matches("p") &&
    node.children.length === 1 &&
    node.firstElementChild.classList.contains("math") &&
    node.firstElementChild.classList.contains("display");

  const parents = new Set(
    [...content.querySelectorAll("p > .math.display:only-child")].map((span) => span.parentElement.parentElement),
  );

  parents.forEach((parent) => {
    let node = parent.firstElementChild;
    while (node) {
      if (!isMathOnlyParagraph(node)) {
        node = node.nextElementSibling;
        continue;
      }

      const run = [];
      let cursor = node;
      while (isMathOnlyParagraph(cursor)) {
        run.push(cursor);
        cursor = cursor.nextElementSibling;
      }

      if (run.length > 1) {
        const group = document.createElement("div");
        group.className = "formula-group";
        parent.insertBefore(group, run[0]);
        run.forEach((paragraph) => group.appendChild(paragraph));
      }
      node = cursor;
    }
  });
})();

// Home-page markdown is transformed into route cards instead of a long article.
(() => {
  const content = document.querySelector("#doc-content");
  if (!content || !document.body.classList.contains("page-home")) {
    return;
  }

  const firstParagraph = content.firstElementChild;
  const intro = pageSummaries.get("Blast-Wave Docs");
  if (firstParagraph?.matches("p") && normalizeText(firstParagraph.textContent) === normalizeText(intro)) {
    firstParagraph.remove();
  }

  [...content.querySelectorAll("h2")].forEach((heading) => {
    const section = document.createElement("section");
    section.className = "home-section";
    heading.parentNode.insertBefore(section, heading);
    section.appendChild(heading);

    const grid = document.createElement("div");
    grid.className = "home-card-grid";
    section.appendChild(grid);

    let node = nextContentSibling(section);
    while (node && !(node.nodeType === Node.ELEMENT_NODE && node.matches("h2"))) {
      if (node.nodeType === Node.ELEMENT_NODE && node.matches("h3")) {
        const card = document.createElement("article");
        card.className = "home-card";
        card.appendChild(node);
        let candidate = nextContentSibling(section);
        while (isCardBodyNode(candidate)) {
          card.appendChild(candidate);
          candidate = nextContentSibling(section);
        }
        grid.appendChild(card);
        node = candidate;
      } else if (node.textContent?.trim()) {
        const next = node.nextSibling;
        section.appendChild(node);
        node = next;
      } else {
        const next = node.nextSibling;
        node.remove();
        node = next;
      }
    }
  });

  function nextContentSibling(anchor) {
    let candidate = anchor.nextSibling;
    while (candidate && candidate.nodeType === Node.TEXT_NODE && !candidate.textContent.trim()) {
      const next = candidate.nextSibling;
      candidate.remove();
      candidate = next;
    }
    return candidate;
  }

  function isCardBodyNode(candidate) {
    return candidate?.nodeType === Node.ELEMENT_NODE && !candidate.matches("h2, h3");
  }
})();

// Active-section tracking keeps each long document table of contents navigable.
(() => {
  const toc = document.querySelector("#TOC");
  const content = document.querySelector("#doc-content");
  if (!toc || !content || !("IntersectionObserver" in window)) {
    return;
  }

  const links = new Map();
  toc.querySelectorAll('a[href^="#"]').forEach((link) => {
    const id = decodeURIComponent(link.getAttribute("href").slice(1));
    links.set(id, link);
  });

  const headings = [...content.querySelectorAll("h1, h2, h3")].filter((heading) =>
    links.has(heading.id),
  );
  if (headings.length === 0) {
    return;
  }

  const setActive = (id) => {
    links.forEach((link) => link.classList.remove("is-active"));
    const activeLink = links.get(id);
    if (activeLink) {
      activeLink.classList.add("is-active");
    }
  };

  const observer = new IntersectionObserver(
    (entries) => {
      const visible = entries
        .filter((entry) => entry.isIntersecting)
        .sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);
      if (visible[0]?.target.id) {
        setActive(visible[0].target.id);
      }
    },
    { rootMargin: "-15% 0px -70% 0px", threshold: 0.01 },
  );

  headings.forEach((heading) => observer.observe(heading));
})();
