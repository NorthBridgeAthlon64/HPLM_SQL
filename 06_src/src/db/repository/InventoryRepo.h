#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct Inventory {
    int     inventoryId = 0;
    int     componentId = 0;
    int     warehouseId = 0;
    int     quantity = 0;
    int     reservedQuantity = 0;
    double  avgCost = 0;
};

class InventoryRepo : public QObject {
    Q_OBJECT
public:
    explicit InventoryRepo(QSqlDatabase &db, QObject *p = nullptr);
    Inventory findByComponentAndWarehouse(int componentId, int warehouseId);
    bool addQuantity(int componentId, int warehouseId, int delta, double unitCost = 0);
    bool reserveQuantity(int componentId, int warehouseId, int qty);
private:
    QSqlDatabase &m_db;
};
