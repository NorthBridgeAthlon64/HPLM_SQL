#pragma once

#include <QSqlDatabase>
#include <QString>

/// 数据库连接管理器（单例）
/// 封装 QPSQL 连接的生命周期管理
class DatabaseManager {
public:
    static DatabaseManager &instance();

    /// 连接到 PostgreSQL 数据库
    bool connect(const QString &host, int port, const QString &dbName,
                 const QString &user, const QString &password);

    /// 断开数据库连接
    void disconnect();

    /// 获取数据库连接引用（供 Repository 使用）
    QSqlDatabase &db();

    /// 执行 SQL 文件（逐条执行，以 ; 分隔）
    /// 用于 init.sql / seed.sql 的批量执行
    bool execSqlFile(const QString &filePath);

    /// 获取最近一次错误信息
    QString lastError() const;

    /// 检查数据库连接是否有效
    bool isConnected() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
    QString m_lastError;
};
