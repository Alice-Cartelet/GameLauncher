#include "mainwindow.h"
#include "setupdialog.h"
#include "orderdialog.h"
#include "pathmanagerdialog.h"
#include "executabledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QFileDialog>
#include <QProcess>
#include <QApplication>
#include <QScreen>
#include <QDialog>
#include <QResizeEvent>
#include <QShowEvent>
#include <QScrollBar>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QFileInfo>
#include <QTimer>
#include <QParallelAnimationGroup>
#include <QScreen>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QScreen>
#include <QGuiApplication>
/*void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange && !m_animating) {
        auto* e = static_cast<QWindowStateChangeEvent*>(event);

        if (isMinimized() && !(e->oldState() & Qt::WindowMinimized)) {
            m_restoreGeo = geometry();
            setWindowOpacity(0.0);  // 立即透明，不等 timer
            QTimer::singleShot(0, this, &MainWindow::playMinimizeAnimation);
        }
        else if (!isMinimized() && (e->oldState() & Qt::WindowMinimized)) {
            QTimer::singleShot(0, this, &MainWindow::playRestoreAnimation);
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::playMinimizeAnimation() {
    QRect startGeo  = m_restoreGeo;
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    QRect endGeo(screenGeo.center().x(), screenGeo.bottom(), 1, 1);

    QPixmap snapshot = grab();
    // setWindowOpacity(0.0) 已在 changeEvent 里提前调用，这里不需要了

    auto* ghost = new QWidget(nullptr);
    ghost->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ghost->setAttribute(Qt::WA_TranslucentBackground);
    ghost->setAttribute(Qt::WA_DeleteOnClose);
    ghost->setGeometry(startGeo);

    auto* label = new QLabel(ghost);
    label->setPixmap(snapshot);
    label->setScaledContents(true);
    label->setGeometry(0, 0, startGeo.width(), startGeo.height());

    ghost->show();

    auto* opacity = new QGraphicsOpacityEffect(ghost);
    ghost->setGraphicsEffect(opacity);

    auto* geoAnim = new QPropertyAnimation(ghost, "geometry");
    geoAnim->setDuration(220);
    geoAnim->setStartValue(startGeo);
    geoAnim->setEndValue(endGeo);
    geoAnim->setEasingCurve(QEasingCurve::InQuad);

    auto* fadeAnim = new QPropertyAnimation(opacity, "opacity");
    fadeAnim->setDuration(220);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    auto* group = new QParallelAnimationGroup(ghost);
    group->addAnimation(geoAnim);
    group->addAnimation(fadeAnim);

    connect(group, &QParallelAnimationGroup::finished, ghost, &QWidget::close);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
void MainWindow::playRestoreAnimation() {
    QRect endGeo    = m_restoreGeo;
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    QRect startGeo(screenGeo.center().x(), screenGeo.bottom(), 1, 1);

    // 先移到目标位置并透明，下一帧再 grab
    setWindowOpacity(0.0);
    setGeometry(endGeo);

    QTimer::singleShot(0, this, [this, startGeo, endGeo]() {
        QPixmap snapshot = grab();

        auto* ghost = new QWidget(nullptr);
        ghost->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        ghost->setAttribute(Qt::WA_TranslucentBackground);
        ghost->setAttribute(Qt::WA_DeleteOnClose);
        ghost->setGeometry(startGeo);

        auto* label = new QLabel(ghost);
        label->setPixmap(snapshot);
        label->setScaledContents(true);
        label->setGeometry(0, 0, endGeo.width(), endGeo.height());

        ghost->show();

        auto* opacity = new QGraphicsOpacityEffect(ghost);
        ghost->setGraphicsEffect(opacity);

        auto* geoAnim = new QPropertyAnimation(ghost, "geometry");
        geoAnim->setDuration(220);
        geoAnim->setStartValue(startGeo);
        geoAnim->setEndValue(endGeo);
        geoAnim->setEasingCurve(QEasingCurve::OutQuad);

        auto* fadeAnim = new QPropertyAnimation(opacity, "opacity");
        fadeAnim->setDuration(220);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);

        auto* group = new QParallelAnimationGroup(ghost);
        group->addAnimation(geoAnim);
        group->addAnimation(fadeAnim);

        connect(group, &QParallelAnimationGroup::finished, this, [this, ghost]() {
            ghost->close();
            setWindowOpacity(1.0);
        });

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });
}
*/
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_manager = new GameManager(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowIcon(QIcon(":/icons/default_game.png"));
    QScreen* screen = QApplication::primaryScreen();
    QRect sg = screen->geometry();
    int w = qMin(1400, int(sg.width() * 0.9));
    int h = qMin(860,  int(sg.height() * 0.88));
    setFixedSize(w, h);
    move((sg.width()-w)/2, ((sg.height()-h)/2)*0.5);

    m_notifTimer = new QTimer(this);
    m_notifTimer->setSingleShot(true);

    setupUI();
    checkFirstRun();
}

