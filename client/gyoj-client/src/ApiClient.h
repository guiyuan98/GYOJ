#pragma once

#include "AppConfig.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrlQuery>

#include <functional>

class ApiClient : public QObject {
    Q_OBJECT

public:
    explicit ApiClient(AppConfig config, QObject* parent = nullptr);

    void setServerBaseUrl(const QString& serverBaseUrl);
    void setSessionCookie(const QByteArray& cookie);
    void startExamSession(int contestId, const QString& machineFingerprint);
    void heartbeat(const QString& sessionKey, const QString& machineFingerprint);
    void reportEvent(const QString& sessionKey, const QString& eventType, const QString& severity,
                     const QString& message, const QJsonObject& payload, bool lock);
    void fetchContestProblem(int contestId, const QString& problemId);
    void submitCode(int contestId, int problemId, const QString& language, const QString& code);

signals:
    void sessionStarted(const QJsonObject& payload);
    void heartbeatReceived(const QJsonObject& payload);
    void problemReceived(const QJsonObject& payload);
    void submitReceived(const QJsonObject& payload);
    void apiError(const QString& message);

private:
    QNetworkRequest request(const QString& path) const;
    void postJson(const QString& path, const QJsonObject& body, std::function<void(QJsonObject)> onSuccess);
    void getJson(const QString& path, const QUrlQuery& query, std::function<void(QJsonObject)> onSuccess);

    AppConfig m_config;
    QByteArray m_cookie;
    QNetworkAccessManager m_network;
};
