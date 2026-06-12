#include "ApiClient.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

ApiClient::ApiClient(AppConfig config, QObject* parent)
    : QObject(parent), m_config(std::move(config))
{
}

void ApiClient::setServerBaseUrl(const QString& serverBaseUrl)
{
    m_config.serverBaseUrl = serverBaseUrl.trimmed();
    while (m_config.serverBaseUrl.endsWith('/')) {
        m_config.serverBaseUrl.chop(1);
    }
}

void ApiClient::setSessionCookie(const QByteArray& cookie)
{
    m_cookie = cookie;
}

QNetworkRequest ApiClient::request(const QString& path) const
{
    QUrl url(m_config.serverBaseUrl + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("X-GYOJ-Client-Version", m_config.clientVersion.toUtf8());
    if (!m_cookie.isEmpty()) {
        req.setRawHeader("Cookie", m_cookie);
    }
    return req;
}

void ApiClient::postJson(const QString& path, const QJsonObject& body, std::function<void(QJsonObject)> onSuccess)
{
    auto* reply = m_network.post(request(path), QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess = std::move(onSuccess)] {
        const auto bytes = reply->readAll();
        const auto error = reply->error();
        reply->deleteLater();
        if (error != QNetworkReply::NoError) {
            emit apiError(reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(bytes);
        if (!doc.isObject()) {
            emit apiError("Invalid JSON response");
            return;
        }
        onSuccess(doc.object());
    });
}

void ApiClient::getJson(const QString& path, const QUrlQuery& query, std::function<void(QJsonObject)> onSuccess)
{
    QUrl url(m_config.serverBaseUrl + path);
    url.setQuery(query);
    QNetworkRequest req(url);
    req.setRawHeader("X-GYOJ-Client-Version", m_config.clientVersion.toUtf8());
    if (!m_cookie.isEmpty()) {
        req.setRawHeader("Cookie", m_cookie);
    }
    auto* reply = m_network.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess = std::move(onSuccess)] {
        const auto bytes = reply->readAll();
        const auto error = reply->error();
        reply->deleteLater();
        if (error != QNetworkReply::NoError) {
            emit apiError(reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(bytes);
        if (!doc.isObject()) {
            emit apiError("Invalid JSON response");
            return;
        }
        onSuccess(doc.object());
    });
}

void ApiClient::startExamSession(int contestId, const QString& machineFingerprint)
{
    postJson("/api/proctoring/session/start", {
        {"contest_id", contestId},
        {"machine_fingerprint", machineFingerprint},
        {"client_version", m_config.clientVersion}
    }, [this](const QJsonObject& payload) { emit sessionStarted(payload); });
}

void ApiClient::heartbeat(const QString& sessionKey, const QString& machineFingerprint)
{
    postJson("/api/proctoring/session/heartbeat", {
        {"session_key", sessionKey},
        {"machine_fingerprint", machineFingerprint},
        {"client_version", m_config.clientVersion}
    }, [this](const QJsonObject& payload) { emit heartbeatReceived(payload); });
}

void ApiClient::reportEvent(const QString& sessionKey, const QString& eventType, const QString& severity,
                            const QString& message, const QJsonObject& payload, bool lock)
{
    postJson("/api/proctoring/event", {
        {"session_key", sessionKey},
        {"event_type", eventType},
        {"severity", severity},
        {"message", message},
        {"payload", payload},
        {"lock", lock}
    }, [](const QJsonObject&) {});
}

void ApiClient::fetchContestProblem(int contestId, const QString& problemId)
{
    QUrlQuery query;
    query.addQueryItem("contest_id", QString::number(contestId));
    query.addQueryItem("problem_id", problemId);
    getJson("/api/contest/problem", query, [this](const QJsonObject& payload) { emit problemReceived(payload); });
}

void ApiClient::submitCode(int contestId, int problemId, const QString& language, const QString& code)
{
    postJson("/api/submission", {
        {"contest_id", contestId},
        {"problem_id", problemId},
        {"language", language},
        {"code", code}
    }, [this](const QJsonObject& payload) { emit submitReceived(payload); });
}
