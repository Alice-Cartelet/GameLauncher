#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include "gamemanager.h"

class OrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderDialog(const QList<GameInfo>& games,
                         QWidget* parent = nullptr);

    // 返回用户确认后的路径顺序
    QList<QString> orderedPaths() const;

private slots:
    void moveUp();
    void moveDown();
    void updateButtonStates();

private:
    QListWidget* m_list;
    QPushButton* m_upBtn;
    QPushButton* m_downBtn;
};