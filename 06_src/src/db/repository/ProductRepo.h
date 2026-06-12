#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
struct Product { int productId=0; QString productCode; QString name; QString description; int productManagerId=0; int designerId=0; QString managerName; QString designerName; };
class ProductRepo : public QObject { Q_OBJECT public:
explicit ProductRepo(QSqlDatabase &db, QObject *p=nullptr);
QList<Product> findAll();
Product findById(int id);
bool insert(const Product &p);
bool update(const Product &p);
bool remove(int id);
private: QSqlDatabase &m_db; Product rowToProduct(const QSqlQuery &q);
};
