#include "ComponentPage.h"
#include "db/repository/ComponentRepo.h"
#include "db/repository/SupplierRepo.h"
#include "db/repository/InventoryTransactionRepo.h"
#include "service/BOMImportService.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFileDialog>

ComponentPage::ComponentPage(QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    // 搜索栏
    auto *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText(QStringLiteral("输入编码或名称搜索..."));
    searchLayout->addWidget(m_searchEdit);

    m_supplierCombo = new QComboBox();
    m_supplierCombo->addItem(QStringLiteral("全部供应商"), 0);
    searchLayout->addWidget(m_supplierCombo);

    auto *searchBtn = new QPushButton(QStringLiteral("搜索"));
    searchLayout->addWidget(searchBtn);
    auto *refreshBtn = new QPushButton(QStringLiteral("刷新"));
    searchLayout->addWidget(refreshBtn);
    layout->addLayout(searchLayout);

    // 操作按钮
    auto *btnLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton(QStringLiteral("新增"));
    auto *editBtn = new QPushButton(QStringLiteral("编辑"));
    auto *deleteBtn = new QPushButton(QStringLiteral("删除"));
    auto *importBomBtn = new QPushButton(QStringLiteral("从BOM导入"));
    importBomBtn->setStyleSheet("color: #0066cc; font-weight: bold;");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(importBomBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    // 表格
    m_table = new QTableWidget(0, 8, this);
    m_table->setHorizontalHeaderLabels({
        "ID", QStringLiteral("编码"), QStringLiteral("名称"), QStringLiteral("规格"),
        QStringLiteral("单位"), QStringLiteral("库存量"), QStringLiteral("价格"), QStringLiteral("供应商")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setColumnHidden(0, true);
    layout->addWidget(m_table);

    connect(searchBtn, &QPushButton::clicked, this, &ComponentPage::onSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ComponentPage::onSearch);
    connect(refreshBtn, &QPushButton::clicked, this, &ComponentPage::refresh);
    connect(addBtn, &QPushButton::clicked, this, &ComponentPage::onAdd);
    connect(editBtn, &QPushButton::clicked, this, &ComponentPage::onEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &ComponentPage::onDelete);
    connect(importBomBtn, &QPushButton::clicked, this, &ComponentPage::onImportFromBOM);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ComponentPage::onDoubleClick);

    refresh();
}

void ComponentPage::refresh()
{
    ComponentRepo repo(m_db);
    QList<Component> list = repo.findAll();

    // 刷新供应商下拉
    m_supplierCombo->clear();
    m_supplierCombo->addItem(QStringLiteral("全部供应商"), 0);
    SupplierRepo sRepo(m_db);
    for (const Supplier &s : sRepo.findAll())
        m_supplierCombo->addItem(s.name, s.supplierId);

    m_table->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const Component &c = list[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(c.componentId)));
        m_table->setItem(i, 1, new QTableWidgetItem(c.componentCode));
        m_table->setItem(i, 2, new QTableWidgetItem(c.name));
        m_table->setItem(i, 3, new QTableWidgetItem(c.specification));
        m_table->setItem(i, 4, new QTableWidgetItem(c.unit));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(c.inventoryQuantity, 'f', 0)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(c.currentPrice, 'f', 2)));
        m_table->setItem(i, 7, new QTableWidgetItem(c.supplierName));
    }
}

void ComponentPage::onSearch()
{
    ComponentRepo repo(m_db);
    int sid = m_supplierCombo->currentData().toInt();
    QList<Component> list = repo.search(m_searchEdit->text().trimmed(), sid);

    m_table->setRowCount(0);
    for (int i = 0; i < list.size(); i++) {
        const Component &c = list[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(c.componentId)));
        m_table->setItem(i, 1, new QTableWidgetItem(c.componentCode));
        m_table->setItem(i, 2, new QTableWidgetItem(c.name));
        m_table->setItem(i, 3, new QTableWidgetItem(c.specification));
        m_table->setItem(i, 4, new QTableWidgetItem(c.unit));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(c.inventoryQuantity, 'f', 0)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(c.currentPrice, 'f', 2)));
        m_table->setItem(i, 7, new QTableWidgetItem(c.supplierName));
    }
}

