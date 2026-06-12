#include "VersionCostRepo.h"
VersionCostRepo::VersionCostRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}

