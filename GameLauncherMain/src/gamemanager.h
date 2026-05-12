#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QByteArray>

struct GameInfo {
    QString  name;
    QString  folderName;
    QString  customName;
    QString  path;
    QString  execPath;
    QString  iconPath;
    QString  bgPath;
    QString  description;
    int      playCount     = 0;
    qint64   totalPlaySecs = 0;
    QString  lastPlayed;
};

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject* parent = nullptr);

    bool            hasConfig() const;

    QStringList     getScanPaths() const;
    void            setScanPaths(const QStringList& paths);
    void            addScanPath(const QString& path);
    void            removeScanPath(const QString& path);
    QString         getScanPath() const;
    void            setScanPath(const QString& path);

    void            scanGames();

    QList<GameInfo> getGames() const;
    GameInfo&       getGame(int index);
    int             gameCount() const { return m_games.size(); }

    void setGameExec(int index, const QString& execPath);
    void setGameIcon(int index, const QString& iconPath);
    void setGameBg(int index, const QString& bgPath);
    void setGameDescription(int index, const QString& desc);
    void setGameCustomName(int index, const QString& name);
    void recordLaunch(int index);

    QString globalBgPath() const;
    QString setGlobalBg(const QString& path);

    // ── 排序 ──────────────────────────────────────────────────
    void applyOrder(const QList<QString>& orderedPaths);
    void saveOrder();
    void loadOrder();

    void save();

signals:
    void gamesChanged();

private:
    void       load();
    QString    configFilePath() const;
    QString    orderFilePath() const;
    QString    dataDirectory() const;
    QByteArray encryptDecrypt(const QByteArray& data) const;

    QStringList     m_scanPaths;
    QString         m_globalBgPath;
    QList<GameInfo> m_games;
};