#include "TestRecordRepo.h"
#include <QSqlError>
TestRecordRepo::TestRecordRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
TestRecord TestRecordRepo::rowToRecord(const QSqlQuery &q) {
    TestRecord r;
    r.testId=q.value("test_id").toInt(); r.batchId=q.value("batch_id").toInt();
    r.testerId=q.value("tester_id").toInt(); r.testDate=q.value("test_date").toDate();
    r.testItem=q.value("test_item").toString(); r.testResult=q.value("test_result").toString();
    r.testData=q.value("test_data").toString();
    return r;
}
QList<TestRecord> TestRecordRepo::findByBatch(int batchId) {
    QList<TestRecord> list; QSqlQuery q(m_db);
    q.prepare("SELECT * FROM TestRecord WHERE batch_id=:bid ORDER BY test_date");
    q.bindValue(":bid",batchId);
    if(q.exec()) while(q.next()) list.append(rowToRecord(q));
    return list;
}
bool TestRecordRepo::insert(const TestRecord &t) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO TestRecord (batch_id,tester_id,test_date,test_item,test_result,test_data) VALUES (:bid,:tid,:td,:ti,:tr,:data)");
    q.bindValue(":bid",t.batchId); q.bindValue(":tid",t.testerId); q.bindValue(":td",t.testDate);
    q.bindValue(":ti",t.testItem); q.bindValue(":tr",t.testResult); q.bindValue(":data",t.testData);
    return q.exec();
}
