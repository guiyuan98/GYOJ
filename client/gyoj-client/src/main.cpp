#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

static bool isRunningAsAdmin()
{
#ifdef Q_OS_WIN
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &administratorsGroup)) {
        CheckTokenMembership(nullptr, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }
    return isAdmin == TRUE;
#else
    return true;
#endif
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    const bool demoMode = app.arguments().contains("--demo");
    QSharedMemory singleInstance("GYOJ-Exam-Client-Single-Instance");
    if (!demoMode && !singleInstance.create(1)) {
        QMessageBox::critical(nullptr, "GYOJ", "The exam client is already running.");
        return 1;
    }
    if (!demoMode && !isRunningAsAdmin()) {
        QMessageBox::critical(nullptr, "GYOJ", "Please run the exam client as administrator.");
        return 1;
    }
    MainWindow window;
    return app.exec();
}
