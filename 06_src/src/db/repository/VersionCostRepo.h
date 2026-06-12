#pragma once
#include <QSqlDatabase>
class VersionCostRepo {
public:
    explicit VersionCostRepo(QSqlDatabase &db);
private:
    QSqlDatabase &m_db;
};
