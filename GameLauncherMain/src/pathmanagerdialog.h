#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

class PathManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit PathManagerDialog(const QStringList& currentPaths,
                               QWidget* parent = nullptr);
    QStringList paths() const;

private slots:
    void onAdd();
    void onRemove();
    void updateRemoveState();

private:
    QListWidget* m_list;
    QPushButton* m_removeBtn;
};