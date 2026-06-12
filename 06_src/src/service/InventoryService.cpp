#include "InventoryService.h"
#include "db/DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

InventoryService::InventoryService(QSqlDatabase &db, QObject *p)
    : QObject(p), m_invRepo(db), m_txRepo(db), m_compRepo(db) {}

bool InventoryService::inbound(const QString &txNo, int componentId, int warehouseId,
                                int quantity, double unitPrice, int operatorId)
{
    QSqlDatabase &db = DatabaseManager::instance().db();
    db.transaction();

    // 1. 插入库存流水
    InventoryTrans tx;
    tx.transactionNo = txNo;
    tx.componentId = componentId;
    tx.warehouseId = warehouseId;
    tx.transactionType = "in";
    tx.quantity = quantity;
    tx.unitPrice = unitPrice;
    tx.totalPrice = quantity * unitPrice;
    tx.transactionDate = QDate::currentDate();
    tx.operatorId = operatorId;
    if (!m_txRepo.insert(tx)) { db.rollback(); return false; }

    // 2. 更新库存数量（移动平均成本在 addQuantity 中自动处理）
    if (!m_invRepo.addQuantity(componentId, warehouseId, quantity, unitPrice)) {
        db.rollback(); return false;
    }

    // 3. 更新元器件当前价格
    QSqlQuery q(db);
    q.prepare("UPDATE Component SET current_price = :price WHERE component_id = :id");
    q.bindValue(":price", unitPrice);
    q.bindValue(":id", componentId);
    if (!q.exec()) { db.rollback(); return false; }

    db.commit();
    return true;
}
