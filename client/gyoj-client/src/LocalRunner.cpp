#include "LocalRunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

LocalRunner::LocalRunner(AppConfig config, QObject* parent)
    : QObject(parent), m_config(std::move(config))
{
}

bool LocalRunner::assignToJobObject(qint64 pid)
{
#ifdef Q_OS_WIN
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job) {
        return false;
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &info, sizeof(info));
    HANDLE process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!process) {
        CloseHandle(job);
        return false;
    }
    const BOOL ok = AssignProcessToJobObject(job, process);
    CloseHandle(process);
    return ok;
#else
    Q_UNUSED(pid);
    return true;
#endif
}

LocalRunResult LocalRunner::runCppSample(const QString& code, const QString& input)
{
    LocalRunResult result;
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        result.compileOutput = "Failed to create temporary directory.";
        return result;
    }

    const QString sourcePath = tempDir.filePath("main.cpp");
    const QString exePath = tempDir.filePath("main.exe");
    QFile source(sourcePath);
    if (!source.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.compileOutput = "Failed to write source file.";
        return result;
    }
    QTextStream out(&source);
    out << code;
    source.close();

    QString compiler = m_config.compilerPath;
    if (QDir::isRelativePath(compiler)) {
        compiler = QCoreApplication::applicationDirPath() + "/" + compiler;
    }

    QProcess compilerProcess;
    compilerProcess.start(compiler, {"-std=c++14", "-O2", "-pipe", sourcePath, "-o", exePath});
    if (!compilerProcess.waitForStarted()) {
        result.compileOutput = "Failed to start compiler: " + compiler;
        return result;
    }
    if (!compilerProcess.waitForFinished(15000)) {
        compilerProcess.kill();
        result.compileOutput = "Compilation timed out.";
        return result;
    }
    result.compileOutput = QString::fromLocal8Bit(compilerProcess.readAllStandardError());
    if (compilerProcess.exitCode() != 0) {
        return result;
    }
    result.compiled = true;

    QProcess runProcess;
    runProcess.start(exePath);
    if (!runProcess.waitForStarted()) {
        result.stderrText = "Failed to start compiled executable.";
        return result;
    }
    assignToJobObject(runProcess.processId());
    runProcess.write(input.toUtf8());
    runProcess.closeWriteChannel();
    if (!runProcess.waitForFinished(m_config.localRunTimeoutMs)) {
        result.timedOut = true;
        runProcess.kill();
        runProcess.waitForFinished();
    }
    result.exitCode = runProcess.exitCode();
    result.stdoutText = QString::fromLocal8Bit(runProcess.readAllStandardOutput());
    result.stderrText = QString::fromLocal8Bit(runProcess.readAllStandardError());
    return result;
}
