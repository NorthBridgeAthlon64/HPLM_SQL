#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include "db/repository/EmployeeRepo.h"

class LoginPage;
class DashboardPage;
class ComponentPage;
class ProductVersionPage;
class ProductionBatchPage;
class RepairPage;
class TracePage;
class FeedbackPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onLoginSucceeded(const Employee &user);
    void onExportBOM();

private:
    void setupMainTabs();
    void setupMenuBar();

    QStackedWidget *m_stack;
    LoginPage      *m_loginPage;
    QTabWidget     *m_tabWidget;

    DashboardPage      *m_dashboardPage;
    ComponentPage      *m_componentPage;
    ProductVersionPage *m_productVersionPage;
    ProductionBatchPage *m_productionBatchPage;
    RepairPage         *m_repairPage;
    TracePage          *m_tracePage;
    FeedbackPage       *m_feedbackPage;

    QLabel *m_userLabel;
};
