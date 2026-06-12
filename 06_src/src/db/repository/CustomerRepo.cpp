#include "CustomerRepo.h"
#include <QSqlError>
CustomerRepo::CustomerRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
Customer CustomerRepo::rowToCustomer(const QSqlQuery &q) {
    Customer c;
    c.customerId = q.value("customer_id").toInt();
    c.customerCode = q.value("customer_code").toString();
    c.name = q.value("name").toString();
    c.phone = q.value("phone").toString();
    c.email = q.value("email").toString();
    c.address = q.value("address").toString();
    c.registerDate = q.value("register_date").toDate();
    c.tier = q.value("tier").toInt();
    return c;
}
QList<Customer> CustomerRepo::findAll() {
    QList<Customer> list;
    QSqlQuery q(m_db); q.exec("SELECT * FROM Customer ORDER BY customer_id");
    while (q.next()) list.append(rowToCustomer(q));
    return list;
}
Customer CustomerRepo::findById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT * FROM Customer WHERE customer_id=:id"); q.bindValue(":id",id);
    if (q.exec() && q.next()) return rowToCustomer(q);
    return Customer();
}
QList<Customer> CustomerRepo::findByName(const QString &kw) {
    QList<Customer> list;
    QSqlQuery q(m_db); q.prepare("SELECT * FROM Customer WHERE name ILIKE :kw OR customer_code ILIKE :kw2");
    q.bindValue(":kw", "%"+kw+"%"); q.bindValue(":kw2", "%"+kw+"%");
    if (q.exec()) while (q.next()) list.append(rowToCustomer(q));
    return list;
}
bool CustomerRepo::insert(const Customer &c) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Customer (customer_code,name,phone,email,address,register_date,tier) VALUES (:code,:name,:phone,:email,:addr,:reg,:tier)");
    q.bindValue(":code",c.customerCode); q.bindValue(":name",c.name); q.bindValue(":phone",c.phone);
    q.bindValue(":email",c.email); q.bindValue(":addr",c.address); q.bindValue(":reg",c.registerDate); q.bindValue(":tier",c.tier);
    return q.exec();
}
bool CustomerRepo::update(const Customer &c) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE Customer SET customer_code=:code,name=:name,phone=:phone,email=:email,address=:addr,tier=:tier WHERE customer_id=:id");
    q.bindValue(":code",c.customerCode); q.bindValue(":name",c.name); q.bindValue(":phone",c.phone);
    q.bindValue(":email",c.email); q.bindValue(":addr",c.address); q.bindValue(":tier",c.tier); q.bindValue(":id",c.customerId);
    return q.exec();
}
bool CustomerRepo::remove(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM Customer WHERE customer_id=:id"); q.bindValue(":id",id); return q.exec();
}
