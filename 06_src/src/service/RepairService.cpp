#include "RepairService.h"
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

RepairService::RepairService(QSqlDatabase &db, QObject *p)
    : QObject(p), m_repairRepo(db), m_matRepo(db), m_invRepo(db), m_txRepo(db) {}

bool RepairService::addRepairMaterial(int repairId, int componentId, int warehouseId,
                                       int quantity, double unitPrice, int operatorId)
{
    QSqlDatabase &db = m_repairRepo.m_db;

    // 检查库存
    Inventory inv = m_invRepo.findByComponentAndWarehouse(componentId, warehouseId);
    if (inv.quantity < quantity) {
        qWarning() << "RepairService: insufficient inventory";
        return false;
    }

    db.transaction();

    // 扣减库存
    m_invRepo.addQuantity(componentId, warehouseId, -quantity);

    // 插入维修用料
    RepairMaterial mat;
    mat.repairId = repairId;
    mat.componentId = componentId;
    mat.quantity = quantity;
    mat.unitCostAtTime = unitPrice;
    if (!m_matRepo.insert(mat)) { db.rollback(); return false; }

    // 累加维修费用
    m_repairRepo.addCost(repairId, quantity * unitPrice);

    db.commit();
    return true;
}

bool RepairService::completeRepair(int repairId)
{
    return m_repairRepo.updateStatus(repairId, "completed", QDate::currentDate());
}
