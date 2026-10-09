#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfSearchModel>
#include <QPdfView>
#include <QPixmap>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeView>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_document(new QPdfDocument(this)),
      m_pdfView(new QPdfView(this)),
      m_bookmarks(new QPdfBookmarkModel(this)),
      m_searchModel(new QPdfSearchModel(this))
{
    setMinimumSize(1000, 680);
    resize(1380, 900);
    setDockNestingEnabled(true);

    m_pdfView->setDocument(m_document);
    m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
    m_pdfView->setSearchModel(m_searchModel);
    setCentralWidget(m_pdfView);

    m_bookmarks->setDocument(m_document);
    m_searchModel->setDocument(m_document);
    createActions();
    createMenus();
    createToolBars();
    createSidebar();

    m_pageSpin = new QSpinBox(this);
    m_pageSpin->setMinimum(1);
    m_pageSpin->setMaximum(1);
    m_pageSpin->setPrefix(tr("Page "));
    m_pageSpin->setToolTip(tr("Go to page"));
    connect(m_pageSpin, &QSpinBox::valueChanged, this, &MainWindow::goToPage);

    m_pageCountLabel = new QLabel(tr("of 0"), this);
    m_zoomCombo = new QComboBox(this);
    m_zoomCombo->setEditable(true);
    m_zoomCombo->addItems({tr("Fit page"), tr("Fit width"), QStringLiteral("50%"),
                           QStringLiteral("75%"), QStringLiteral("100%"),
                           QStringLiteral("125%"), QStringLiteral("150%"),
                           QStringLiteral("200%")});
    m_zoomCombo->setCurrentText(tr("Fit page"));
    m_zoomCombo->setToolTip(tr("Zoom level"));
    connect(m_zoomCombo, &QComboBox::currentTextChanged, this, [this](const QString &value) {
        if (value == tr("Fit page")) {
            m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
        } else if (value == tr("Fit width")) {
            m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
        } else {
            QString numeric = value;
            numeric.remove(QLatin1Char('%'));
            bool ok = false;
            const qreal factor = numeric.toDouble(&ok) / 100.0;
            if (ok && factor > 0.05 && factor < 10.0) {
                m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
                m_pdfView->setZoomFactor(factor);
            }
        }
    });

    auto *navigationBar = addToolBar(tr("Document navigation"));
    navigationBar->setObjectName(QStringLiteral("DocumentNavigation"));
    navigationBar->setMovable(false);
    navigationBar->addWidget(new QLabel(QStringLiteral("  "), this));
    navigationBar->addWidget(m_pageSpin);
    navigationBar->addWidget(m_pageCountLabel);
    navigationBar->addSeparator();
    navigationBar->addWidget(new QLabel(tr("Zoom "), this));
    navigationBar->addWidget(m_zoomCombo);

    m_statusLabel = new QLabel(tr("Open a PDF to get started"), this);
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->setSizeGripEnabled(true);

    connect(m_document, &QPdfDocument::statusChanged, this, &MainWindow::updateDocumentState);
    connect(m_document, &QPdfDocument::pageCountChanged, this, [this](int) {
        updateDocumentState();
        refreshThumbnails();
    });
    connect(m_searchModel, &QPdfSearchModel::countChanged, this, &MainWindow::updateSearchStatus);
    connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged,
            this, [this](int page) {
        if (page >= 0 && m_pageSpin) {
            m_pageSpin->blockSignals(true);
            m_pageSpin->setValue(page + 1);
            m_pageSpin->blockSignals(false);
            syncThumbnailSelection(page);
        }
    });

    applyTheme(false);
    updateDocumentState();
}

