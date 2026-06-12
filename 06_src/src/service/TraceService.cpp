#include "TraceService.h"
#include <QSqlDatabase>
#include <QDebug>

TraceService::TraceService(QSqlDatabase &db, QObject *p)
    : QObject(p), m_repairRepo(db), m_batchRepo(db), m_matRepo(db), m_testRepo(db), m_compRepo(db) {}

TraceResult TraceService::traceByRepairNo(const QString &repairNo)
{
    TraceResult result;
    result.repair = m_repairRepo.findByRepairNo(repairNo);
    if (result.repair.repairId == 0) {
        qWarning() << "TraceService: repair not found:" << repairNo;
        return result;
    }

    // 查批次
    if (result.repair.batchId > 0) {
        result.batch = m_batchRepo.findById(result.repair.batchId);
        // 批次投料
        result.materials = m_matRepo.findByBatch(result.repair.batchId);
        // 测试记录
        result.testRecords = m_testRepo.findByBatch(result.repair.batchId);
        // 收集供应商
        for (const BatchMaterial &bm : result.materials) {
            Component c = m_compRepo.findById(bm.componentId);
            if (c.supplierId > 0)
                result.suppliers.append(c.supplierName);
        }
    }

    return result;
}

TraceResult TraceService::traceByBatchNo(const QString &batchNo)
{
    TraceResult result;
    result.batch = m_batchRepo.findByBatchNo(batchNo);
    if (result.batch.batchId == 0) {
        qWarning() << "TraceService: batch not found:" << batchNo;
        return result;
    }

    result.materials = m_matRepo.findByBatch(result.batch.batchId);
    result.testRecords = m_testRepo.findByBatch(result.batch.batchId);
    for (const BatchMaterial &bm : result.materials) {
        Component c = m_compRepo.findById(bm.componentId);
        if (c.supplierId > 0)
            result.suppliers.append(c.supplierName);
    }

    return result;
}
