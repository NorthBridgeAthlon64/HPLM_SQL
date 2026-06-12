#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct RepairRecord {
    int     repairId = 0;
    QString repairNo;
    int     versionId = 0;
    int     batchId = 0;
    int     customerId = 0;
    QDate   receiveDate;
    QDate   completeDate;
    QString faultDescription;
    QString repairStatus;
    int     repairmanId = 0;
    double  repairCost = 0;
    // 关联
    QString customerName;
    QString versionNumber;
};

class RepairRecordRepo : public QObject {
    Q_OBJECT
public:
    explicit RepairRecordRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<RepairRecord> findAll();
    RepairRecord findByRepairNo(const QString &repairNo);
    RepairRecord findById(int id);
    int pendingCount();
    bool insert(RepairRecord &r);
    bool updateStatus(int repairId, const QString &status, const QDate &completeDate = QDate());
    bool addCost(int repairId, double amount);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    RepairRecord rowToRepair(const QSqlQuery &q);
};
