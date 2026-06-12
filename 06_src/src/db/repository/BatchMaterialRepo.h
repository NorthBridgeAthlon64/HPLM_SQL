#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct BatchMaterial {
    int     batchMaterialId = 0;
    int     batchId = 0;
    int     componentId = 0;
    int     usedQuantity = 0;
    double  unitCostAtTime = 0;
    double  totalCost = 0;
    QString componentCode;
    QString componentName;
    QString unit;
};

class BatchMaterialRepo : public QObject {
    Q_OBJECT
public:
    explicit BatchMaterialRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<BatchMaterial> findByBatch(int batchId);
    bool insert(const BatchMaterial &m);
private:
    QSqlDatabase &m_db;
    BatchMaterial rowToMaterial(const QSqlQuery &q);
};
