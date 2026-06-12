#pragma once
#include <QObject>
#include <QWidget>
class QLineEdit;
class LoginPage : public QWidget { Q_OBJECT public: explicit LoginPage(QWidget *p = nullptr); signals: void loginSucceeded(int employeeId); };