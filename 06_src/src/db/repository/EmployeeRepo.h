#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

struct Employee {
    int     employeeId = 0;
    QString employeeNo;
    QString name;
    QString password;
    QString position;
    QString phone;
    QString email;
};

class EmployeeRepo : public QObject {
    Q_OBJECT
public:
    explicit EmployeeRepo(QSqlDatabase &db, QObject *parent = nullptr);

    /// 根据工号查询（用于登录）
    Employee findByEmployeeNo(const QString &employeeNo);

    /// 根据 ID 查询
    Employee findById(int employeeId);

    /// 查询全部员工
    QList<Employee> findAll();

    /// 插入新员工（注册）
    bool insert(const Employee &e);

    /// 更新员工信息
    bool update(const Employee &e);

    /// 删除员工
    bool remove(int employeeId);

private:
    QSqlDatabase &m_db;
    Employee rowToEmployee(const QSqlQuery &q);
};