void MainWindow::createActions()
{
    auto *openAction = new QAction(tr("&Open PDF…"), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openDocument);

    auto *closeAction = new QAction(tr("Close &Document"), this);
    closeAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+W")));
    connect(closeAction, &QAction::triggered, this, [this] {
        m_document->close();
        m_currentFile.clear();
        m_searchModel->setSearchString(QString());
        m_searchIndex = -1;
        refreshThumbnails();
        updateDocumentState();
    });

    auto *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::closeAllWindows);

    auto *zoomInAction = new QAction(tr("Zoom &In"), this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

    auto *zoomOutAction = new QAction(tr("Zoom &Out"), this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

    auto *fitPageAction = new QAction(tr("Fit &Page"), this);
    connect(fitPageAction, &QAction::triggered, this, &MainWindow::setFitPage);

    auto *fitWidthAction = new QAction(tr("Fit Page &Width"), this);
    connect(fitWidthAction, &QAction::triggered, this, &MainWindow::setFitWidth);

    m_sidebarAction = new QAction(tr("Navigation &Pane"), this);
    m_sidebarAction->setCheckable(true);
    m_sidebarAction->setChecked(true);

    m_darkThemeAction = new QAction(tr("&Dark Theme"), this);
    m_darkThemeAction->setCheckable(true);
    connect(m_darkThemeAction, &QAction::toggled, this, &MainWindow::applyTheme);

    auto *aboutAction = new QAction(tr("&About DocuReader Exchange"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    setProperty("actionOpen", QVariant::fromValue(static_cast<QObject *>(openAction)));
    setProperty("actionClose", QVariant::fromValue(static_cast<QObject *>(closeAction)));
    setProperty("actionExit", QVariant::fromValue(static_cast<QObject *>(exitAction)));
    setProperty("actionZoomIn", QVariant::fromValue(static_cast<QObject *>(zoomInAction)));
    setProperty("actionZoomOut", QVariant::fromValue(static_cast<QObject *>(zoomOutAction)));
    setProperty("actionFitPage", QVariant::fromValue(static_cast<QObject *>(fitPageAction)));
    setProperty("actionFitWidth", QVariant::fromValue(static_cast<QObject *>(fitWidthAction)));
    setProperty("actionAbout", QVariant::fromValue(static_cast<QObject *>(aboutAction)));
}

void MainWindow::createMenus()
{
    auto action = [this](const char *name) {
        return qobject_cast<QAction *>(property(name).value<QObject *>());
    };

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(action("actionOpen"));
    fileMenu->addAction(action("actionClose"));
    fileMenu->addSeparator();
    fileMenu->addAction(action("actionExit"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Find…"), this, [this] {
        if (m_searchEdit) {
            m_searchEdit->setEnabled(m_document->pageCount() > 0);
            m_searchEdit->setFocus();
            m_searchEdit->selectAll();
        }
    }, QKeySequence::Find);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_sidebarAction);
    viewMenu->addSeparator();
    viewMenu->addAction(action("actionZoomIn"));
    viewMenu->addAction(action("actionZoomOut"));
    viewMenu->addAction(action("actionFitPage"));
    viewMenu->addAction(action("actionFitWidth"));
    viewMenu->addSeparator();
    viewMenu->addAction(m_darkThemeAction);

    QMenu *documentMenu = menuBar()->addMenu(tr("&Document"));
    documentMenu->addAction(tr("Next Page"), this, [this] {
        if (m_document->pageCount() > 0)
            goToPage(qMin(m_document->pageCount(), m_pdfView->pageNavigator()->currentPage() + 2));
    }, QKeySequence::MoveToNextPage);
    documentMenu->addAction(tr("Previous Page"), this, [this] {
        if (m_document->pageCount() > 0)
            goToPage(qMax(1, m_pdfView->pageNavigator()->currentPage()));
    }, QKeySequence::MoveToPreviousPage);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("Document Properties…"), this, [this] {
        if (m_currentFile.isEmpty()) {
            QMessageBox::information(this, tr("Document Properties"), tr("No document is open."));
            return;
        }
        const QFileInfo info(m_currentFile);
        QMessageBox::information(this, tr("Document Properties"),
            tr("File: %1\nSize: %2 KB\nPages: %3")
                .arg(info.fileName()).arg(info.size() / 1024).arg(m_document->pageCount()));
    });

    QMenu *windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(tr("Next Search Result"), this, &MainWindow::nextSearchResult,
                          QKeySequence(QStringLiteral("F3")));
    windowMenu->addAction(tr("Previous Search Result"), this, &MainWindow::previousSearchResult,
                          QKeySequence(QStringLiteral("Shift+F3")));
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(action("actionAbout"));
}

void MainWindow::createToolBars()
{
    auto action = [this](const char *name) {
        return qobject_cast<QAction *>(property(name).value<QObject *>());
    };
    QToolBar *toolbar = addToolBar(tr("Main"));
    toolbar->setObjectName(QStringLiteral("MainToolbar"));
    toolbar->setMovable(false);
    toolbar->addAction(action("actionOpen"));
    toolbar->addSeparator();
    toolbar->addAction(action("actionZoomOut"));
    toolbar->addAction(action("actionZoomIn"));
    toolbar->addAction(action("actionFitPage"));
    toolbar->addAction(action("actionFitWidth"));
    toolbar->addSeparator();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Find text in PDF…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaximumWidth(280);
    m_searchEdit->setEnabled(false);
    m_searchEdit->setToolTip(tr("Enter text and press Enter to search the open PDF."));
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::performSearch);
    toolbar->addWidget(m_searchEdit);

    auto *next = new QAction(tr("Next Match"), this);
    next->setShortcut(QKeySequence(QStringLiteral("F3")));
    connect(next, &QAction::triggered, this, &MainWindow::nextSearchResult);
    toolbar->addAction(next);
    auto *previous = new QAction(tr("Previous Match"), this);
    previous->setShortcut(QKeySequence(QStringLiteral("Shift+F3")));
    connect(previous, &QAction::triggered, this, &MainWindow::previousSearchResult);
    toolbar->addAction(previous);
}

