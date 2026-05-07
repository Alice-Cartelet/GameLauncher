#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPropertyAnimation>

class BackgroundWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal crossfade READ crossfade WRITE setCrossfade)
public:
    explicit BackgroundWidget(QWidget* parent = nullptr);

    void setBackground(const QString& imagePath);
    void setDefaultGradient();

    qreal crossfade() const { return m_crossfade; }
    void  setCrossfade(qreal v) { m_crossfade = v; update(); }

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void startCrossfade();

    QPixmap m_currentPixmap;   // what's shown right now
    QPixmap m_nextPixmap;      // what we're fading INTO (only valid during transition)
    QString m_imagePath;       // original path for re-scale on resize
    qreal   m_crossfade  = 1.0;
    bool    m_useGradient = true;
    bool    m_transitioning = false;
};
