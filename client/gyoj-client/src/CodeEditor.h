#pragma once

#include <QWidget>

class QPlainTextEdit;

class CodeEditor : public QWidget {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget* parent = nullptr);

    QString code() const;
    void setCode(const QString& code);
    void setReadOnly(bool readOnly);

signals:
    void codeChanged();

private:
    QPlainTextEdit* m_editor = nullptr;
};

