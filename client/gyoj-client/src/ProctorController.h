#pragma once

#include "AppConfig.h"

#include <QObject>
#include <QSet>
#include <QTimer>

class QApplication;

class ProctorController : public QObject {
    Q_OBJECT

public:
    explicit ProctorController(AppConfig config, QObject* parent = nullptr);
    ~ProctorController() override;

    QString machineFingerprint() const;
    void start();
    void stop();
    void lockScreen(const QString& reason);
    void unlockScreen();
    bool applyNetworkAllowlist();
    bool restoreNetwork();

signals:
    void violationDetected(const QString& eventType, const QString& message, bool lock);
    void lockedChanged(bool locked, const QString& reason);

private:
    void installKeyboardHook();
    void uninstallKeyboardHook();
    void scanProcesses();
    void onClipboardChanged();

    AppConfig m_config;
    QTimer m_processTimer;
    bool m_locked = false;
    QString m_lockReason;
    QSet<QString> m_reportedProcesses;
};

