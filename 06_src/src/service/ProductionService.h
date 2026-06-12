#pragma once
#include <QObject>
#include <QList>
#include "db/repository/ProductionBatchRepo.h"
#include "db/repository/BatchMaterialRepo.h"
#include "db/repository/ProductBOMRepo.h"
#include "db/repository/InventoryRepo.h"
#include "db/repository/InventoryTransactionRepo.h"

class ProductionService : public QObject {
    Q_OBJECT
public:
    explicit ProductionService(QSqlDatabase &db, QObject *p = nullptr);
    /// 创建生产批次：BOM→库存检查→扣减→投料
    bool createBatch(ProductionBatch &batch, QStringList &shortageList);
    /// 获取版本的 BOM 列表
    QList<BOMItem> getBOM(int versionId);

private:
    ProductionBatchRepo m_batchRepo;
    BatchMaterialRepo m_matRepo;
    ProductBOMRepo m_bomRepo;
    InventoryRepo m_invRepo;
    InventoryTransactionRepo m_txRepo;
};
