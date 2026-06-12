#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTextBrowser>
#include <QLineEdit>

class TracePage : public QWidget {
    Q_OBJECT
public:
    explicit TracePage(QSqlDatabase &db, QWidget *p = nullptr);
private slots:
    void onSearch();
private:
    QSqlDatabase &m_db;
    QLineEdit *m_inputEdit;
    QTextBrowser *m_resultBrowser;
};
