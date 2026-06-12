#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QPushButton>

class ProductionBatchPage : public QWidget {
    Q_OBJECT
public:
    explicit ProductionBatchPage(QSqlDatabase &db, QWidget *p = nullptr);
private slots:
    void refresh();
    void onCreateBatch();
    void onAddTest();
    void onBatchClicked(int row, int);
private:
    QSqlDatabase &m_db;
    QTableWidget *m_batchTable;
    QTableWidget *m_detailTable;
    QTableWidget *m_testTable;
    int m_selectedBatchId = 0;
};
