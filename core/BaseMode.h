/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BASEMODE_H
#define BASEMODE_H

#include <QObject>

#include <KPluginMetaData>

#include "systemsettingsview_export.h"

class QAction;
class MenuItem;
class ModuleView;
class KAboutData;
class KConfigDialog;
class KConfigGroup;
class QAbstractItemView;
template<typename T> class QList;

/**
 * @brief Provides a interface for System Settings views
 *
 * BaseMode is a standard interface for all plugins to System Settings to allow them to provide
 * their own interface to KDE control modules.\n
 *
 * The developer need only ensure that they perform all initialization of their plugin in
 * initEvent() to ensure that the plugin is displayed, and initial actions are loaded.
 *
 * @author Ben Cooksley <bcooksley@kde.org>
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
 */
class SYSTEMSETTINGSVIEW_EXPORT BaseMode : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ApplicationMode applicationMode READ applicationMode CONSTANT)

    /**
     * System Settings main application is allowed privileged access to handle tooltips
     */
    friend class SettingsBase;

public:
    // Main mode of the app.
    // At the moment SystemSettings and InfoCenter are supported:
    // Changes mainly the set of module listed on the left menu
    enum ApplicationMode {
        SystemSettings = 0,
        InfoCenter,
    };
    Q_ENUM(ApplicationMode)

    virtual bool defaultsIndicatorsVisible() const;
    virtual void toggleDefaultsIndicatorsVisibility();

    void init();

    /**
     * Constructs a BaseMode for use in System Settings.\n
     * Plugin developers should perform all initialisation in initEvent() not here.
     *
     * @param parent The parent of this BaseMode.
     */
    explicit BaseMode(QObject *parent, const QVariantList &args);
    /**
     * Normal destructor. Plugin developers only need destroy what they created
     * not what is provided by BaseMode itself.
     */
    ~BaseMode() override;

    /**
     * These flags are used to control the presence of the Search and Configure actions on the toolbar
     */
    enum ToolBarItemsFlags {
        NoItems = 0x1, /**< The Toolbar will not have any items added by System Settings */
        Search = 0x2, /**< The Toolbar will have the search bar added by System Settings */
        Configure = 0x4, /**< The Toolbar will have configure added by System Settings */
        Quit = 0x8, /**< The toolbar will have exit added by System Settings */
    };
    Q_DECLARE_FLAGS(ToolBarItems, ToolBarItemsFlags)

    /**
     * Performs internal setup.\n
     * Plugin developers should perform initialisation in initEvent() not here.
     *
     * @param modeService Plugins service object, used for providing extra information to System Settings.
     */
    void init(const KPluginMetaData &metaData);

    /**
     * Prepares the BaseMode for use.\n
     * Plugin developers should perform initialisation here, creating the Models. They should perform widget
     * initialisation the first time mainWidget() is called, not here.
     */
    virtual void initEvent();

    /**
     * Returns the widget to be displayed in the center of System Settings.\n
     * The widget should be created the first time this function is called.
     *
     * @warning This function is called multiple times, ensure the widget is only created once.
     * @returns The main widget of the plugin.
     */
    virtual QWidget *mainWidget();

    /**
     * @returns the application mode of this systemsettings process: SystemSettings or InfoCenter
     */
    ApplicationMode applicationMode() const;

    /**
     * The state of the plugin ( position of the splitter for instance ) should be saved
     * to the configuration object when this is called.
     */
    virtual void saveState();

    /**
     * Used to give focus to the plugin. Plugin should call setFocus() on the appropriate widget
     *
     * @note Failure to reimplement will cause keyboard accessibility and widget focusing problems
     */
    virtual void giveFocus();

    /**
     * Provides access to the ModuleView the application uses to display control modules.\n
     *
     * @warning Failure to reimplement will cause modules not to be checked for configuration
     * changes, and for the module to not be displayed in the About dialog. It must be implemented.
     * @returns The ModuleView used by the plugin for handling modules.
     */
    virtual ModuleView *moduleView() const;

    /**
     * Provides the list of actions the plugin wants System Settings to display in the toolbar when
     * it is loaded. This function does not need to be implemented if adding actions to the toolbar
     * is not required.
     *
     * @returns The list of actions the plugin provides.
     */
    virtual QList<QAction *> &actionsList() const;

    const KPluginMetaData &metaData() const;

    void setStartupModule(const QString &startupModule);
    QString startupModule() const;

    void setStartupModuleArgs(const QStringList &startupModuleArgs);
    QStringList startupModuleArgs() const;

    virtual void reloadStartupModule() = 0;

public Q_SLOTS:
    /**
     * Called when the text in the search box changes allowing the display to be filtered.
     *
     * @warning Search will not work in the view if this function is not implemented.
     */
    virtual void searchChanged(const QString &text);

    /**
     * Allows views to add custom configuration pages to the System Settings configure dialog
     *
     * @warning Deleting the config object will cause System Settings to crash
     */
    virtual void addConfiguration(KConfigDialog *config);

    /**
     * Allows views to load their configuration before the configuration dialog is shown
     * Views should revert any changes that have not been saved
     */
    virtual void loadConfiguration();

    /**
     * Should be implemented to ensure that views settings are saved when the user confirms their changes
     * Views should also apply the configuration at the same time
     */
    virtual void saveConfiguration();

Q_SIGNALS:
    /**
     * Triggers a reload of the views actions by the host application.
     *
     * @warning Actions previously contained in the list must not be destroyed before this has been emitted.
     */
    void actionsChanged();

    /**
     * Should be emitted when the type ( list of modules / display of module )
     * of displayed view is changed.
     *
     * @param state Determines whether changes have been made in the view.
     * @warning Failure to Q_EMIT this will result in inconsistent application headers and change state.
     */
    void viewChanged(bool state);

    /**
     * Causes System Settings to hide / show the toolbar items specified.
     * This is used to control the display of the Configure and Search actions
     *
     * @param items The items that are wanted in the toolbar
     */
    void changeToolBarItems(BaseMode::ToolBarItems items);

protected:
    /**
     * Returns the root item of the list of categorised modules.
     * This is usually passed to the constructor of MenuModel.
     *
     * @warning This is shared between all views, and should not be deleted manually.
     * @returns The root menu item as provided by System Settings.
     */
    MenuItem *rootItem() const;

    /**
     * Returns (if present) an item that corresponds to a KCM which should be used as startup page.
     *
     * @warning This is shared between all views, and should not be deleted manually.
     * @returns The item to load as startup page. It may be nullptr
     */
    MenuItem *homeItem() const;

    /**
     * Provides access to the configuration for the plugin.
     *
     * @returns The configuration group for the plugin.
     */
    KConfigGroup &config() const;

    /**
     * Provides access to item views used by the plugin.
     * This is currently used to show the enhanced tooltips.
     *
     * @returns A list of pointers to item views used by the mode.
     *          The list can be empty.
     */
    virtual QList<QAbstractItemView *> views() const;

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BaseMode::ToolBarItems)

#endif
