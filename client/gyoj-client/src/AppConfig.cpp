#include "AppConfig.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QStringList jsonArrayToStringList(const QJsonValue& value, const QStringList& fallback)
{
    if (!value.isArray()) {
        return fallback;
    }
    QStringList result;
    for (const auto& item : value.toArray()) {
        result << item.toString();
    }
    return result;
}

AppConfig AppConfig::load()
{
    AppConfig config;
    QFile file(QCoreApplication::applicationDirPath() + "/gyoj-client.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return config;
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return config;
    }
    const auto obj = doc.object();
    config.serverBaseUrl = obj.value("serverBaseUrl").toString(config.serverBaseUrl);
    config.clientVersion = obj.value("clientVersion").toString(config.clientVersion);
    config.compilerPath = obj.value("compilerPath").toString(config.compilerPath);
    config.heartbeatSeconds = obj.value("heartbeatSeconds").toInt(config.heartbeatSeconds);
    config.localRunTimeoutMs = obj.value("localRunTimeoutMs").toInt(config.localRunTimeoutMs);
    config.enableFirewallLock = obj.value("enableFirewallLock").toBool(config.enableFirewallLock);
    config.defaultProcessBlacklist = jsonArrayToStringList(obj.value("processBlacklist"), config.defaultProcessBlacklist);
    config.networkAllowlist = jsonArrayToStringList(obj.value("networkAllowlist"), config.networkAllowlist);
    return config;
}

