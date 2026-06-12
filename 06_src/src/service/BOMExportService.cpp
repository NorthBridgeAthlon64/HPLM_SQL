#include "BOMExportService.h"

#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

BOMExportService::BOMExportService(QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

bool BOMExportService::exportToCSV(int versionId, const QString &filePath, QString &error)
{
    QString content = generateBOMContent(versionId, error);
    if (content.isEmpty() && !error.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = QStringLiteral("无法写入文件: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

QString BOMExportService::generateBOMContent(int versionId, QString &error)
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT ROW_NUMBER() OVER (ORDER BY pb.bom_id) AS row_no, "
        "       pb.quantity, "
        "       c.name AS comment, "
        "       pb.position AS designator, "
        "       c.specification, "
        "       c.component_code AS mfr_part, "
        "       COALESCE(s.name, '') AS manufacturer "
        "FROM ProductBOM pb "
        "JOIN Component c ON pb.component_id = c.component_id "
        "LEFT JOIN Supplier s ON c.supplier_id = s.supplier_id "
        "WHERE pb.version_id = :vid "
        "ORDER BY pb.bom_id"
    );
    q.bindValue(":vid", versionId);

    if (!q.exec()) {
        error = q.lastError().text();
        return {};
    }

    QStringList lines;
    // 表头（嘉立创 BOM 标准格式）
    lines.append("No.\tQuantity\tComment\tDesignator\tFootprint\tValue\tManufacturer Part\tManufacturer\tSupplier Part\tSupplier");

    int rowNum = 0;
    while (q.next()) {
        rowNum++;
        QString spec = q.value("specification").toString();

        // 拆分 specification → Footprint + Value
        QString footprint, value;
        int spaceIdx = spec.indexOf(' ');
        if (spaceIdx > 0) {
            footprint = spec.left(spaceIdx);
            value    = spec.mid(spaceIdx + 1);
        } else {
            footprint = spec;
            // value 留空
        }

        QStringList cols;
        cols << QString::number(rowNum)
             << q.value("quantity").toString()
             << q.value("comment").toString()
             << q.value("designator").toString()
             << footprint
             << value
             << q.value("mfr_part").toString()
             << q.value("manufacturer").toString()
             << ""    // Supplier Part 留空
             << "";   // Supplier 留空

        lines.append(cols.join('\t'));
    }

    if (lines.size() <= 1) {
        error = QStringLiteral("该版本 BOM 清单为空，请先配置 BOM");
        return {};
    }

    return lines.join('\n') + '\n';
}

QString BOMExportService::versionDisplayName(int versionId)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT p.name || ' ' || pv.version_number "
              "FROM ProductVersion pv JOIN Product p ON pv.product_id = p.product_id "
              "WHERE pv.version_id = :vid");
    q.bindValue(":vid", versionId);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return QStringLiteral("未知版本");
}
