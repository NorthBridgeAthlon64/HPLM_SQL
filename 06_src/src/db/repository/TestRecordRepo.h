#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct TestRecord {
    int     testId = 0;
    int     batchId = 0;
    int     testerId = 0;
    QDate   testDate;
    QString testItem;
    QString testResult;
    QString testData;
};

class TestRecordRepo : public QObject {
    Q_OBJECT
public:
    explicit TestRecordRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<TestRecord> findByBatch(int batchId);
    bool insert(const TestRecord &t);
private:
    QSqlDatabase &m_db;
    TestRecord rowToRecord(const QSqlQuery &q);
};
