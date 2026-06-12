#include "DashboardPage.h"
#include "db/repository/ComponentRepo.h"
#include "db/repository/RepairRecordRepo.h"
#include <QSqlQuery>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>

DashboardPage::DashboardPage(QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(QStringLiteral("系统仪表盘"));
    QFont f; f.setPointSize(16); f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    // 卡片区域
    auto *cardLayout = new QGridLayout();

    auto *lowGroup = new QGroupBox(QStringLiteral("低库存预警"));
    auto *lowVBox = new QVBoxLayout(lowGroup);
    m_lowStockLabel = new QLabel(QStringLiteral("加载中..."));
    lowVBox->addWidget(m_lowStockLabel);
    cardLayout->addWidget(lowGroup, 0, 0);

    auto *repairGroup = new QGroupBox(QStringLiteral("待处理维修工单"));
    auto *repVBox = new QVBoxLayout(repairGroup);
    m_pendingRepairLabel = new QLabel(QStringLiteral("加载中..."));
    m_pendingRepairLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #d9534f;");
    repVBox->addWidget(m_pendingRepairLabel);
    cardLayout->addWidget(repairGroup, 0, 1);

    layout->addLayout(cardLayout);

    // 批次质量统计表
    auto *qualityGroup = new QGroupBox(QStringLiteral("近30天批次质量统计"));
    auto *qVBox = new QVBoxLayout(qualityGroup);
    m_qualityTable = new QTableWidget(0, 2, this);
    m_qualityTable->setHorizontalHeaderLabels({QStringLiteral("质量状态"), QStringLiteral("数量")});
    m_qualityTable->horizontalHeader()->setStretchLastSection(true);
    m_qualityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    qVBox->addWidget(m_qualityTable);
    layout->addWidget(qualityGroup);

    refresh();
}

void DashboardPage::refresh()
{
    // 低库存预警
    QSqlQuery q(m_db);
    q.exec(
        "SELECT c.component_code, c.name, i.quantity, c.min_stock "
        "FROM Component c JOIN Inventory i ON c.component_id = i.component_id "
        "WHERE i.quantity < c.min_stock LIMIT 10"
    );
    QStringList lowItems;
    while (q.next()) {
        lowItems.append(QStringLiteral("%1 %2: 库存%3 < 阈值%4")
            .arg(q.value(0).toString(), q.value(1).toString(),
                 q.value(2).toString(), q.value(3).toString()));
    }
    m_lowStockLabel->setText(lowItems.isEmpty() ? QStringLiteral("当前无低库存元器件")
                                                : lowItems.join("\n"));

    // 待处理维修工单
    RepairRecordRepo repairRepo(m_db);
    int pending = repairRepo.pendingCount();
    m_pendingRepairLabel->setText(QString::number(pending));

    // 批次质量统计
    q.exec(
        "SELECT quality_status, COUNT(*) FROM ProductionBatch "
        "WHERE production_date >= CURRENT_DATE - INTERVAL '30 days' "
        "GROUP BY quality_status"
    );
    m_qualityTable->setRowCount(0);
    int row = 0;
    while (q.next()) {
        m_qualityTable->insertRow(row);
        m_qualityTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        m_qualityTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        row++;
    }
}
