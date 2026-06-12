#include "ProductionBatchPage.h"
#include "db/repository/ProductionBatchRepo.h"
#include "db/repository/ProductVersionRepo.h"
#include "db/repository/BatchMaterialRepo.h"
#include "db/repository/TestRecordRepo.h"
#include "service/ProductionService.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

ProductionBatchPage::ProductionBatchPage(QSqlDatabase &db, QWidget *p)
    : QWidget(p), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    auto *toolbar = new QHBoxLayout();
    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"));
    auto *createBtn  = new QPushButton(QStringLiteral("创建批次"));
    auto *testBtn    = new QPushButton(QStringLiteral("添加测试"));
    toolbar->addWidget(refreshBtn);
    toolbar->addWidget(createBtn);
    toolbar->addWidget(testBtn);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    m_batchTable = new QTableWidget(0, 7, this);
    m_batchTable->setHorizontalHeaderLabels({
        "ID", QStringLiteral("批次号"), QStringLiteral("产品"), QStringLiteral("版本"),
        QStringLiteral("投产数量"), QStringLiteral("质量状态"), QStringLiteral("生产日期")
    });
    m_batchTable->horizontalHeader()->setStretchLastSection(true);
    m_batchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_batchTable->setColumnHidden(0, true);
    layout->addWidget(m_batchTable);

    m_detailTable = new QTableWidget(0, 4, this);
    m_detailTable->setHorizontalHeaderLabels({QStringLiteral("元器件"), QStringLiteral("编码"), QStringLiteral("用量"), QStringLiteral("单价")});
    layout->addWidget(m_detailTable);

    m_testTable = new QTableWidget(0, 4, this);
    m_testTable->setHorizontalHeaderLabels({QStringLiteral("测试项目"), QStringLiteral("结果"), QStringLiteral("日期"), QStringLiteral("数据")});
    layout->addWidget(m_testTable);

    connect(refreshBtn, &QPushButton::clicked, this, &ProductionBatchPage::refresh);
    connect(createBtn, &QPushButton::clicked, this, &ProductionBatchPage::onCreateBatch);
    connect(testBtn, &QPushButton::clicked, this, &ProductionBatchPage::onAddTest);
    connect(m_batchTable, &QTableWidget::cellClicked, this, &ProductionBatchPage::onBatchClicked);

    refresh();
}

void ProductionBatchPage::refresh()
{
    ProductionBatchRepo repo(m_db);
    QList<ProductionBatch> list = repo.findAll();
    m_batchTable->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const auto &b = list[i];
        m_batchTable->insertRow(i);
        m_batchTable->setItem(i, 0, new QTableWidgetItem(QString::number(b.batchId)));
        m_batchTable->setItem(i, 1, new QTableWidgetItem(b.batchNo));
        m_batchTable->setItem(i, 2, new QTableWidgetItem(b.productName));
        m_batchTable->setItem(i, 3, new QTableWidgetItem(b.versionNumber));
        m_batchTable->setItem(i, 4, new QTableWidgetItem(QString::number(b.quantity)));
        m_batchTable->setItem(i, 5, new QTableWidgetItem(b.qualityStatus));
        m_batchTable->setItem(i, 6, new QTableWidgetItem(b.productionDate.toString("yyyy-MM-dd")));
    }
}

void ProductionBatchPage::onCreateBatch()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("创建生产批次"));
    auto *f = new QFormLayout(&dlg);

    auto *verCombo = new QComboBox();
    ProductVersionRepo verRepo(m_db);
    for (const Product &p : ProductRepo(m_db).findAll())
        for (const ProductVersion &v : verRepo.findByProduct(p.productId))
            verCombo->addItem(QStringLiteral("%1 %2").arg(p.name, v.versionNumber), v.versionId);

    auto *bnEdit = new QLineEdit();
    auto *qtySpin = new QSpinBox(); qtySpin->setRange(1, 99999); qtySpin->setValue(1);

    f->addRow(QStringLiteral("产品版本："), verCombo);
    f->addRow(QStringLiteral("批次号："), bnEdit);
    f->addRow(QStringLiteral("投产数量："), qtySpin);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    ProductionBatch b;
    b.batchNo   = bnEdit->text().trimmed();
    b.versionId = verCombo->currentData().toInt();
    b.quantity  = qtySpin->value();

    ProductionService svc(m_db);
    QStringList shortages;
    if (svc.createBatch(b, shortages)) {
        QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("生产批次创建成功"));
        refresh();
    } else {
        QMessageBox::warning(this, QStringLiteral("失败"),
            QStringLiteral("创建失败，缺料:\n%1").arg(shortages.join("\n")));
    }
}

void ProductionBatchPage::onAddTest()
{
    if (!m_selectedBatchId) { QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择批次")); return; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("添加测试记录"));
    auto *f = new QFormLayout(&dlg);
    auto *itemEdit  = new QLineEdit();
    auto *resultCombo = new QComboBox(); resultCombo->addItems({"pass", "fail", "retest"});
    auto *dataEdit  = new QLineEdit();
    f->addRow(QStringLiteral("测试项目："), itemEdit);
    f->addRow(QStringLiteral("结果："), resultCombo);
    f->addRow(QStringLiteral("数据："), dataEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    TestRecord t;
    t.batchId    = m_selectedBatchId;
    t.testerId   = 1;
    t.testDate   = QDate::currentDate();
    t.testItem   = itemEdit->text().trimmed();
    t.testResult = resultCombo->currentText();
    t.testData   = dataEdit->text().trimmed();

    TestRecordRepo repo(m_db);
    if (repo.insert(t)) {
        ProductionBatchRepo bRepo(m_db);
        bRepo.updateStatus(m_selectedBatchId, t.testResult == "pass" ? "qualified" : "defective");
        onBatchClicked(m_batchTable->currentRow(), 0);
    }
}

void ProductionBatchPage::onBatchClicked(int row, int)
{
    int bid = m_batchTable->item(row, 0)->text().toInt();
    m_selectedBatchId = bid;

    BatchMaterialRepo matRepo(m_db);
    QList<BatchMaterial> mats = matRepo.findByBatch(bid);
    m_detailTable->setRowCount(0);
    for (int i = 0; i < mats.size(); i++) {
        const auto &m = mats[i];
        m_detailTable->insertRow(i);
        m_detailTable->setItem(i, 0, new QTableWidgetItem(m.componentName));
        m_detailTable->setItem(i, 1, new QTableWidgetItem(m.componentCode));
        m_detailTable->setItem(i, 2, new QTableWidgetItem(QString::number(m.usedQuantity)));
        m_detailTable->setItem(i, 3, new QTableWidgetItem(QString::number(m.unitCostAtTime, 'f', 2)));
    }

    TestRecordRepo testRepo(m_db);
    QList<TestRecord> tests = testRepo.findByBatch(bid);
    m_testTable->setRowCount(0);
    for (int i = 0; i < tests.size(); i++) {
        const auto &t = tests[i];
        m_testTable->insertRow(i);
        m_testTable->setItem(i, 0, new QTableWidgetItem(t.testItem));
        m_testTable->setItem(i, 1, new QTableWidgetItem(t.testResult));
        m_testTable->setItem(i, 2, new QTableWidgetItem(t.testDate.toString("yyyy-MM-dd")));
        m_testTable->setItem(i, 3, new QTableWidgetItem(t.testData));
    }
}