void MainWindow::setupUI() {
    auto* central = new QWidget(this);
    central->setObjectName("mainContainer");
    central->setStyleSheet("QWidget#mainContainer { background: transparent; }");
    setCentralWidget(central);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0,0,0,0);
    rootLayout->setSpacing(0);
    m_bgWidget = new BackgroundWidget(central);
    m_bgWidget->setDefaultGradient();
    auto* outerLayout = new QVBoxLayout(m_bgWidget);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);
    setupTitleBar();
    outerLayout->addWidget(m_titleBar);
    auto* body = new QWidget(m_bgWidget);
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0,0,0,0);
    bodyLayout->setSpacing(0);
    setupSidebar();
    bodyLayout->addWidget(m_sidebar);
    setupMainArea();
    bodyLayout->addWidget(m_scrollArea, 1);
    outerLayout->addWidget(body, 1);
    // Notification bar
    m_notifBar = new QWidget(m_bgWidget);
    m_notifBar->setMaximumHeight(0);
    m_notifBar->setStyleSheet("background: transparent;");
    m_notifLabel = new QLabel(m_notifBar);
    m_notifLabel->setAlignment(Qt::AlignCenter);
    auto* nb = new QVBoxLayout(m_notifBar);
    nb->setContentsMargins(0,2,0,8);
    nb->addWidget(m_notifLabel);
    outerLayout->addWidget(m_notifBar);
    rootLayout->addWidget(m_bgWidget);
    // Detail panel
    m_detailPanel = new DetailPanel(m_bgWidget);
    m_detailPanel->move(m_bgWidget->width(), 0);
    m_detailPanel->setFixedHeight(m_bgWidget->height());
    m_detailPanel->hide();
    connect(m_detailPanel, &DetailPanel::launchRequested,     this, &MainWindow::onGameLaunch);
    //connect(m_detailPanel, &DetailPanel::changeIconRequested, this, &MainWindow::onChangeGameIcon);
    connect(m_detailPanel, &DetailPanel::changeBgRequested,   this, &MainWindow::onChangeGameBg);
    connect(m_detailPanel, &DetailPanel::changeExecRequested, this, &MainWindow::onChangeGameExec);
    connect(m_detailPanel, &DetailPanel::descriptionChanged,  this, &MainWindow::onDescriptionChanged);
    connect(m_detailPanel, &DetailPanel::nameChanged,         this, &MainWindow::onGameNameChanged);

    // 面板弹出/收起：动画修改网格右边距，让卡片区自然向左收缩
    connect(m_detailPanel, &DetailPanel::panelClosed, this, [this]() {
        setGridRightMargin(28, 350);
    });
}
void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    static bool isFirstShow = true;
    if (isFirstShow) {
        isFirstShow = false;
        QTimer::singleShot(50, this, [this]() {
            QString savedBg = m_manager->globalBgPath();
            if (!savedBg.isEmpty() && QFile::exists(savedBg)) {
                m_bgWidget->setBackground(savedBg);
            }
        });
    }
}