void ComponentPage::onAdd()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("新增元器件"));
    auto *f = new QFormLayout(&dlg);

    auto *codeEdit = new QLineEdit();
    auto *nameEdit = new QLineEdit();
    auto *specEdit = new QLineEdit();
    auto *unitEdit = new QLineEdit("个");
    auto *minSpin  = new QSpinBox(); minSpin->setRange(0, 999999); minSpin->setValue(0);
    auto *priceSpin = new QDoubleSpinBox(); priceSpin->setRange(0, 999999); priceSpin->setDecimals(2);
    auto *supCombo = new QComboBox();
    SupplierRepo sRepo(m_db);
    for (const Supplier &s : sRepo.findAll())
        supCombo->addItem(s.name, s.supplierId);

    f->addRow(QStringLiteral("编码："), codeEdit);
    f->addRow(QStringLiteral("名称："), nameEdit);
    f->addRow(QStringLiteral("规格："), specEdit);
    f->addRow(QStringLiteral("单位："), unitEdit);
    f->addRow(QStringLiteral("最低库存："), minSpin);
    f->addRow(QStringLiteral("当前价格："), priceSpin);
    f->addRow(QStringLiteral("供应商："), supCombo);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    Component c;
    c.componentCode = codeEdit->text().trimmed();
    c.name          = nameEdit->text().trimmed();
    c.specification = specEdit->text().trimmed();
    c.unit          = unitEdit->text().trimmed();
    c.minStock      = minSpin->value();
    c.currentPrice  = priceSpin->value();
    c.supplierId    = supCombo->currentData().toInt();

    ComponentRepo repo(m_db);
    if (repo.insert(c)) refresh();
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("新增失败"));
}

void ComponentPage::onEdit()
{
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一行")); return; }
    int id = m_table->item(row, 0)->text().toInt();

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("编辑元器件"));
    auto *f = new QFormLayout(&dlg);

    auto *codeEdit = new QLineEdit(m_table->item(row, 1)->text());
    auto *nameEdit = new QLineEdit(m_table->item(row, 2)->text());
    auto *specEdit = new QLineEdit(m_table->item(row, 3)->text());
    auto *unitEdit = new QLineEdit(m_table->item(row, 4)->text());
    auto *minSpin  = new QSpinBox(); minSpin->setRange(0, 999999); minSpin->setValue(0);
    auto *priceSpin = new QDoubleSpinBox(); priceSpin->setRange(0, 999999); priceSpin->setDecimals(2);
    priceSpin->setValue(m_table->item(row, 6)->text().toDouble());
    auto *supCombo = new QComboBox();
    SupplierRepo sRepo(m_db);
    for (const Supplier &s : sRepo.findAll())
        supCombo->addItem(s.name, s.supplierId);

    f->addRow(QStringLiteral("编码："), codeEdit);
    f->addRow(QStringLiteral("名称："), nameEdit);
    f->addRow(QStringLiteral("规格："), specEdit);
    f->addRow(QStringLiteral("单位："), unitEdit);
    f->addRow(QStringLiteral("最低库存："), minSpin);
    f->addRow(QStringLiteral("当前价格："), priceSpin);
    f->addRow(QStringLiteral("供应商："), supCombo);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    f->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    Component c;
    c.componentId   = id;
    c.componentCode = codeEdit->text().trimmed();
    c.name          = nameEdit->text().trimmed();
    c.specification = specEdit->text().trimmed();
    c.unit          = unitEdit->text().trimmed();
    c.minStock      = minSpin->value();
    c.currentPrice  = priceSpin->value();
    c.supplierId    = supCombo->currentData().toInt();

    ComponentRepo repo(m_db);
    if (repo.update(c)) refresh();
    else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("编辑失败"));
}

