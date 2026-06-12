#pragma once
#include <QObject>
#include "db/repository/EmployeeRepo.h"

class AuthService : public QObject {
    Q_OBJECT
public:
    explicit AuthService(QSqlDatabase &db, QObject *parent = nullptr);

    /// 登录认证，成功返回员工对象，失败返回空（employeeId==0）
    Employee login(const QString &employeeNo, const QString &password);

    /// 注册新员工
    bool registerEmployee(const Employee &e);

    /// 获取当前登录用户
    Employee currentUser() const;

private:
    EmployeeRepo m_repo;
    Employee m_currentUser;
};
