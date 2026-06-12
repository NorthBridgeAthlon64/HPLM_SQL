#include "ProductRepo.h"
#include <QSqlError>
ProductRepo::ProductRepo(QSqlDatabase &db) : m_db(db) {}
Product ProductRepo::rowToProduct(const QSqlQuery &q) {
    Product p;
    p.productId=q.value("product_id").toInt(); p.productCode=q.value("product_code").toString();
    p.name=q.value("name").toString(); p.description=q.value("description").toString();
    p.productManagerId=q.value("product_manager_id").toInt(); p.designerId=q.value("designer_id").toInt();
    p.managerName=q.value("manager_name").toString(); p.designerName=q.value("designer_name").toString();
    return p;
}
QList<Product> ProductRepo::findAll() {
    QList<Product> list; QSqlQuery q(m_db);
    q.exec("SELECT p.*, COALESCE(m.name,'') AS manager_name, COALESCE(d.name,'') AS designer_name FROM Product p LEFT JOIN Employee m ON p.product_manager_id=m.employee_id LEFT JOIN Employee d ON p.designer_id=d.employee_id ORDER BY p.product_id");
    while(q.next()) list.append(rowToProduct(q)); return list;
}
Product ProductRepo::findById(int id) {
    QSqlQuery q(m_db); q.prepare("SELECT p.*, COALESCE(m.name,'') AS manager_name, COALESCE(d.name,'') AS designer_name FROM Product p LEFT JOIN Employee m ON p.product_manager_id=m.employee_id LEFT JOIN Employee d ON p.designer_id=d.employee_id WHERE p.product_id=:id"); q.bindValue(":id",id);
    if(q.exec()&&q.next()) return rowToProduct(q); return Product();
}
bool ProductRepo::insert(const Product &p) {
    QSqlQuery q(m_db); q.prepare("INSERT INTO Product (product_code,name,description,product_manager_id,designer_id) VALUES (:code,:name,:desc,:pm,:ds)");
    q.bindValue(":code",p.productCode); q.bindValue(":name",p.name); q.bindValue(":desc",p.description); q.bindValue(":pm",p.productManagerId>0?p.productManagerId:QVariant()); q.bindValue(":ds",p.designerId>0?p.designerId:QVariant());
    return q.exec();
}
bool ProductRepo::update(const Product &p) {
    QSqlQuery q(m_db); q.prepare("UPDATE Product SET product_code=:code,name=:name,description=:desc,product_manager_id=:pm,designer_id=:ds WHERE product_id=:id");
    q.bindValue(":code",p.productCode); q.bindValue(":name",p.name); q.bindValue(":desc",p.description); q.bindValue(":pm",p.productManagerId>0?p.productManagerId:QVariant()); q.bindValue(":ds",p.designerId>0?p.designerId:QVariant()); q.bindValue(":id",p.productId);
    return q.exec();
}
bool ProductRepo::remove(int id) { QSqlQuery q(m_db); q.prepare("DELETE FROM Product WHERE product_id=:id"); q.bindValue(":id",id); return q.exec(); }