void MainWindow::setupTitleBar() {
    m_titleBar = new QWidget(m_bgWidget);
    m_titleBar->setFixedHeight(60);
    m_titleBar->setStyleSheet(
        "background: rgba(0,0,0,0.35);"
        "border-bottom: 1px solid rgba(255,255,255,0.06);"
        );

    auto* layout = new QHBoxLayout(m_titleBar);
    layout->setContentsMargins(24, 0, 16, 0);
    layout->setSpacing(0);

    auto* logo = new QLabel("⬡  LOCAL GAME", m_titleBar);
    logo->setStyleSheet(
        "color: #ffffff; font-size: 18px; font-weight: 800;"
        "font-family: 'Segoe UI'; letter-spacing: 5px; background: transparent;"
        );
    layout->addWidget(logo);
    layout->addStretch();

    m_gameCountLabel = new QLabel("", m_titleBar);
    m_gameCountLabel->setStyleSheet(
        "color: rgba(255,255,255,0.4); font-size: 12px;"
        "font-family: 'Segoe UI'; background: transparent;"
        );
    layout->addWidget(m_gameCountLabel);
    layout->addSpacing(24);

    auto makeCtrl = [&](const QString& icon, const QString& color, auto slot) {
        auto* btn = new QPushButton(icon, m_titleBar);
        btn->setFixedSize(32, 32);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background: rgba(255,255,255,0.07); border: none;
                border-radius: 16px; color: rgba(255,255,255,0.6); font-size: 13px;
            }
            QPushButton:hover { background: %1; color: white; }
        )").arg(color));
        connect(btn, &QPushButton::clicked, this, slot);
        layout->addWidget(btn);
        layout->addSpacing(6);
    };

    makeCtrl("─", "rgba(255,200,0,0.4)", [this]() { showMinimized(); });
    makeCtrl("✕", "rgba(255,60,60,0.5)",  [this]() { close(); });

    m_titleBar->installEventFilter(this);
    m_titleBar->setMouseTracking(true);
}

void MainWindow::setupSidebar() {
    m_sidebar = new QWidget(m_bgWidget);
    m_sidebar->setFixedWidth(56);
    m_sidebar->setStyleSheet(
        "background: rgba(0,0,0,0.3);"
        "border-right: 1px solid rgba(255,255,255,0.05);"
        );

    auto* layout = new QVBoxLayout(m_sidebar);
    layout->setContentsMargins(8, 20, 8, 20);
    layout->setSpacing(8);

    auto makeBtn = [&](const QString& icon, const QString& tooltip) -> QPushButton* {
        auto* btn = new QPushButton(icon, m_sidebar);
        btn->setFixedSize(40, 40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setStyleSheet(R"(
            QPushButton {
                background: rgba(255,255,255,0.05);
                border: 1px solid rgba(255,255,255,0.07);
                border-radius: 10px; color: rgba(255,255,255,0.75);
                font-size: 15px; text-align: center; padding: 0;
            }
            QPushButton:hover {
                background: rgba(74,158,255,0.15);
                border-color: rgba(74,158,255,0.3); color: #ffffff;
            }
            QPushButton:pressed { background: rgba(74,158,255,0.25); }
        )");
        layout->addWidget(btn, 0, Qt::AlignHCenter);
        return btn;
    };

    m_rescanBtn         = makeBtn("↺", "重新扫描");
    m_changeScanPathBtn = makeBtn("📁", "修改目录");
    m_reorderBtn        = makeBtn("⇅", "调整排序");
    m_changeBgBtn       = makeBtn("🖼", "更换背景");
    m_appgameinfo       = makeBtn("ℹ", "应用信息");
    layout->addStretch();

    m_statusLabel = new QLabel("", m_sidebar);
    m_statusLabel->setStyleSheet(
        "color: rgba(255,255,255,0.25); font-size: 10px;"
        "font-family: 'Segoe UI'; background: transparent;"
        );
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    connect(m_rescanBtn,         &QPushButton::clicked, this, &MainWindow::onRescan);
    connect(m_changeBgBtn,       &QPushButton::clicked, this, &MainWindow::onChangeBg);
    connect(m_changeScanPathBtn, &QPushButton::clicked, this, &MainWindow::onChangeScanPath);
    connect(m_reorderBtn, &QPushButton::clicked, this, &MainWindow::onReorder);
    connect(m_appgameinfo, &QPushButton::clicked, this, &MainWindow::appgameinfo);
}

void MainWindow::setSidebarExpanded(bool /*expanded*/) {
    // 已简化为纯图标模式，无展开功能
}
void MainWindow::onReorder() {
    if (m_manager->getGames().isEmpty()) {
        showNotification("✗  没有游戏可以排序", true);
        return;
    }

    auto* dlg = new OrderDialog(m_manager->getGames(), this);

    connect(dlg, &QDialog::accepted, this, [this, dlg]() {
        QList<QString> ordered = dlg->orderedPaths();
        m_manager->applyOrder(ordered);
        rebuildCards();
        showNotification("✓  排列顺序已保存");
    });

    dlg->show();
}
void MainWindow::setupMainArea() {
    m_scrollArea = new QScrollArea(m_bgWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; }
        QScrollBar:vertical {
            background: rgba(255,255,255,0.03); width: 6px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,0.18); border-radius: 3px; min-height: 40px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background: transparent;");
    m_gridContainer->setAttribute(Qt::WA_TranslucentBackground);
    m_scrollArea->setWidget(m_gridContainer);
    m_scrollArea->viewport()->setStyleSheet("background: transparent;");
    m_scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
}

void MainWindow::checkFirstRun() {
    if (!m_manager->hasConfig()) {
        SetupDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            m_manager->setScanPath(dlg.selectedPath());
            m_manager->scanGames();
        }
    }
    rebuildCards();
}

