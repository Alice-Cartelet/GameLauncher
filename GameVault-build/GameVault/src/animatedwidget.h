#pragma once
#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>

class AnimatedWidget : public QWidget {
    Q_OBJECT
public:
    explicit AnimatedWidget(QWidget* parent = nullptr);

    void fadeIn(int duration = 400);
    void fadeOut(int duration = 300);
    void slideInFromBottom(int duration = 500);
    void popIn(int duration = 350);

protected:
    QGraphicsOpacityEffect* m_opacityEffect;
};
