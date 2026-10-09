# Feature parity tracker

This project is an independent modern implementation inspired by the workflow of classic PDF exchange software. It does not use Adobe proprietary source code or claim official compatibility.

Status key:
- **Implemented (unverified):** present in source, not yet compiled or runtime-tested.
- **Planned:** not implemented yet.
- **Needs design:** decide implementation and dependencies before coding.

## Viewing and navigation

| Feature | Status | Notes |
|---|---|---|
| Open PDF | Implemented (unverified) | File dialog and error message |
| Multi-page PDF canvas | Implemented (unverified) | Qt PDF QPdfView |
| Page number navigation | Implemented (unverified) | Page spin box |
| Zoom in/out | Implemented (unverified) | Toolbar and shortcuts |
| Fit page / fit width | Implemented (unverified) | Zoom selector and toolbar |
| PDF outline/bookmarks | Implemented (unverified) | Navigation dock |
| Page thumbnails | Planned | Add a thumbnail view with rendered page previews |
| Search text | Implemented (unverified) | QPdfSearchModel, Enter to search |
| Search result highlighting | Implemented (unverified) | Current match and matches are highlighted by QPdfView |
| Next/previous search result | Implemented (unverified) | F3 / Shift+F3 |
| Recent documents | Planned | Persist list in QSettings |
| Multiple documents/tabs | Planned | Add a document-tab/controller layer |

## Document commands

| Feature | Status | Notes |
|---|---|---|
| Close document | Implemented (unverified) | Current document |
| Document properties | Partial | File name, size, page count only |
| Save a copy | Planned | Preserve source file and choose destination |
| Print dialog and page ranges | Planned | Check Qt print integration and platform behavior |
| Rotate pages persistently | Planned | Requires writing a modified PDF |
| Insert/delete/reorder/extract pages | Planned | Select a PDF manipulation backend |
| Combine PDFs | Planned | Must preserve page order and document integrity |

## Markup, forms, and security

| Feature | Status | Notes |
|---|---|---|
| Text notes and highlighting | Planned | Need annotation model and persistent export |
| Drawing/shape annotations | Planned | Include undo/redo |
| AcroForm field interaction | Needs design | Verify Qt PDF support or choose an additional backend |
| Password-protected PDFs | Needs design | Test encrypted PDFs and clear error paths |
| Digital signature validation | Needs design | Security-sensitive; requires a dedicated, validated library |
| Accessibility and keyboard navigation | Planned | Test screen readers, focus order, and shortcuts |

## Release quality

| Feature | Status | Notes |
|---|---|---|
| CMake build | Implemented (unverified) | CMake 3.21+, C++17, Qt 6.6+ |
| Automated unit/UI tests | Planned | Add Qt Test targets and sample PDFs |
| CI build on Windows/Linux | Planned | Build and package in CI |
| Installer/package | Planned | Bundle Qt runtime and required plugins |
| Runtime verification | Not yet done | Must compile and manually test before calling features complete |

## Next implementation order

1. Build and fix compiler/API issues.
2. Add page thumbnails.
3. Add recent documents and robust open/close error handling.
4. Add print and save-copy workflows.
5. Add page operations using a selected PDF manipulation backend.
6. Add annotation persistence and regression tests.