void MainWindow::rebuildCards() {
    for (auto* c : m_cards) { c->hide(); c->deleteLater(); }
    m_cards.clear();
    delete m_gridContainer->layout();

    auto games = m_manager->getGames();
    m_gameCountLabel->setText(QString("%1 个游戏").arg(games.size()));

    if (games.isEmpty()) {
        auto* layout = new QVBoxLayout(m_gridContainer);
        layout->setAlignment(Qt::AlignCenter);
        auto* lbl = new QLabel("未找到游戏\n请检查路径并重新扫描", m_gridContainer);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet(
            "color: rgba(255,255,255,0.2); font-size: 18px;"
            "font-family: 'Segoe UI'; font-weight: 300;"
            );
        layout->addWidget(lbl);
        return;
    }

    auto* grid = new QGridLayout(m_gridContainer);
    grid->setContentsMargins(28, 28, 28, 28);
    grid->setSpacing(22);

    int cols = 3;
    for (int i = 0; i < games.size(); ++i) {
        auto* card = new GameCard(i, games[i], m_gridContainer);
        connect(card, &GameCard::launchRequested, this, &MainWindow::onGameLaunch);
        connect(card, &GameCard::detailRequested, this, &MainWindow::onGameDetail);
        grid->addWidget(card, i / cols, i % cols);
        m_cards.append(card);
    }

    for (auto* card : m_cards) {
        auto* effect = new QGraphicsOpacityEffect(card);
        effect->setOpacity(0.0);
        card->setGraphicsEffect(effect);
    }
    animateCardsIn();
}