void ComponentPage::onDelete()
{
    int row = m_table->currentRow();
    if (row < 0) return;
    int id = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 2)->text();

    if (QMessageBox::question(this, QStringLiteral("确认删除"),
        QStringLiteral("确定删除元器件 \"%1\" 吗？").arg(name)) == QMessageBox::Yes)
    {
        ComponentRepo repo(m_db);
        if (repo.remove(id)) refresh();
        else QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("删除失败"));
    }
}

void ComponentPage::onDoubleClick(int row, int)
{
    int id = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 2)->text();

    InventoryTransactionRepo repo(m_db);
    QList<InventoryTrans> list = repo.findByComponent(id);

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("库存流水 - %1").arg(name));
    dlg.resize(700, 400);
    auto *layout = new QVBoxLayout(&dlg);

    auto *t = new QTableWidget(0, 7, &dlg);
    t->setHorizontalHeaderLabels({
        QStringLiteral("流水号"), QStringLiteral("类型"), QStringLiteral("数量"),
        QStringLiteral("单价"), QStringLiteral("总价"), QStringLiteral("日期"), QStringLiteral("仓库")
    });
    t->horizontalHeader()->setStretchLastSection(true);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for (int i = 0; i < list.size(); i++) {
        const InventoryTrans &tx = list[i];
        t->insertRow(i);
        t->setItem(i, 0, new QTableWidgetItem(tx.transactionNo));
        t->setItem(i, 1, new QTableWidgetItem(tx.transactionType));
        t->setItem(i, 2, new QTableWidgetItem(QString::number(tx.quantity)));
        t->setItem(i, 3, new QTableWidgetItem(QString::number(tx.unitPrice, 'f', 2)));
        t->setItem(i, 4, new QTableWidgetItem(QString::number(tx.totalPrice, 'f', 2)));
        t->setItem(i, 5, new QTableWidgetItem(tx.transactionDate.toString("yyyy-MM-dd")));
        t->setItem(i, 6, new QTableWidgetItem(tx.warehouseName));
    }
    layout->addWidget(t);
    dlg.exec();
}

void ComponentPage::onImportFromBOM()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 BOM 文件"), QString(),
        QStringLiteral("嘉立创 BOM 文件 (*.csv);;所有文件 (*)")
    );
    if (filePath.isEmpty()) return;

    BOMImportService svc(m_db);
    QString error;
    QList<BOMRow> rows = svc.parseCSV(filePath, error);

    if (rows.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("校验失败"),
            QStringLiteral("BOM 文件校验未通过:\n%1").arg(error));
        return;
    }

    svc.matchComponents(rows);

    // 统计
    int matched = 0, partial = 0, missing = 0;
    for (const BOMRow &r : rows) {
        switch (r.status) {
        case BOMRow::Matched:  matched++;  break;
        case BOMRow::Partial:  partial++;  break;
        case BOMRow::Missing:  missing++;  break;
        }
    }

    QString msg = QStringLiteral("BOM 文件解析结果:\n\n"
        "🟢 已匹配: %1 行（数据库中已存在，跳过）\n"
        "🟡 待确认: %2 行（名称匹配但规格不同）\n"
        "🔴 需新建: %3 行\n\n"
        "将创建 %4 个新元器件和关联供应商。\n继续？")
        .arg(matched).arg(partial).arg(missing).arg(partial + missing);

    if (QMessageBox::question(this, QStringLiteral("批量导入确认"), msg) != QMessageBox::Yes)
        return;

    QStringList log;
    if (svc.createMissingComponents(rows, log)) {
        QMessageBox::information(this, QStringLiteral("导入成功"), log.join('\n'));
        refresh();
    } else {
        QMessageBox::warning(this, QStringLiteral("导入失败"), log.join('\n'));
    }
}
