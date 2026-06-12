#include "CodeEditor.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QVBoxLayout>

CodeEditor::CodeEditor(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_editor = new QPlainTextEdit(this);
    m_editor->setTabStopDistance(4 * QFontMetrics(m_editor->font()).horizontalAdvance(' '));
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    const auto mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_editor->setFont(mono);
    layout->addWidget(m_editor);
    connect(m_editor, &QPlainTextEdit::textChanged, this, &CodeEditor::codeChanged);
}

QString CodeEditor::code() const
{
    return m_editor->toPlainText();
}

void CodeEditor::setCode(const QString& code)
{
    m_editor->setPlainText(code);
}

void CodeEditor::setReadOnly(bool readOnly)
{
    m_editor->setReadOnly(readOnly);
}

