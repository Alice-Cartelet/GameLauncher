#include "gamemanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QStandardPaths>

GameManager::GameManager(QObject* parent) : QObject(parent) {
    load();
}

// ── 配置状态 ────────────────────────────────────────────────

bool GameManager::hasConfig() const {
    return !m_scanPaths.isEmpty();
}

// ── 多路径接口 ───────────────────────────────────────────────

QStringList GameManager::getScanPaths() const {
    return m_scanPaths;
}

void GameManager::setScanPaths(const QStringList& paths) {
    m_scanPaths.clear();
    for (const QString& p : paths) {
        QString t = p.trimmed();
        if (!t.isEmpty() && !m_scanPaths.contains(t))
            m_scanPaths.append(t);
    }
    save();
}

void GameManager::addScanPath(const QString& path) {
    QString t = path.trimmed();
    if (!t.isEmpty() && !m_scanPaths.contains(t)) {
        m_scanPaths.append(t);
        save();
    }
}

void GameManager::removeScanPath(const QString& path) {
    m_scanPaths.removeAll(path.trimmed());
    save();
}

// ── 旧版单路径兼容接口 ────────────────────────────────────────

QString GameManager::getScanPath() const {
    return m_scanPaths.isEmpty() ? QString() : m_scanPaths.first();
}

void GameManager::setScanPath(const QString& path) {
    QString t = path.trimmed();
    if (t.isEmpty()) return;
    if (!m_scanPaths.contains(t))
        m_scanPaths.prepend(t);
    save();
}

// ── 扫描 ─────────────────────────────────────────────────────

void GameManager::scanGames() {
    if (m_scanPaths.isEmpty()) return;

    // 保留已有游戏的自定义信息
    QMap<QString, GameInfo> existing;
    for (const auto& g : m_games)
        existing[g.path] = g;

    m_games.clear();
    QSet<QString> seen; // 防止多个扫描路径扫到同一个子文件夹

    for (const QString& scanPath : m_scanPaths) {
        QDir dir(scanPath);
        if (!dir.exists()) continue;

        QStringList folders = dir.entryList(
            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

        for (const QString& folder : folders) {
            QString fullPath = dir.absoluteFilePath(folder);
            if (seen.contains(fullPath)) continue;
            seen.insert(fullPath);

            GameInfo info;
            info.folderName = folder;
            info.path       = fullPath;

            if (existing.contains(fullPath)) {
                const GameInfo& e  = existing[fullPath];
                info.execPath      = e.execPath;
                info.iconPath      = e.iconPath;
                info.bgPath        = e.bgPath;
                info.description   = e.description;
                info.playCount     = e.playCount;
                info.totalPlaySecs = e.totalPlaySecs;
                info.lastPlayed    = e.lastPlayed;
                info.customName    = e.customName;
            }
            info.name = info.customName.isEmpty() ? info.folderName : info.customName;
            m_games.append(info);
        }
    }
    loadOrder();
    save();
    emit gamesChanged();
}

// ── 游戏列表访问 ──────────────────────────────────────────────

QList<GameInfo> GameManager::getGames() const { return m_games; }
GameInfo& GameManager::getGame(int index)      { return m_games[index]; }

// ── 游戏信息修改 ──────────────────────────────────────────────

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
    m_games[index].lastPlayed =
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    save();
}

// ── 全局背景 ──────────────────────────────────────────────────

QString GameManager::globalBgPath() const { return m_globalBgPath; }

QString GameManager::setGlobalBg(const QString& sourcePath) {
    if (sourcePath.isEmpty()) return m_globalBgPath;
    QFileInfo srcInfo(sourcePath);
    if (!srcInfo.exists()) return m_globalBgPath;

    QString ext      = srcInfo.suffix();
    QString destPath = dataDirectory() + "/global_bg." + ext;

    if (srcInfo.absoluteFilePath() == QFileInfo(destPath).absoluteFilePath())
        return destPath;

    if (!m_globalBgPath.isEmpty() && m_globalBgPath != destPath)
        QFile::remove(m_globalBgPath);

    if (QFile::exists(destPath))
        QFile::remove(destPath);

    if (QFile::copy(sourcePath, destPath)) {
        m_globalBgPath = destPath;
        save();
        return destPath;
    }
    return m_globalBgPath;
}

// ── 路径工具 ──────────────────────────────────────────────────

QString GameManager::dataDirectory() const {
    static QString cachedDir;
    if (!cachedDir.isEmpty()) return cachedDir;

    QString appDir = QCoreApplication::applicationDirPath();
    QFile test(appDir + "/.write_test");
    if (test.open(QIODevice::WriteOnly)) {
        test.close(); test.remove();
        cachedDir = appDir;
    } else {
        cachedDir = QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);
        QDir().mkpath(cachedDir);
    }
    return cachedDir;
}

QString GameManager::configFilePath() const {
    return dataDirectory() + "/Gameinfo.dll";
}

