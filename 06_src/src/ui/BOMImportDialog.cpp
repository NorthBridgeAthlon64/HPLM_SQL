#include "BOMImportDialog.h"
#include "db/repository/ProductVersionRepo.h"
#include "db/repository/ProductRepo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QHeaderView>

BOMImportDialog::BOMImportDialog(QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), m_db(db), m_service(db)
{
    setWindowTitle(QStringLiteral("BOM 文件导入"));
    resize(900, 550);

    auto *mainLayout = new QVBoxLayout(this);

    // ── 文件选择 ──
    auto *fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel(QStringLiteral("文件:")));
    m_filePathEdit = new QLineEdit();
    m_filePathEdit->setReadOnly(true);
    m_filePathEdit->setPlaceholderText(QStringLiteral("请选择嘉立创 EDA 导出的 BOM 文件 (.csv)"));
    fileLayout->addWidget(m_filePathEdit);
    auto *browseBtn = new QPushButton(QStringLiteral("浏览..."));
    fileLayout->addWidget(browseBtn);
    mainLayout->addLayout(fileLayout);

    // ── 版本选择 ──
    auto *verLayout = new QHBoxLayout();
    verLayout->addWidget(new QLabel(QStringLiteral("目标版本:")));
    m_versionCombo = new QComboBox();

    // 填充版本列表
    ProductVersionRepo verRepo(m_db);
    ProductRepo prodRepo(m_db);
    for (const Product &p : prodRepo.findAll()) {
        for (const ProductVersion &v : verRepo.findByProduct(p.productId)) {
            m_versionCombo->addItem(
                QStringLiteral("%1 %2 [%3]").arg(p.name, v.versionNumber, v.status),
                v.versionId);
        }
    }
    verLayout->addWidget(m_versionCombo);
    verLayout->addStretch();
    mainLayout->addLayout(verLayout);

    // ── 预览表格 ──
    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("状态"), QStringLiteral("名称"), QStringLiteral("用量"),
        QStringLiteral("位号"), QStringLiteral("封装"), QStringLiteral("说明")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_table);

    // ── 统计 ──
    m_statsLabel = new QLabel();
    mainLayout->addWidget(m_statsLabel);

    // ── 操作按钮 ──
    auto *btnLayout = new QHBoxLayout();
    m_importBtn        = new QPushButton(QStringLiteral("导入全部"));
    m_importMatchedBtn = new QPushButton(QStringLiteral("仅导入已匹配"));
    auto *cancelBtn    = new QPushButton(QStringLiteral("取消"));
    btnLayout->addWidget(m_importBtn);
    btnLayout->addWidget(m_importMatchedBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    m_importBtn->setEnabled(false);
    m_importMatchedBtn->setEnabled(false);

    connect(browseBtn,    &QPushButton::clicked, this, &BOMImportDialog::onBrowse);
    connect(m_importBtn,  &QPushButton::clicked, this, &BOMImportDialog::onImport);
    connect(m_importMatchedBtn, &QPushButton::clicked, this, &BOMImportDialog::onImportMatchedOnly);
    connect(cancelBtn,    &QPushButton::clicked, this, &QDialog::reject);
}

void BOMImportDialog::onBrowse()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 BOM 文件"), QString(),
        QStringLiteral("嘉立创 BOM 文件 (*.csv);;所有文件 (*)")
    );
    if (filePath.isEmpty()) return;

    m_filePathEdit->setText(filePath);

    QString error;
    QList<BOMRow> rows = m_service.parseCSV(filePath, error);

    if (rows.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("校验失败"),
            QStringLiteral("BOM 文件校验未通过:\n%1").arg(error));
        m_importBtn->setEnabled(false);
        m_importMatchedBtn->setEnabled(false);
        m_table->setRowCount(0);
        m_statsLabel->setText(QStringLiteral("❌ 校验失败，请修正文件后重新导入"));
        return;
    }

    // 匹配
    m_service.matchComponents(rows);
    setData(rows, QFileInfo(filePath).fileName());

    if (!error.isEmpty()) {
        // 重复行合并提醒
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("检测到重复行（相同 Comment+Footprint），已自动合并。\n\n%1").arg(error));
    }
}

