#pragma once
#include <QWidget>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QComboBox>

class FeedbackPage : public QWidget {
    Q_OBJECT
public:
    explicit FeedbackPage(QSqlDatabase &db, QWidget *p = nullptr);
private slots:
    void refresh();
    void onSubmitFeedback();
    void onApprove();
    void onReject();
private:
    QSqlDatabase &m_db;
    QTableWidget *m_feedbackTable;
    QComboBox *m_filterCombo;
};
