#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

class ComponentPage : public QWidget {
    Q_OBJECT
public:
    explicit ComponentPage(QSqlDatabase &db, QWidget *parent = nullptr);

private slots:
    void refresh();
    void onSearch();
    void onAdd();
    void onEdit();
    void onDelete();
    void onDoubleClick(int row, int col);

private:
    QSqlDatabase &m_db;
    QTableWidget *m_table;
    QLineEdit *m_searchEdit;
    QComboBox *m_supplierCombo;
};
