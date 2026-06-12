#include "TracePage.h"
#include "service/TraceService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

TracePage::TracePage(QSqlDatabase &db, QWidget *p) : QWidget(p), m_db(db)
{
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(QStringLiteral("故障产品元器件溯源"));
    QFont f; f.setPointSize(14); f.setBold(true); title->setFont(f);
    layout->addWidget(title);

    auto *inputLayout = new QHBoxLayout();
    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText(QStringLiteral("输入维修单号或批次号（如REP20260001 / B20240001）"));
    inputLayout->addWidget(m_inputEdit);
    auto *searchBtn = new QPushButton(QStringLiteral("溯源"));
    inputLayout->addWidget(searchBtn);
    layout->addLayout(inputLayout);

    m_resultBrowser = new QTextBrowser();
    m_resultBrowser->setStyleSheet("font-family: Consolas, monospace; font-size: 13px;");
    layout->addWidget(m_resultBrowser);

    connect(searchBtn, &QPushButton::clicked, this, &TracePage::onSearch);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TracePage::onSearch);
}

void TracePage::onSearch()
{
    QString input = m_inputEdit->text().trimmed();
    if (input.isEmpty()) return;

    TraceService svc(m_db);
    TraceResult result;

    if (input.startsWith("REP", Qt::CaseInsensitive))
        result = svc.traceByRepairNo(input);
    else
        result = svc.traceByBatchNo(input);

    QString html;
    html += "<h3>溯源报告</h3>";

    if (result.repair.repairId > 0) {
        html += QStringLiteral("<p><b>维修单号:</b> %1 &nbsp; | &nbsp; <b>客户:</b> %2 &nbsp; | &nbsp; <b>版本:</b> %3</p>")
            .arg(result.repair.repairNo, result.repair.customerName, result.repair.versionNumber);
        html += QStringLiteral("<p><b>故障描述:</b> %1</p>").arg(result.repair.faultDescription);
    }

    if (result.batch.batchId > 0) {
        html += QStringLiteral("<hr><p><b>生产批次:</b> %1 | <b>产品:</b> %2 %3 | <b>投产:</b> %4台 | <b>质量:</b> %5</p>")
            .arg(result.batch.batchNo, result.batch.productName, result.batch.versionNumber)
            .arg(result.batch.quantity).arg(result.batch.qualityStatus);

        if (!result.materials.isEmpty()) {
            html += QStringLiteral("<p><b>批次投料清单:</b></p><table border='1' cellpadding='4' cellspacing='0'><tr><th>元器件</th><th>编码</th><th>用量</th><th>单价</th><th>总价</th></tr>");
            for (const BatchMaterial &m : result.materials)
                html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                    .arg(m.componentName, m.componentCode).arg(m.usedQuantity)
                    .arg(m.unitCostAtTime, 0, 'f', 2).arg(m.totalCost, 0, 'f', 2);
            html += "</table>";
        }

        if (!result.testRecords.isEmpty()) {
            html += "<p><b>测试记录:</b></p><table border='1' cellpadding='4' cellspacing='0'><tr><th>项目</th><th>结果</th><th>日期</th></tr>";
            for (const TestRecord &t : result.testRecords)
                html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
                    .arg(t.testItem, t.testResult, t.testDate.toString("yyyy-MM-dd"));
            html += "</table>";
        }

        if (!result.suppliers.isEmpty()) {
            html += "<p><b>关联供应商:</b> " + result.suppliers.join(", ") + "</p>";
        }
    } else if (result.repair.repairId == 0) {
        html += "<p style='color:red;'>未找到相关记录</p>";
    }

    m_resultBrowser->setHtml(html);
}
