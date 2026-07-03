# Documentation

API docs are generated with [Doxygen](https://www.doxygen.nl/), themed with
[Doxygen Awesome CSS](https://jothepro.github.io/doxygen-awesome-css/)
(vendored under `doxygen-awesome-css/`, pinned to release `v2.4.2`).

## Generating

Requires Doxygen installed separately (not vendored) — `sudo apt install
doxygen graphviz` (Graphviz is optional but enables the class/collaboration
diagrams; Doxygen just skips them with a warning if it's missing).

**Use Doxygen ≤ 1.15.x.** `doxygen-awesome-css v2.4.2` (the vendored theme)
is confirmed compatible only up to Doxygen 1.15.0. Newer Doxygen releases
(1.16+) have changed the treeview's generated HTML/CSS in ways the theme
doesn't know about — this isn't hypothetical, it's what happened while
setting this up: the sidebar rendered with an unstyled/empty-looking tree
under Doxygen 1.17.0, and switching to 1.15.0 fixed it immediately with no
other changes. `make doc` checks your installed Doxygen version and prints
a warning if it's newer than 1.15. Then:

```bash
make doc
```

which is a thin wrapper around `doxygen docs/Doxyfile`. Output goes to
`docs/generated/html/index.html` (gitignored — it's a build artifact,
regenerate it any time from source).

## Coverage

`docs/Doxyfile` currently scans all of `include/` and `src/` (minus the
vendored third-party code in `glad/`, `KHR/`, `stb_image/`). Only `Core/` has
actual `@brief`/`@param`/`@return` comments written so far — `App/` will show
up in the generated tree but mostly undocumented until that pass happens.
`WARN_IF_UNDOCUMENTED = YES` means running Doxygen will print exactly what's
still missing.

## Theme setup notes

The full Doxygen Awesome setup is wired up:

- CSS: `doxygen-awesome.css` + the `sidebar-only` layout variant — sidebar
  navigation, automatic light/dark mode following the OS/browser
  `prefers-color-scheme`.
- `docs/header.html` / `docs/footer.html` — custom templates (generated via
  `doxygen -w html`, then hand-edited) that inject the theme's JS
  extensions: a manual dark/light toggle button, per-snippet copy buttons,
  paragraph permalinks, and an interactive table-of-contents. Doxygen's own
  native equivalents (`PAGE_OUTLINE_PANEL`, `HTML_COPY_CLIPBOARD`) are
  turned off in the Doxyfile so they don't double up with these.
- `HAVE_DOT = YES` — class/collaboration/include diagrams via Graphviz.

**The extension `<script>` tags live in `docs/footer.html`, not
`docs/header.html` — this matters, don't move them back.** The dark-mode
toggle button is appended next to the search box (`#MSearchBox`), which is
itself created by a `$(document).ready()` callback registered from a
`<script>` tag in the *body* (`menu.js`'s `initMenu()` call). jQuery fires
`ready()` callbacks in registration order. A version of this that
registered the toggle's callback from the `<head>` ran *before* that body
script did, so `getElementById("MSearchBox")` returned `null` and the
toggle silently never appeared — the page still built fine, it just looked
identical to having no theme at all (dark background, no way to switch).
Putting the extensions in the footer (very end of `<body>`) guarantees they
register after the menu/search box already exists.

`docs/header.html`/`docs/footer.html` were generated with `doxygen -w html`
from Doxygen **1.9.8** (this project's actual installed version). If you
regenerate them from a different Doxygen version, re-add the four
`<script>` blocks before `</body>` in the footer (not the header) — see the
["Customize"/"Extensions" docs](https://github.com/jothepro/doxygen-awesome-css)
for the exact snippet, and double check the ordering reasoning above still
holds.