void MainWindow::animateCardsIn() {
    for (int i = 0; i < m_cards.size(); ++i) {
        auto* card = m_cards[i];
        auto* effect = static_cast<QGraphicsOpacityEffect*>(card->graphicsEffect());
        if (!effect) continue;

        QTimer::singleShot(i * 55, card, [effect, card]() {
            auto* anim = new QPropertyAnimation(effect, "opacity", card);
            anim->setDuration(480);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);

            QObject::connect(anim, &QPropertyAnimation::finished, card, [card, effect]() {
                effect->setOpacity(1.0);
                card->setGraphicsEffect(nullptr);
            });

            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
}
void MainWindow::onRescan() {
    m_statusLabel->setText("扫描中...");
    m_manager->scanGames();
    rebuildCards();
    m_statusLabel->setText("扫描完成");
    showNotification("✓  游戏库已更新，共 " + QString::number(m_manager->getGames().size()) + " 个游戏");
}
void MainWindow::appgameinfo() {
    auto* dlg = new QDialog(this);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::NonModal);
    dlg->setFixedSize(360, 260);

    auto* container = new QWidget(dlg);
    container->setObjectName("aboutBox");
    container->setStyleSheet(R"(
        QWidget#aboutBox {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #0d1117, stop:1 #161b27);
            border-radius: 16px;
            border: 1px solid rgba(255,255,255,0.08);
        }
    )");

    auto* layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);

    auto* inner = new QVBoxLayout(container);
    inner->setContentsMargins(36, 32, 36, 32);
    inner->setSpacing(10);

    auto* title = new QLabel("LOCAL GAME", container);
    title->setStyleSheet(
        "color:#fff;font-size:22px;font-weight:800;"
        "font-family:'Segoe UI';letter-spacing:6px;background:transparent;");
    inner->addWidget(title);

    auto* sub = new QLabel(
        "应用版本：v 2.2.3     编译时间：2026-05-12\n"
        "开发者：Alice-Cartelet\n"
        "Github：https://github.com/Alice-Cartelet\n"
        "本产品基于Qt框架构建，依照Qt官方许可体系，采用GNU公共许可证第三版（LGPL v3）进行授权与分发。",
        container);
    sub->setWordWrap(true);
    sub->setStyleSheet(
        "color:rgba(255,255,255,0.35);font-size:12px;"
        "font-family:'Segoe UI';background:transparent;");
    inner->addWidget(sub);

    auto* updateLabel = new QLabel("正在检查更新", container);
    updateLabel->setStyleSheet(
        "color:rgba(255,255,255,0.25);font-size:12px;"
        "font-family:'Segoe UI';background:transparent;");
    inner->addWidget(updateLabel);
    auto* manager = new QNetworkAccessManager(dlg);
    auto* reply = manager->get(QNetworkRequest(QUrl("https://nuist.com.cn/gml")));
    auto* timer = new QTimer(dlg);
    timer->setSingleShot(true);
    timer->start(1000);
    connect(timer, &QTimer::timeout, dlg, [reply, updateLabel]() {
        reply->abort();
        updateLabel->setText("检查更新超时");
    });

    connect(reply, &QNetworkReply::finished, dlg, [reply, updateLabel, timer]() {
        timer->stop();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            updateLabel->setText("检查更新失败");
            return;
        }
        QString latest = QString(reply->readAll()).trimmed();
        QString current = "2.2.3";
        if (latest > current) {
            updateLabel->setText(QString("发现新版本：v %1  点击下载").arg(latest));
            updateLabel->setStyleSheet(
                "color:rgba(74,158,255,0.9);font-size:12px;"
                "font-family:'Segoe UI';background:transparent;"
                "text-decoration:underline;");
            updateLabel->setCursor(Qt::PointingHandCursor);
            struct ClickLabel : public QObject {
                bool eventFilter(QObject*, QEvent* e) override {
                    if (e->type() == QEvent::MouseButtonPress) {
                        QDesktopServices::openUrl(QUrl("https://github.com/Alice-Cartelet"));
                        return true;
                    }
                    return false;
                }
            };
            auto* cl = new ClickLabel();
            cl->setParent(updateLabel);
            updateLabel->installEventFilter(cl);
        } else {
            updateLabel->setText("您已经是最新版本：v "+ current);
        }
    });

    inner->addStretch();

    auto* closeBtn = new QPushButton("关  闭", container);
    closeBtn->setFixedHeight(42);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #4a9eff, stop:1 #7c5cfc);
            border:none; border-radius:10px; color:#fff;
            font-size:13px; font-weight:700;
            font-family:'Segoe UI'; letter-spacing:3px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5eaeff, stop:1 #9070ff);
        }
    )");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    inner->addWidget(closeBtn);

    auto* effect = new QGraphicsOpacityEffect(dlg);
    effect->setOpacity(0.0);
    dlg->setGraphicsEffect(effect);
    auto* anim = new QPropertyAnimation(effect, "opacity", dlg);
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(qApp, &QApplication::focusChanged,
                     dlg, [this, dlg](QWidget*, QWidget* now) {
                         if (now && !dlg->isAncestorOf(now) && now != dlg) {
                             struct EatOne : public QObject {
                                 bool eventFilter(QObject*, QEvent* e) override {
                                     if (e->type() == QEvent::MouseButtonPress ||
                                         e->type() == QEvent::MouseButtonRelease) {
                                         deleteLater();
                                         return true;
                                     }
                                     return false;
                                 }
                             };
                             auto* eater = new EatOne();
                             eater->setParent(dlg);
                             qApp->installEventFilter(eater);
                             dlg->accept();
                         }
                     });

    dlg->show();
}
void MainWindow::onChangeBg() {
    QString file = QFileDialog::getOpenFileName(this, "选择背景图片", "",
                                                "图片文件 (*.jpg *.jpeg *.png *.bmp *.webp)");
    if (!file.isEmpty()) {
        QString localBgPath = m_manager->setGlobalBg(file);
        if (!localBgPath.isEmpty()) {
            m_bgWidget->setBackground(localBgPath);
            showNotification("✓  背景已更新");
        }
    }
}

