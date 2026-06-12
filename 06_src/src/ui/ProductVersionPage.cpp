#include "ProductVersionPage.h"
#include "db/repository/ProductRepo.h"
#include "db/repository/ProductVersionRepo.h"
#include "db/repository/ProductBOMRepo.h"
#include "db/repository/ComponentRepo.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

ProductVersionPage::ProductVersionPage(QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    // 工具栏
    auto *toolbar = new QHBoxLayout();
    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"));
    auto *newVerBtn   = new QPushButton(QStringLiteral("新建版本"));
    auto *addBomBtn   = new QPushButton(QStringLiteral("添加BOM"));
    auto *editBomBtn  = new QPushButton(QStringLiteral("修改用量"));
    auto *delBomBtn   = new QPushButton(QStringLiteral("删除BOM行"));
    toolbar->addWidget(refreshBtn);
    toolbar->addWidget(newVerBtn);
    toolbar->addWidget(addBomBtn);
    toolbar->addWidget(editBomBtn);
    toolbar->addWidget(delBomBtn);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    // QSplitter：左产品树 + 右 BOM 表
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // 左：产品→版本树
    m_tree = new QTreeWidget();
    m_tree->setHeaderLabels({QStringLiteral("产品 / 版本")});
    m_tree->setMinimumWidth(200);
    m_splitter->addWidget(m_tree);

    // 右：BOM 表
    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_versionInfoLabel = new QLabel(QStringLiteral("请选择左侧版本查看BOM"));
    m_versionInfoLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    rightLayout->addWidget(m_versionInfoLabel);

    m_bomTable = new QTableWidget(0, 6, this);
    m_bomTable->setHorizontalHeaderLabels({
        "BOM ID", QStringLiteral("元器件编码"), QStringLiteral("名称"),
        QStringLiteral("规格"), QStringLiteral("用量"), QStringLiteral("PCB位置")
    });
    m_bomTable->horizontalHeader()->setStretchLastSection(true);
    m_bomTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bomTable->setColumnHidden(0, true);
    rightLayout->addWidget(m_bomTable);

    m_costLabel = new QLabel();
    rightLayout->addWidget(m_costLabel);

    m_splitter->addWidget(rightWidget);
    layout->addWidget(m_splitter);

    connect(refreshBtn, &QPushButton::clicked, this, &ProductVersionPage::refresh);
    connect(newVerBtn, &QPushButton::clicked, this, &ProductVersionPage::onNewVersion);
    connect(addBomBtn, &QPushButton::clicked, this, &ProductVersionPage::onAddBOM);
    connect(editBomBtn, &QPushButton::clicked, this, &ProductVersionPage::onEditBOM);
    connect(delBomBtn, &QPushButton::clicked, this, &ProductVersionPage::onDeleteBOM);
    connect(m_tree, &QTreeWidget::itemClicked, this, &ProductVersionPage::onTreeItemClicked);

    refresh();
}

void ProductVersionPage::refresh()
{
    m_tree->clear();
    ProductRepo prodRepo(m_db);
    ProductVersionRepo verRepo(m_db);

    for (const Product &p : prodRepo.findAll()) {
        auto *prodItem = new QTreeWidgetItem(m_tree);
        prodItem->setText(0, QStringLiteral("%1 %2").arg(p.productCode, p.name));
        prodItem->setData(0, Qt::UserRole, -p.productId); // 产品用负数区分

        for (const ProductVersion &v : verRepo.findByProduct(p.productId)) {
            auto *verItem = new QTreeWidgetItem(prodItem);
            verItem->setText(0, QStringLiteral("%1 [%2] %3").arg(v.versionNumber, v.status, v.versionTitle));
            verItem->setData(0, Qt::UserRole, v.versionId);
        }
    }
    m_tree->expandAll();
}

void ProductVersionPage::onTreeItemClicked(QTreeWidgetItem *item, int)
{
    int vid = item->data(0, Qt::UserRole).toInt();
    if (vid <= 0) { m_bomTable->setRowCount(0); return; } // 产品节点忽略

    m_selectedVersionId = vid;

    ProductVersionRepo verRepo(m_db);
    ProductVersion v = verRepo.findById(vid);
    m_versionInfoLabel->setText(QStringLiteral("版本: %1 %2 [%3] - 状态:%4")
        .arg(v.productName, v.versionNumber, v.versionTitle, v.status));

    loadBOM(vid);
}

