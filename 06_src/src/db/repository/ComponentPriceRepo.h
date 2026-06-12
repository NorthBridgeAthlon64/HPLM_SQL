#pragma once
#include <QObject>
class QSqlDatabase;
class ComponentPriceRepo : public QObject { Q_OBJECT public: explicit ComponentPriceRepo(QSqlDatabase &db, QObject *p = nullptr); QSqlDatabase &m_db; };

