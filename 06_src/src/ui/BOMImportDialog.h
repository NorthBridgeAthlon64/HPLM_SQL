#pragma once

#include <QDialog>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include "service/BOMImportService.h"

/// BOM 导入预览对话框
/// 展示三色标记的元器件匹配结果，用户可选择版本并确认导入
class BOMImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit BOMImportDialog(QSqlDatabase &db, QWidget *parent = nullptr);

    /// 设置预加载的 BOM 行（由调用方先 parse + match 再传入）
    void setData(const QList<BOMRow> &rows, const QString &fileName);

    /// 获取统计信息（供外部显示）
    int matchedCount() const  { return m_countMatched; }
    int partialCount() const  { return m_countPartial; }
    int missingCount() const  { return m_countMissing; }

private slots:
    void onBrowse();
    void onImport();
    void onImportMatchedOnly();

private:
    void refreshTable();
    void updateStats();

    QSqlDatabase &m_db;
    BOMImportService m_service;

    // UI 组件
    QLineEdit   *m_filePathEdit;
    QComboBox   *m_versionCombo;
    QTableWidget *m_table;
    QLabel      *m_statsLabel;
    QPushButton *m_importBtn;
    QPushButton *m_importMatchedBtn;

    // 数据
    QList<BOMRow> m_rows;
    int m_countMatched  = 0;
    int m_countPartial  = 0;
    int m_countMissing  = 0;
};