void ProductVersionPage::loadBOM(int versionId)
{
    ProductBOMRepo bomRepo(m_db);
    QList<BOMItem> list = bomRepo.findByVersion(versionId);

    m_bomTable->setRowCount(0);
    double totalCost = 0;
    for (int i = 0; i < list.size(); i++) {
        const BOMItem &b = list[i];
        m_bomTable->insertRow(i);
        m_bomTable->setItem(i, 0, new QTableWidgetItem(QString::number(b.bomId)));
        m_bomTable->setItem(i, 1, new QTableWidgetItem(b.componentCode));
        m_bomTable->setItem(i, 2, new QTableWidgetItem(b.componentName));
        m_bomTable->setItem(i, 3, new QTableWidgetItem(b.specification));
        m_bomTable->setItem(i, 4, new QTableWidgetItem(QString::number(b.quantity)));
        m_bomTable->setItem(i, 5, new QTableWidgetItem(b.position));
        totalCost += b.quantity * b.unitPrice;
    }
    m_costLabel->setText(QStringLiteral("预估单台物料成本: ¥%1").arg(totalCost, 0, 'f', 2));
}

void ProductVersionPage::onNewVersion()
{
    // 取当前选中产品
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item || item->data(0, Qt::UserRole).toInt() > 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先在左侧选中一个产品（根节点）"));
        return;
    }
    int productId = -item->data(0, Qt::UserRole).toInt();

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("新建产品版本"));
    auto *f = new QFormLayout(&dlg);
    auto *vnEdit = new QLineEdit("v1.0");
    auto *vtEdit = new QLineEdit();
    auto *vnoteEdit = new QLineEdit();
    auto *stCombo = new QComboBox();
    stCombo->addItems({"draft", "released", "deprecated"});

    f->addRow(QStringLiteral("版本号："), vnEdit);
    f->addRow(QStringLiteral("版本标题："), vtEdit);
    f->addRow(QStringLiteral("版本说明："), vnoteEdit);
    f->addRow(QStringLiteral("状态："), stCombo);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    ProductVersion v;
    v.productId = productId;
    v.versionNumber = vnEdit->text().trimmed();
    v.versionTitle  = vtEdit->text().trimmed();
    v.versionNote   = vnoteEdit->text().trimmed();
    v.status   = stCombo->currentText();
    v.releaseDate = QDate::currentDate();

    ProductVersionRepo repo(m_db);
    if (repo.insert(v)) refresh();
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("新建失败"));
}

void ProductVersionPage::onAddBOM()
{
    if (!m_selectedVersionId) { QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择版本")); return; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("添加BOM元器件"));
    auto *f = new QFormLayout(&dlg);

    auto *compCombo = new QComboBox();
    ComponentRepo cRepo(m_db);
    for (const Component &c : cRepo.findAll())
        compCombo->addItem(QStringLiteral("%1 %2").arg(c.componentCode, c.name), c.componentId);

    auto *qtySpin = new QSpinBox(); qtySpin->setRange(1, 99999);
    auto *posEdit = new QLineEdit();

    f->addRow(QStringLiteral("元器件："), compCombo);
    f->addRow(QStringLiteral("用量："), qtySpin);
    f->addRow(QStringLiteral("PCB位置："), posEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    BOMItem b;
    b.versionId = m_selectedVersionId;
    b.componentId = compCombo->currentData().toInt();
    b.quantity = qtySpin->value();
    b.position = posEdit->text().trimmed();

    ProductBOMRepo repo(m_db);
    if (repo.insert(b)) loadBOM(m_selectedVersionId);
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("添加失败"));
}

void ProductVersionPage::onEditBOM()
{
    int row = m_bomTable->currentRow();
    if (row < 0) { QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择BOM行")); return; }
    int bomId = m_bomTable->item(row, 0)->text().toInt();

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("修改BOM用量"));
    auto *f = new QFormLayout(&dlg);
    auto *qtySpin = new QSpinBox(); qtySpin->setRange(1, 99999); qtySpin->setValue(m_bomTable->item(row, 4)->text().toInt());
    auto *posEdit = new QLineEdit(m_bomTable->item(row, 5)->text());

    f->addRow(QStringLiteral("用量："), qtySpin);
    f->addRow(QStringLiteral("PCB位置："), posEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    BOMItem b;
    b.bomId = bomId;
    b.quantity = qtySpin->value();
    b.position = posEdit->text().trimmed();

    ProductBOMRepo repo(m_db);
    if (repo.update(b)) loadBOM(m_selectedVersionId);
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("修改失败"));
}

void ProductVersionPage::onDeleteBOM()
{
    int row = m_bomTable->currentRow();
    if (row < 0) return;
    int bomId = m_bomTable->item(row, 0)->text().toInt();
    QString name = m_bomTable->item(row, 2)->text();

    if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定从BOM中删除 \"%1\" 吗？").arg(name)) == QMessageBox::Yes)
    {
        ProductBOMRepo repo(m_db);
        if (repo.remove(bomId)) loadBOM(m_selectedVersionId);
        else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("删除失败"));
    }
}
