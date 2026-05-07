#pragma once
#include <QWidget>
#include <QPropertyAnimation>
#include "gamemanager.h"

class GameCard : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)
public:
    explicit GameCard(int index, const GameInfo& info, QWidget* parent = nullptr);
    void updateInfo(const GameInfo& info);

    qreal hoverProgress() const { return m_hoverProgress; }
    void  setHoverProgress(qreal v) { m_hoverProgress = v; update(); }

    int gameIndex() const { return m_index; }

signals:
    void launchRequested(int index);
    void detailRequested(int index);

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*)      override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*)    override;

private:
    void animateHover(bool enter);

    int      m_index;
    GameInfo m_info;
    qreal    m_hoverProgress = 0.0;

    QPropertyAnimation* m_hoverAnim;
    QPixmap m_bgPixmap;
    QString m_bgPath;
};
