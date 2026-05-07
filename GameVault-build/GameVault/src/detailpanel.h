#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QFutureWatcher>
#include "gamemanager.h"

class DetailPanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal slideProgress READ slideProgress WRITE setSlideProgress)
    Q_PROPERTY(qreal fadeOpacity READ fadeOpacity WRITE setFadeOpacity)

public:
    explicit DetailPanel(QWidget* parent = nullptr);
    ~DetailPanel() override;

    void showGame(int index, const GameInfo& info);
    void hidePanel();

    qreal slideProgress() const { return m_slideProgress; }
    void setSlideProgress(qreal v);

    qreal fadeOpacity() const { return m_fadeOpacity; }
    void setFadeOpacity(qreal v);

signals:
    void changeBgRequested(int index);
    void changeExecRequested(int index);
    void descriptionChanged(int index, const QString& desc);
    void nameChanged(int index, const QString& name);
    void launchRequested(int index);
    void panelClosed();

protected:
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void setupUI();
    void applyInfo(const GameInfo& info);
    void loadBgAsync(const QString& path);
    void setDescEditMode(bool edit);
    void toggleDescEditMode();
    void setNameEditMode(bool edit);

    int         m_gameIndex = -1;
    qreal       m_slideProgress = 0.0;
    qreal       m_fadeOpacity = 1.0;
    bool        m_visible = false;
    bool        m_isEditingDesc = false;
    bool        m_isEditingName = false;

    GameInfo    m_pendingInfo;
    int         m_pendingIndex = -1;

    // 异步图片加载：用 token 丢弃过期结果
    QString     m_loadingBgPath;   // 当前正在加载的路径

    QPropertyAnimation* m_anim;
    QPropertyAnimation* m_slideOutAnim;
    QPropertyAnimation* m_fadeAnim;

    QWidget*                m_contentContainer;
    QGraphicsOpacityEffect* m_contentEffect;

    QLabel*      m_nameLabel;
    QLineEdit*   m_nameEdit;
    QPushButton* m_editNameBtn;
    QLabel*      m_statsLabel;
    QTextEdit*   m_descEdit;
    QPushButton* m_launchBtn;
    QPushButton* m_bgBtn;
    QPushButton* m_execBtn;
    QPushButton* m_closeBtn;
    QPushButton* m_editDescBtn;

    QPixmap m_bgPixmap;
};