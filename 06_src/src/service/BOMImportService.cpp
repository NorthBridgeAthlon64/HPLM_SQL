#include "BOMImportService.h"
#include "db/DatabaseManager.h"

#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

BOMImportService::BOMImportService(QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

// ============================================================
// CSV 解析
// ============================================================

QList<BOMRow> BOMImportService::parseCSV(const QString &filePath, QString &error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QStringLiteral("无法打开文件: %1").arg(filePath);
        return {};
    }

    QTextStream in(&file);

    // 读取首行表头
    QString headerLine = in.readLine();
    if (headerLine.isEmpty()) {
        error = QStringLiteral("文件为空");
        return {};
    }

    QStringList headers = headerLine.split('\t');
    // 检查关键列是否存在
    if (headers.size() < 10 ||
        !headers.at(2).contains("Comment") ||
        !headers.at(4).contains("Footprint")) {
        error = QStringLiteral("表头格式不匹配嘉立创 BOM 标准。\n"
                               "期望列: No./Quantity/Comment/Designator/Footprint/Value/...\n"
                               "实际表头: %1").arg(headerLine);
        return {};
    }

    // 逐行解析
    QList<BOMRow> rows;
    for (int lineNo = 2; !in.atEnd(); lineNo++) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        BOMRow row;
        QString lineErr;
        if (parseLine(line, lineNo, row, lineErr)) {
            rows.append(row);
        } else {
            error += QStringLiteral("第 %1 行: %2\n").arg(lineNo).arg(lineErr);
        }
    }
    file.close();

    // 校验
    if (!validate(rows, error)) {
        return {};
    }

    // 合并重复行
    mergeDuplicateRows(rows);

    return rows;
}

bool BOMImportService::parseLine(const QString &line, int lineNo, BOMRow &row, QString &error)
{
    QStringList cols = line.split('\t');
    if (cols.size() < 10) {
        error = QStringLiteral("列数异常: 期望 10, 实际 %1").arg(cols.size());
        return false;
    }

    row.rowNo = lineNo - 1;  // 去掉表头后的序号

    // Quantity
    QString qtyStr = cols.at(1).trimmed();
    if (!qtyStr.isEmpty()) {
        bool ok;
        row.quantity = qtyStr.toInt(&ok);
        if (!ok || row.quantity <= 0) {
            error = QStringLiteral("数量无效: '%1'").arg(qtyStr);
            return false;
        }
    }

    row.comment     = cols.at(2).trimmed();
    row.designator  = cols.at(3).trimmed();
    row.footprint   = cols.at(4).trimmed();
    row.value       = cols.at(5).trimmed();
    row.mfrPart     = cols.at(6).trimmed();
    row.manufacturer= cols.at(7).trimmed();

    // Comment 不能为空
    if (row.comment.isEmpty()) {
        error = QStringLiteral("缺少元器件名称（Comment 列为空）");
        return false;
    }

    return true;
}

// ============================================================
// 校验
// ============================================================

bool BOMImportService::validate(const QList<BOMRow> &rows, QString &error)
{
    if (rows.isEmpty()) {
        error += QStringLiteral("\nBOM 文件无有效数据行（仅含表头）");
        return false;
    }

    // 检查是否有列数错误
    if (error.contains("列数异常")) {
        return false;
    }

    // 检查是否有数量无效
    if (error.contains("数量无效")) {
        return false;
    }

    // 检查是否有空 Comment
    if (error.contains("缺少元器件名称")) {
        return false;
    }

    return true;
}

void BOMImportService::mergeDuplicateRows(QList<BOMRow> &rows)
{
    for (int i = 0; i < rows.size(); i++) {
        for (int j = i + 1; j < rows.size(); j++) {
            if (rows[i].comment == rows[j].comment &&
                rows[i].footprint == rows[j].footprint) {
                // 合并：quantity 累加，designator 拼接
                rows[i].quantity += rows[j].quantity;
                if (!rows[i].designator.isEmpty() && !rows[j].designator.isEmpty())
                    rows[i].designator += "," + rows[j].designator;
                else if (!rows[j].designator.isEmpty())
                    rows[i].designator = rows[j].designator;

                rows[i].statusMessage = QStringLiteral("已合并重复行");
                rows.removeAt(j);
                j--;  // 回退一步
            }
        }
    }
}

// ============================================================
// 元器件匹配（三级递进）
// ============================================================

