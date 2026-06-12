#pragma once
#include <QSqlDatabase>
class ComponentPriceRepo {
public:
    explicit ComponentPriceRepo(QSqlDatabase &db);
private:
    QSqlDatabase &m_db;
};
