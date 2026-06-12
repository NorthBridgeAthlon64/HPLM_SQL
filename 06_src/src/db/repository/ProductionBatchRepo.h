#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct ProductionBatch {
    int     batchId = 0;
    QString batchNo;
    int     versionId = 0;
    int     quantity = 0;
    QDate   productionDate;
    int     testerId = 0;
    QString qualityStatus;
    double  batchMaterialCost = 0;
    double  batchLaborCost = 0;
    double  batchTotalCost = 0;
    QString versionNumber;
    QString productName;
};

class ProductionBatchRepo : public QObject {
    Q_OBJECT
public:
    explicit ProductionBatchRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<ProductionBatch> findAll();
    ProductionBatch findById(int id);
    ProductionBatch findByBatchNo(const QString &batchNo);
    bool insert(ProductionBatch &b);
    bool updateStatus(int batchId, const QString &status);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    ProductionBatch rowToBatch(const QSqlQuery &q);
};
