#include "backgroundwidget.h"
#include <QPainter>
#include <QLinearGradient>
#include <QResizeEvent>

BackgroundWidget::BackgroundWidget(QWidget* parent) : QWidget(parent) {
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void BackgroundWidget::setBackground(const QString& imagePath) {
    if (imagePath.isEmpty()) { setDefaultGradient(); return; }

    QPixmap px(imagePath);
    if (px.isNull()) { setDefaultGradient(); return; }

    m_imagePath   = imagePath;
    m_useGradient = false;
    m_nextPixmap  = px.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_crossfade   = 0.0;
    m_transitioning = true;
    startCrossfade();
}

void BackgroundWidget::setDefaultGradient() {
    m_useGradient   = true;
    m_imagePath     = QString();
    m_currentPixmap = QPixmap();
    m_nextPixmap    = QPixmap();
    m_crossfade     = 1.0;
    m_transitioning = false;
    update();
}

void BackgroundWidget::startCrossfade() {
    auto* anim = new QPropertyAnimation(this, "crossfade", this);
    anim->setDuration(700);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        // Transition done: next becomes current, crossfade resets to 0
        // so paintEvent just draws currentPixmap at full opacity (opacity = 1-0 = 1)
        m_currentPixmap = m_nextPixmap;
        m_nextPixmap    = QPixmap();
        m_crossfade     = 0.0;   // <-- KEY FIX: reset so current draws at full opacity
        m_transitioning = false;
        update();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void BackgroundWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_useGradient) {
        QLinearGradient grad(0, 0, width(), height());
        grad.setColorAt(0.0, QColor(10, 12, 20));
        grad.setColorAt(0.4, QColor(18, 22, 38));
        grad.setColorAt(1.0, QColor( 8, 10, 16));
        p.fillRect(rect(), grad);
        p.setPen(QColor(255,255,255,5));
        for (int y = 0; y < height(); y += 4)
            for (int x = (y/4)%4; x < width(); x += 8)
                p.drawPoint(x, y);
        return;
    }

    // --- Image mode ---
    if (m_transitioning) {
        // Old image fading out
        if (!m_currentPixmap.isNull()) {
            QRect d = m_currentPixmap.rect();
            d.moveCenter(rect().center());
            p.setOpacity(1.0 - m_crossfade);
            p.drawPixmap(d, m_currentPixmap);
        } else {
            // No old image: fill dark so new image fades in over black
            p.setOpacity(1.0 - m_crossfade);
            p.fillRect(rect(), QColor(10,12,20));
        }
        // New image fading in
        if (!m_nextPixmap.isNull()) {
            QRect d = m_nextPixmap.rect();
            d.moveCenter(rect().center());
            p.setOpacity(m_crossfade);
            p.drawPixmap(d, m_nextPixmap);
        }
    } else {
        // Steady state: just draw current at full opacity
        if (!m_currentPixmap.isNull()) {
            QRect d = m_currentPixmap.rect();
            d.moveCenter(rect().center());
            p.setOpacity(1.0);
            p.drawPixmap(d, m_currentPixmap);
        }
    }

    // Dark vignette overlay for readability
    p.setOpacity(1.0);
    QLinearGradient vignette(0, 0, 0, height());
    vignette.setColorAt(0.0, QColor(0,0,0,120));
    vignette.setColorAt(0.4, QColor(0,0,0, 40));
    vignette.setColorAt(1.0, QColor(0,0,0,180));
    p.fillRect(rect(), vignette);
}

void BackgroundWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (!m_imagePath.isEmpty()) {
        QPixmap px(m_imagePath);
        if (!px.isNull()) {
            m_currentPixmap = px.scaled(e->size(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            m_nextPixmap    = QPixmap();
            m_transitioning = false;
            m_crossfade     = 0.0;
            update();
        }
    }
}
