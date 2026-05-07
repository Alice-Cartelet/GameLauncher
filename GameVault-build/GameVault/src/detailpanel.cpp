#include "detailpanel.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLinearGradient>
#include <QPainterPath>
#include <QDir>
#include <QLineEdit>
#include <QApplication>
#include <QMouseEvent>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

DetailPanel::DetailPanel(QWidget* parent) : QWidget(parent) {
    setFixedWidth(360);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    qApp->installEventFilter(this);

    m_anim = new QPropertyAnimation(this, "slideProgress", this);
    m_anim->setEasingCurve(QEasingCurve::OutExpo);

    // 切换游戏：滑出用的动画（借用 slideProgress 属性，但方向相反）
    m_slideOutAnim = new QPropertyAnimation(this, "slideProgress", this);
    m_slideOutAnim->setEasingCurve(QEasingCurve::InCubic);
    connect(m_slideOutAnim, &QPropertyAnimation::finished, this, [this]() {
        if (m_pendingIndex != -1) {
            m_gameIndex = m_pendingIndex;
            applyInfo(m_pendingInfo);
            m_pendingIndex = -1;
            setFadeOpacity(1.0);

            // 滑回来
            m_anim->stop();
            m_anim->setDuration(400);
            m_anim->setStartValue(m_slideProgress);
            m_anim->setEndValue(1.0);
            m_anim->setEasingCurve(QEasingCurve::OutExpo);
            m_anim->start();
        }
    });

    m_fadeAnim = new QPropertyAnimation(this, "fadeOpacity", this);

    setupUI();
    hide();
}

// 【新增】安全释放全局事件拦截
DetailPanel::~DetailPanel() {
    qApp->removeEventFilter(this);
}

