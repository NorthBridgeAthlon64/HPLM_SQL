#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct Supplier {
    int     supplierId = 0;
    QString supplierCode;
    QString name;
    QString contactPerson;
    QString phone;
    QString address;
    int     rating = 3;
};

class SupplierRepo : public QObject {
    Q_OBJECT
public:
    explicit SupplierRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<Supplier> findAll();
    Supplier findById(int id);
    bool insert(const Supplier &s);
    bool update(const Supplier &s);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    Supplier rowToSupplier(const QSqlQuery &q);
};