void BOMImportService::matchComponents(QList<BOMRow> &rows)
{
    for (BOMRow &row : rows) {
        // 第1级：Manufacturer Part 精确匹配（最可靠凭据）
        if (!row.mfrPart.isEmpty()) {
            QSqlQuery q(m_db);
            q.prepare("SELECT component_id, specification FROM Component WHERE component_code = :code");
            q.bindValue(":code", row.mfrPart);
            if (q.exec() && q.next()) {
                row.matchedComponentId = q.value("component_id").toInt();
                row.status = BOMRow::Matched;
                row.statusMessage = QStringLiteral("制造商型号精确匹配");
                continue;
            }
        }

        // 第2级：Comment 精确匹配
        {
            QSqlQuery q(m_db);
            q.prepare("SELECT component_id, specification, current_price FROM Component WHERE name = :name");
            q.bindValue(":name", row.comment);
            if (q.exec() && q.next()) {
                int cid = q.value("component_id").toInt();
                QString spec = q.value("specification").toString();

                // 检查规格是否一致
                if (!row.footprint.isEmpty() &&
                    spec.contains(row.footprint, Qt::CaseInsensitive)) {
                    // Footprint 也匹配 → 完全匹配
                    row.matchedComponentId = cid;
                    row.status = BOMRow::Matched;
                    row.statusMessage = QStringLiteral("名称+封装匹配");
                    continue;
                } else if (spec.isEmpty()) {
                    // 规格为空，也算匹配
                    row.matchedComponentId = cid;
                    row.status = BOMRow::Matched;
                    row.statusMessage = QStringLiteral("名称匹配（规格未录入）");
                    continue;
                } else {
                    // 名称匹配但规格不同
                    row.matchedComponentId = cid;
                    row.status = BOMRow::Partial;
                    row.statusMessage = QStringLiteral("名称匹配但规格不同(DB:%1 BOM:%2)")
                        .arg(spec, row.footprint);
                    continue;
                }
            }
        }

        // 第3级：Comment + Footprint + Value 组合匹配（松匹配）
        {
            QSqlQuery q(m_db);
            q.prepare("SELECT component_id, specification FROM Component "
                       "WHERE name = :name "
                       "  AND specification ILIKE '%'||:fp||'%' "
                       "  AND specification ILIKE '%'||:val||'%'");
            q.bindValue(":name", row.comment);
            q.bindValue(":fp",   row.footprint);
            q.bindValue(":val",  row.value);
            if (q.exec() && q.next()) {
                row.matchedComponentId = q.value("component_id").toInt();
                row.status = BOMRow::Partial;
                row.statusMessage = QStringLiteral("名称+规格组合匹配（版本可能不同）");
                continue;
            }
        }

        // 全部未命中
        row.status = BOMRow::Missing;
        row.statusMessage = QStringLiteral("数据库无此元器件，将自动创建");
    }
}

// ============================================================
// 批量导入
// ============================================================

