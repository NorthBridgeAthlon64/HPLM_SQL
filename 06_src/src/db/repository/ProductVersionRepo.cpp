#include "ProductVersionRepo.h"
#include <QSqlError>
#include <QDebug>
ProductVersionRepo::ProductVersionRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
ProductVersion ProductVersionRepo::rowToVersion(const QSqlQuery &q) {
    ProductVersion v;
    v.versionId=q.value("version_id").toInt(); v.productId=q.value("product_id").toInt();
    v.versionNumber=q.value("version_number").toString(); v.versionTitle=q.value("version_title").toString();
    v.versionNote=q.value("version_note").toString(); v.releaseDate=q.value("release_date").toDate();
    v.status=q.value("status").toString(); v.releaseEngineerId=q.value("release_engineer_id").toInt();
    v.totalDevelopmentCost=q.value("total_development_cost").toDouble();
    v.estimatedUnitCost=q.value("estimated_unit_cost").toDouble();
    v.recommendedPrice=q.value("recommended_price").toDouble();
    v.productName=q.value("product_name").toString(); v.engineerName=q.value("engineer_name").toString();
    return v;
}
QList<ProductVersion> ProductVersionRepo::findByProduct(int pid) {
    QList<ProductVersion> list; QSqlQuery q(m_db);
    q.prepare("SELECT pv.*, p.name AS product_name, COALESCE(e.name,'') AS engineer_name FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id LEFT JOIN Employee e ON pv.release_engineer_id=e.employee_id WHERE pv.product_id=:pid ORDER BY pv.version_number DESC");
    q.bindValue(":pid",pid);
    if(q.exec()) while(q.next()) list.append(rowToVersion(q));
    return list;
}
ProductVersion ProductVersionRepo::findById(int id) {
    QSqlQuery q(m_db);
    q.prepare("SELECT pv.*, p.name AS product_name, COALESCE(e.name,'') AS engineer_name FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id LEFT JOIN Employee e ON pv.release_engineer_id=e.employee_id WHERE pv.version_id=:id");
    q.bindValue(":id",id); if(q.exec()&&q.next()) return rowToVersion(q); return ProductVersion();
}
bool ProductVersionRepo::insert(const ProductVersion &v) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO ProductVersion (product_id,version_number,version_title,version_note,release_date,status,release_engineer_id,total_development_cost,estimated_unit_cost,recommended_price) VALUES (:pid,:vn,:vt,:vnote,:rd,:st,:reid,:tdc,:euc,:rp)");
    q.bindValue(":pid",v.productId); q.bindValue(":vn",v.versionNumber); q.bindValue(":vt",v.versionTitle);
    q.bindValue(":vnote",v.versionNote); q.bindValue(":rd",v.releaseDate);
    q.bindValue(":st",v.status); q.bindValue(":reid",v.releaseEngineerId>0?v.releaseEngineerId:QVariant());
    q.bindValue(":tdc",v.totalDevelopmentCost); q.bindValue(":euc",v.estimatedUnitCost>0?v.estimatedUnitCost:QVariant());
    q.bindValue(":rp",v.recommendedPrice>0?v.recommendedPrice:QVariant());
    if(!q.exec()){qWarning()<<"ProductVersionRepo::insert"<<q.lastError().text();return false;}
    return true;
}
bool ProductVersionRepo::update(const ProductVersion &v) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE ProductVersion SET version_number=:vn,version_title=:vt,version_note=:vnote,release_date=:rd,status=:st,release_engineer_id=:reid,total_development_cost=:tdc,estimated_unit_cost=:euc,recommended_price=:rp WHERE version_id=:id");
    q.bindValue(":id",v.versionId); q.bindValue(":vn",v.versionNumber); q.bindValue(":vt",v.versionTitle);
    q.bindValue(":vnote",v.versionNote); q.bindValue(":rd",v.releaseDate); q.bindValue(":st",v.status);
    q.bindValue(":reid",v.releaseEngineerId>0?v.releaseEngineerId:QVariant());
    q.bindValue(":tdc",v.totalDevelopmentCost); q.bindValue(":euc",v.estimatedUnitCost>0?v.estimatedUnitCost:QVariant());
    q.bindValue(":rp",v.recommendedPrice>0?v.recommendedPrice:QVariant());
    return q.exec();
}
bool ProductVersionRepo::remove(int id) { QSqlQuery q(m_db); q.prepare("DELETE FROM ProductVersion WHERE version_id=:id"); q.bindValue(":id",id); return q.exec(); }