void MainWindow::createSidebar()
{
    auto *dock = new QDockWidget(tr("Navigation"), this);
    dock->setObjectName(QStringLiteral("NavigationDock"));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    m_navigationTabs = new QTabWidget(dock);
    m_thumbnailList = new QListWidget(m_navigationTabs);
    m_thumbnailList->setViewMode(QListView::IconMode);
    m_thumbnailList->setIconSize(QSize(104, 136));
    m_thumbnailList->setResizeMode(QListView::Adjust);
    m_thumbnailList->setMovement(QListView::Static);
    m_thumbnailList->setWrapping(false);
    m_thumbnailList->setSpacing(8);
    m_thumbnailList->setWordWrap(true);
    m_thumbnailList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_navigationTabs->addTab(m_thumbnailList, tr("Thumbnails"));

    m_outlineView = new QTreeView(m_navigationTabs);
    m_outlineView->setModel(m_bookmarks);
    m_outlineView->setHeaderHidden(true);
    m_outlineView->setAlternatingRowColors(true);
    m_outlineView->setUniformRowHeights(true);
    m_navigationTabs->addTab(m_outlineView, tr("Bookmarks"));

    dock->setWidget(m_navigationTabs);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(m_thumbnailList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < m_document->pageCount())
            m_pdfView->pageNavigator()->jump(row);
    });
    connect(m_outlineView, &QTreeView::activated, this, [this](const QModelIndex &index) {
        const int page = m_bookmarks->data(index, QPdfBookmarkModel::Role::Page).toInt();
        if (page >= 0)
            m_pdfView->pageNavigator()->jump(page);
    });
    connect(m_sidebarAction, &QAction::toggled, dock, &QDockWidget::setVisible);
    connect(dock, &QDockWidget::visibilityChanged, m_sidebarAction, &QAction::setChecked);
}

void MainWindow::openDocument()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open PDF"), QString(), tr("PDF documents (*.pdf);;All files (*)"));
    if (!fileName.isEmpty())
        openDocument(fileName);
}

