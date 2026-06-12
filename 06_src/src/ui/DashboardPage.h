#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QSqlDatabase &db, QWidget *parent = nullptr);
    void refresh();

private:
    QSqlDatabase &m_db;
    QLabel *m_lowStockLabel;
    QLabel *m_pendingRepairLabel;
    QTableWidget *m_qualityTable;
};
