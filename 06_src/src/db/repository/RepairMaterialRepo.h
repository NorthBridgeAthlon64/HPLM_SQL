#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct RepairMaterial {
    int     repairMaterialId = 0;
    int     repairId = 0;
    int     componentId = 0;
    int     quantity = 1;
    double  unitCostAtTime = 0;
    double  totalCost = 0;
    QString componentCode;
    QString componentName;
};

class RepairMaterialRepo : public QObject {
    Q_OBJECT
public:
    explicit RepairMaterialRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<RepairMaterial> findByRepair(int repairId);
    bool insert(const RepairMaterial &m);
private:
    QSqlDatabase &m_db;
    RepairMaterial rowToMaterial(const QSqlQuery &q);
};
