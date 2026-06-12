/// ============================================
/// HPLM 程序入口
/// ============================================
#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QDir>

#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("HPLM");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("HPLM");

    // 读取数据库配置
    // config/db.ini 位于可执行文件同级的 config/ 目录下
    QString configPath = QDir(app.applicationDirPath()).filePath("config/db.ini");

    QSettings settings(configPath, QSettings::IniFormat);
    QString host     = settings.value("Database/host", "localhost").toString();
    int     port     = settings.value("Database/port", 5432).toInt();
    QString dbName   = settings.value("Database/name", "hplm").toString();
    QString user     = settings.value("Database/user", "postgres").toString();
    QString password = settings.value("Database/password", "postgres").toString();

    // 初始化数据库连接
    auto &dbManager = DatabaseManager::instance();
    if (!dbManager.connect(host, port, dbName, user, password)) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("数据库连接失败"),
            QStringLiteral("无法连接到 PostgreSQL 数据库。\n"
                           "请检查 config/db.ini 配置是否正确。\n\n"
                           "错误详情：%1").arg(dbManager.lastError())
        );
        return 1;
    }

    // 首次运行：自动建表 + 导入演示数据
    QString sqlDir = QDir(app.applicationDirPath()).filePath("sql");
    dbManager.execSqlFile(QDir(sqlDir).filePath("init.sql"));
    // seed.sql 使用 ON CONFLICT DO NOTHING，可安全重复执行
    dbManager.execSqlFile(QDir(sqlDir).filePath("seed.sql"));

    // 启动主窗口
    MainWindow mainWindow;
    mainWindow.setWindowTitle(QStringLiteral("硬件产品全生命周期管理系统（HPLM）"));
    mainWindow.resize(1280, 800);
    mainWindow.show();

    return app.exec();
}
