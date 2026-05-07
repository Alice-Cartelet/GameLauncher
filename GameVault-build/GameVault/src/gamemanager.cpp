#include "gamemanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QStandardPaths>

GameManager::GameManager(QObject* parent) : QObject(parent) {
    load();
}

bool GameManager::hasConfig() const { return !m_scanPath.isEmpty(); }
QString GameManager::getScanPath() const { return m_scanPath; }

void GameManager::setScanPath(const QString& path) {
    m_scanPath = path;
    save();
}

void GameManager::scanGames() {
    if (m_scanPath.isEmpty()) return;

    QDir dir(m_scanPath);
    QStringList folders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QMap<QString, GameInfo> existing;
    for (const auto& g : m_games) existing[g.path] = g;

    m_games.clear();
    for (const QString& folder : folders) {
        QString fullPath = dir.absoluteFilePath(folder);
        GameInfo info;
        info.folderName = folder;
        info.path = fullPath;
        if (existing.contains(fullPath)) {
            info.execPath      = existing[fullPath].execPath;
            info.iconPath      = existing[fullPath].iconPath;
            info.bgPath        = existing[fullPath].bgPath;
            info.description   = existing[fullPath].description;
            info.playCount     = existing[fullPath].playCount;
            info.totalPlaySecs = existing[fullPath].totalPlaySecs;
            info.lastPlayed    = existing[fullPath].lastPlayed;
            info.customName    = existing[fullPath].customName;
        }
        info.name = info.customName.isEmpty() ? info.folderName : info.customName;
        m_games.append(info);
    }
    save();
    emit gamesChanged();
}

QList<GameInfo> GameManager::getGames() const { return m_games; }
GameInfo& GameManager::getGame(int index) { return m_games[index]; }

void GameManager::setGameExec(int index, const QString& v) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].execPath = v; save();
}
void GameManager::setGameIcon(int index, const QString& v) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].iconPath = v; save();
}
void GameManager::setGameBg(int index, const QString& v) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].bgPath = v; save();
}
void GameManager::setGameCustomName(int index, const QString& v) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].customName = v.trimmed();
    m_games[index].name = m_games[index].customName.isEmpty()
                          ? m_games[index].folderName
                          : m_games[index].customName;
    save();
}
void GameManager::setGameDescription(int index, const QString& v) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].description = v; save();
}
void GameManager::recordLaunch(int index) {
    if (index < 0 || index >= m_games.size()) return;
    m_games[index].playCount++;
    m_games[index].lastPlayed = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    save();
}

QString GameManager::globalBgPath() const { return m_globalBgPath; }

QString GameManager::setGlobalBg(const QString& sourcePath) {
    if (sourcePath.isEmpty()) return m_globalBgPath;

    QFileInfo srcInfo(sourcePath);
    if (!srcInfo.exists()) return m_globalBgPath;

    QString ext = srcInfo.suffix();
    QString destPath = dataDirectory() + "/global_bg." + ext;

    if (srcInfo.absoluteFilePath() == QFileInfo(destPath).absoluteFilePath()) {
        return destPath;
    }

    if (!m_globalBgPath.isEmpty() && m_globalBgPath != destPath) {
        QFile::remove(m_globalBgPath);
    }

    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }

    if (QFile::copy(sourcePath, destPath)) {
        m_globalBgPath = destPath;
        save();
        return destPath;
    }

    return m_globalBgPath;
}

QString GameManager::dataDirectory() const {
    static QString cachedDir;
    if (!cachedDir.isEmpty()) return cachedDir;

    QString appDir = QCoreApplication::applicationDirPath();
    QFile test(appDir + "/.write_test");

    if (test.open(QIODevice::WriteOnly)) {
        test.close();
        test.remove();
        cachedDir = appDir;
    } else {
        cachedDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(cachedDir);
    }
    return cachedDir;
}

QString GameManager::configFilePath() const {
    return dataDirectory() + "/gameinfo.dll";
}

// === 异或加密/解密实现 ===
QByteArray GameManager::encryptDecrypt(const QByteArray& data) const {
    QByteArray key = "StgManager_MySecretKey_2026_!@#";
    QByteArray result = data;

    for (int i = 0; i < result.size(); ++i) {
        result[i] = result[i] ^ key[i % key.size()];
    }
    return result;
}

void GameManager::save() {
    QJsonObject root;
    root["scanPath"] = m_scanPath;
    root["globalBg"] = m_globalBgPath;

    QJsonArray arr;
    for (const auto& g : m_games) {
        QJsonObject obj;
        obj["name"]          = g.name;
        obj["folderName"]    = g.folderName;
        obj["customName"]    = g.customName;
        obj["path"]          = g.path;
        obj["execPath"]      = g.execPath;
        obj["iconPath"]      = g.iconPath;
        obj["bgPath"]        = g.bgPath;
        obj["description"]   = g.description;
        obj["playCount"]     = g.playCount;
        obj["totalPlaySecs"] = g.totalPlaySecs;
        obj["lastPlayed"]    = g.lastPlayed;
        arr.append(obj);
    }
    root["games"] = arr;

    QByteArray jsonData = QJsonDocument(root).toJson(QJsonDocument::Compact);
    QByteArray encryptedData = encryptDecrypt(jsonData);

    QByteArray fileHeader = "STG_ENC_V1";

    QString path = configFilePath();
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(fileHeader + encryptedData);
        f.flush();
        f.close();
    }
}

void GameManager::load() {
    QFile f(configFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;

    QByteArray rawData = f.readAll();
    f.close();

    if (rawData.isEmpty()) return;

    QByteArray jsonData;
    QByteArray fileHeader = "STG_ENC_V1";

    if (rawData.startsWith(fileHeader)) {
        QByteArray encryptedData = rawData.mid(fileHeader.size());
        jsonData = encryptDecrypt(encryptedData);
    } else {
        // 兼容读取未经加密的旧数据
        jsonData = rawData;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (doc.isNull()) return;
    QJsonObject root  = doc.object();
    m_scanPath        = root["scanPath"].toString();
    m_globalBgPath    = root["globalBg"].toString();

    QJsonArray arr = root["games"].toArray();
    for (const auto& v : arr) {
        QJsonObject obj = v.toObject();
        GameInfo g;
        g.folderName    = obj["folderName"].toString();
        g.customName    = obj["customName"].toString();
        g.path          = obj["path"].toString();
        g.execPath      = obj["execPath"].toString();
        g.iconPath      = obj["iconPath"].toString();
        g.bgPath        = obj["bgPath"].toString();
        g.description   = obj["description"].toString();
        g.playCount     = obj["playCount"].toInt();
        g.totalPlaySecs = obj["totalPlaySecs"].toInteger();
        g.lastPlayed    = obj["lastPlayed"].toString();
        // 兼容旧数据：folderName 为空时用旧 name 字段
        if (g.folderName.isEmpty()) g.folderName = obj["name"].toString();
        g.name = g.customName.isEmpty() ? g.folderName : g.customName;
        m_games.append(g);
    }
}