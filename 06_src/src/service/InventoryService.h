#pragma once
#include <QObject>
#include "db/repository/InventoryRepo.h"
#include "db/repository/InventoryTransactionRepo.h"
#include "db/repository/ComponentRepo.h"

class InventoryService : public QObject {
    Q_OBJECT
public:
    explicit InventoryService(QSqlDatabase &db, QObject *p = nullptr);

    /// 入库操作（事务内：插流水→更新单价→更新库存）
    bool inbound(const QString &txNo, int componentId, int warehouseId,
                 int quantity, double unitPrice, int operatorId);

private:
    InventoryRepo m_invRepo;
    InventoryTransactionRepo m_txRepo;
    ComponentRepo m_compRepo;
};
