#include "ProductBOMRepo.h"
#include <QSqlError>
ProductBOMRepo::ProductBOMRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
BOMItem ProductBOMRepo::rowToBOM(const QSqlQuery &q) {
    BOMItem b;
    b.bomId=q.value("bom_id").toInt(); b.versionId=q.value("version_id").toInt();
    b.componentId=q.value("component_id").toInt(); b.quantity=q.value("quantity").toInt();
    b.position=q.value("position").toString();
    b.componentCode=q.value("component_code").toString(); b.componentName=q.value("component_name").toString();
    b.specification=q.value("specification").toString(); b.unitPrice=q.value("unit_price").toDouble();
    return b;
}
QList<BOMItem> ProductBOMRepo::findByVersion(int versionId) {
    QList<BOMItem> list; QSqlQuery q(m_db);
    q.prepare("SELECT pb.*, c.component_code, c.name AS component_name, c.specification, COALESCE(c.current_price,0) AS unit_price FROM ProductBOM pb JOIN Component c ON pb.component_id=c.component_id WHERE pb.version_id=:vid ORDER BY pb.bom_id");
    q.bindValue(":vid",versionId);
    if(q.exec()) while(q.next()) list.append(rowToBOM(q));
    return list;
}
bool ProductBOMRepo::insert(const BOMItem &b) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO ProductBOM (version_id,component_id,quantity,position) VALUES (:vid,:cid,:qty,:pos)");
    q.bindValue(":vid",b.versionId); q.bindValue(":cid",b.componentId);
    q.bindValue(":qty",b.quantity); q.bindValue(":pos",b.position);
    return q.exec();
}
bool ProductBOMRepo::update(const BOMItem &b) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE ProductBOM SET quantity=:qty, position=:pos WHERE bom_id=:id");
    q.bindValue(":qty",b.quantity); q.bindValue(":pos",b.position); q.bindValue(":id",b.bomId);
    return q.exec();
}
bool ProductBOMRepo::remove(int bomId) {
    QSqlQuery q(m_db); q.prepare("DELETE FROM ProductBOM WHERE bom_id=:id"); q.bindValue(":id",bomId); return q.exec();
}
