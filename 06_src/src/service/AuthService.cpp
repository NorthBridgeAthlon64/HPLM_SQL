#include "AuthService.h"
#include <QDebug>

AuthService::AuthService(QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_repo(db) {}

Employee AuthService::login(const QString &employeeNo, const QString &password)
{
    Employee emp = m_repo.findByEmployeeNo(employeeNo);
    if (emp.employeeId == 0) {
        qInfo() << "Login failed: employee not found:" << employeeNo;
        return Employee();
    }
    // 直接对比明文密码（课程作业级别）
    if (emp.password != password) {
        qInfo() << "Login failed: wrong password for:" << employeeNo;
        return Employee();
    }
    m_currentUser = emp;
    qInfo() << "Login success:" << emp.name << "(" << emp.position << ")";
    return emp;
}

bool AuthService::registerEmployee(const Employee &e)
{
    if (e.employeeNo.isEmpty() || e.name.isEmpty() || e.password.isEmpty()) {
        qWarning() << "Registration failed: required fields empty";
        return false;
    }
    return m_repo.insert(e);
}

Employee AuthService::currentUser() const
{
    return m_currentUser;
}
