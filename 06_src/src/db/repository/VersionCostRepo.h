#pragma once
#include <QObject>
class QSqlDatabase;
class VersionCostRepo : public QObject { Q_OBJECT public: explicit VersionCostRepo(QSqlDatabase &db, QObject *p = nullptr); QSqlDatabase &m_db; };

