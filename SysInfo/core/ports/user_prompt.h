#ifndef USER_PROMPT_H
#define USER_PROMPT_H

#include <QString>

/**
 * @brief Puts a message in front of the user and, where the caller needs one,
 *        collects an answer.
 *
 * The interface App depends on for the two messages it raises itself, which
 * keeps widget types out of the composition layer. MessageBoxPrompt in ui/ is
 * the widget-backed implementation the application runs with.
 *
 * Both calls are synchronous and belong to the GUI thread.
 */
class UserPrompt
{
public:
    virtual ~UserPrompt() = default;

    /**
     * @brief Reports a failure the user has to know about.
     * @param title Window title.
     * @param text  Body; carries the rich-text subset Qt understands.
     */
    virtual void showError(const QString& title, const QString& text) = 0;

    /**
     * @brief Asks the user to confirm an action, returning once they answer.
     * @param title Window title.
     * @param text  Question; carries the rich-text subset Qt understands.
     * @return true if the user confirmed; false on refusal and on dismissal.
     */
    virtual bool confirm(const QString& title, const QString& text) = 0;
};

#endif // USER_PROMPT_H
