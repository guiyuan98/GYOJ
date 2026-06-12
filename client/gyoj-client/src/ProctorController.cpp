#include "ProctorController.h"

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QProcess>
#include <QScreen>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
static HHOOK g_keyboardHook = nullptr;
static ProctorController* g_controller = nullptr;
#endif

ProctorController::ProctorController(AppConfig config, QObject* parent)
    : QObject(parent), m_config(std::move(config))
{
    m_processTimer.setInterval(2000);
    connect(&m_processTimer, &QTimer::timeout, this, &ProctorController::scanProcesses);
}

ProctorController::~ProctorController()
{
    stop();
}

QString ProctorController::machineFingerprint() const
{
    const QString raw = QSysInfo::machineUniqueId() + "|" + QHostInfo::localHostName() + "|" + QSysInfo::prettyProductName();
    return QString::fromLatin1(QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

void ProctorController::start()
{
    installKeyboardHook();
    m_processTimer.start();
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &ProctorController::onClipboardChanged,
            Qt::UniqueConnection);
    if (m_config.enableFirewallLock && !m_config.networkAllowlist.isEmpty()) {
        applyNetworkAllowlist();
    }
}

void ProctorController::stop()
{
    uninstallKeyboardHook();
    m_processTimer.stop();
    restoreNetwork();
}

void ProctorController::lockScreen(const QString& reason)
{
    if (m_locked && m_lockReason == reason) {
        return;
    }
    m_locked = true;
    m_lockReason = reason;
    emit lockedChanged(true, reason);
}

void ProctorController::unlockScreen()
{
    m_locked = false;
    m_lockReason.clear();
    emit lockedChanged(false, {});
}

bool ProctorController::applyNetworkAllowlist()
{
#ifdef Q_OS_WIN
    QProcess::execute("netsh", {"advfirewall", "set", "allprofiles", "firewallpolicy", "blockinbound,blockoutbound"});
    for (const auto& host : m_config.networkAllowlist) {
        QProcess::execute("netsh", {"advfirewall", "firewall", "add", "rule",
                                    "name=GYOJ Allow " + host, "dir=out", "action=allow",
                                    "remoteip=" + host, "enable=yes"});
    }
    return true;
#else
    return false;
#endif
}

bool ProctorController::restoreNetwork()
{
#ifdef Q_OS_WIN
    QProcess::execute("netsh", {"advfirewall", "set", "allprofiles", "firewallpolicy", "blockinbound,allowoutbound"});
    for (const auto& host : m_config.networkAllowlist) {
        QProcess::execute("netsh", {"advfirewall", "firewall", "delete", "rule",
                                    "name=GYOJ Allow " + host, "dir=out"});
    }
    return true;
#else
    return false;
#endif
}

#ifdef Q_OS_WIN
static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        auto* info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        const bool alt = GetAsyncKeyState(VK_MENU) & 0x8000;
        const bool ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        const bool block = (alt && info->vkCode == VK_TAB) ||
                           (alt && info->vkCode == VK_F4) ||
                           (ctrl && info->vkCode == VK_ESCAPE) ||
                           info->vkCode == VK_LWIN ||
                           info->vkCode == VK_RWIN;
        if (block && g_controller) {
            emit g_controller->violationDetected("focus_lost", "Blocked system shortcut", true);
            g_controller->lockScreen("blocked_shortcut");
            return 1;
        }
    }
    return CallNextHookEx(g_keyboardHook, code, wParam, lParam);
}
#endif

void ProctorController::installKeyboardHook()
{
#ifdef Q_OS_WIN
    g_controller = this;
    g_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandleW(nullptr), 0);
#endif
}

void ProctorController::uninstallKeyboardHook()
{
#ifdef Q_OS_WIN
    if (g_keyboardHook) {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = nullptr;
    }
    g_controller = nullptr;
#endif
}

void ProctorController::scanProcesses()
{
#ifdef Q_OS_WIN
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }
    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry)) {
        do {
            const QString name = QString::fromWCharArray(entry.szExeFile).toLower();
            if (m_config.defaultProcessBlacklist.contains(name, Qt::CaseInsensitive) && !m_reportedProcesses.contains(name)) {
                m_reportedProcesses.insert(name);
                emit violationDetected("process", "Blacklisted process detected: " + name, true);
                lockScreen("blacklisted_process");
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
#endif
}

void ProctorController::onClipboardChanged()
{
    QApplication::clipboard()->clear();
    emit violationDetected("clipboard", "Clipboard operation was blocked", false);
}
