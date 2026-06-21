#include "DatabaseManager.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QDebug>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

bool DatabaseManager::connect(const QString &host, int port, const QString &dbName,
                               const QString &user, const QString &password)
{
    if (m_db.isOpen()) {
        m_db.close();
    }

    // 使用 QPSQL 驱动连接 PostgreSQL
    m_db = QSqlDatabase::addDatabase("QPSQL", "hplm_connection");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        qCritical() << "Database connection failed:" << m_lastError;
        return false;
    }

    qInfo() << "Connected to PostgreSQL:" << host << ":" << port << "/" << dbName;
    return true;
}

void DatabaseManager::disconnect()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

QSqlDatabase &DatabaseManager::db()
{
    return m_db;
}

bool DatabaseManager::execSqlFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("无法打开 SQL 文件: %1").arg(filePath);
        qWarning() << m_lastError;
        return false;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    // 按 ; 分割 SQL 语句，并过滤空语句和纯注释行
    // 简化处理：按 ; 拆分，忽略空行
    QStringList statements;
    // 先去掉行注释再分割，避免行内 ; 干扰
    QString cleaned;
    for (const QString &line : sql.split('\n')) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("--") || trimmed.isEmpty()) {
            continue;
        }
        cleaned += trimmed + ' ';
    }

    statements = cleaned.split(';', Qt::SkipEmptyParts);

    QSqlQuery query(m_db);
    int total = statements.size();
    int success = 0;

    for (const QString &stmt : statements) {
        QString s = stmt.trimmed();
        if (s.isEmpty()) continue;

        if (!query.exec(s)) {
            // 忽略已存在的对象错误（幂等执行）
            QString err = query.lastError().text();
            if (err.contains("already exists") || err.contains("duplicate key")) {
                // 幂等执行，忽略
                continue;
            }
            qWarning() << "SQL execution warning:" << err;
            qWarning() << "Statement:" << s.left(120);
        } else {
            success++;
        }
    }

    qInfo() << "Executed SQL file:" << filePath
            << "(" << success << "/" << total << "statements)";
    return true;
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::isConnected() const
{
    return m_db.isOpen();
}
