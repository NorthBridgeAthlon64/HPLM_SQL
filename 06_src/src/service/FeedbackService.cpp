#include "FeedbackService.h"
#include <QDebug>

FeedbackService::FeedbackService(QSqlDatabase &db, QObject *p)
    : QObject(p), m_repo(db) {}

bool FeedbackService::submitFeedback(const ProductFeedback &fb, QString &errorMsg)
{
    if (fb.rating < 1 || fb.rating > 5) {
        errorMsg = "评分必须在 1-5 之间";
        return false;
    }
    if (fb.comment.trimmed().isEmpty()) {
        errorMsg = "评价内容不能为空";
        return false;
    }
    // 防重复：检查是否已提交
    QList<ProductFeedback> existing = m_repo.findByCustomer(fb.customerId);
    for (const ProductFeedback &f : existing) {
        if (f.versionId == fb.versionId) {
            errorMsg = "您已对该版本提交过评价，不可重复评价";
            return false;
        }
    }
    return m_repo.insert(fb);
}

bool FeedbackService::approveFeedback(int feedbackId)
{
    return m_repo.updateStatus(feedbackId, "approved");
}

bool FeedbackService::rejectFeedback(int feedbackId)
{
    return m_repo.updateStatus(feedbackId, "rejected");
}