// 【核心新增 1】捕获点击外部主界面的空白处
bool DetailPanel::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        if (m_visible && m_slideProgress > 0.0) {
            auto* me = static_cast<QMouseEvent*>(event);
            QPoint globalPos = me->globalPosition().toPoint();

            // 1. 判断点击是否发生在详情栏的外部
            QRect panelRect = QRect(mapToGlobal(QPoint(0, 0)), size());
            if (!panelRect.contains(globalPos)) {

                QWidget* clickedWidget = QApplication::widgetAt(globalPos);
                bool isInteractive = false;

                // 2. 向上追溯，看看点击的是不是其他的游戏卡片或按钮
                QWidget* w = clickedWidget;
                while (w) {
                    QString cName = w->metaObject()->className();
                    // 如果点的是别的卡片，放行（让卡片去触发交叉淡出切换）
                    if (cName == "GameCard" || cName == "QPushButton") {
                        isInteractive = true;
                        break;
                    }
                    w = w->parentWidget();
                }

                // 3. 如果点击的是纯粹的空白背景或滚动区域，就收起面板
                if (!isInteractive) {
                    hidePanel();
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// 【核心新增 2】捕获点击详情栏内部的空白处
void DetailPanel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        hidePanel();
    }
    QWidget::mousePressEvent(event);
}

void DetailPanel::setSlideProgress(qreal v) {
    m_slideProgress = v;
    if (parentWidget()) {
        int pw = parentWidget()->width();
        int x = int(pw - width() * v);
        move(x, 0);
        setFixedHeight(parentWidget()->height());
    }
    update();
}

void DetailPanel::setFadeOpacity(qreal v) {
    m_fadeOpacity = v;
    if (m_contentEffect) {
        m_contentEffect->setOpacity(v);
    }
    update();
}

void DetailPanel::showGame(int index, const GameInfo& info) {
    if (m_visible && m_gameIndex != index && m_gameIndex != -1) {
        // 切换游戏：先滑出到右边，再滑回来（带新游戏内容）
        if (m_slideOutAnim->state() == QAbstractAnimation::Running && m_pendingIndex == index) {
            return;
        }
        m_pendingIndex = index;
        m_pendingInfo = info;

        m_anim->stop();
        m_slideOutAnim->stop();
        m_slideOutAnim->setDuration(220);
        m_slideOutAnim->setStartValue(m_slideProgress);
        m_slideOutAnim->setEndValue(0.0);
        m_slideOutAnim->start();
    } else {
        m_gameIndex = index;
        applyInfo(info);
        setFadeOpacity(1.0);

        if (!m_visible) {
            show();
            raise();
            m_visible = true;
        }
        m_slideOutAnim->stop();
        m_anim->stop();
        m_anim->setDuration(450);
        m_anim->setStartValue(m_slideProgress);
        m_anim->setEndValue(1.0);
        m_anim->setEasingCurve(QEasingCurve::OutExpo);
        m_anim->start();
    }
}

void DetailPanel::hidePanel() {
    m_anim->stop();
    m_anim->setDuration(350);
    m_anim->setStartValue(m_slideProgress);
    m_anim->setEndValue(0.0);
    connect(m_anim, &QPropertyAnimation::finished, this, [this]() {
        if (m_slideProgress < 0.01) {
            hide();
            m_visible = false;
        }
    }, Qt::SingleShotConnection);
    m_anim->start();
    emit panelClosed();
}

void DetailPanel::applyInfo(const GameInfo& info) {
    m_nameLabel->setText(info.name);
    m_nameEdit->setText(info.customName.isEmpty() ? info.folderName : info.customName);
    m_nameEdit->setPlaceholderText(info.folderName);

    QString statsStr = QString("<span style='color:rgba(255,255,255,0.9); font-weight:bold;'>%1</span> 次游玩")
                           .arg(info.playCount);
    if (info.playCount == 0) statsStr = "✨ 尚未游玩";

    if (!info.lastPlayed.isEmpty()) {
        statsStr += QString("&nbsp;&nbsp;•&nbsp;&nbsp;最后游玩时间: <span style='color:rgba(255,255,255,0.75);'>%1</span>")
                        .arg(info.lastPlayed);
    }
    m_statsLabel->setText(statsStr);

    m_descEdit->setPlainText(info.description);

    setDescEditMode(false);
    setNameEditMode(false);

    // 先清空旧背景，动画先跑，图片异步加载完再渲染
    m_bgPixmap = QPixmap();
    update();
    loadBgAsync(info.bgPath);
}

void DetailPanel::loadBgAsync(const QString& path) {
    if (path.isEmpty()) return;

    // 记录本次请求的路径作为 token，忽略已过期的回调
    m_loadingBgPath = path;

    // 目标尺寸：面板实际大小，子线程里直接缩到位，主线程 fromImage 几乎零耗时
    const QSize targetSize = size().isEmpty() ? QSize(360, 860) : size();

    auto* watcher = new QFutureWatcher<QImage>(this);
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher, path]() {
        watcher->deleteLater();
        if (m_loadingBgPath != path) return;
        QImage img = watcher->result();
        if (!img.isNull()) {
            m_bgPixmap = QPixmap::fromImage(std::move(img));
            update();
        }
    });

    // 子线程：IO解码 + 缩放，全部在工作线程完成
    watcher->setFuture(QtConcurrent::run([path, targetSize]() -> QImage {
        QImage img(path);
        if (img.isNull()) return img;
        // 按比例缩放至面板大小（KeepAspectRatioByExpanding 与 paintEvent 保持一致）
        // 使用 FastTransformation 保证子线程速度，视觉差异极小
        QSize scaled = img.size().scaled(targetSize, Qt::KeepAspectRatioByExpanding);
        if (scaled.width() > img.width() * 3) {
            // 图片比面板小很多时不放大，避免模糊
            return img.convertToFormat(QImage::Format_RGB32);
        }
        return img.scaled(scaled, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                  .convertToFormat(QImage::Format_RGB32);
    }));
}

void DetailPanel::toggleDescEditMode() {
    setDescEditMode(!m_isEditingDesc);
}

