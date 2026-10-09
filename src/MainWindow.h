#pragma once

#include <QMainWindow>
#include <QString>

class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QPdfDocument;
class QPdfBookmarkModel;
class QPdfView;
class QSpinBox;
class QTreeView;
class QToolBar;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void openDocument();
    void openDocument(const QString &fileName);
    void updateDocumentState();
    void goToPage(int page);
    void zoomIn();
    void zoomOut();
    void setFitPage();
    void setFitWidth();
    void toggleSidebar();
    void toggleDarkTheme();
    void showAbout();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createSidebar();
    void applyTheme(bool dark);
    void updateWindowTitle();
    void updateZoomLabel();

    QPdfDocument *m_document = nullptr;
    QPdfView *m_pdfView = nullptr;
    QPdfBookmarkModel *m_bookmarks = nullptr;
    QTreeView *m_outlineView = nullptr;
    QSpinBox *m_pageSpin = nullptr;
    QComboBox *m_zoomCombo = nullptr;
    QLabel *m_pageCountLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QAction *m_sidebarAction = nullptr;
    QAction *m_darkThemeAction = nullptr;
    QString m_currentFile;
    bool m_darkTheme = false;
};
