#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include "db/repository/RepairRecordRepo.h"
#include "db/repository/ProductionBatchRepo.h"
#include "db/repository/BatchMaterialRepo.h"
#include "db/repository/TestRecordRepo.h"
#include "db/repository/ComponentRepo.h"
#include "db/repository/SupplierRepo.h"

/// 溯源结果
struct TraceResult {
    RepairRecord repair;
    ProductionBatch batch;
    QList<BatchMaterial> materials;
    QList<TestRecord> testRecords;
    QStringList suppliers;
};

class TraceService : public QObject {
    Q_OBJECT
public:
    explicit TraceService(QSqlDatabase &db, QObject *p = nullptr);
    TraceResult traceByRepairNo(const QString &repairNo);
    TraceResult traceByBatchNo(const QString &batchNo);

private:
    RepairRecordRepo m_repairRepo;
    ProductionBatchRepo m_batchRepo;
    BatchMaterialRepo m_matRepo;
    TestRecordRepo m_testRepo;
    ComponentRepo m_compRepo;
};
