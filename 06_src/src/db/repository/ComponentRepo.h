#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct Component {
    int     componentId = 0;
    QString componentCode;
    QString name;
    QString specification;
    QString unit;
    int     minStock = 0;
    double  currentPrice = 0;
    int     supplierId = 0;
    // 关联查询用
    QString supplierName;
    double  inventoryQuantity = 0;
};

class ComponentRepo : public QObject {
    Q_OBJECT
public:
    explicit ComponentRepo(QSqlDatabase &db, QObject *p = nullptr);
    /// 查询全部（含供应商名和库存量）
    QList<Component> findAll();
    /// 按关键字搜索（编码/名称）
    QList<Component> search(const QString &keyword, int supplierId = 0);
    Component findById(int id);
    bool insert(const Component &c);
    bool update(const Component &c);
    bool remove(int id);
private:
    QSqlDatabase &m_db;
    Component rowToComponent(const QSqlQuery &q);
};
