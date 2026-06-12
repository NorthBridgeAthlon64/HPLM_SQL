#include "MainWindow.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "ComponentPage.h"
#include "ProductVersionPage.h"
#include "ProductionBatchPage.h"
#include "RepairPage.h"
#include "TracePage.h"
#include "FeedbackPage.h"
#include "db/DatabaseManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto &db = DatabaseManager::instance().db();

    m_stack = new QStackedWidget(this);

    // 登录页
    m_loginPage = new LoginPage(db);
    m_stack->addWidget(m_loginPage);

    // 主功能区（TabWidget，延迟创建，登录后再建）
    m_tabWidget = nullptr;
    m_stack->addWidget(new QWidget()); // 占位，登录后替换

    setCentralWidget(m_stack);

    // 状态栏
    m_userLabel = new QLabel(QStringLiteral("未登录"));
    statusBar()->addPermanentWidget(m_userLabel);

    connect(m_loginPage, &LoginPage::loginSucceeded, this, &MainWindow::onLoginSucceeded);
}

void MainWindow::onLoginSucceeded(const Employee &user)
{
    if (!m_tabWidget) {
        setupMainTabs();
    }

    m_userLabel->setText(QStringLiteral("当前用户: %1 (%2)").arg(user.name, user.position));
    m_stack->setCurrentIndex(1);

    // 刷新仪表盘
    m_dashboardPage->refresh();
}

void MainWindow::setupMainTabs()
{
    auto &db = DatabaseManager::instance().db();

    m_tabWidget = new QTabWidget();

    m_dashboardPage      = new DashboardPage(db);
    m_componentPage      = new ComponentPage(db);
    m_productVersionPage = new ProductVersionPage(db);
    m_productionBatchPage = new ProductionBatchPage(db);
    m_repairPage         = new RepairPage(db);
    m_tracePage          = new TracePage(db);
    m_feedbackPage       = new FeedbackPage(db);

    m_tabWidget->addTab(m_dashboardPage,      QStringLiteral("仪表盘"));
    m_tabWidget->addTab(m_componentPage,      QStringLiteral("元器件管理"));
    m_tabWidget->addTab(m_productVersionPage, QStringLiteral("产品版本与BOM"));
    m_tabWidget->addTab(m_productionBatchPage, QStringLiteral("生产批次与测试"));
    m_tabWidget->addTab(m_repairPage,         QStringLiteral("维修工单"));
    m_tabWidget->addTab(m_tracePage,          QStringLiteral("故障溯源"));
    m_tabWidget->addTab(m_feedbackPage,       QStringLiteral("客户评价"));

    // 替换占位
    m_stack->removeWidget(m_stack->widget(1));
    m_stack->addWidget(m_tabWidget);
}