// ── 加密 ──────────────────────────────────────────────────────

QByteArray GameManager::encryptDecrypt(const QByteArray& data) const {
    const QByteArray key = "StgManager_MySecretKey_2026_!@#";
    QByteArray result = data;
    for (int i = 0; i < result.size(); ++i)
        result[i] = result[i] ^ key[i % key.size()];
    return result;
}

// ── 持久化 ────────────────────────────────────────────────────

void GameManager::save() {
    QJsonObject root;

    // 写新格式（数组）
    QJsonArray pathArr;
    for (const auto& p : m_scanPaths) pathArr.append(p);
    root["scanPaths"] = pathArr;
    // 向后兼容：旧版只认 scanPath，保留第一条
    root["scanPath"]  = m_scanPaths.isEmpty() ? "" : m_scanPaths.first();
    root["globalBg"]  = m_globalBgPath;

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

    QByteArray jsonData      = QJsonDocument(root).toJson(QJsonDocument::Compact);
    QByteArray encryptedData = encryptDecrypt(jsonData);
    const QByteArray header  = "STG_ENC_V1";

    QFile f(configFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(header + encryptedData);
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

    const QByteArray header = "STG_ENC_V1";
    QByteArray jsonData;
    if (rawData.startsWith(header))
        jsonData = encryptDecrypt(rawData.mid(header.size()));
    else
        jsonData = rawData; // 兼容旧明文数据

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) return;

    QJsonObject root = doc.object();

    // 优先读新格式 scanPaths，降级读旧 scanPath
    QJsonArray pathArr = root["scanPaths"].toArray();
    if (!pathArr.isEmpty()) {
        for (const auto& v : pathArr) {
            QString p = v.toString().trimmed();
            if (!p.isEmpty() && !m_scanPaths.contains(p))
                m_scanPaths.append(p);
        }
    } else {
        QString old = root["scanPath"].toString().trimmed();
        if (!old.isEmpty()) m_scanPaths.append(old);
    }

    m_globalBgPath = root["globalBg"].toString();

    for (const auto& v : root["games"].toArray()) {
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
        // 兼容旧数据：folderName 为空时回退到 name 字段
        if (g.folderName.isEmpty())
            g.folderName = obj["name"].toString();
        g.name = g.customName.isEmpty() ? g.folderName : g.customName;
        m_games.append(g);
    }
    loadOrder();
}
// ── 排序持久化 ────────────────────────────────────────────────

QString GameManager::orderFilePath() const {
    return dataDirectory() + "/GameCfg.dll";
}

// orderedPaths：用户拖拽后，按新顺序排列的 game.path 列表
void GameManager::applyOrder(const QList<QString>& orderedPaths) {
    QMap<QString, GameInfo> byPath;
    for (const auto& g : m_games)
        byPath[g.path] = g;

    QList<GameInfo> reordered;
    reordered.reserve(orderedPaths.size());
    for (const QString& p : orderedPaths) {
        if (byPath.contains(p))
            reordered.append(byPath.take(p));
    }
    // 把不在 orderedPaths 里的游戏（理论上不应有）追加到末尾
    for (const auto& g : byPath)
        reordered.append(g);

    m_games = reordered;
    save();      // 更新 gameinfo.dll（游戏数据顺序）
    saveOrder(); // 更新 gameorder.dll（路径顺序索引）
    emit gamesChanged();
}

void GameManager::saveOrder() {
    QJsonArray arr;
    for (const auto& g : m_games)
        arr.append(g.path);

    QByteArray jsonData      = QJsonDocument(arr).toJson(QJsonDocument::Compact);
    QByteArray encryptedData = encryptDecrypt(jsonData);
    const QByteArray header  = "STG_ENC_V1";

    QFile f(orderFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(header + encryptedData);
        f.flush();
        f.close();
    }
}

void GameManager::loadOrder() {
    QFile f(orderFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray rawData = f.readAll();
    f.close();
    if (rawData.isEmpty()) return;

    const QByteArray header = "STG_ENC_V1";
    QByteArray jsonData;
    if (rawData.startsWith(header))
        jsonData = encryptDecrypt(rawData.mid(header.size()));
    else
        jsonData = rawData;

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isArray()) return;

    QJsonArray arr = doc.array();
    QList<QString> orderedPaths;
    for (const auto& v : arr)
        orderedPaths.append(v.toString());

    if (orderedPaths.isEmpty()) return;

    // 按保存的顺序重排，不在列表里的（新游戏）追加到末尾
    QMap<QString, GameInfo> byPath;
    for (const auto& g : m_games)
        byPath[g.path] = g;

    QList<GameInfo> reordered;
    for (const QString& p : orderedPaths) {
        if (byPath.contains(p))
            reordered.append(byPath.take(p));
    }
    for (const auto& g : byPath) // 新游戏追加末尾
        reordered.append(g);

    m_games = reordered;
}