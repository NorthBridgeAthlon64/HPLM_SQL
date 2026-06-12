#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct Customer {
    int     customerId = 0;
    QString customerCode;
    QString name;
    QString phone;
    QString email;
    QString address;
    QDate   registerDate;
    int     tier = 1;
};

class CustomerRepo : public QObject {
    Q_OBJECT
public:
    explicit CustomerRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<Customer> findAll();
    Customer findById(int id);
    QList<Customer> findByName(const QString &keyword);
    bool insert(const Customer &c);
    bool update(const Customer &c);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    Customer rowToCustomer(const QSqlQuery &q);
};