void DetailPanel::setNameEditMode(bool edit) {
    m_isEditingName = edit;
    if (edit) {
        m_nameLabel->hide();
        m_nameEdit->show();
        m_nameEdit->setFocus();
        m_nameEdit->selectAll();
        m_editNameBtn->setText("✓");
        m_editNameBtn->setStyleSheet(R"(
            QPushButton {
                background: transparent; border: none;
                color: #4a9eff; font-size: 12px; font-weight: bold; font-family: 'Segoe UI';
            }
            QPushButton:hover { color: #6bb9ff; }
        )");
    } else {
        m_nameLabel->show();
        m_nameEdit->hide();
        m_editNameBtn->setText("✎");
        m_editNameBtn->setStyleSheet(R"(
            QPushButton {
                background: transparent; border: none;
                color: rgba(255,255,255,0.2); font-size: 12px; font-family: 'Segoe UI';
            }
            QPushButton:hover { color: rgba(255,255,255,0.7); }
        )");
    }
}

void DetailPanel::setDescEditMode(bool edit) {
    m_isEditingDesc = edit;
    m_descEdit->setReadOnly(!edit);

    if (edit) {
        m_editDescBtn->setText("✓ 保存");
        m_editDescBtn->setStyleSheet(R"(
            QPushButton {
                background: transparent; border: none;
                color: #4a9eff; font-size: 11px; font-weight: bold; font-family: 'Segoe UI';
            }
            QPushButton:hover { color: #6bb9ff; }
        )");

        m_descEdit->setStyleSheet(R"(
            QTextEdit {
                background: rgba(0,0,0,0.4);
                border: 1px solid rgba(255,255,255,0.15);
                border-radius: 8px;
                color: rgba(255,255,255,0.95);
                font-size: 13px; font-family: 'Segoe UI'; line-height: 1.5;
                padding: 8px;
            }
            QScrollBar:vertical { width: 4px; background: transparent; }
            QScrollBar::handle:vertical { background: rgba(255,255,255,0.2); border-radius:2px; }
        )");
        m_descEdit->setFocus();
    } else {
        m_editDescBtn->setText("✎ 编辑");
        m_editDescBtn->setStyleSheet(R"(
            QPushButton {
                background: transparent; border: none;
                color: rgba(255,255,255,0.25); font-size: 11px; font-family: 'Segoe UI';
            }
            QPushButton:hover { color: rgba(255,255,255,0.7); }
        )");

        m_descEdit->setStyleSheet(R"(
            QTextEdit {
                background: transparent;
                border: 1px solid transparent;
                color: rgba(255,255,255,0.85);
                font-size: 13px; font-family: 'Segoe UI'; line-height: 1.5;
                padding: 0px;
            }
            QScrollBar:vertical { width: 4px; background: transparent; }
            QScrollBar::handle:vertical { background: rgba(255,255,255,0.2); border-radius:2px; }
        )");

        emit descriptionChanged(m_gameIndex, m_descEdit->toPlainText());
    }
}

void DetailPanel::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentEffect = new QGraphicsOpacityEffect(this);
    m_contentEffect->setOpacity(1.0);
    m_contentContainer->setGraphicsEffect(m_contentEffect);
    mainLayout->addWidget(m_contentContainer);

    auto* root = new QVBoxLayout(m_contentContainer);
    root->setContentsMargins(32, 32, 32, 32);
    root->setSpacing(0);

    auto* topRow = new QHBoxLayout();
    m_closeBtn = new QPushButton("✕", m_contentContainer);
    m_closeBtn->setFixedSize(36, 36);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(0,0,0,0.25); border: none; border-radius: 18px;
            color: rgba(255,255,255,0.7); font-size: 14px;
        }
        QPushButton:hover { background: rgba(0,0,0,0.6); color: white; }
    )");
    topRow->addStretch();
    topRow->addWidget(m_closeBtn);
    root->addLayout(topRow);

    root->addStretch(1);

    // 游戏名称行：大标题 + 小铅笔按钮 + 隐藏的内联输入框
    auto* nameRow = new QHBoxLayout();
    nameRow->setContentsMargins(0, 0, 0, 0);
    nameRow->setSpacing(6);
    nameRow->setAlignment(Qt::AlignBottom);

    m_nameLabel = new QLabel(m_contentContainer);
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_nameLabel->setStyleSheet(
        "color: #ffffff; font-size: 26px; font-weight: 900;"
        "font-family: 'Segoe UI', 'Microsoft YaHei'; line-height: 1.1;"
        "background: transparent; letter-spacing: -1px;"
        );

    m_nameEdit = new QLineEdit(m_contentContainer);
    m_nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_nameEdit->setFixedHeight(36);
    m_nameEdit->hide();
    m_nameEdit->setStyleSheet(R"(
        QLineEdit {
            background: rgba(0,0,0,0.4);
            border: 1px solid rgba(74,158,255,0.5);
            border-radius: 6px;
            color: #ffffff;
            font-size: 18px; font-weight: 700;
            font-family: 'Segoe UI', 'Microsoft YaHei';
            padding: 2px 8px;
        }
        QLineEdit:focus { border-color: rgba(74,158,255,0.9); }
    )");

    m_editNameBtn = new QPushButton("✎", m_contentContainer);
    m_editNameBtn->setFixedSize(22, 22);
    m_editNameBtn->setCursor(Qt::PointingHandCursor);
    m_editNameBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent; border: none;
            color: rgba(255,255,255,0.2); font-size: 12px; font-family: 'Segoe UI';
        }
        QPushButton:hover { color: rgba(255,255,255,0.7); }
    )");

    nameRow->addWidget(m_nameLabel);
    nameRow->addWidget(m_nameEdit);
    nameRow->addWidget(m_editNameBtn, 0, Qt::AlignBottom);
    root->addLayout(nameRow);
    root->addSpacing(12);

    m_statsLabel = new QLabel(m_contentContainer);
    m_statsLabel->setTextFormat(Qt::RichText);
    m_statsLabel->setStyleSheet(
        "color: rgba(255,255,255,0.55); font-size: 12px;"
        "font-family: 'Segoe UI'; background: transparent;"
        );
    root->addWidget(m_statsLabel);
    root->addSpacing(24);

    auto* descHeaderRow = new QHBoxLayout();
    auto* descTitle = new QLabel("简  介", m_contentContainer);
    descTitle->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold; font-family: 'Segoe UI'; letter-spacing: 1px;");

    m_editDescBtn = new QPushButton("✎ 编辑", m_contentContainer);
    m_editDescBtn->setCursor(Qt::PointingHandCursor);

    descHeaderRow->addWidget(descTitle);
    descHeaderRow->addStretch();
    descHeaderRow->addWidget(m_editDescBtn);
    root->addLayout(descHeaderRow);
    root->addSpacing(6);

    m_descEdit = new QTextEdit(m_contentContainer);
    m_descEdit->setFixedHeight(90);
    m_descEdit->setPlaceholderText("暂无简介...");
    root->addWidget(m_descEdit);
    root->addSpacing(32);

    m_launchBtn = new QPushButton("开 始 游 戏", m_contentContainer);
    m_launchBtn->setFixedHeight(54);
    m_launchBtn->setCursor(Qt::PointingHandCursor);
    m_launchBtn->setStyleSheet(R"(
        QPushButton {
            background: #ffffff; border: none; border-radius: 27px;
            color: #000000; font-size: 16px; font-weight: 800;
            font-family: 'Segoe UI'; letter-spacing: 2px;
        }
        QPushButton:hover { background: #e0e0e0; transform: scale(1.02); }
        QPushButton:pressed { background: #b0b0b0; }
    )");
    root->addWidget(m_launchBtn);
    root->addSpacing(16);

    auto* editRow = new QHBoxLayout();
    editRow->setSpacing(12);
    m_bgBtn   = new QPushButton("🖼 更换背景", m_contentContainer);
    m_execBtn = new QPushButton("⚙ 设置路径", m_contentContainer);
    QString ghostBtnStyle = R"(
        QPushButton {
            background: rgba(255,255,255,0.05); border: none; border-radius: 16px;
            color: rgba(255,255,255,0.6); font-size: 12px; font-weight: 600; font-family: 'Segoe UI'; padding: 8px 0;
        }
        QPushButton:hover { background: rgba(255,255,255,0.15); color: #fff; }
    )";
    m_bgBtn->setStyleSheet(ghostBtnStyle);
    m_execBtn->setStyleSheet(ghostBtnStyle);
    m_bgBtn->setCursor(Qt::PointingHandCursor);
    m_execBtn->setCursor(Qt::PointingHandCursor);

    editRow->addWidget(m_bgBtn, 1);
    editRow->addWidget(m_execBtn, 1);
    root->addLayout(editRow);

    connect(m_closeBtn,    &QPushButton::clicked, this, &DetailPanel::hidePanel);
    connect(m_launchBtn,   &QPushButton::clicked, this, [this]() { emit launchRequested(m_gameIndex); });
    connect(m_bgBtn,       &QPushButton::clicked, this, [this]() { emit changeBgRequested(m_gameIndex); });
    connect(m_execBtn,     &QPushButton::clicked, this, [this]() { emit changeExecRequested(m_gameIndex); });
    connect(m_editDescBtn, &QPushButton::clicked, this, &DetailPanel::toggleDescEditMode);

    // 自定义名称：点击小铅笔 / 回车 / 失焦保存
    connect(m_editNameBtn, &QPushButton::clicked, this, [this]() {
        if (!m_isEditingName) {
            setNameEditMode(true);
        } else {
            QString newName = m_nameEdit->text().trimmed();
            m_nameLabel->setText(newName.isEmpty() ? m_nameEdit->placeholderText() : newName);
            setNameEditMode(false);
            emit nameChanged(m_gameIndex, newName);
        }
    });
    connect(m_nameEdit, &QLineEdit::returnPressed, m_editNameBtn, &QPushButton::click);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {
        if (m_isEditingName) m_editNameBtn->click();
    });
}

