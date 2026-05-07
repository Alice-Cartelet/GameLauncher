#include "setupdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QPainter>
#include <QLinearGradient>
#include <QTimer>

SetupDialog::SetupDialog(QWidget* parent) : QDialog(parent) {
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(600, 400);
    setupUI();
    QTimer::singleShot(50, this, &SetupDialog::animateIn);
}

QString SetupDialog::selectedPath() const {
    return m_selectedPath;
}

void SetupDialog::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // Main container
    auto* container = new QWidget(this);
    container->setObjectName("setupContainer");
    container->setStyleSheet(R"(
        QWidget#setupContainer {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #0d1117, stop:0.5 #161b27, stop:1 #0a0e18);
            border-radius: 20px;
            border: 1px solid rgba(255,255,255,0.08);
        }
    )");

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(60, 55, 60, 55);
    layout->setSpacing(0);

    // Accent line
    auto* accentLine = new QWidget(container);
    accentLine->setFixedHeight(3);
    accentLine->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 transparent, stop:0.2 #4a9eff, stop:0.8 #7c5cfc, stop:1 transparent);"
        "border-radius: 2px;"
    );
    layout->addWidget(accentLine);
    layout->addSpacing(32);

    m_titleLabel = new QLabel("GAME VAULT", container);
    m_titleLabel->setStyleSheet(
        "color: #ffffff;"
        "font-family: 'Segoe UI', sans-serif;"
        "font-size: 32px;"
        "font-weight: 800;"
        "letter-spacing: 8px;"
    );
    layout->addWidget(m_titleLabel);
    layout->addSpacing(10);

    m_subtitleLabel = new QLabel("选择您的游戏库根目录", container);
    m_subtitleLabel->setStyleSheet(
        "color: rgba(255,255,255,0.45);"
        "font-family: 'Segoe UI', sans-serif;"
        "font-size: 13px;"
        "letter-spacing: 1px;"
    );
    layout->addWidget(m_subtitleLabel);
    layout->addSpacing(45);

    // Path input row
    auto* inputRow = new QHBoxLayout();
    inputRow->setSpacing(10);

    m_pathEdit = new QLineEdit(container);
    m_pathEdit->setPlaceholderText("D:\\Games  或  /home/user/games");
    m_pathEdit->setFixedHeight(48);
    m_pathEdit->setStyleSheet(R"(
        QLineEdit {
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.12);
            border-radius: 10px;
            color: #e8eaf0;
            font-size: 13px;
            padding: 0 18px;
            font-family: 'Segoe UI', sans-serif;
        }
        QLineEdit:focus {
            border: 1px solid rgba(74,158,255,0.6);
            background: rgba(74,158,255,0.05);
        }
        QLineEdit::placeholder {
            color: rgba(255,255,255,0.25);
        }
    )");

    m_browseBtn = new QPushButton("浏览", container);
    m_browseBtn->setFixedSize(80, 48);
    m_browseBtn->setCursor(Qt::PointingHandCursor);
    m_browseBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(74,158,255,0.15);
            border: 1px solid rgba(74,158,255,0.4);
            border-radius: 10px;
            color: #4a9eff;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
        }
        QPushButton:hover {
            background: rgba(74,158,255,0.25);
            border-color: rgba(74,158,255,0.7);
        }
        QPushButton:pressed {
            background: rgba(74,158,255,0.35);
        }
    )");

    inputRow->addWidget(m_pathEdit);
    inputRow->addWidget(m_browseBtn);
    layout->addLayout(inputRow);
    layout->addSpacing(30);

    // Confirm button
    m_confirmBtn = new QPushButton("开始扫描", container);
    m_confirmBtn->setFixedHeight(52);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #4a9eff, stop:1 #7c5cfc);
            border: none;
            border-radius: 12px;
            color: #ffffff;
            font-size: 15px;
            font-weight: 700;
            font-family: 'Segoe UI', sans-serif;
            letter-spacing: 2px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5eaeff, stop:1 #9070ff);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #3a8eef, stop:1 #6c4cec);
        }
    )");
    layout->addWidget(m_confirmBtn);
    layout->addStretch();

    root->addWidget(container);

    connect(m_browseBtn,  &QPushButton::clicked, this, &SetupDialog::onBrowse);
    connect(m_confirmBtn, &QPushButton::clicked, this, &SetupDialog::onConfirm);
}

void SetupDialog::animateIn() {
    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    auto* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(500);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SetupDialog::onBrowse() {
    QString dir = QFileDialog::getExistingDirectory(this, "选择游戏库目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void SetupDialog::onConfirm() {
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty()) {
        m_pathEdit->setStyleSheet(m_pathEdit->styleSheet() +
            "QLineEdit { border-color: rgba(255,80,80,0.7); }");
        return;
    }
    m_selectedPath = path;
    accept();
}
