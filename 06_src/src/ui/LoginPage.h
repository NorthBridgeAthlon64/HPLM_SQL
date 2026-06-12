#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include "db/repository/EmployeeRepo.h"

class LoginPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginPage(QSqlDatabase &db, QWidget *parent = nullptr);

signals:
    void loginSucceeded(const Employee &user);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QSqlDatabase &m_db;
    QLineEdit *m_employeeNoEdit;
    QLineEdit *m_passwordEdit;
    QLabel *m_statusLabel;
};