void MainWindow::onChangeScanPath() {
    auto* dlg = new PathManagerDialog(m_manager->getScanPaths(), this);

    connect(dlg, &QDialog::accepted, this, [this, dlg]() {
        QStringList newPaths = dlg->paths();
        if (newPaths.isEmpty()) {
            showNotification("✗  至少需要保留一个扫描目录", true);
            return;
        }
        m_manager->setScanPaths(newPaths);
        m_manager->scanGames();
        rebuildCards();
        showNotification(
            "✓  目录已更新，共 " +
            QString::number(m_manager->getGames().size()) + " 个游戏");
    });

    dlg->show();
}

void MainWindow::launchGame(int index) {
    auto& game = m_manager->getGame(index);

    if (game.execPath.isEmpty()) {
        ExecutableDialog dlg(game.path, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString exec = dlg.selectedExec();
            if (!exec.isEmpty()) {
                m_manager->setGameExec(index, exec);
                game.execPath = exec;
            } else return;
        } else return;
    }

    m_manager->recordLaunch(index);
    if (index < m_cards.size())
        m_cards[index]->updateInfo(m_manager->getGame(index));
    if (m_detailPanel->isVisible())
        m_detailPanel->showGame(index, m_manager->getGame(index));

    bool ok = QProcess::startDetached(game.execPath, {}, QFileInfo(game.execPath).absolutePath());
    if (ok) {
        showNotification("🚀  正在启动: " + game.name);
        QTimer::singleShot(800, this, [this]() { showMinimized(); });
    } else {
        showNotification("✗  启动失败: " + game.execPath, true);
    }
}

void MainWindow::onGameLaunch(int index) { launchGame(index); }

void MainWindow::onGameDetail(int index) {
    bool wasVisible = m_detailPanel->isVisible();
    m_detailPanel->showGame(index, m_manager->getGame(index));
    m_detailPanel->raise();

    if (!wasVisible) {
        setGridRightMargin(28 + m_detailPanel->width(), 450);
    }
}

void MainWindow::onChangeGameIcon(int index) {

    showNotification("  ✗  此功能已废弃", true);
}

void MainWindow::onChangeGameBg(int index) {
    QString file = QFileDialog::getOpenFileName(this, "选择游戏背景", "",
                                                "图片文件 (*.jpg *.jpeg *.png *.bmp *.webp)");
    if (!file.isEmpty()) {
        m_manager->setGameBg(index, file);
        if (index < m_cards.size())
            m_cards[index]->updateInfo(m_manager->getGame(index));
        showNotification("✓  游戏背景已更新");
    }
}

