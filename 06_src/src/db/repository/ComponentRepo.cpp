#include "ComponentRepo.h"
#include <QSqlError>
#include <QDebug>

ComponentRepo::ComponentRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}

Component ComponentRepo::rowToComponent(const QSqlQuery &q)
{
    Component c;
    c.componentId    = q.value("component_id").toInt();
    c.componentCode  = q.value("component_code").toString();
    c.name           = q.value("name").toString();
    c.specification  = q.value("specification").toString();
    c.unit           = q.value("unit").toString();
    c.minStock       = q.value("min_stock").toInt();
    c.currentPrice   = q.value("current_price").toDouble();
    c.supplierId     = q.value("supplier_id").toInt();
    c.supplierName   = q.value("supplier_name").toString();
    c.inventoryQuantity = q.value("inventory_qty").toDouble();
    return c;
}

QList<Component> ComponentRepo::findAll()
{
    QSqlQuery query(m_db);
    query.exec(
        "SELECT c.*, COALESCE(s.name, '') AS supplier_name, "
        "COALESCE(SUM(i.quantity), 0) AS inventory_qty "
        "FROM Component c "
        "LEFT JOIN Supplier s ON c.supplier_id = s.supplier_id "
        "LEFT JOIN Inventory i ON c.component_id = i.component_id "
        "GROUP BY c.component_id, s.name "
        "ORDER BY c.component_id"
    );
    QList<Component> list;
    while (query.next()) list.append(rowToComponent(query));
    return list;
}

QList<Component> ComponentRepo::search(const QString &keyword, int supplierId)
{
    QSqlQuery query(m_db);
    QString sql =
        "SELECT c.*, COALESCE(s.name, '') AS supplier_name, "
        "COALESCE(SUM(i.quantity), 0) AS inventory_qty "
        "FROM Component c "
        "LEFT JOIN Supplier s ON c.supplier_id = s.supplier_id "
        "LEFT JOIN Inventory i ON c.component_id = i.component_id "
        "WHERE (c.component_code ILIKE :kw OR c.name ILIKE :kw2) ";
    if (supplierId > 0) sql += "AND c.supplier_id = :sid ";
    sql += "GROUP BY c.component_id, s.name ORDER BY c.component_id";

    query.prepare(sql);
    query.bindValue(":kw", "%" + keyword + "%");
    query.bindValue(":kw2", "%" + keyword + "%");
    if (supplierId > 0) query.bindValue(":sid", supplierId);

    QList<Component> list;
    if (query.exec())
        while (query.next()) list.append(rowToComponent(query));
    return list;
}

Component ComponentRepo::findById(int id)
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT c.*, COALESCE(s.name, '') AS supplier_name "
        "FROM Component c LEFT JOIN Supplier s ON c.supplier_id = s.supplier_id "
        "WHERE c.component_id = :id"
    );
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return rowToComponent(q);
    return Component();
}

bool ComponentRepo::insert(const Component &c)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Component (component_code, name, specification, unit, min_stock, current_price, supplier_id) "
              "VALUES (:code, :name, :spec, :unit, :min, :price, :sid)");
    q.bindValue(":code", c.componentCode);
    q.bindValue(":name", c.name);
    q.bindValue(":spec", c.specification);
    q.bindValue(":unit", c.unit);
    q.bindValue(":min",  c.minStock);
    q.bindValue(":price", c.currentPrice);
    q.bindValue(":sid",  c.supplierId > 0 ? c.supplierId : QVariant());
    return q.exec();
}

bool ComponentRepo::update(const Component &c)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE Component SET component_code=:code, name=:name, specification=:spec, "
              "unit=:unit, min_stock=:min, current_price=:price, supplier_id=:sid "
              "WHERE component_id=:id");
    q.bindValue(":code", c.componentCode);
    q.bindValue(":name", c.name);
    q.bindValue(":spec", c.specification);
    q.bindValue(":unit", c.unit);
    q.bindValue(":min",  c.minStock);
    q.bindValue(":price", c.currentPrice);
    q.bindValue(":sid",  c.supplierId > 0 ? c.supplierId : QVariant());
    q.bindValue(":id",   c.componentId);
    return q.exec();
}

bool ComponentRepo::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Component WHERE component_id = :id");
    q.bindValue(":id", id);
    return q.exec();
}
