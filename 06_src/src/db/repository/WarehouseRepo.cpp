#include "WarehouseRepo.h"
#include <QSqlError>
WarehouseRepo::WarehouseRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
Warehouse WarehouseRepo::rowToWarehouse(const QSqlQuery &q) {
    Warehouse w;
    w.warehouseId = q.value("warehouse_id").toInt();
    w.warehouseCode = q.value("warehouse_code").toString();
    w.name = q.value("name").toString();
    w.location = q.value("location").toString();
    return w;
}
QList<Warehouse> WarehouseRepo::findAll() {
    QList<Warehouse> list;
    QSqlQuery q(m_db); q.exec("SELECT * FROM Warehouse ORDER BY warehouse_id");
    while (q.next()) list.append(rowToWarehouse(q));
    return list;
}
Warehouse WarehouseRepo::findById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT * FROM Warehouse WHERE warehouse_id=:id"); q.bindValue(":id",id);
    if (q.exec() && q.next()) return rowToWarehouse(q);
    return Warehouse();
}
bool WarehouseRepo::insert(const Warehouse &w) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Warehouse (warehouse_code,name,location) VALUES (:code,:name,:loc)");
    q.bindValue(":code",w.warehouseCode); q.bindValue(":name",w.name); q.bindValue(":loc",w.location);
    return q.exec();
}
bool WarehouseRepo::update(const Warehouse &w) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE Warehouse SET warehouse_code=:code,name=:name,location=:loc WHERE warehouse_id=:id");
    q.bindValue(":code",w.warehouseCode); q.bindValue(":name",w.name); q.bindValue(":loc",w.location); q.bindValue(":id",w.warehouseId);
    return q.exec();
}
bool WarehouseRepo::remove(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM Warehouse WHERE warehouse_id=:id"); q.bindValue(":id",id); return q.exec();
}