void MainWindow::onChangeGameExec(int index) {
    auto& game = m_manager->getGame(index);
    auto* dlg = new ExecutableDialog(game.path, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::NonModal);

    connect(dlg, &QDialog::accepted, this, [this, dlg, index]() {
        QString exec = dlg->selectedExec();
        if (!exec.isEmpty()) {
            m_manager->setGameExec(index, exec);
            showNotification("✓  可执行文件已更新");
        }
    });

    QObject::connect(qApp, &QApplication::focusChanged,
                     dlg, [this, dlg](QWidget*, QWidget* now) {
                         if (now && !dlg->isAncestorOf(now) && now != dlg) {
                             struct EatOne : public QObject {
                                 bool eventFilter(QObject*, QEvent* e) override {
                                     if (e->type() == QEvent::MouseButtonPress ||
                                         e->type() == QEvent::MouseButtonRelease) {
                                         deleteLater();
                                         return true;
                                     }
                                     return false;
                                 }
                             };
                             showNotification("×  游戏启动文件未更新",true);
                             auto* eater = new EatOne();
                             eater->setParent(dlg);
                             qApp->installEventFilter(eater);
                             dlg->accept();
                         }
                     });

    dlg->show();
}

void MainWindow::onDescriptionChanged(int index, const QString& desc) {
    m_manager->setGameDescription(index, desc);
    if (index < m_cards.size())
        m_cards[index]->updateInfo(m_manager->getGame(index));
}

void MainWindow::onGameNameChanged(int index, const QString& name) {
    m_manager->setGameCustomName(index, name);
    if (index < m_cards.size())
        m_cards[index]->updateInfo(m_manager->getGame(index));
    // 更新 titlebar 游戏数不变，但若面板仍显示则刷新名称
}

void MainWindow::showNotification(const QString& msg, bool isError) {
    m_notifTimer->stop();
    m_notifLabel->setText(msg);
    QString color = isError ? "rgba(255,70,70,0.9)" : "rgba(40,200,120,0.9)";
    m_notifBar->setStyleSheet(QString(
                                  "background: %1; border-top: 1px solid rgba(255,255,255,0.1);"
                                  ).arg(color));
    m_notifLabel->setStyleSheet(
        "color: white; font-size: 16px; font-family: 'Segoe UI';"
        "font-weight: 600; background: transparent; padding:2px 0;"
        );
    m_notifBar->setMaximumHeight(0);
    auto* anim = new QPropertyAnimation(m_notifBar, "maximumHeight", this);
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(48);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    m_notifTimer->singleShot(1600, this, [this]() {
        auto* a = new QPropertyAnimation(m_notifBar, "maximumHeight", this);
        a->setDuration(250);
        a->setStartValue(48);
        a->setEndValue(0);
        a->setEasingCurve(QEasingCurve::InCubic);
        a->start(QAbstractAnimation::DeleteWhenStopped);
    });
}
void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    m_bgWidget->resize(size());
    if (m_detailPanel) {
        m_detailPanel->setFixedHeight(m_bgWidget->height());
        if (!m_detailPanel->isVisible()) {
            m_detailPanel->move(m_bgWidget->width(), 0);
        }
    }
}

void MainWindow::setGridRightMargin(int targetRight, int duration) {
    auto* layout = qobject_cast<QGridLayout*>(m_gridContainer->layout());
    if (!layout) return;

    int left, top, right, bottom;
    layout->getContentsMargins(&left, &top, &right, &bottom);

    auto* anim = new QVariantAnimation(this);
    anim->setDuration(duration);
    anim->setStartValue(right);
    anim->setEndValue(targetRight);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this, left, top, bottom](const QVariant& v) {
        auto* l = qobject_cast<QGridLayout*>(m_gridContainer->layout());
        if (l) l->setContentsMargins(left, top, v.toInt(), bottom);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragPos = me->globalPosition().toPoint() - frameGeometry().topLeft();
                m_dragging = true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (m_dragging && (me->buttons() & Qt::LeftButton))
                move(me->globalPosition().toPoint() - m_dragPos);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}