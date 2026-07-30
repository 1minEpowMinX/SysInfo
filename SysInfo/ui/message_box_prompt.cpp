#include "message_box_prompt.h"

#include <QMessageBox>

void MessageBoxPrompt::showError(const QString& title, const QString& text)
{
    QMessageBox::critical(nullptr, title, text);
}

bool MessageBoxPrompt::confirm(const QString& title, const QString& text)
{
    const auto reply = QMessageBox::question(nullptr, title, text,
                                             QMessageBox::Yes | QMessageBox::No);
    return reply == QMessageBox::Yes;
}
