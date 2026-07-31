#ifndef ONBOARDING_FLAGS_H
#define ONBOARDING_FLAGS_H

/**
 * @brief Reports whether each one-shot onboarding notice is still due, and
 *        clears the welcome flag once its notice has been shown.
 *
 * The interface WelcomeNotifier depends on, so that deciding when a notice is
 * due is separable from where the answer is stored. SettingsManager is the
 * implementation the application runs with, backed by QSettings.
 *
 * A flag reads true until something clears it, so an untouched profile sees
 * every notice once. Each flag is readable and clearable here even though no
 * single consumer does both: WelcomeNotifier retires the welcome flag as it
 * shows the message, while the tray-guide hint is retired by the user ticking
 * "Don't show again" in the dialog it opens and cleared by App. Splitting a
 * flag's reader from its writer across the interface boundary would leave its
 * lifetime with no one place to read it in.
 */
class OnboardingFlags
{
public:
    virtual ~OnboardingFlags() = default;

    /// @return true if the welcome tray notification should still be shown.
    virtual bool showWelcome() const = 0;

    /// Persists the welcome flag.
    virtual void setShowWelcome(bool value) = 0;

    /// @return true if the Windows tray-guide hint should still be shown.
    virtual bool showTrayGuide() const = 0;

    /// Persists the tray-guide flag.
    virtual void setShowTrayGuide(bool value) = 0;
};

#endif // ONBOARDING_FLAGS_H
