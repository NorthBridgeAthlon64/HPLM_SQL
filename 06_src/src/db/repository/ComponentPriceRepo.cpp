#include "ComponentPriceRepo.h"
ComponentPriceRepo::ComponentPriceRepo(QSqlDatabase &db, QObject *p) : QObject(p), m_db(db) {}

