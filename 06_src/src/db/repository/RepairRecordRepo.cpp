#include "RepairRecordRepo.h"
#include <QSqlError>
RepairRecordRepo::RepairRecordRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
RepairRecord RepairRecordRepo::rowToRepair(const QSqlQuery &q) {
    RepairRecord r;
    r.repairId=q.value("repair_id").toInt(); r.repairNo=q.value("repair_no").toString();
    r.versionId=q.value("version_id").toInt(); r.batchId=q.value("batch_id").toInt();
    r.customerId=q.value("customer_id").toInt(); r.receiveDate=q.value("receive_date").toDate();
    r.completeDate=q.value("complete_date").toDate();
    r.faultDescription=q.value("fault_description").toString();
    r.repairStatus=q.value("repair_status").toString(); r.repairmanId=q.value("repairman_id").toInt();
    r.repairCost=q.value("repair_cost").toDouble();
    r.customerName=q.value("customer_name").toString(); r.versionNumber=q.value("version_number").toString();
    return r;
}
QList<RepairRecord> RepairRecordRepo::findAll() {
    QList<RepairRecord> list; QSqlQuery q(m_db);
    q.exec("SELECT r.*, c.name AS customer_name, pv.version_number FROM RepairRecord r JOIN Customer c ON r.customer_id=c.customer_id JOIN ProductVersion pv ON r.version_id=pv.version_id ORDER BY r.receive_date DESC");
    while(q.next()) list.append(rowToRepair(q)); return list;
}
RepairRecord RepairRecordRepo::findByRepairNo(const QString &rn) {
    QSqlQuery q(m_db);
    q.prepare("SELECT r.*, c.name AS customer_name, pv.version_number FROM RepairRecord r JOIN Customer c ON r.customer_id=c.customer_id JOIN ProductVersion pv ON r.version_id=pv.version_id WHERE r.repair_no=:rn");
    q.bindValue(":rn",rn); if(q.exec()&&q.next()) return rowToRepair(q); return RepairRecord();
}
RepairRecord RepairRecordRepo::findById(int id) {
    QSqlQuery q(m_db);
    q.prepare("SELECT r.*, c.name AS customer_name, pv.version_number FROM RepairRecord r JOIN Customer c ON r.customer_id=c.customer_id JOIN ProductVersion pv ON r.version_id=pv.version_id WHERE r.repair_id=:id");
    q.bindValue(":id",id); if(q.exec()&&q.next()) return rowToRepair(q); return RepairRecord();
}
int RepairRecordRepo::pendingCount() {
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM RepairRecord WHERE repair_status IN ('received','in_progress')");
    if(q.next()) return q.value(0).toInt();
    return 0;
}
bool RepairRecordRepo::insert(RepairRecord &r) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO RepairRecord (repair_no,version_id,batch_id,customer_id,receive_date,fault_description,repair_status,repairman_id,repair_cost) VALUES (:rn,:vid,:bid,:cid,:rd,:fd,:st,:rmid,:rc) RETURNING repair_id");
    q.bindValue(":rn",r.repairNo); q.bindValue(":vid",r.versionId); q.bindValue(":bid",r.batchId>0?r.batchId:QVariant());
    q.bindValue(":cid",r.customerId); q.bindValue(":rd",r.receiveDate);
    q.bindValue(":fd",r.faultDescription); q.bindValue(":st",r.repairStatus);
    q.bindValue(":rmid",r.repairmanId>0?r.repairmanId:QVariant()); q.bindValue(":rc",r.repairCost);
    if(q.exec()&&q.next()){ r.repairId=q.value("repair_id").toInt(); return true; }
    return false;
}
bool RepairRecordRepo::updateStatus(int repairId, const QString &status, const QDate &completeDate) {
    QSqlQuery q(m_db);
    if(completeDate.isValid())
        q.prepare("UPDATE RepairRecord SET repair_status=:st, complete_date=:cd WHERE repair_id=:id");
    else
        q.prepare("UPDATE RepairRecord SET repair_status=:st WHERE repair_id=:id");
    q.bindValue(":st",status); q.bindValue(":id",repairId);
    if(completeDate.isValid()) q.bindValue(":cd",completeDate);
    return q.exec();
}
bool RepairRecordRepo::addCost(int repairId, double amount) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE RepairRecord SET repair_cost = repair_cost + :amt WHERE repair_id=:id");
    q.bindValue(":amt",amount); q.bindValue(":id",repairId); return q.exec();
}
bool RepairRecordRepo::remove(int id) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM RepairRecord WHERE repair_id=:id"); q.bindValue(":id",id); return q.exec();
}
