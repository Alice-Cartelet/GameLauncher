#include "gamecard.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QEnterEvent>
#include <QMouseEvent>
#include <cmath>

static const int RADIUS = 14;

GameCard::GameCard(int index, const GameInfo& info, QWidget* parent)
    : QWidget(parent), m_index(index), m_info(info)
{
    setFixedSize(210, 290);
    // Transparent background so global bg shows through
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

    m_hoverAnim = new QPropertyAnimation(this, "hoverProgress", this);

    updateInfo(info);
}

void GameCard::updateInfo(const GameInfo& info) {
    m_info   = info;
    m_bgPath = info.bgPath;
    if (!info.bgPath.isEmpty()) {
        QPixmap raw(info.bgPath);
        if (!raw.isNull())
            m_bgPixmap = raw.scaled(size(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        else
            m_bgPixmap = QPixmap();
    } else {
        m_bgPixmap = QPixmap();
    }
    update();
}

void GameCard::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect r = rect();
    qreal t = m_hoverProgress;  // 0 = idle(空闲), 1 = hovered(悬浮)

    // --- Clipping (圆角裁剪) ---
    QPainterPath clip;
    clip.addRoundedRect(r, RADIUS, RADIUS);
    p.setClipPath(clip);

    // --- 1. 卡片背景图 ---
    qreal bgOpacity = 0.9 + 0.85 * t;
    p.setOpacity(bgOpacity);

    if (!m_bgPixmap.isNull()) {
        QRect d = m_bgPixmap.rect();
        d.moveCenter(r.center());
        p.drawPixmap(d, m_bgPixmap);
    } else {
        int hash = 0;
        for (QChar c : m_info.name) hash = hash * 31 + c.unicode();
        float h1 = std::fmod(std::abs(hash) * 137.508f, 360.f);
        float h2 = std::fmod(h1 + 45.f, 360.f);
        QColor c1 = QColor::fromHsvF(h1/360.f, 0.50f, 0.30f);
        QColor c2 = QColor::fromHsvF(h2/360.f, 0.60f, 0.18f);
        QLinearGradient grad(0, 0, r.width(), r.height());
        grad.setColorAt(0, c1);
        grad.setColorAt(1, c2);
        p.fillRect(r, grad);
    }

    // --- 2. 基础黑透遮罩 ---
    qreal maskAlpha = 0.35 - 0.30 * t;
    p.setOpacity(1.0);
    p.fillRect(r, QColor(0, 0, 0, int(255 * maskAlpha)));

    // --- 3. 上下渐变阴影 (保证文字在任何背景下都绝对清晰) ---
    // 底部阴影（托底游戏名和游玩数据）
    qreal bottomStr = 0.35 + 0.50 * t;
    QLinearGradient bottom(0, r.height() * 0.45, 0, r.height());
    bottom.setColorAt(0, QColor(0,0,0,0));
    bottom.setColorAt(1, QColor(0,0,0, int(255 * bottomStr)));
    p.fillRect(r, bottom);

     //【新增】顶部阴影（悬浮时浮现，专门用来托底简介文字）
    if (t > 0.0) {
        qreal topStr = 0.4 * t;
        QLinearGradient top(0, 0, 0, r.height() * 0.1);
        top.setColorAt(0, QColor(0,0,0, int(255 * topStr)));
        top.setColorAt(1, Qt::transparent);
        p.fillRect(r, top);
    }

    // --- Top accent glow on hover (顶部科技感发光边) ---
    if (t > 0.0) {
        QLinearGradient topGlow(0,0,0,6);
        topGlow.setColorAt(0, QColor(80,160,255, int(220*t)));
        topGlow.setColorAt(1, Qt::transparent);
        p.fillRect(QRect(0,0,r.width(),6), topGlow);
    }

    // --- Game name (游戏名称，保持在底部上方) ---
    {
        qreal nameOpacity = 0.70 + 0.30 * t;
        p.setOpacity(nameOpacity);
        QFont f("Segoe UI", 13, QFont::Bold);
        p.setFont(f);
        p.setPen(Qt::white);
        QRect nameRect(14, r.height() - 90, r.width() - 28, 44);
        p.drawText(nameRect, Qt::AlignLeft | Qt::AlignBottom | Qt::TextWordWrap, m_info.name);
    }

    // --- Hover detail panel (悬浮弹出的详细信息) ---
    if (t > 0.01) {
        p.setOpacity(t);

        // Play count & Last played (保持在最底部)
        QString countStr = m_info.playCount > 0
                               ? QString("▶  游玩 %1 次").arg(m_info.playCount)
                               : "▶  尚未游玩";
        QFont sf("Segoe UI", 9);
        p.setFont(sf);
        p.setPen(QColor(160, 210, 255, 210));
        p.drawText(QRect(14, r.height()-46, r.width()-28, 18), Qt::AlignLeft, countStr);

        if (!m_info.lastPlayed.isEmpty()) {
            p.setPen(QColor(140,170,210,160));
            p.drawText(QRect(14, r.height()-28, r.width()-28, 16),
                       Qt::AlignLeft, "最后: " + m_info.lastPlayed);
        }

         /*Right-click hint (右上角编辑提示)
        QFont hf("Segoe UI", 8);
        p.setFont(hf);
        p.setPen(QColor(255,255,255,100));
        p.drawText(QRect(0, 10, r.width()-12, 16), Qt::AlignLeft, "   简 介");

        // 【重新设计的简介区域：移至顶部，严格裁剪】
        if (!m_info.description.isEmpty()) {
            QFont df("Segoe UI", 9);
            p.setFont(df);
            // 用偏冰蓝的亮白色，高级且高对比度
            p.setPen(QColor(235,240,255, int(230 * t)));

            // 移到顶部 (y=32开始)，给予 82px 的奢侈高度
            QRect descRect(14, 32, r.width()-28, 82);

            p.save();
            p.setClipRect(descRect); // ✂️ 强行裁剪，文字再长也绝不会溢出重叠
            p.drawText(descRect,
                       Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                       m_info.description);
            p.restore();
        }*/
    }

    // --- Border (边框) ---
    p.setClipping(false);
    p.setOpacity(1.0);
    QPainterPath border;
    border.addRoundedRect(r.adjusted(0,0,-1,-1), RADIUS, RADIUS);
    qreal ba = 0.18 + 0.22 * t;
    p.strokePath(border, QPen(QColor(255,255,255,int(255*ba)), 1.0));
}
void GameCard::enterEvent(QEnterEvent* e) {
    QWidget::enterEvent(e);
    animateHover(true);
}

void GameCard::leaveEvent(QEvent* e) {
    QWidget::leaveEvent(e);
    animateHover(false);
}

void GameCard::mousePressEvent(QMouseEvent* e) {
    emit detailRequested(m_index);
}

void GameCard::animateHover(bool enter) {
    m_hoverAnim->stop();
    m_hoverAnim->setDuration(enter ? 260 : 380);
    m_hoverAnim->setStartValue(m_hoverProgress);
    m_hoverAnim->setEndValue(enter ? 1.0 : 0.0);
    m_hoverAnim->setEasingCurve(enter ? QEasingCurve::OutCubic : QEasingCurve::InOutCubic);
    m_hoverAnim->start();
}

void GameCard::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (!m_bgPath.isEmpty()) {
        QPixmap raw(m_bgPath);
        if (!raw.isNull())
            m_bgPixmap = raw.scaled(e->size(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
}
