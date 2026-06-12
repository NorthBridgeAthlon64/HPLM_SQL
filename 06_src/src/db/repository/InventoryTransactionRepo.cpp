#include "InventoryTransactionRepo.h"
#include <QSqlError>
InventoryTransactionRepo::InventoryTransactionRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
InventoryTrans InventoryTransactionRepo::rowToTrans(const QSqlQuery &q) {
    InventoryTrans t;
    t.transactionId=q.value("transaction_id").toInt(); t.transactionNo=q.value("transaction_no").toString();
    t.componentId=q.value("component_id").toInt(); t.warehouseId=q.value("warehouse_id").toInt();
    t.transactionType=q.value("transaction_type").toString(); t.quantity=q.value("quantity").toInt();
    t.unitPrice=q.value("unit_price").toDouble(); t.totalPrice=q.value("total_price").toDouble();
    t.transactionDate=q.value("transaction_date").toDate(); t.referenceNo=q.value("reference_no").toString();
    t.operatorId=q.value("operator_id").toInt();
    t.componentName=q.value("component_name").toString(); t.warehouseName=q.value("warehouse_name").toString();
    return t;
}
QList<InventoryTrans> InventoryTransactionRepo::findByComponent(int componentId) {
    QList<InventoryTrans> list; QSqlQuery q(m_db);
    q.prepare("SELECT it.*, c.name AS component_name, w.name AS warehouse_name FROM InventoryTransaction it JOIN Component c ON it.component_id=c.component_id JOIN Warehouse w ON it.warehouse_id=w.warehouse_id WHERE it.component_id=:cid ORDER BY it.transaction_date DESC");
    q.bindValue(":cid",componentId);
    if(q.exec()) while(q.next()) list.append(rowToTrans(q));
    return list;
}
QList<InventoryTrans> InventoryTransactionRepo::findAll() {
    QList<InventoryTrans> list; QSqlQuery q(m_db);
    q.exec("SELECT it.*, c.name AS component_name, w.name AS warehouse_name FROM InventoryTransaction it JOIN Component c ON it.component_id=c.component_id JOIN Warehouse w ON it.warehouse_id=w.warehouse_id ORDER BY it.transaction_date DESC LIMIT 200");
    while(q.next()) list.append(rowToTrans(q));
    return list;
}
bool InventoryTransactionRepo::insert(const InventoryTrans &t) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO InventoryTransaction (transaction_no,component_id,warehouse_id,transaction_type,quantity,unit_price,total_price,transaction_date,reference_no,operator_id) VALUES (:no,:cid,:wid,:type,:qty,:up,:tp,:date,:ref,:oid)");
    q.bindValue(":no",t.transactionNo); q.bindValue(":cid",t.componentId); q.bindValue(":wid",t.warehouseId);
    q.bindValue(":type",t.transactionType); q.bindValue(":qty",t.quantity);
    q.bindValue(":up",t.unitPrice); q.bindValue(":tp",t.totalPrice);
    q.bindValue(":date",t.transactionDate); q.bindValue(":ref",t.referenceNo); q.bindValue(":oid",t.operatorId);
    return q.exec();
}
