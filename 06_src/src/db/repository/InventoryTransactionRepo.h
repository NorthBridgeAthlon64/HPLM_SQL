#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct InventoryTrans {
    int     transactionId = 0;
    QString transactionNo;
    int     componentId = 0;
    int     warehouseId = 0;
    QString transactionType;
    int     quantity = 0;
    double  unitPrice = 0;
    double  totalPrice = 0;
    QDate   transactionDate;
    QString referenceNo;
    int     operatorId = 0;
    QString componentName;
    QString warehouseName;
};

class InventoryTransactionRepo : public QObject {
    Q_OBJECT
public:
    explicit InventoryTransactionRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<InventoryTrans> findByComponent(int componentId);
    QList<InventoryTrans> findAll();
    bool insert(const InventoryTrans &t);
private:
    QSqlDatabase &m_db;
    InventoryTrans rowToTrans(const QSqlQuery &q);
};
