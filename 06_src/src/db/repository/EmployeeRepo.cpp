#include "EmployeeRepo.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

EmployeeRepo::EmployeeRepo(QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

Employee EmployeeRepo::rowToEmployee(const QSqlQuery &q)
{
    Employee e;
    e.employeeId = q.value("employee_id").toInt();
    e.employeeNo = q.value("employee_no").toString();
    e.name       = q.value("name").toString();
    e.password   = q.value("password").toString();
    e.position   = q.value("position").toString();
    e.phone      = q.value("phone").toString();
    e.email      = q.value("email").toString();
    return e;
}

Employee EmployeeRepo::findByEmployeeNo(const QString &employeeNo)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Employee WHERE employee_no = :no");
    query.bindValue(":no", employeeNo);

    if (query.exec() && query.next()) {
        return rowToEmployee(query);
    }
    return Employee();
}

Employee EmployeeRepo::findById(int employeeId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Employee WHERE employee_id = :id");
    query.bindValue(":id", employeeId);

    if (query.exec() && query.next()) {
        return rowToEmployee(query);
    }
    return Employee();
}

QList<Employee> EmployeeRepo::findAll()
{
    QList<Employee> list;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM Employee ORDER BY employee_id");

    while (query.next()) {
        list.append(rowToEmployee(query));
    }
    return list;
}

bool EmployeeRepo::insert(const Employee &e)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO Employee (employee_no, name, password, position, phone, email) "
        "VALUES (:no, :name, :pwd, :pos, :phone, :email)"
    );
    query.bindValue(":no",    e.employeeNo);
    query.bindValue(":name",  e.name);
    query.bindValue(":pwd",   e.password);
    query.bindValue(":pos",   e.position);
    query.bindValue(":phone", e.phone);
    query.bindValue(":email", e.email);

    if (!query.exec()) {
        qWarning() << "EmployeeRepo::insert failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool EmployeeRepo::update(const Employee &e)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE Employee SET name=:name, password=:pwd, position=:pos, "
        "phone=:phone, email=:email WHERE employee_id=:id"
    );
    query.bindValue(":id",    e.employeeId);
    query.bindValue(":name",  e.name);
    query.bindValue(":pwd",   e.password);
    query.bindValue(":pos",   e.position);
    query.bindValue(":phone", e.phone);
    query.bindValue(":email", e.email);

    if (!query.exec()) {
        qWarning() << "EmployeeRepo::update failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool EmployeeRepo::remove(int employeeId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Employee WHERE employee_id = :id");
    query.bindValue(":id", employeeId);

    if (!query.exec()) {
        qWarning() << "EmployeeRepo::remove failed:" << query.lastError().text();
        return false;
    }
    return true;
}
