#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QString>

class ExecutableDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExecutableDialog(const QString& gamePath, QWidget* parent = nullptr);
    QString selectedExec() const;

private slots:
    void onConfirm();

private:
    void scanExecutables(const QString& path);
    void setupUI();

    QListWidget* m_list;
    QPushButton* m_confirmBtn;
    QPushButton* m_browseBtn;
    QString      m_gamePath;
    QString      m_selectedExec;
};
