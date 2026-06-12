#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>

/// BOM CSV 导出服务
/// 从指定 ProductVersion 生成嘉立创格式 TSV 文件
class BOMExportService : public QObject {
    Q_OBJECT
public:
    explicit BOMExportService(QSqlDatabase &db, QObject *parent = nullptr);

    /// 导出 BOM 为 TSV 文件（UTF-8, Tab 分隔）
    bool exportToCSV(int versionId, const QString &filePath, QString &error);

    /// 生成 BOM 文本内容（可用于预览）
    QString generateBOMContent(int versionId, QString &error);

    /// 获取版本信息（产品名 + 版本号）
    QString versionDisplayName(int versionId);

private:
    QSqlDatabase &m_db;
};
