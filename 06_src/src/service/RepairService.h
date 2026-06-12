#pragma once
#include <QObject>
#include "db/repository/RepairRecordRepo.h"
#include "db/repository/RepairMaterialRepo.h"
#include "db/repository/InventoryRepo.h"
#include "db/repository/InventoryTransactionRepo.h"

class RepairService : public QObject {
    Q_OBJECT
public:
    explicit RepairService(QSqlDatabase &db, QObject *p = nullptr);
    bool addRepairMaterial(int repairId, int componentId, int warehouseId,
                           int quantity, double unitPrice, int operatorId);
    bool completeRepair(int repairId);

private:
    RepairRecordRepo m_repairRepo;
    RepairMaterialRepo m_matRepo;
    InventoryRepo m_invRepo;
    InventoryTransactionRepo m_txRepo;
};
