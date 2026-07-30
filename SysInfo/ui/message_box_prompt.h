#ifndef MESSAGE_BOX_PROMPT_H
#define MESSAGE_BOX_PROMPT_H

#include "app/user_prompt.h"

/**
 * @brief Serves UserPrompt through QMessageBox.
 *
 * Parents every box to nullptr: SysInfo lives in the tray and has no main
 * window to centre a dialog on.
 *
 * Holds no state, so one instance serves the whole run.
 */
class MessageBoxPrompt : public UserPrompt
{
public:
    /// Shows a critical box carrying @p title and @p text.
    void showError(const QString& title, const QString& text) override;

    /**
     * @brief Shows a Yes/No question box carrying @p title and @p text.
     * @return true if the user chose Yes.
     */
    bool confirm(const QString& title, const QString& text) override;
};

#endif // MESSAGE_BOX_PROMPT_H