void MainWindow::openDocument(const QString &fileName)
{
    const QPdfDocument::Error error = m_document->load(fileName);
    if (error != QPdfDocument::Error::None) {
        QMessageBox::warning(this, tr("Unable to Open PDF"),
                             tr("The document could not be opened. It may be damaged, encrypted, or unsupported."));
        return;
    }
    m_currentFile = fileName;
    if (m_searchEdit)
        m_searchEdit->setEnabled(true);
    m_searchModel->setSearchString(QString());
    m_searchIndex = -1;
    m_pdfView->setCurrentSearchResultIndex(-1);
    m_pdfView->pageNavigator()->jump(0);
    refreshThumbnails();
    updateDocumentState();
}

void MainWindow::updateDocumentState()
{
    const bool hasDocument = m_document->pageCount() > 0;
    if (m_pageSpin) {
        m_pageSpin->setEnabled(hasDocument);
        m_pageSpin->setMaximum(qMax(1, m_document->pageCount()));
        m_pageSpin->setValue(hasDocument ? m_pdfView->pageNavigator()->currentPage() + 1 : 1);
    }
    if (m_pageCountLabel)
        m_pageCountLabel->setText(tr("of %1").arg(m_document->pageCount()));
    if (m_searchEdit)
        m_searchEdit->setEnabled(hasDocument);
    if (m_statusLabel) {
        m_statusLabel->setText(hasDocument
            ? tr("%1 pages  •  %2").arg(m_document->pageCount()).arg(QFileInfo(m_currentFile).fileName())
            : tr("Open a PDF to get started"));
    }
    updateWindowTitle();
}

void MainWindow::goToPage(int page)
{
    if (m_document->pageCount() > 0)
        m_pdfView->pageNavigator()->jump(qBound(0, page - 1, m_document->pageCount() - 1));
}

void MainWindow::zoomIn()
{
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(qMin(5.0, m_pdfView->zoomFactor() * 1.15));
    updateZoomLabel();
}

void MainWindow::zoomOut()
{
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(qMax(0.1, m_pdfView->zoomFactor() / 1.15));
    updateZoomLabel();
}

void MainWindow::setFitPage()
{
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
    if (m_zoomCombo)
        m_zoomCombo->setCurrentText(tr("Fit page"));
}

void MainWindow::setFitWidth()
{
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
    if (m_zoomCombo)
        m_zoomCombo->setCurrentText(tr("Fit width"));
}

void MainWindow::toggleSidebar()
{
    if (auto *dock = findChild<QDockWidget *>(QStringLiteral("NavigationDock")))
        dock->setVisible(m_sidebarAction && m_sidebarAction->isChecked());
}

void MainWindow::toggleDarkTheme()
{
    applyTheme(!m_darkTheme);
}

void MainWindow::applyTheme(bool dark)
{
    m_darkTheme = dark;
    if (m_darkThemeAction && m_darkThemeAction->isChecked() != dark) {
        m_darkThemeAction->blockSignals(true);
        m_darkThemeAction->setChecked(dark);
        m_darkThemeAction->blockSignals(false);
    }
    if (dark) {
        qApp->setStyleSheet(QStringLiteral(
            "QMainWindow, QMenuBar, QMenu, QToolBar, QStatusBar, QDockWidget { background:#20242b; color:#e8eaed; }"
            "QMenuBar::item:selected, QMenu::item:selected { background:#354052; }"
            "QLineEdit, QSpinBox, QComboBox, QTreeView, QListWidget, QTabWidget::pane { background:#292f38; color:#e8eaed; border:1px solid #424a57; padding:4px; }"
            "QToolBar { spacing:6px; border:0; padding:5px; }"
            "QToolButton { padding:6px; border-radius:4px; }"
            "QToolButton:hover { background:#354052; }"));
    } else {
        qApp->setStyleSheet(QStringLiteral(
            "QToolBar { spacing:6px; border:0; padding:5px; }"
            "QToolButton { padding:6px; border-radius:4px; }"
            "QToolButton:hover { background:#e8eef7; }"
            "QTreeView, QListWidget { alternate-background-color:#f5f7fa; }"));
    }
}

