#pragma once

#include "ApiClient.h"
#include "AppConfig.h"
#include "CodeEditor.h"
#include "LocalRunner.h"
#include "ProctorController.h"

#include <QMainWindow>
#include <QTimer>

class QLabel;
class QLineEdit;
class QQuickView;
class QSplitter;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void navigateOj();
    void setOjUrl(const QString& url);
    void toggleLocalTools();
    void startSession();
    void runSample();
    void submitCode();
    void setLocked(bool locked, const QString& reason);
    void handleApiError(const QString& message);

    AppConfig m_config;
    ApiClient* m_api = nullptr;
    ProctorController* m_proctor = nullptr;
    LocalRunner* m_runner = nullptr;
    QQuickView* m_webView = nullptr;
    QWidget* m_webContainer = nullptr;
    QSplitter* m_splitter = nullptr;
    QWidget* m_toolsPanel = nullptr;
    CodeEditor* m_editor = nullptr;
    QTextEdit* m_sampleInput = nullptr;
    QTextEdit* m_output = nullptr;
    QLineEdit* m_urlInput = nullptr;
    QLineEdit* m_cookieInput = nullptr;
    QLineEdit* m_contestIdInput = nullptr;
    QLineEdit* m_problemIdInput = nullptr;
    QLineEdit* m_languageInput = nullptr;
    QLabel* m_status = nullptr;
    QWidget* m_lockOverlay = nullptr;
    QTimer m_heartbeat;
    QString m_sessionKey;
    int m_contestId = 1;
    int m_problemInternalId = 0;
    bool m_demoMode = false;
};
