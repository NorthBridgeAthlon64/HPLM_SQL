#include "RepairMaterialRepo.h"
#include <QSqlError>
RepairMaterialRepo::RepairMaterialRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
RepairMaterial RepairMaterialRepo::rowToMaterial(const QSqlQuery &q) {
    RepairMaterial m;
    m.repairMaterialId=q.value("repair_material_id").toInt(); m.repairId=q.value("repair_id").toInt();
    m.componentId=q.value("component_id").toInt(); m.quantity=q.value("quantity").toInt();
    m.unitCostAtTime=q.value("unit_cost_at_time").toDouble(); m.totalCost=q.value("total_cost").toDouble();
    m.componentCode=q.value("component_code").toString(); m.componentName=q.value("component_name").toString();
    return m;
}
QList<RepairMaterial> RepairMaterialRepo::findByRepair(int repairId) {
    QList<RepairMaterial> list; QSqlQuery q(m_db);
    q.prepare("SELECT rm.*, c.component_code, c.name AS component_name FROM RepairMaterial rm JOIN Component c ON rm.component_id=c.component_id WHERE rm.repair_id=:rid ORDER BY rm.repair_material_id");
    q.bindValue(":rid",repairId);
    if(q.exec()) while(q.next()) list.append(rowToMaterial(q));
    return list;
}
bool RepairMaterialRepo::insert(const RepairMaterial &m) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO RepairMaterial (repair_id,component_id,quantity,unit_cost_at_time) VALUES (:rid,:cid,:qty,:uc)");
    q.bindValue(":rid",m.repairId); q.bindValue(":cid",m.componentId);
    q.bindValue(":qty",m.quantity); q.bindValue(":uc",m.unitCostAtTime);
    return q.exec();
}