void MainWindow::updateWindowTitle()
{
    const QString name = m_currentFile.isEmpty() ? tr("Untitled") : QFileInfo(m_currentFile).fileName();
    setWindowTitle(tr("%1 — DocuReader Exchange").arg(name));
}

void MainWindow::updateZoomLabel()
{
    if (m_zoomCombo)
        m_zoomCombo->setCurrentText(QStringLiteral("%1%").arg(qRound(m_pdfView->zoomFactor() * 100)));
}

void MainWindow::performSearch()
{
    if (!m_searchEdit || m_document->pageCount() == 0)
        return;
    const QString query = m_searchEdit->text().trimmed();
    m_searchModel->setSearchString(query);
    m_searchIndex = -1;
    m_pdfView->setCurrentSearchResultIndex(-1);
    updateSearchStatus();
    if (!query.isEmpty())
        nextSearchResult();
}

void MainWindow::nextSearchResult()
{
    const int count = m_searchModel->rowCount();
    if (count <= 0) {
        updateSearchStatus();
        return;
    }
    m_searchIndex = (m_searchIndex + 1) % count;
    m_pdfView->setCurrentSearchResultIndex(m_searchIndex);
    const int page = m_searchModel->data(m_searchModel->index(m_searchIndex, 0),
                                         QPdfSearchModel::Role::Page).toInt();
    if (page >= 0)
        m_pdfView->pageNavigator()->jump(page);
    updateSearchStatus();
}

void MainWindow::previousSearchResult()
{
    const int count = m_searchModel->rowCount();
    if (count <= 0) {
        updateSearchStatus();
        return;
    }
    m_searchIndex = (m_searchIndex <= 0) ? count - 1 : m_searchIndex - 1;
    m_pdfView->setCurrentSearchResultIndex(m_searchIndex);
    const int page = m_searchModel->data(m_searchModel->index(m_searchIndex, 0),
                                         QPdfSearchModel::Role::Page).toInt();
    if (page >= 0)
        m_pdfView->pageNavigator()->jump(page);
    updateSearchStatus();
}

void MainWindow::updateSearchStatus()
{
    if (!m_statusLabel)
        return;
    const int count = m_searchModel->rowCount();
    if (m_searchModel->searchString().isEmpty()) {
        updateDocumentState();
        return;
    }
    if (count == 0)
        m_statusLabel->setText(tr("No matches for ‘%1’").arg(m_searchModel->searchString()));
    else
        m_statusLabel->setText(tr("Match %1 of %2 for ‘%3’")
            .arg(m_searchIndex + 1).arg(count).arg(m_searchModel->searchString()));
}

void MainWindow::refreshThumbnails()
{
    if (!m_thumbnailList)
        return;

    m_thumbnailList->blockSignals(true);
    m_thumbnailList->clear();
    const int count = m_document->pageCount();
    for (int page = 0; page < count; ++page) {
        auto *item = new QListWidgetItem(tr("Page %1").arg(page + 1));
        item->setData(Qt::UserRole, page);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(128, 174));

        const QImage image = m_document->render(page, QSize(104, 136));
        if (!image.isNull())
            item->setIcon(QIcon(QPixmap::fromImage(image)));
        else
            item->setText(tr("Page %1\nPreview unavailable").arg(page + 1));
        m_thumbnailList->addItem(item);
    }
    m_thumbnailList->blockSignals(false);
    syncThumbnailSelection(m_pdfView->pageNavigator()->currentPage());
}

void MainWindow::syncThumbnailSelection(int page)
{
    if (!m_thumbnailList || page < 0 || page >= m_thumbnailList->count())
        return;
    m_thumbnailList->blockSignals(true);
    m_thumbnailList->setCurrentRow(page);
    m_thumbnailList->blockSignals(false);
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About DocuReader Exchange"),
        tr("<h3>DocuReader Exchange</h3>"
           "<p>A modern PDF reader inspired by classic document-exchange applications.</p>"
           "<p>Built with C++ and Qt 6.</p>"
           "<p>This is an independent project and is not affiliated with Adobe.</p>"));
}
