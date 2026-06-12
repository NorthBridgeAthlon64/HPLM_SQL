#include "ProductionService.h"
#include <QSqlDatabase>
#include <QDebug>

ProductionService::ProductionService(QSqlDatabase &db, QObject *p)
    : QObject(p), m_batchRepo(db), m_matRepo(db), m_bomRepo(db), m_invRepo(db), m_txRepo(db) {}

QList<BOMItem> ProductionService::getBOM(int versionId)
{
    return m_bomRepo.findByVersion(versionId);
}

bool ProductionService::createBatch(ProductionBatch &batch, QStringList &shortageList)
{
    QSqlDatabase &db = m_batchRepo.m_db;

    // 1. 获取 BOM 清单
    QList<BOMItem> bomList = m_bomRepo.findByVersion(batch.versionId);
    if (bomList.isEmpty()) {
        shortageList.append("BOM 清单为空，无法生产");
        return false;
    }

    // 2. 检查库存是否充足（以第一个仓库为准, WH-A01 = warehouse_id 1）
    db.transaction();
    for (const BOMItem &bom : bomList) {
        Inventory inv = m_invRepo.findByComponentAndWarehouse(bom.componentId, 1);
        int needed = bom.quantity * batch.quantity;
        if (inv.quantity < needed) {
            shortageList.append(
                QStringLiteral("%1(%2): 需要 %3, 库存 %4")
                    .arg(bom.componentName, bom.componentCode)
                    .arg(needed).arg(inv.quantity)
            );
        }
    }
    if (!shortageList.isEmpty()) {
        db.rollback();
        return false;
    }

    // 3. 插入生产批次
    batch.qualityStatus = "pending";
    batch.productionDate = QDate::currentDate();
    if (!m_batchRepo.insert(batch)) { db.rollback(); return false; }

    // 4. 逐项扣减库存并插入投料记录
    for (const BOMItem &bom : bomList) {
        int needed = bom.quantity * batch.quantity;
        m_invRepo.addQuantity(bom.componentId, 1, -needed);

        BatchMaterial mat;
        mat.batchId = batch.batchId;
        mat.componentId = bom.componentId;
        mat.usedQuantity = needed;
        mat.unitCostAtTime = bom.unitPrice;
        m_matRepo.insert(mat);
    }

    db.commit();
    return true;
}
