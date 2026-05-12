#include "pathmanagerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

PathManagerDialog::PathManagerDialog(const QStringList& currentPaths,
                                     QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(520, 440);

    // ── 外层容器（圆角卡片） ──────────────────────────────────
    auto* root = new QWidget(this);
    root->setObjectName("pmContainer");
    root->setStyleSheet(R"(
        QWidget#pmContainer {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #0d1117, stop:1 #161b27);
            border-radius: 16px;
            border: 1px solid rgba(255,255,255,0.08);
        }
    )");

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(root);

    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(28, 28, 28, 24);
    layout->setSpacing(14);

    // ── 标题 ──────────────────────────────────────────────────
    auto* title = new QLabel("📁  目录管理", root);
    title->setStyleSheet(
        "color:#fff; font-size:18px; font-weight:800;"
        "font-family:'Segoe UI'; letter-spacing:3px; background:transparent;");
    layout->addWidget(title);

    auto* sub = new QLabel(
        "可添加多个扫描目录，启动器将合并显示所有目录下的游戏。\n"
        "重新扫描后生效。", root);
    sub->setWordWrap(true);
    sub->setStyleSheet(
        "color:rgba(255,255,255,0.32); font-size:12px;"
        "font-family:'Segoe UI'; background:transparent;");
    layout->addWidget(sub);

    // ── 分割线 ────────────────────────────────────────────────
    auto* line = new QWidget(root);
    line->setFixedHeight(1);
    line->setStyleSheet("background: rgba(255,255,255,0.07);");
    layout->addWidget(line);

    // ── 路径列表 ──────────────────────────────────────────────
    m_list = new QListWidget(root);
    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_list->setStyleSheet(R"(
        QListWidget {
            background: rgba(255,255,255,0.04);
            border: 1px solid rgba(255,255,255,0.07);
            border-radius: 10px;
            color: rgba(255,255,255,0.82);
            font-size: 12px;
            font-family: 'Segoe UI';
            padding: 4px;
            outline: none;
        }
        QListWidget::item {
            padding: 9px 14px;
            border-radius: 6px;
            margin: 1px 2px;
        }
        QListWidget::item:selected {
            background: rgba(74,158,255,0.22);
            color: #fff;
        }
        QListWidget::item:hover:!selected {
            background: rgba(255,255,255,0.05);
        }
        QScrollBar:vertical {
            background: transparent; width: 5px; margin: 4px 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,0.14);
            border-radius: 2px; min-height: 30px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }
    )");

    for (const QString& p : currentPaths)
        m_list->addItem(p);

    layout->addWidget(m_list, 1);

    connect(m_list, &QListWidget::itemSelectionChanged,
            this,   &PathManagerDialog::updateRemoveState);

    // ── 操作按钮行 ────────────────────────────────────────────
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(10);

    auto makeBtn = [&](const QString& text,
                       const QString& bg,
                       const QString& hover,
                       int minW = 0) -> QPushButton*
    {
        auto* btn = new QPushButton(text, root);
        btn->setFixedHeight(38);
        if (minW > 0) btn->setMinimumWidth(minW);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background: %1;
                border: none; border-radius: 9px;
                color: #fff; font-size: 13px; font-weight: 600;
                font-family: 'Segoe UI'; letter-spacing: 1px;
                padding: 0 16px;
            }
            QPushButton:hover   { background: %2; }
            QPushButton:pressed { opacity: 0.8; }
            QPushButton:disabled {
                background: rgba(255,255,255,0.05);
                color: rgba(255,255,255,0.18);
            }
        )").arg(bg, hover));
        return btn;
    };

    auto* addBtn = makeBtn(
        "＋  添加目录",
        "rgba(74,158,255,0.22)", "rgba(74,158,255,0.38)", 130);

    m_removeBtn = makeBtn(
        "－  移除选中",
        "rgba(255,70,70,0.18)", "rgba(255,70,70,0.32)", 130);
    m_removeBtn->setEnabled(false);

    actionRow->addWidget(addBtn);
    actionRow->addWidget(m_removeBtn);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    // ── 提示文字 ──────────────────────────────────────────────
    auto* hint = new QLabel("提示：支持多选，按 Shift / Ctrl 选中多条后批量移除", root);
    hint->setStyleSheet(
        "color:rgba(255,255,255,0.18); font-size:11px;"
        "font-family:'Segoe UI'; background:transparent;");
    layout->addWidget(hint);

    // ── 确认/取消行 ───────────────────────────────────────────
    auto* line2 = new QWidget(root);
    line2->setFixedHeight(1);
    line2->setStyleSheet("background: rgba(255,255,255,0.07);");
    layout->addWidget(line2);

    auto* confirmRow = new QHBoxLayout();
    confirmRow->setSpacing(10);

    auto* cancelBtn = makeBtn(
        "取  消",
        "rgba(255,255,255,0.07)", "rgba(255,255,255,0.12)", 100);

    auto* confirmBtn = makeBtn(
        "✓  确认",
        "qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #4a9eff, stop:1 #7c5cfc)",
        "qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #5eaeff, stop:1 #9070ff)", 110);

    confirmRow->addStretch();
    confirmRow->addWidget(cancelBtn);
    confirmRow->addWidget(confirmBtn);
    layout->addLayout(confirmRow);

    connect(addBtn,     &QPushButton::clicked, this, &PathManagerDialog::onAdd);
    connect(m_removeBtn,&QPushButton::clicked, this, &PathManagerDialog::onRemove);
    connect(cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);

    // ── 淡入动画 ──────────────────────────────────────────────
    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);
    auto* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(280);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QStringList PathManagerDialog::paths() const {
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i)
        result.append(m_list->item(i)->text());
    return result;
}

void PathManagerDialog::onAdd() {
    // 支持一次选多个目录：连续调用直到用户取消
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择游戏扫描目录", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;

    // 去重
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->text() == dir) return;

    m_list->addItem(dir);
    m_list->scrollToBottom();
}

void PathManagerDialog::onRemove() {
    // 倒序删除，避免索引偏移
    QList<QListWidgetItem*> selected = m_list->selectedItems();
    for (auto* item : selected)
        delete item;
    updateRemoveState();
}

void PathManagerDialog::updateRemoveState() {
    m_removeBtn->setEnabled(!m_list->selectedItems().isEmpty());
}