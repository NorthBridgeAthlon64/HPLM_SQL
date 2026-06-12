#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct BOMItem {
    int     bomId = 0;
    int     versionId = 0;
    int     componentId = 0;
    int     quantity = 1;
    QString position;
    QString componentCode;
    QString componentName;
    QString specification;
    double  unitPrice = 0;
};

class ProductBOMRepo : public QObject {
    Q_OBJECT
public:
    explicit ProductBOMRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<BOMItem> findByVersion(int versionId);
    bool insert(const BOMItem &b);
    bool update(const BOMItem &b);
    bool remove(int bomId);
private:
    QSqlDatabase &m_db;
    BOMItem rowToBOM(const QSqlQuery &q);
};
