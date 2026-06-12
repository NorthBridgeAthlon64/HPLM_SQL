#include "ProductFeedbackRepo.h"
#include <QSqlError>
ProductFeedbackRepo::ProductFeedbackRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}
ProductFeedback ProductFeedbackRepo::rowToFeedback(const QSqlQuery &q) {
    ProductFeedback f;
    f.feedbackId=q.value("feedback_id").toInt(); f.versionId=q.value("version_id").toInt();
    f.customerId=q.value("customer_id").toInt(); f.rating=q.value("rating").toInt();
    f.comment=q.value("comment").toString(); f.feedbackDate=q.value("feedback_date").toDate();
    f.status=q.value("status").toString();
    f.customerName=q.value("customer_name").toString(); f.versionNumber=q.value("version_number").toString();
    return f;
}
QList<ProductFeedback> ProductFeedbackRepo::findByVersion(int versionId) {
    QList<ProductFeedback> list; QSqlQuery q(m_db);
    q.prepare("SELECT pf.*, c.name AS customer_name, pv.version_number FROM ProductFeedback pf JOIN Customer c ON pf.customer_id=c.customer_id JOIN ProductVersion pv ON pf.version_id=pv.version_id WHERE pf.version_id=:vid AND pf.status='approved' ORDER BY pf.feedback_date DESC");
    q.bindValue(":vid",versionId);
    if(q.exec()) while(q.next()) list.append(rowToFeedback(q)); return list;
}
QList<ProductFeedback> ProductFeedbackRepo::findByCustomer(int customerId) {
    QList<ProductFeedback> list; QSqlQuery q(m_db);
    q.prepare("SELECT pf.*, c.name AS customer_name, pv.version_number FROM ProductFeedback pf JOIN Customer c ON pf.customer_id=c.customer_id JOIN ProductVersion pv ON pf.version_id=pv.version_id WHERE pf.customer_id=:cid");
    q.bindValue(":cid",customerId);
    if(q.exec()) while(q.next()) list.append(rowToFeedback(q)); return list;
}
QList<ProductFeedback> ProductFeedbackRepo::lowRated(double threshold) {
    QList<ProductFeedback> list; QSqlQuery q(m_db);
    q.prepare("SELECT pf.*, c.name AS customer_name, pv.version_number FROM ProductFeedback pf JOIN Customer c ON pf.customer_id=c.customer_id JOIN ProductVersion pv ON pf.version_id=pv.version_id WHERE pf.status='approved' AND pf.rating < :th ORDER BY pf.feedback_date DESC");
    q.bindValue(":th",threshold);
    if(q.exec()) while(q.next()) list.append(rowToFeedback(q)); return list;
}
bool ProductFeedbackRepo::insert(const ProductFeedback &f) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO ProductFeedback (version_id,customer_id,rating,comment,feedback_date,status) VALUES (:vid,:cid,:r,:cm,:fd,:st)");
    q.bindValue(":vid",f.versionId); q.bindValue(":cid",f.customerId); q.bindValue(":r",f.rating);
    q.bindValue(":cm",f.comment); q.bindValue(":fd",f.feedbackDate); q.bindValue(":st",f.status);
    return q.exec();
}
bool ProductFeedbackRepo::updateStatus(int feedbackId, const QString &status) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE ProductFeedback SET status=:st WHERE feedback_id=:id");
    q.bindValue(":st",status); q.bindValue(":id",feedbackId); return q.exec();
}