bool BOMImportService::executeImport(int versionId, const QList<BOMRow> &rows, QStringList &log)
{
    QSqlDatabase &db = m_db;

    // 前置检查：仓库是否存在
    QSqlQuery checkQ(db);
    checkQ.exec("SELECT COUNT(*) FROM Warehouse WHERE warehouse_id = 1");
    checkQ.next();
    if (checkQ.value(0).toInt() == 0) {
        log.append(QStringLiteral("错误: 仓库 WH-A01 不存在，请先在元器件管理中创建仓库"));
        return false;
    }

    // 前置检查：ProductVersion 是否存在
    checkQ.prepare("SELECT COUNT(*) FROM ProductVersion WHERE version_id = :vid");
    checkQ.bindValue(":vid", versionId);
    checkQ.exec();
    checkQ.next();
    if (checkQ.value(0).toInt() == 0) {
        log.append(QStringLiteral("错误: 目标版本不存在 (version_id=%1)").arg(versionId));
        return false;
    }

    int newComponents = 0;
    int newSuppliers  = 0;
    int bomRows       = 0;

    db.transaction();

    for (const BOMRow &row : rows) {
        int componentId = row.matchedComponentId;

        // --- 🔴 缺失：新建元器件 ---
        if (componentId == 0) {
            // 处理供应商
            int supplierId = 0;
            if (!row.manufacturer.isEmpty()) {
                QSqlQuery sq(db);
                sq.prepare("SELECT supplier_id FROM Supplier WHERE name = :name");
                sq.bindValue(":name", row.manufacturer);
                if (sq.exec() && sq.next()) {
                    supplierId = sq.value(0).toInt();
                } else {
                    // 自动创建供应商
                    QString supCode = QStringLiteral("AUTO-%1").arg(QUuid::createUuid().toString(QUuid::Id128).left(8).toUpper());
                    sq.prepare("INSERT INTO Supplier (supplier_code, name, rating) VALUES (:code, :name, 3) RETURNING supplier_id");
                    sq.bindValue(":code", supCode);
                    sq.bindValue(":name", row.manufacturer);
                    if (sq.exec() && sq.next()) {
                        supplierId = sq.value(0).toInt();
                        newSuppliers++;
                        log.append(QStringLiteral("  📦 自动创建供应商: %1 (id=%2)").arg(row.manufacturer).arg(supplierId));
                    } else {
                        log.append(QStringLiteral("  ⚠ 创建供应商失败: %1").arg(row.manufacturer));
                    }
                }
            }

            // 生成 component_code（查重）
            QString code = generateComponentCode(row);
            int seq = 0;
            while (codeExists(code)) {
                seq++;
                code = generateComponentCode(row, seq);
            }

            QString spec;
            if (!row.footprint.isEmpty() && !row.value.isEmpty())
                spec = row.footprint + " " + row.value;
            else if (!row.footprint.isEmpty())
                spec = row.footprint;
            else if (!row.value.isEmpty())
                spec = row.value;

            QSqlQuery cq(db);
            cq.prepare("INSERT INTO Component (component_code, name, specification, unit, min_stock, current_price, supplier_id) "
                        "VALUES (:code, :name, :spec, '个', 0, 0, :sid) RETURNING component_id");
            cq.bindValue(":code", code);
            cq.bindValue(":name", row.comment);
            cq.bindValue(":spec", spec);
            cq.bindValue(":sid",  supplierId > 0 ? supplierId : QVariant(QVariant::Int));
            if (cq.exec() && cq.next()) {
                componentId = cq.value(0).toInt();
                newComponents++;
                log.append(QStringLiteral("  ✅ 新建元器件: %1 (id=%2, code=%3)").arg(row.comment).arg(componentId).arg(code));

                // 初始化库存为 0
                QSqlQuery iq(db);
                iq.prepare("INSERT INTO Inventory (component_id, warehouse_id, quantity, avg_cost) "
                            "VALUES (:cid, 1, 0, NULL) ON CONFLICT (component_id, warehouse_id) DO NOTHING");
                iq.bindValue(":cid", componentId);
                iq.exec();
            } else {
                log.append(QStringLiteral("  ❌ 创建元器件失败: %1 (%2)").arg(row.comment, cq.lastError().text()));
                db.rollback();
                return false;
            }
        }

        // --- 插入 ProductBOM ---
        {
            QSqlQuery bq(db);
            bq.prepare("INSERT INTO ProductBOM (version_id, component_id, quantity, position) "
                        "VALUES (:vid, :cid, :qty, :pos) "
                        "ON CONFLICT (version_id, component_id) DO UPDATE "
                        "SET quantity = ProductBOM.quantity + :qty2, "
                        "    position = CASE "
                        "        WHEN ProductBOM.position = '' THEN :pos2 "
                        "        WHEN ProductBOM.position ILIKE '%' || :pos3 || '%' THEN ProductBOM.position "
                        "        ELSE LEFT(ProductBOM.position || ',' || :pos4, 500) END");
            bq.bindValue(":vid",  versionId);
            bq.bindValue(":cid",  componentId);
            bq.bindValue(":qty",  row.quantity);
            bq.bindValue(":pos",  row.designator);
            bq.bindValue(":qty2", row.quantity);
            bq.bindValue(":pos3", row.designator);
            bq.bindValue(":pos4", row.designator);
            if (!bq.exec()) {
                log.append(QStringLiteral("  ❌ 插入 BOM 失败: %1 → %2").arg(row.comment, bq.lastError().text()));
                db.rollback();
                return false;
            }
            bomRows++;
        }
    }

    db.commit();

    log.prepend(QStringLiteral("导入完成！"));
    log.append(QStringLiteral(""));
    log.append(QStringLiteral("--- 汇总 ---"));
    log.append(QStringLiteral("✅ 新建元器件: %1 个").arg(newComponents));
    log.append(QStringLiteral("📋 导入 BOM 行: %1 行（版本 %2）").arg(bomRows).arg(versionId));
    if (newSuppliers > 0)
        log.append(QStringLiteral("📦 自动创建供应商: %1 个").arg(newSuppliers));
    log.append(QStringLiteral("⚠ 库存为 0，请在元器件管理页手动入库补货"));

    return true;
}

// ============================================================
// 工具
// ============================================================

QString BOMImportService::generateComponentCode(const BOMRow &row, int seq)
{
    QString fp = row.footprint;
    if (fp.isEmpty()) fp = "GEN";

    QString comment = row.comment;
    // 简化 comment：去掉逗号、空格、特殊字符
    comment.replace(',', '-');
    comment.replace(' ', '-');
    comment = comment.left(30);

    QString code = QStringLiteral("LCSC-%1-%2").arg(fp, comment);
    if (seq > 0) {
        code += QStringLiteral("-%1").arg(seq);
    }
    return code;
}

bool BOMImportService::codeExists(const QString &code)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM Component WHERE component_code = :code");
    q.bindValue(":code", code);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}
