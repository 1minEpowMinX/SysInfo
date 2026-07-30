#ifndef ONBOARDING_FLAGS_H
#define ONBOARDING_FLAGS_H

/**
 * @brief Reads and writes the one-shot flags that gate the onboarding notices.
 *
 * The interface WelcomeNotifier depends on, so that deciding when a notice is
 * due is separable from where the answer is stored. SettingsManager is the
 * implementation the application runs with, backed by QSettings.
 *
 * A flag reads true until something clears it, so an untouched profile sees
 * every notice once.
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
