#include "BatchMaterialRepo.h"
#include <QSqlError>
BatchMaterialRepo::BatchMaterialRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
BatchMaterial BatchMaterialRepo::rowToMaterial(const QSqlQuery &q) {
    BatchMaterial m;
    m.batchMaterialId=q.value("batch_material_id").toInt(); m.batchId=q.value("batch_id").toInt();
    m.componentId=q.value("component_id").toInt(); m.usedQuantity=q.value("used_quantity").toInt();
    m.unitCostAtTime=q.value("unit_cost_at_time").toDouble(); m.totalCost=q.value("total_cost").toDouble();
    m.componentCode=q.value("component_code").toString(); m.componentName=q.value("component_name").toString();
    m.unit=q.value("unit").toString();
    return m;
}
QList<BatchMaterial> BatchMaterialRepo::findByBatch(int batchId) {
    QList<BatchMaterial> list; QSqlQuery q(m_db);
    q.prepare("SELECT bm.*, c.component_code, c.name AS component_name, c.unit FROM BatchMaterial bm JOIN Component c ON bm.component_id=c.component_id WHERE bm.batch_id=:bid ORDER BY bm.batch_material_id");
    q.bindValue(":bid",batchId);
    if(q.exec()) while(q.next()) list.append(rowToMaterial(q));
    return list;
}
bool BatchMaterialRepo::insert(const BatchMaterial &m) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO BatchMaterial (batch_id,component_id,used_quantity,unit_cost_at_time) VALUES (:bid,:cid,:qty,:uc)");
    q.bindValue(":bid",m.batchId); q.bindValue(":cid",m.componentId);
    q.bindValue(":qty",m.usedQuantity); q.bindValue(":uc",m.unitCostAtTime);
    return q.exec();
}
