# DocuReader Exchange

A modern C++17 / Qt 6 PDF application inspired by the classic Adobe Acrobat Exchange workflow. This project is an independent implementation and is not affiliated with Adobe.

## Current starter features

- Qt Widgets desktop shell with File, Edit, View, Document, Tools, Window, and Help menus
- PDF opening through Qt PDF
- Multi-page viewing, fit-to-page/fit-to-width, zoom controls, and page navigation
- Navigation sidebar with rendered page thumbnails and document outline/bookmarks
- Light and dark interface themes
- Print action where the Qt PDF backend supports it
- Status bar with page count and current file

## Requirements

- CMake 3.21+
- C++17 compiler
- Qt 6.4+ with the **Widgets**, **Pdf**, and **PdfWidgets** modules

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the executable produced in `build` (on multi-configuration generators it may be under `build/Release`).

## Notes

This is the foundation for a modern Exchange-style PDF tool, not a finished feature-for-feature recreation. Search, annotations, page insertion/deletion/reordering, form workflows, and robust print/export should be added and tested as separate milestones. Qt PDF provides rendering and document navigation; advanced editing may require additional libraries and a document model.

## Project direction

Keep the visual language modern and accessible while preserving the classic application's practical document-centric workflow: a main document canvas, a navigable left pane, explicit page controls, and clear File/Document commands.
