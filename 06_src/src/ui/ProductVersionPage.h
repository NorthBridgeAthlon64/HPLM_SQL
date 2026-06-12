#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class ProductVersionPage : public QWidget {
    Q_OBJECT
public:
    explicit ProductVersionPage(QSqlDatabase &db, QWidget *parent = nullptr);

private slots:
    void refresh();
    void onTreeItemClicked(QTreeWidgetItem *item, int col);
    void onNewVersion();
    void onAddBOM();
    void onEditBOM();
    void onDeleteBOM();
    void onImportBOM();

private:
    void loadBOM(int versionId);
    QSqlDatabase &m_db;
    QSplitter *m_splitter;
    QTreeWidget *m_tree;
    QTableWidget *m_bomTable;
    QLabel *m_versionInfoLabel;
    QLabel *m_costLabel;
    int m_selectedVersionId = 0;
};
