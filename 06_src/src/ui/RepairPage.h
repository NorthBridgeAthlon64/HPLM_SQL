#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QPushButton>

class RepairPage : public QWidget {
    Q_OBJECT
public:
    explicit RepairPage(QSqlDatabase &db, QWidget *p = nullptr);
private slots:
    void refresh();
    void onCreateRepair();
    void onAddMaterial();
    void onComplete();
    void onRepairClicked(int row, int);
private:
    QSqlDatabase &m_db;
    QTableWidget *m_repairTable;
    QTableWidget *m_matTable;
    int m_selectedRepairId = 0;
};
