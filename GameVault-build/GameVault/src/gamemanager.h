#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QByteArray> // 新增引入 QByteArray

struct GameInfo {
    QString  name;           // 显示名（优先 customName，否则文件夹名）
    QString  folderName;     // 原始文件夹名
    QString  customName;     // 用户自定义名称（空则使用 folderName）
    QString  path;
    QString  execPath;
    QString  iconPath;
    QString  bgPath;
    QString  description;
    int      playCount   = 0;
    qint64   totalPlaySecs = 0;
    QString  lastPlayed;
};

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject* parent = nullptr);

    bool    hasConfig() const;
    QString getScanPath() const;
    void    setScanPath(const QString& path);
    void    scanGames();

    QList<GameInfo>  getGames() const;
    GameInfo&        getGame(int index);
    int              gameCount() const { return m_games.size(); }

    void setGameExec(int index, const QString& execPath);
    void setGameIcon(int index, const QString& iconPath);
    void setGameBg(int index, const QString& bgPath);
    void setGameDescription(int index, const QString& desc);
    void setGameCustomName(int index, const QString& name);
    void recordLaunch(int index);

    QString globalBgPath() const;

    // 返回 QString，用于将复制后的本地路径返回给主界面
    QString setGlobalBg(const QString& path);

    void save();

signals:
    void gamesChanged();

private:
    void    load();
    QString configFilePath() const;
    QString dataDirectory() const;

    // 【新增】自定义加密/解密辅助函数
    QByteArray encryptDecrypt(const QByteArray& data) const;

    QString         m_scanPath;
    QString         m_globalBgPath;
    QList<GameInfo> m_games;
};