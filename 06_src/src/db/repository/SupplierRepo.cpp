#include "SupplierRepo.h"
#include <QSqlError>
#include <QDebug>

SupplierRepo::SupplierRepo(QSqlDatabase &db, QObject *p)
    : QObject(p), m_db(db) {}

Supplier SupplierRepo::rowToSupplier(const QSqlQuery &q)
{
    Supplier s;
    s.supplierId    = q.value("supplier_id").toInt();
    s.supplierCode  = q.value("supplier_code").toString();
    s.name          = q.value("name").toString();
    s.contactPerson = q.value("contact_person").toString();
    s.phone         = q.value("phone").toString();
    s.address       = q.value("address").toString();
    s.rating        = q.value("rating").toInt();
    return s;
}

QList<Supplier> SupplierRepo::findAll()
{
    QList<Supplier> list;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM Supplier ORDER BY supplier_id");
    while (query.next()) list.append(rowToSupplier(query));
    return list;
}

Supplier SupplierRepo::findById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Supplier WHERE supplier_id = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) return rowToSupplier(query);
    return Supplier();
}

bool SupplierRepo::insert(const Supplier &s)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO Supplier (supplier_code, name, contact_person, phone, address, rating) "
                  "VALUES (:code, :name, :contact, :phone, :addr, :rating)");
    query.bindValue(":code",    s.supplierCode);
    query.bindValue(":name",    s.name);
    query.bindValue(":contact", s.contactPerson);
    query.bindValue(":phone",   s.phone);
    query.bindValue(":addr",    s.address);
    query.bindValue(":rating",  s.rating);
    if (!query.exec()) { qWarning() << "SupplierRepo::insert" << query.lastError().text(); return false; }
    return true;
}

bool SupplierRepo::update(const Supplier &s)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Supplier SET supplier_code=:code, name=:name, contact_person=:contact, "
                  "phone=:phone, address=:addr, rating=:rating WHERE supplier_id=:id");
    query.bindValue(":code",    s.supplierCode);
    query.bindValue(":name",    s.name);
    query.bindValue(":contact", s.contactPerson);
    query.bindValue(":phone",   s.phone);
    query.bindValue(":addr",    s.address);
    query.bindValue(":rating",  s.rating);
    query.bindValue(":id",      s.supplierId);
    if (!query.exec()) { qWarning() << "SupplierRepo::update" << query.lastError().text(); return false; }
    return true;
}

bool SupplierRepo::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Supplier WHERE supplier_id = :id");
    q.bindValue(":id", id);
    return q.exec();
}
