#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

struct ProductVersion {
    int     versionId = 0;
    int     productId = 0;
    QString versionNumber;
    QString versionTitle;
    QString versionNote;
    QDate   releaseDate;
    QString status;
    int     releaseEngineerId = 0;
    double  totalDevelopmentCost = 0;
    double  estimatedUnitCost = 0;
    double  recommendedPrice = 0;
    QString productName;
    QString engineerName;
};

class ProductVersionRepo : public QObject {
    Q_OBJECT
public:
    explicit ProductVersionRepo(QSqlDatabase &db, QObject *p = nullptr);
    QList<ProductVersion> findByProduct(int productId);
    ProductVersion findById(int id);
    bool insert(const ProductVersion &v);
    bool update(const ProductVersion &v);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    ProductVersion rowToVersion(const QSqlQuery &q);
};
