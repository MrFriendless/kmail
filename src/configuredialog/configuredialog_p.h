// -*- c++ -*-
// configuredialog_p.h: classes internal to ConfigureDialog
// see configuredialog.h for details.

#ifndef CONFIGURE_DIALOG_PRIVATE_H
#define CONFIGURE_DIALOG_PRIVATE_H

#include "kmail_export.h"
#include "configmodule.h"

#include <kcmodule.h>

class QTabWidget;
class ConfigureDialog;

// Individual tab of a ConfigModuleWithTabs
class ConfigModuleTab : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigModuleTab(QWidget *parent = nullptr)
        : QWidget(parent)
        , mEmitChanges(true)
    {
    }

    ~ConfigModuleTab()
    {
    }

    virtual void save() = 0;
    void defaults();
Q_SIGNALS:
    // forwarded to the ConfigModule
    void changed(bool);
public Q_SLOTS:
    void slotEmitChanged();
    void load();
protected:
    bool mEmitChanges;
private:
    // reimplement this for loading values of settings which are available
    // via GlobalSettings
    virtual void doLoadFromGlobalSettings()
    {
    }

    // reimplement this for loading values of settings which are not available
    // via GlobalSettings
    virtual void doLoadOther()
    {
    }

    // reimplement this for loading default values of settings which are
    // not available via GlobalSettings (KConfigXT).
    virtual void doResetToDefaultsOther()
    {
    }
};

/*
 * ConfigModuleWithTabs represents a kcm with several tabs.
 * It simply forwards load and save operations to all tabs.
 */
class KMAIL_EXPORT ConfigModuleWithTabs : public ConfigModule
{
    Q_OBJECT
public:
    explicit ConfigModuleWithTabs(QWidget *parent = nullptr);
    ~ConfigModuleWithTabs() override
    {
    }

    // don't reimplement any of those methods
    void load() override;
    void save() override;
    void defaults() override;

protected:
    void showEvent(QShowEvent *event) override;
    void addTab(ConfigModuleTab *tab, const QString &title);

private:
    QTabWidget *mTabWidget = nullptr;
    bool mWasInitialized = false;
};

#endif // _CONFIGURE_DIALOG_PRIVATE_H_
