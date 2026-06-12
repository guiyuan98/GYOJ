#pragma once

#include "AppConfig.h"

#include <QObject>
#include <QString>

struct LocalRunResult {
    bool compiled = false;
    bool timedOut = false;
    int exitCode = -1;
    QString stdoutText;
    QString stderrText;
    QString compileOutput;
};

class LocalRunner : public QObject {
    Q_OBJECT

public:
    explicit LocalRunner(AppConfig config, QObject* parent = nullptr);
    LocalRunResult runCppSample(const QString& code, const QString& input);

private:
    bool assignToJobObject(qint64 pid);

    AppConfig m_config;
};

