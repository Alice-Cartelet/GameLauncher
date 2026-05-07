#include "executabledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

ExecutableDialog::ExecutableDialog(const QString& gamePath, QWidget* parent)
    : QDialog(parent), m_gamePath(gamePath)
{
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(520, 480);
    setupUI();
    scanExecutables(gamePath);

    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);
    QTimer::singleShot(30, this, [effect, this]() {
        auto* anim = new QPropertyAnimation(effect, "opacity", this);
        anim->setDuration(400);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QString ExecutableDialog::selectedExec() const {
    return m_selectedExec;
}

void ExecutableDialog::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);

    auto* container = new QWidget(this);
    container->setObjectName("execContainer");
    container->setStyleSheet(R"(
        QWidget#execContainer {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #131720, stop:1 #0c1018);
            border-radius: 18px;
            border: 1px solid rgba(255,255,255,0.07);
        }
    )");

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(0);

    auto* titleRow = new QHBoxLayout();
    titleRow->setContentsMargins(0,0,0,0);

    auto* title = new QLabel("选择启动程序", container);
    title->setStyleSheet(
        "color: #ffffff; font-size: 20px; font-weight: 700;"
        "font-family: 'Segoe UI'; letter-spacing: 3px;"
        );
    titleRow->addWidget(title);
    titleRow->addStretch();

    auto* closeBtn = new QPushButton("✕", container);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
    QPushButton {
        background: rgba(255,255,255,0.07); border: none;
        border-radius: 14px; color: rgba(255,255,255,0.45); font-size: 11px;
    }
    QPushButton:hover { background: rgba(255,60,60,0.45); color: white; }
)");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    titleRow->addWidget(closeBtn);

    layout->addLayout(titleRow);
    layout->addSpacing(6);

    auto* sub = new QLabel("从游戏目录中选择可执行文件", container);
    sub->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 12px; font-family: 'Segoe UI';");
    layout->addWidget(sub);
    layout->addSpacing(24);

    m_list = new QListWidget(container);
    m_list->setStyleSheet(R"(
        QListWidget {
            background: rgba(255,255,255,0.04);
            border: 1px solid rgba(255,255,255,0.08);
            border-radius: 10px;
            color: #d0d8e8;
            font-size: 13px;
            font-family: 'Segoe UI';
            outline: none;
        }
        QListWidget::item {
            padding: 10px 16px;
            border-bottom: 1px solid rgba(255,255,255,0.04);
        }
        QListWidget::item:selected {
            background: rgba(74,158,255,0.18);
            color: #4a9eff;
            border-left: 3px solid #4a9eff;
        }
        QListWidget::item:hover {
            background: rgba(255,255,255,0.06);
        }
        QScrollBar:vertical {
            background: transparent; width: 6px;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,0.2); border-radius: 3px;
        }
    )");
    layout->addWidget(m_list);
    layout->addSpacing(20);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    m_browseBtn = new QPushButton("手动选择", container);
    m_browseBtn->setFixedHeight(44);
    m_browseBtn->setCursor(Qt::PointingHandCursor);
    m_browseBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(255,255,255,0.06);
            border: 1px solid rgba(255,255,255,0.12);
            border-radius: 10px;
            color: rgba(255,255,255,0.7);
            font-size: 13px; font-family: 'Segoe UI';
        }
        QPushButton:hover { background: rgba(255,255,255,0.1); }
    )");

    m_confirmBtn = new QPushButton("确认启动", container);
    m_confirmBtn->setFixedHeight(44);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #4a9eff, stop:1 #7c5cfc);
            border: none; border-radius: 10px;
            color: white; font-size: 13px; font-weight: 700;
            font-family: 'Segoe UI'; letter-spacing: 1px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5eaeff, stop:1 #9070ff);
        }
    )");

    btnRow->addWidget(m_browseBtn);
    btnRow->addWidget(m_confirmBtn);
    layout->addLayout(btnRow);

    root->addWidget(container);

    connect(m_confirmBtn, &QPushButton::clicked, this, &ExecutableDialog::onConfirm);
    connect(m_browseBtn,  &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择可执行文件", m_gamePath,
#ifdef Q_OS_WIN
            "可执行文件 (*.exe *.bat *.cmd)"
#else
            "所有文件 (*)"
#endif
        );
        if (!file.isEmpty()) {
            m_selectedExec = file;
            accept();
        }
    });
}

void ExecutableDialog::scanExecutables(const QString& path) {
    QDir dir(path);
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.exe" << "*.bat";
#else
    // No filter - show all files in root; optionally look for executables
    filters << "*";
#endif

    QStringList files = dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
    // Also search one level deep
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto& sub : subDirs) {
        QDir subDir(dir.absoluteFilePath(sub));
        for (const auto& f : subDir.entryList(filters, QDir::Files)) {
            files.append(sub + "/" + f);
        }
    }

    for (const QString& f : files) {
        m_list->addItem(f);
    }
}

void ExecutableDialog::onConfirm() {
    auto* item = m_list->currentItem();
    if (!item) {
        return;
    }
    QString rel = item->text();
    m_selectedExec = QDir(m_gamePath).absoluteFilePath(rel);
    accept();
}
