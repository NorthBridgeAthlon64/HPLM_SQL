#include "ProductionBatchRepo.h"
#include <QSqlError>
ProductionBatchRepo::ProductionBatchRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
ProductionBatch ProductionBatchRepo::rowToBatch(const QSqlQuery &q) {
    ProductionBatch b;
    b.batchId=q.value("batch_id").toInt(); b.batchNo=q.value("batch_no").toString();
    b.versionId=q.value("version_id").toInt(); b.quantity=q.value("quantity").toInt();
    b.productionDate=q.value("production_date").toDate(); b.testerId=q.value("tester_id").toInt();
    b.qualityStatus=q.value("quality_status").toString();
    b.batchMaterialCost=q.value("batch_material_cost").toDouble();
    b.batchLaborCost=q.value("batch_labor_cost").toDouble();
    b.batchTotalCost=q.value("batch_total_cost").toDouble();
    b.versionNumber=q.value("version_number").toString(); b.productName=q.value("product_name").toString();
    return b;
}
QList<ProductionBatch> ProductionBatchRepo::findAll() {
    QList<ProductionBatch> list; QSqlQuery q(m_db);
    q.exec("SELECT pb.*, pv.version_number, p.name AS product_name FROM ProductionBatch pb JOIN ProductVersion pv ON pb.version_id=pv.version_id JOIN Product p ON pv.product_id=p.product_id ORDER BY pb.production_date DESC");
    while(q.next()) list.append(rowToBatch(q));
    return list;
}
ProductionBatch ProductionBatchRepo::findById(int id) {
    QSqlQuery q(m_db);
    q.prepare("SELECT pb.*, pv.version_number, p.name AS product_name FROM ProductionBatch pb JOIN ProductVersion pv ON pb.version_id=pv.version_id JOIN Product p ON pv.product_id=p.product_id WHERE pb.batch_id=:id");
    q.bindValue(":id",id); if(q.exec()&&q.next()) return rowToBatch(q); return ProductionBatch();
}
ProductionBatch ProductionBatchRepo::findByBatchNo(const QString &batchNo) {
    QSqlQuery q(m_db);
    q.prepare("SELECT pb.*, pv.version_number, p.name AS product_name FROM ProductionBatch pb JOIN ProductVersion pv ON pb.version_id=pv.version_id JOIN Product p ON pv.product_id=p.product_id WHERE pb.batch_no=:bn");
    q.bindValue(":bn",batchNo); if(q.exec()&&q.next()) return rowToBatch(q); return ProductionBatch();
}
bool ProductionBatchRepo::insert(ProductionBatch &b) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO ProductionBatch (batch_no,version_id,quantity,production_date,tester_id,quality_status,batch_material_cost,batch_labor_cost,batch_total_cost) VALUES (:bn,:vid,:qty,:pd,:tid,:qs,:bmc,:blc,:btc) RETURNING batch_id");
    q.bindValue(":bn",b.batchNo); q.bindValue(":vid",b.versionId); q.bindValue(":qty",b.quantity);
    q.bindValue(":pd",b.productionDate); q.bindValue(":tid",b.testerId>0?b.testerId:QVariant());
    q.bindValue(":qs",b.qualityStatus); q.bindValue(":bmc",b.batchMaterialCost>0?b.batchMaterialCost:QVariant());
    q.bindValue(":blc",b.batchLaborCost); q.bindValue(":btc",b.batchTotalCost>0?b.batchTotalCost:QVariant());
    if(q.exec()&&q.next()){ b.batchId=q.value("batch_id").toInt(); return true; }
    return false;
}
bool ProductionBatchRepo::updateStatus(int batchId, const QString &status) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE ProductionBatch SET quality_status=:qs WHERE batch_id=:id");
    q.bindValue(":qs",status); q.bindValue(":id",batchId); return q.exec();
}
bool ProductionBatchRepo::remove(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM ProductionBatch WHERE batch_id=:id"); q.bindValue(":id",id); return q.exec();
}
