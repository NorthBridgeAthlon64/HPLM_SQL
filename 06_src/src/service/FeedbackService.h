#pragma once
#include <QObject>
#include "db/repository/ProductFeedbackRepo.h"

class FeedbackService : public QObject {
    Q_OBJECT
public:
    explicit FeedbackService(QSqlDatabase &db, QObject *p = nullptr);
    /// 提交评价（校验评分 1-5，防重复）
    bool submitFeedback(const ProductFeedback &fb, QString &errorMsg);
    bool approveFeedback(int feedbackId);
    bool rejectFeedback(int feedbackId);

private:
    ProductFeedbackRepo m_repo;
};