void DetailPanel::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect r = rect();

    QLinearGradient baseBg(0, 0, 0, height());
    baseBg.setColorAt(0.0, QColor(20, 20, 25, 255));
    baseBg.setColorAt(1.0, QColor( 5,  5,  8, 255));
    p.fillRect(r, baseBg);

    if (m_fadeOpacity > 0.0) {
        p.setOpacity(m_fadeOpacity);
        if (!m_bgPixmap.isNull()) {
            // pixmap 已在子线程预缩放至面板尺寸，直接居中裁切绘制，无需再 scale
            int x = (m_bgPixmap.width()  - r.width())  / 2;
            int y = (m_bgPixmap.height() - r.height()) / 2;
            p.drawPixmap(0, 0, m_bgPixmap, x, y, r.width(), r.height());

            QLinearGradient maskGrad(0, 0, 0, r.height());
            maskGrad.setColorAt(0.0, QColor(0, 0, 0, 0));
            maskGrad.setColorAt(0.3, QColor(0, 0, 0, 20));
            maskGrad.setColorAt(0.6, QColor(0, 0, 0, 140));
            maskGrad.setColorAt(0.85, QColor(0, 0, 0, 230));
            maskGrad.setColorAt(1.0, QColor(0, 0, 0, 255));
            p.fillRect(r, maskGrad);
        }
    }

    p.setOpacity(m_fadeOpacity);
    QLinearGradient leftLine(0, 0, 0, height());
    leftLine.setColorAt(0.0, QColor(255,255,255,0));
    leftLine.setColorAt(0.5, QColor(255,255,255,60));
    leftLine.setColorAt(1.0, QColor(255,255,255,0));
    p.fillRect(QRect(0, 0, 1, height()), leftLine);
}