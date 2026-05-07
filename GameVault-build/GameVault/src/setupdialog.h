#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>

class SetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetupDialog(QWidget* parent = nullptr);
    QString selectedPath() const;

private slots:
    void onBrowse();
    void onConfirm();

private:
    void setupUI();
    void animateIn();

    QLineEdit*   m_pathEdit;
    QPushButton* m_browseBtn;
    QPushButton* m_confirmBtn;
    QLabel*      m_titleLabel;
    QLabel*      m_subtitleLabel;
    QString      m_selectedPath;
};
