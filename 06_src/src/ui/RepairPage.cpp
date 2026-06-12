#include "RepairPage.h"
#include "db/repository/RepairRecordRepo.h"
#include "db/repository/RepairMaterialRepo.h"
#include "db/repository/CustomerRepo.h"
#include "db/repository/ProductVersionRepo.h"
#include "db/repository/ProductRepo.h"
#include "db/repository/ComponentRepo.h"
#include "service/RepairService.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDate>

RepairPage::RepairPage(QSqlDatabase &db, QWidget *p)
    : QWidget(p), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    auto *toolbar = new QHBoxLayout();
    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"));
    auto *createBtn  = new QPushButton(QStringLiteral("创建工单"));
    auto *matBtn     = new QPushButton(QStringLiteral("添加换料"));
    auto *doneBtn    = new QPushButton(QStringLiteral("完成维修"));
    toolbar->addWidget(refreshBtn);
    toolbar->addWidget(createBtn);
    toolbar->addWidget(matBtn);
    toolbar->addWidget(doneBtn);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    m_repairTable = new QTableWidget(0, 7, this);
    m_repairTable->setHorizontalHeaderLabels({
        "ID", QStringLiteral("单号"), QStringLiteral("客户"), QStringLiteral("版本"),
        QStringLiteral("接修日期"), QStringLiteral("状态"), QStringLiteral("费用")
    });
    m_repairTable->horizontalHeader()->setStretchLastSection(true);
    m_repairTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_repairTable->setColumnHidden(0, true);
    layout->addWidget(m_repairTable);

    m_matTable = new QTableWidget(0, 4, this);
    m_matTable->setHorizontalHeaderLabels({QStringLiteral("元器件"), QStringLiteral("编码"), QStringLiteral("数量"), QStringLiteral("单价")});
    layout->addWidget(m_matTable);

    connect(refreshBtn, &QPushButton::clicked, this, &RepairPage::refresh);
    connect(createBtn, &QPushButton::clicked, this, &RepairPage::onCreateRepair);
    connect(matBtn, &QPushButton::clicked, this, &RepairPage::onAddMaterial);
    connect(doneBtn, &QPushButton::clicked, this, &RepairPage::onComplete);
    connect(m_repairTable, &QTableWidget::cellClicked, this, &RepairPage::onRepairClicked);

    refresh();
}

void RepairPage::refresh()
{
    RepairRecordRepo repo(m_db);
    QList<RepairRecord> list = repo.findAll();
    m_repairTable->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const auto &r = list[i];
        m_repairTable->insertRow(i);
        m_repairTable->setItem(i, 0, new QTableWidgetItem(QString::number(r.repairId)));
        m_repairTable->setItem(i, 1, new QTableWidgetItem(r.repairNo));
        m_repairTable->setItem(i, 2, new QTableWidgetItem(r.customerName));
        m_repairTable->setItem(i, 3, new QTableWidgetItem(r.versionNumber));
        m_repairTable->setItem(i, 4, new QTableWidgetItem(r.receiveDate.toString("yyyy-MM-dd")));
        m_repairTable->setItem(i, 5, new QTableWidgetItem(r.repairStatus));
        m_repairTable->setItem(i, 6, new QTableWidgetItem(QString::number(r.repairCost, 'f', 2)));
    }
}

void RepairPage::onCreateRepair()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("创建维修工单"));
    auto *f = new QFormLayout(&dlg);

    auto *rnEdit   = new QLineEdit();
    auto *custCombo = new QComboBox();
    for (const Customer &c : CustomerRepo(m_db).findAll())
        custCombo->addItem(c.name, c.customerId);
    auto *verCombo = new QComboBox();
    for (const Product &p : ProductRepo(m_db).findAll())
        for (const ProductVersion &v : ProductVersionRepo(m_db).findByProduct(p.productId))
            verCombo->addItem(QStringLiteral("%1 %2").arg(p.name, v.versionNumber), v.versionId);
    auto *faultEdit = new QLineEdit();

    f->addRow(QStringLiteral("维修单号："), rnEdit);
    f->addRow(QStringLiteral("客户："), custCombo);
    f->addRow(QStringLiteral("版本："), verCombo);
    f->addRow(QStringLiteral("故障描述："), faultEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    RepairRecord r;
    r.repairNo   = rnEdit->text().trimmed();
    r.customerId = custCombo->currentData().toInt();
    r.versionId  = verCombo->currentData().toInt();
    r.faultDescription = faultEdit->text().trimmed();
    r.receiveDate = QDate::currentDate();
    r.repairStatus = "received";

    RepairRecordRepo repo(m_db);
    if (repo.insert(r)) refresh();
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("创建失败"));
}

void RepairPage::onAddMaterial()
{
    if (!m_selectedRepairId) { QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择工单")); return; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("添加维修换料"));
    auto *f = new QFormLayout(&dlg);
    auto *compCombo = new QComboBox();
    for (const Component &c : ComponentRepo(m_db).findAll())
        compCombo->addItem(QStringLiteral("%1 %2").arg(c.componentCode, c.name), c.componentId);
    auto *qtySpin   = new QSpinBox(); qtySpin->setRange(1, 9999);
    auto *priceSpin = new QDoubleSpinBox(); priceSpin->setRange(0, 999999); priceSpin->setDecimals(2);

    f->addRow(QStringLiteral("元器件："), compCombo);
    f->addRow(QStringLiteral("数量："), qtySpin);
    f->addRow(QStringLiteral("单价："), priceSpin);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    RepairService svc(m_db);
    if (svc.addRepairMaterial(m_selectedRepairId, compCombo->currentData().toInt(), 1,
            qtySpin->value(), priceSpin->value(), 1)) {
        onRepairClicked(m_repairTable->currentRow(), 0);
        refresh();
    } else {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("库存不足"));
    }
}

void RepairPage::onComplete()
{
    if (!m_selectedRepairId) return;
    RepairService svc(m_db);
    if (svc.completeRepair(m_selectedRepairId)) refresh();
}

void RepairPage::onRepairClicked(int row, int)
{
    int rid = m_repairTable->item(row, 0)->text().toInt();
    m_selectedRepairId = rid;

    RepairMaterialRepo repo(m_db);
    QList<RepairMaterial> list = repo.findByRepair(rid);
    m_matTable->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const auto &m = list[i];
        m_matTable->insertRow(i);
        m_matTable->setItem(i, 0, new QTableWidgetItem(m.componentName));
        m_matTable->setItem(i, 1, new QTableWidgetItem(m.componentCode));
        m_matTable->setItem(i, 2, new QTableWidgetItem(QString::number(m.quantity)));
        m_matTable->setItem(i, 3, new QTableWidgetItem(QString::number(m.unitCostAtTime, 'f', 2)));
    }
}
