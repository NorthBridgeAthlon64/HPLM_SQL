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
#include "service/BOMExportService.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSqlQuery>

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
        setupMenuBar();
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

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));

    auto *exportAction = fileMenu->addAction(QStringLiteral("导出 BOM..."));
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportBOM);

    fileMenu->addSeparator();
    auto *exitAction = fileMenu->addAction(QStringLiteral("退出(&X)"));
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::onExportBOM()
{
    auto &db = DatabaseManager::instance().db();
    BOMExportService exportSvc(db);

    // 收集所有版本供用户选择
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("导出 BOM - 选择版本"));
    auto *layout = new QVBoxLayout(&dlg);

    auto *combo = new QComboBox();
    QSqlQuery q(db);
    q.exec("SELECT pv.version_id, p.name || ' ' || pv.version_number AS label "
           "FROM ProductVersion pv JOIN Product p ON pv.product_id = p.product_id "
           "ORDER BY p.product_id, pv.version_number");
    while (q.next())
        combo->addItem(q.value("label").toString(), q.value("version_id"));
    layout->addWidget(new QLabel(QStringLiteral("选择要导出的产品版本:")));
    layout->addWidget(combo);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(bb);

    if (dlg.exec() != QDialog::Accepted || combo->currentData().toInt() <= 0)
        return;

    int versionId = combo->currentData().toInt();
    QString defaultName = exportSvc.versionDisplayName(versionId).replace(' ', '_') + "_BOM.csv";
    QString filePath = QFileDialog::getSaveFileName(this, QStringLiteral("保存 BOM 文件"),
        defaultName, QStringLiteral("TSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    QString error;
    if (exportSvc.exportToCSV(versionId, filePath, error)) {
        QMessageBox::information(this, QStringLiteral("导出成功"),
            QStringLiteral("BOM 已导出到:\n%1").arg(filePath));
    } else {
        QMessageBox::warning(this, QStringLiteral("导出失败"), error);
    }
}
