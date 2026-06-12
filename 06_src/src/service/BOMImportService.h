#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>

/// BOM 文件中的一行数据
struct BOMRow {
    int     rowNo = 0;         // CSV 行号（从 1 开始）
    int     quantity = 1;      // 单台用量
    QString comment;           // 元器件描述/名称
    QString designator;        // PCB 位号（如 C1,C20）
    QString footprint;         // 封装（如 C0603）
    QString value;             // 参数值（如 10μF, 100nF, 10K）
    QString mfrPart;           // 制造商型号
    QString manufacturer;      // 制造商名称
    int     matchedComponentId = 0;  // 匹配到的 component_id，0=未匹配

    enum MatchStatus { Matched, Partial, Missing };
    MatchStatus status = Missing;
    QString statusMessage;     // 给用户的提示信息
};

/// BOM CSV 导入服务
/// 职责：解析 TSV → 校验 → 匹配元器件 → 事务批量导入
class BOMImportService : public QObject {
    Q_OBJECT
public:
    explicit BOMImportService(QSqlDatabase &db, QObject *parent = nullptr);

    // ---- CSV 解析与校验 ----

    /// 解析嘉立创 BOM 文件（TSV，Tab 分隔）
    /// @return rows 解析出的 BOM 行列表；若校验失败返回空列表，error 包含详细信息
    QList<BOMRow> parseCSV(const QString &filePath, QString &error);

    // ---- 元器件匹配 ----

    /// 逐行匹配数据库中的元器件（三级递进：Mfr Part → Comment → 规格验证）
    void matchComponents(QList<BOMRow> &rows);

    // ---- 批量导入 ----

    /// 在事务内执行导入
    /// @param versionId 目标产品版本 ID
    /// @param rows 已匹配的 BOM 行
    /// @param log 输出导入日志（成功/失败原因）
    /// @return true=全部成功, false=已回滚
    bool executeImport(int versionId, const QList<BOMRow> &rows, QStringList &log);

    // ---- 工具 ----

    /// 自动生成 component_code（基于 Footprint + Comment + 序号）
    static QString generateComponentCode(const BOMRow &row, int seq = 0);

    /// 检查 component_code 是否已存在
    bool codeExists(const QString &code);

private:
    QSqlDatabase &m_db;

    /// 解析单行 TSV 数据
    bool parseLine(const QString &line, int lineNo, BOMRow &row, QString &error);

    /// 校验 BOM 数据完整性
    bool validate(const QList<BOMRow> &rows, QString &error);

    /// 合并 Comment + Footprint 相同的重复行（累加 quantity，拼接 designator）
    void mergeDuplicateRows(QList<BOMRow> &rows);
};
