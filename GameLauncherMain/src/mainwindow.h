#pragma once
#include <QMainWindow>
#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QList>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QMouseEvent>
#include <QShowEvent>
#include "gamemanager.h"
#include "backgroundwidget.h"
#include "gamecard.h"
#include "detailpanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
protected:
    void resizeEvent(QResizeEvent* e) override;
    //void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
private slots:
    void onChangeScanPath();
    void appgameinfo();
    void onRescan();
    void onReorder();
    void onChangeBg();
    void onGameLaunch(int index);
    void onGameDetail(int index);
    void onChangeGameIcon(int index);
    void onChangeGameBg(int index);
    void onChangeGameExec(int index);
    void onDescriptionChanged(int index, const QString& desc);
    void onGameNameChanged(int index, const QString& name);
    void rebuildCards();
private:
    bool m_animating = false;
    QRect m_restoreGeo;
    void setupUI();
    void setupTitleBar();
    void setupSidebar();
    //void playMinimizeAnimation();
    //void playRestoreAnimation();
    void setupMainArea();
    void checkFirstRun();
    void showNotification(const QString& msg, bool isError = false);
    void animateCardsIn();
    void launchGame(int index);
    void fadeOtherCards(GameCard* hoveredCard, bool isHovering);
    void setSidebarExpanded(bool expanded);
    void setGridRightMargin(int targetRight, int duration);
    GameManager*      m_manager;
    BackgroundWidget* m_bgWidget;
    QPushButton*      m_reorderBtn = nullptr;
    DetailPanel*      m_detailPanel;
    QWidget*          m_titleBar;
    QWidget*          m_sidebar;
    QScrollArea*      m_scrollArea;
    QWidget*          m_gridContainer;
    QLabel*           m_statusLabel;
    QLabel*           m_gameCountLabel;
    QPushButton*      m_rescanBtn;
    QPushButton*      m_changeBgBtn;
    QPushButton*      m_appgameinfo;
    QPushButton*      m_changeScanPathBtn;
    QWidget*          m_notifBar;
    QLabel*           m_notifLabel;
    QTimer*           m_notifTimer;
    QList<GameCard*>  m_cards;
    QPoint            m_dragPos;
    bool              m_dragging = false;
    bool              m_sidebarExpanded = false;
    QPropertyAnimation* m_sidebarAnim = nullptr;
};