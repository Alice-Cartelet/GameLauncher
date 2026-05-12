#include "orderdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

OrderDialog::OrderDialog(const QList<GameInfo>& games, QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(520, 540);

    // ── 外层卡片 ─────────────────────────────────────────────
    auto* root = new QWidget(this);
    root->setObjectName("odContainer");
    root->setStyleSheet(R"(
        QWidget#odContainer {
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

    // ── 标题区 ────────────────────────────────────────────────
    auto* title = new QLabel("⇅  游戏排列顺序", root);
    title->setStyleSheet(
        "color:#fff; font-size:18px; font-weight:800;"
        "font-family:'Segoe UI'; letter-spacing:3px; background:transparent;");
    layout->addWidget(title);

    auto* sub = new QLabel(
        "拖拽行或使用右侧箭头调整顺序，确认后刷新主界面。", root);
    sub->setStyleSheet(
        "color:rgba(255,255,255,0.32); font-size:12px;"
        "font-family:'Segoe UI'; background:transparent;");
    layout->addWidget(sub);

    auto* line = new QWidget(root);
    line->setFixedHeight(1);
    line->setStyleSheet("background: rgba(255,255,255,0.07);");
    layout->addWidget(line);

    // ── 列表 + 上下按钮 ───────────────────────────────────────
    auto* listRow = new QHBoxLayout();
    listRow->setSpacing(10);

    m_list = new QListWidget(root);
    m_list->setDragDropMode(QAbstractItemView::InternalMove);
    m_list->setDefaultDropAction(Qt::MoveAction);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setStyleSheet(R"(
        QListWidget {
            background: rgba(255,255,255,0.04);
            border: 1px solid rgba(255,255,255,0.07);
            border-radius: 10px;
            color: rgba(255,255,255,0.85);
            font-size: 13px;
            font-family: 'Segoe UI';
            padding: 4px;
            outline: none;
        }
        QListWidget::item {
            padding: 10px 14px;
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
        /* 拖拽指示线 */
        QListWidget::item:drop-enabled {
            background: rgba(74,158,255,0.12);
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

    // 填充列表项，用 path 存在 UserRole 里作为唯一标识
    for (int i = 0; i < games.size(); ++i) {
        const auto& g = games[i];
        // 序号 + 名称，序号在确认时重新生成，这里仅展示
        auto* item = new QListWidgetItem(
            QString("  %1.  %2").arg(i + 1, 3).arg(g.name));
        item->setData(Qt::UserRole, g.path);
        // 拖拽手柄提示
        item->setToolTip(g.path);
        m_list->addItem(item);
    }

    connect(m_list, &QListWidget::itemSelectionChanged,
            this,   &OrderDialog::updateButtonStates);

    // 拖拽完成后刷新序号显示
    connect(m_list->model(), &QAbstractItemModel::rowsMoved,
            this, [this](const QModelIndex&, int, int, const QModelIndex&, int) {
                for (int i = 0; i < m_list->count(); ++i) {
                    auto* item = m_list->item(i);
                    QString path = item->data(Qt::UserRole).toString();
                    // 只更新序号，保留原名
                    QString oldText = item->text().trimmed();
                    // 去掉旧序号（格式 "  N.  名称"）
                    int dotIdx = oldText.indexOf('.');
                    QString namePart = (dotIdx >= 0)
                                           ? oldText.mid(dotIdx + 1).trimmed()
                                           : oldText;
                    item->setText(QString("  %1.  %2").arg(i + 1, 3).arg(namePart));
                }
            });

    listRow->addWidget(m_list, 1);

    // ── 右侧上下箭头 ──────────────────────────────────────────
    auto* arrowCol = new QVBoxLayout();
    arrowCol->setSpacing(8);
    arrowCol->addStretch();

    auto makeArrow = [&](const QString& text) -> QPushButton* {
        auto* btn = new QPushButton(text, root);
        btn->setFixedSize(38, 38);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setEnabled(false);
        btn->setStyleSheet(R"(
            QPushButton {
                background: rgba(255,255,255,0.07);
                border: 1px solid rgba(255,255,255,0.09);
                border-radius: 9px;
                color: rgba(255,255,255,0.7);
                font-size: 16px;
            }
            QPushButton:hover:!disabled {
                background: rgba(74,158,255,0.25);
                border-color: rgba(74,158,255,0.4);
                color: #fff;
            }
            QPushButton:pressed:!disabled {
                background: rgba(74,158,255,0.38);
            }
            QPushButton:disabled {
                color: rgba(255,255,255,0.18);
                border-color: rgba(255,255,255,0.05);
            }
        )");
        return btn;
    };

    m_upBtn   = makeArrow("↑");
    m_downBtn = makeArrow("↓");

    connect(m_upBtn,   &QPushButton::clicked, this, &OrderDialog::moveUp);
    connect(m_downBtn, &QPushButton::clicked, this, &OrderDialog::moveDown);

    arrowCol->addWidget(m_upBtn);
    arrowCol->addWidget(m_downBtn);
    arrowCol->addStretch();
    listRow->addLayout(arrowCol);

    layout->addLayout(listRow, 1);

    // ── 提示 ──────────────────────────────────────────────────
    auto* hint = new QLabel("💡  可直接拖拽行来调整顺序", root);
    hint->setStyleSheet(
        "color:rgba(255,255,255,0.18); font-size:11px;"
        "font-family:'Segoe UI'; background:transparent;");
    layout->addWidget(hint);

    // ── 确认/取消 ─────────────────────────────────────────────
    auto* line2 = new QWidget(root);
    line2->setFixedHeight(1);
    line2->setStyleSheet("background: rgba(255,255,255,0.07);");
    layout->addWidget(line2);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

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
        )").arg(bg, hover));
        return btn;
    };

    auto* cancelBtn = makeBtn(
        "取  消",
        "rgba(255,255,255,0.07)", "rgba(255,255,255,0.12)", 100);
    auto* confirmBtn = makeBtn(
        "✓  确认",
        "qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #4a9eff, stop:1 #7c5cfc)",
        "qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #5eaeff, stop:1 #9070ff)", 110);

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);
    layout->addLayout(btnRow);

    connect(cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);

    // ── 淡入 ──────────────────────────────────────────────────
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

QList<QString> OrderDialog::orderedPaths() const {
    QList<QString> result;
    for (int i = 0; i < m_list->count(); ++i)
        result.append(m_list->item(i)->data(Qt::UserRole).toString());
    return result;
}

void OrderDialog::moveUp() {
    int row = m_list->currentRow();
    if (row <= 0) return;
    auto* item = m_list->takeItem(row);
    m_list->insertItem(row - 1, item);
    m_list->setCurrentRow(row - 1);

    // 刷新序号
    for (int i = 0; i < m_list->count(); ++i) {
        auto* it = m_list->item(i);
        int dot = it->text().indexOf('.');
        QString name = (dot >= 0) ? it->text().mid(dot + 1).trimmed() : it->text().trimmed();
        it->setText(QString("  %1.  %2").arg(i + 1, 3).arg(name));
    }
    updateButtonStates();
}

void OrderDialog::moveDown() {
    int row = m_list->currentRow();
    if (row < 0 || row >= m_list->count() - 1) return;
    auto* item = m_list->takeItem(row);
    m_list->insertItem(row + 1, item);
    m_list->setCurrentRow(row + 1);

    for (int i = 0; i < m_list->count(); ++i) {
        auto* it = m_list->item(i);
        int dot = it->text().indexOf('.');
        QString name = (dot >= 0) ? it->text().mid(dot + 1).trimmed() : it->text().trimmed();
        it->setText(QString("  %1.  %2").arg(i + 1, 3).arg(name));
    }
    updateButtonStates();
}

void OrderDialog::updateButtonStates() {
    int row = m_list->currentRow();
    m_upBtn->setEnabled(row > 0);
    m_downBtn->setEnabled(row >= 0 && row < m_list->count() - 1);
}