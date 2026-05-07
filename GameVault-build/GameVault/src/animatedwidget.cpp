#include "animatedwidget.h"
#include <QSequentialAnimationGroup>

AnimatedWidget::AnimatedWidget(QWidget* parent) : QWidget(parent) {
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);
}

void AnimatedWidget::fadeIn(int duration) {
    auto* anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedWidget::fadeOut(int duration) {
    auto* anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    anim->setDuration(duration);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedWidget::slideInFromBottom(int duration) {
    auto* opAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    opAnim->setDuration(duration);
    opAnim->setStartValue(0.0);
    opAnim->setEndValue(1.0);
    opAnim->setEasingCurve(QEasingCurve::OutExpo);

    QPoint end = pos();
    QPoint start = QPoint(end.x(), end.y() + 40);
    move(start);

    auto* posAnim = new QPropertyAnimation(this, "pos", this);
    posAnim->setDuration(duration);
    posAnim->setStartValue(start);
    posAnim->setEndValue(end);
    posAnim->setEasingCurve(QEasingCurve::OutExpo);

    opAnim->start(QAbstractAnimation::DeleteWhenStopped);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedWidget::popIn(int duration) {
    m_opacityEffect->setOpacity(1.0);
    // pop-in via geometry scale effect using animation on minimumSize trick
    auto* anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    anim->setDuration(duration);
    anim->setKeyValueAt(0.0, 0.0);
    anim->setKeyValueAt(0.6, 1.0);
    anim->setKeyValueAt(1.0, 1.0);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