void BOMImportDialog::setData(const QList<BOMRow> &rows, const QString & /*fileName*/)
{
    m_rows = rows;
    refreshTable();
    updateStats();
    m_importBtn->setEnabled(true);
    m_importMatchedBtn->setEnabled(m_countMatched > 0);
}

void BOMImportDialog::refreshTable()
{
    m_table->setRowCount(0);
    m_countMatched = m_countPartial = m_countMissing = 0;

    for (int i = 0; i < m_rows.size(); i++) {
        const BOMRow &row = m_rows[i];
        m_table->insertRow(i);

        QString statusText;
        QColor  bgColor;
        switch (row.status) {
        case BOMRow::Matched:
            statusText = QStringLiteral("🟢 已匹配");
            bgColor = QColor(200, 255, 200);  // 浅绿
            m_countMatched++;
            break;
        case BOMRow::Partial:
            statusText = QStringLiteral("🟡 待确认");
            bgColor = QColor(255, 255, 200);  // 浅黄
            m_countPartial++;
            break;
        case BOMRow::Missing:
            statusText = QStringLiteral("🔴 需新建");
            bgColor = QColor(255, 200, 200);  // 浅红
            m_countMissing++;
            break;
        }

        auto *sItem = new QTableWidgetItem(statusText);
        auto *nItem = new QTableWidgetItem(row.comment);
        auto *qItem = new QTableWidgetItem(QString::number(row.quantity));
        auto *dItem = new QTableWidgetItem(row.designator);
        auto *fItem = new QTableWidgetItem(row.footprint);
        auto *mItem = new QTableWidgetItem(row.statusMessage);

        sItem->setBackground(bgColor);
        nItem->setBackground(bgColor);
        qItem->setBackground(bgColor);
        dItem->setBackground(bgColor);
        fItem->setBackground(bgColor);
        mItem->setBackground(bgColor);

        m_table->setItem(i, 0, sItem);
        m_table->setItem(i, 1, nItem);
        m_table->setItem(i, 2, qItem);
        m_table->setItem(i, 3, dItem);
        m_table->setItem(i, 4, fItem);
        m_table->setItem(i, 5, mItem);
    }
}

void BOMImportDialog::updateStats()
{
    m_statsLabel->setText(
        QStringLiteral("🟢 已匹配: %1  |  🟡 待确认: %2  |  🔴 需新建: %3  |  总计: %4")
            .arg(m_countMatched).arg(m_countPartial).arg(m_countMissing).arg(m_rows.size())
    );
}

void BOMImportDialog::onImport()
{
    int versionId = m_versionCombo->currentData().toInt();
    if (versionId <= 0) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请选择目标版本"));
        return;
    }

    QString msg = QStringLiteral("确认导入？\n\n"
        "🟢 已匹配: %1 行\n"
        "🔴 需新建: %2 个元器件\n"
        "📋 总计: %3 行 BOM\n\n"
        "此操作不可撤销。").arg(m_countMatched).arg(m_countMissing).arg(m_rows.size());

    if (QMessageBox::question(this, QStringLiteral("确认导入"), msg) != QMessageBox::Yes)
        return;

    QStringList log;
    if (m_service.executeImport(versionId, m_rows, log)) {
        QMessageBox::information(this, QStringLiteral("导入成功"), log.join('\n'));
        accept();
    } else {
        QMessageBox::warning(this, QStringLiteral("导入失败"), log.join('\n'));
    }
}

void BOMImportDialog::onImportMatchedOnly()
{
    // 只筛选已匹配和部分匹配的行
    QList<BOMRow> matched;
    for (const BOMRow &row : m_rows) {
        if (row.status != BOMRow::Missing)
            matched.append(row);
    }
    if (matched.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("没有可导入的已匹配行"));
        return;
    }

    int versionId = m_versionCombo->currentData().toInt();
    QString msg = QStringLiteral("仅导入已匹配的 %1 行 BOM，确认？").arg(matched.size());
    if (QMessageBox::question(this, QStringLiteral("确认导入"), msg) != QMessageBox::Yes)
        return;

    QStringList log;
    if (m_service.executeImport(versionId, matched, log)) {
        QMessageBox::information(this, QStringLiteral("导入成功"), log.join('\n'));
        accept();
    } else {
        QMessageBox::warning(this, QStringLiteral("导入失败"), log.join('\n'));
    }
}
