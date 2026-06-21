#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct ProductFeedback {
    int     feedbackId = 0;
    int     versionId = 0;
    int     customerId = 0;
    int     rating = 0;
    QString comment;
    QDate   feedbackDate;
    QString status;
    QString customerName;
    QString versionNumber;
};

class ProductFeedbackRepo : public QObject {
    Q_OBJECT
public:
    explicit ProductFeedbackRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<ProductFeedback> findByVersion(int versionId);
    QList<ProductFeedback> findByCustomer(int customerId);
    QList<ProductFeedback> lowRated(double threshold = 3.0);
    bool insert(const ProductFeedback &f);
    bool updateStatus(int feedbackId, const QString &status);
private:
    QSqlDatabase &m_db;
    ProductFeedback rowToFeedback(const QSqlQuery &q);
};
