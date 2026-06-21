#include "InventoryRepo.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
InventoryRepo::InventoryRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
Inventory InventoryRepo::findByComponentAndWarehouse(int componentId, int warehouseId) {
    Inventory inv;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM Inventory WHERE component_id=:cid AND warehouse_id=:wid");
    q.bindValue(":cid", componentId); q.bindValue(":wid", warehouseId);
    if (q.exec() && q.next()) {
        inv.inventoryId = q.value("inventory_id").toInt();
        inv.componentId = q.value("component_id").toInt();
        inv.warehouseId = q.value("warehouse_id").toInt();
        inv.quantity = q.value("quantity").toInt();
        inv.reservedQuantity = q.value("reserved_quantity").toInt();
        inv.avgCost = q.value("avg_cost").toDouble();
    }
    return inv;
}
bool InventoryRepo::addQuantity(int componentId, int warehouseId, int delta, double unitCost) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Inventory (component_id, warehouse_id, quantity, avg_cost) VALUES (:cid, :wid, :qty, :cost) ON CONFLICT (component_id, warehouse_id) DO UPDATE SET quantity = Inventory.quantity + :qty2, avg_cost = CASE WHEN :qty3 < 0 THEN Inventory.avg_cost ELSE (Inventory.avg_cost * Inventory.quantity + :cost2 * :cost3) / GREATEST(Inventory.quantity + :qty4, 1) END");
    q.bindValue(":cid", componentId); q.bindValue(":wid", warehouseId);
    q.bindValue(":qty", delta > 0 ? delta : 0); q.bindValue(":qty2", delta);
    q.bindValue(":qty3", delta); q.bindValue(":qty4", delta);
    q.bindValue(":cost", unitCost); q.bindValue(":cost2", unitCost);
    q.bindValue(":cost3", delta);
    if (!q.exec()) { qWarning() << "InventoryRepo::addQuantity" << q.lastError().text(); return false; }
    return true;
}
bool InventoryRepo::reserveQuantity(int componentId, int warehouseId, int qty) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE Inventory SET reserved_quantity = reserved_quantity + :qty WHERE component_id=:cid AND warehouse_id=:wid AND quantity - reserved_quantity >= :qty2");
    q.bindValue(":qty", qty); q.bindValue(":qty2", qty);
    q.bindValue(":cid", componentId); q.bindValue(":wid", warehouseId);
    if (!q.exec() || q.numRowsAffected() == 0) { qWarning() << "InventoryRepo::reserveQuantity failed"; return false; }
    return true;
}
