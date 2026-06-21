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

    // 分离 SQL 语句：按 ; 分割，但保护 $$ ... $$ 及 $tag$ ... $tag$ 块不被拆分
    QStringList statements;
    int i = 0;
    int len = sql.length();
    int stmtStart = 0;
    bool inDollar = false;
    QString dollarTag;           // $$ 或 $tag$

    while (i < len) {
        if (!inDollar && sql[i] == '$') {
            // 检查是否是 $$ 或 $tag$
            int j = i + 1;
            while (j < len && sql[j] != '$') j++;
            if (j < len && sql[j] == '$') {
                inDollar = true;
                dollarTag = sql.mid(i, j - i + 1); // $tag$
                i = j + 1;
                continue;
            }
        }

        if (inDollar) {
            // 在 dollar-quoted 块内，找匹配的结束标记
            if (sql[i] == '$') {
                // 尝试匹配结束标记
                if (i + dollarTag.length() <= len &&
                    sql.mid(i, dollarTag.length()) == dollarTag) {
                    inDollar = false;
                    i += dollarTag.length();
                    continue;
                }
            }
            i++;
            continue;
        }

        // 不在 dollar 块内，遇到 ; 就是语句边界
        if (sql[i] == ';') {
            QString stmt = sql.mid(stmtStart, i - stmtStart).trimmed();
            if (!stmt.isEmpty() && !stmt.startsWith("--"))
                statements.append(stmt);
            stmtStart = i + 1;
        }
        i++;
    }
    // 最后一段（无 ; 结尾的）
    QString lastStmt = sql.mid(stmtStart).trimmed();
    if (!lastStmt.isEmpty() && !lastStmt.startsWith("--"))
        statements.append(lastStmt);

    QSqlQuery query(m_db);
    int total = statements.size();
    int success = 0;

    for (const QString &stmt : statements) {
        QString s = stmt.trimmed();
        if (s.isEmpty()) continue;

        if (!query.exec(s)) {
            QString err = query.lastError().text();
            if (err.contains("already exists") || err.contains("duplicate key")) {
                continue;
            }
            qWarning() << "SQL execution warning:" << err << "\n  Statement:" << s.left(120);
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
