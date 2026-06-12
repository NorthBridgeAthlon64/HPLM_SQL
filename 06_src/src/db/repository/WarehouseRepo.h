#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct Warehouse {
    int     warehouseId = 0;
    QString warehouseCode;
    QString name;
    QString location;
};

class WarehouseRepo : public QObject {
    Q_OBJECT
public:
    explicit WarehouseRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<Warehouse> findAll();
    Warehouse findById(int id);
    bool insert(const Warehouse &w);
    bool update(const Warehouse &w);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    Warehouse rowToWarehouse(const QSqlQuery &q);
};
