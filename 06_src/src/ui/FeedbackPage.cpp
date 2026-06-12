#include "FeedbackPage.h"
#include "db/repository/ProductFeedbackRepo.h"
#include "db/repository/CustomerRepo.h"
#include "db/repository/ProductVersionRepo.h"
#include "db/repository/ProductRepo.h"
#include "service/FeedbackService.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDate>
#include <QPushButton>

FeedbackPage::FeedbackPage(QSqlDatabase &db, QWidget *p)
    : QWidget(p), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    auto *toolbar = new QHBoxLayout();
    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"));
    auto *submitBtn  = new QPushButton(QStringLiteral("提交评价"));
    auto *approveBtn = new QPushButton(QStringLiteral("审核通过"));
    auto *rejectBtn  = new QPushButton(QStringLiteral("审核拒绝"));

    m_filterCombo = new QComboBox();
    m_filterCombo->addItem(QStringLiteral("全部评价"), "all");
    m_filterCombo->addItem(QStringLiteral("低评分 (<3星)"), "low");
    toolbar->addWidget(refreshBtn);
    toolbar->addWidget(submitBtn);
    toolbar->addWidget(approveBtn);
    toolbar->addWidget(rejectBtn);
    toolbar->addWidget(m_filterCombo);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    m_feedbackTable = new QTableWidget(0, 7, this);
    m_feedbackTable->setHorizontalHeaderLabels({
        "ID", QStringLiteral("客户"), QStringLiteral("版本"), QStringLiteral("评分"),
        QStringLiteral("评论"), QStringLiteral("日期"), QStringLiteral("状态")
    });
    m_feedbackTable->horizontalHeader()->setStretchLastSection(true);
    m_feedbackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_feedbackTable->setColumnHidden(0, true);
    layout->addWidget(m_feedbackTable);

    connect(refreshBtn, &QPushButton::clicked, this, &FeedbackPage::refresh);
    connect(submitBtn, &QPushButton::clicked, this, &FeedbackPage::onSubmitFeedback);
    connect(approveBtn, &QPushButton::clicked, this, &FeedbackPage::onApprove);
    connect(rejectBtn, &QPushButton::clicked, this, &FeedbackPage::onReject);

    refresh();
}

void FeedbackPage::refresh()
{
    ProductFeedbackRepo repo(m_db);
    QList<ProductFeedback> list;
    if (m_filterCombo->currentData().toString() == "low")
        list = repo.lowRated(3);
    else {
        // 查全部：遍历所有customer和version
        ProductVersionRepo verRepo(m_db);
        QList<ProductVersion> vers = verRepo.findByProduct(1); // 简化
        for (const ProductVersion &v : vers) {
            list.append(repo.findByVersion(v.versionId));
        }
    }

    m_feedbackTable->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const auto &fb = list[i];
        m_feedbackTable->insertRow(i);
        m_feedbackTable->setItem(i, 0, new QTableWidgetItem(QString::number(fb.feedbackId)));
        m_feedbackTable->setItem(i, 1, new QTableWidgetItem(fb.customerName));
        m_feedbackTable->setItem(i, 2, new QTableWidgetItem(fb.versionNumber));
        m_feedbackTable->setItem(i, 3, new QTableWidgetItem(QString::number(fb.rating)));
        m_feedbackTable->setItem(i, 4, new QTableWidgetItem(fb.comment));
        m_feedbackTable->setItem(i, 5, new QTableWidgetItem(fb.feedbackDate.toString("yyyy-MM-dd")));
        m_feedbackTable->setItem(i, 6, new QTableWidgetItem(fb.status));
    }
}

void FeedbackPage::onSubmitFeedback()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("提交评价"));
    auto *f = new QFormLayout(&dlg);

    auto *custCombo = new QComboBox();
    for (const Customer &c : CustomerRepo(m_db).findAll())
        custCombo->addItem(c.name, c.customerId);
    auto *verCombo = new QComboBox();
    for (const Product &p : ProductRepo(m_db).findAll())
        for (const ProductVersion &v : ProductVersionRepo(m_db).findByProduct(p.productId))
            verCombo->addItem(QStringLiteral("%1 %2").arg(p.name, v.versionNumber), v.versionId);
    auto *ratingSpin = new QSpinBox(); ratingSpin->setRange(1, 5);
    auto *commentEdit = new QLineEdit();

    f->addRow(QStringLiteral("客户："), custCombo);
    f->addRow(QStringLiteral("版本："), verCombo);
    f->addRow(QStringLiteral("评分(1-5)："), ratingSpin);
    f->addRow(QStringLiteral("评价内容："), commentEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    ProductFeedback fb;
    fb.versionId  = verCombo->currentData().toInt();
    fb.customerId = custCombo->currentData().toInt();
    fb.rating     = ratingSpin->value();
    fb.comment    = commentEdit->text().trimmed();
    fb.feedbackDate = QDate::currentDate();
    fb.status = "pending";

    FeedbackService svc(m_db);
    QString err;
    if (svc.submitFeedback(fb, err)) refresh();
    else QMessageBox::warning(this, QStringLiteral("失败"), err);
}

void FeedbackPage::onApprove()
{
    int row = m_feedbackTable->currentRow();
    if (row < 0) return;
    FeedbackService svc(m_db);
    svc.approveFeedback(m_feedbackTable->item(row, 0)->text().toInt());
    refresh();
}

void FeedbackPage::onReject()
{
    int row = m_feedbackTable->currentRow();
    if (row < 0) return;
    FeedbackService svc(m_db);
    svc.rejectFeedback(m_feedbackTable->item(row, 0)->text().toInt());
    refresh();
}
