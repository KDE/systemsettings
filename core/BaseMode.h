/*****************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org>   *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA            *
 *****************************************************************************/

#ifndef BASEMODE_H
#define BASEMODE_H

#include <QtCore/QObject>
#include "systemsettingsview_export.h"

#include <KDE/KService>

class QAction;
class MenuItem;
class ModuleView;
class KConfigDialog;
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
 * @author Ben Cooksley <ben@eclipse.endoftheinternet.org>
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
*/
class SYSTEMSETTINGSVIEW_EXPORT BaseMode : public QObject
{
    Q_OBJECT

    /**
     * System Settings main application is allowed privilaged access to handle tooltips
     */
    friend class SettingsBase;

public:
    /**
     * Constructs a BaseMode for use in System Settings.\n
     * Plugin developers should perform all initialisation in initEvent() not here.
     *
     * @param parent The parent of this BaseMode.
     */
    explicit BaseMode( QObject * parent );
    /**
     * Normal destructor. Plugin developers only need destroy what they created
     * not what is provided by BaseMode itself.
     */
    virtual ~BaseMode();

    /**
     * These flags are used to control the presence of the Search and Configure actions on the toolbar
     */
    enum ToolBarItemsFlags {
        NoItems = 0x1, /**< The Toolbar will not have any items added by System Settings */
        Search = 0x2, /**< The Toolbar will have the search bar added by System Settings */
        Configure = 0x4 /**< The Toolbar will have configure added by System Settings */
    };
    Q_DECLARE_FLAGS(ToolBarItems, ToolBarItemsFlags)

    /**
     * Performs internal setup.\n
     * Plugin developers should perform initialisation in initEvent() not here.
     *
     * @param modeService Plugins service object, used for providing extra information to System Settings.
     */
    void init( const KService::Ptr modeService );

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
    virtual QWidget * mainWidget();

    /**
     * Provides information about the plugin, which is used in the About dialog of System Settings.\n
     * This does not need to be implemented, and need only be implemented if the author
     * wants information about the view displayed in the About dialog.
     *
     * @returns The about data of the plugin.
     */
    virtual KAboutData * aboutData();

    /**
     * The state of the plugin ( position of the splitter for instance ) should be saved
     * to the configuration object when this is called.
     */
    virtual void saveState();

    /**
     * Causes the view to unload all modules in the module view, and return to their module selection state
     *
     * @warning Failure to reimplement will cause modules to not be unloaded when changing views.
     * This must be implemented.
     */
    virtual void leaveModuleView();

    /**
     * Used to give focus to the plugin. Plugin should call setFocus() on the appropriate widget
     *
     * @note Failure to reimplement will cause keyboard accessibiltity and widget focusing problems
     */
    virtual void giveFocus();

    /**
     * Provides access to the ModuleView the application uses to display control modules.\n
     *
     * @warning Failure to reimplement will cause modules not to be checked for configuration
     * changes, and for the module to not be displayed in the About dialog. It must be implemented.
     * @returns The ModuleView used by the plugin for handling modules.
     */
    virtual ModuleView * moduleView() const;

    /**
     * Provides the list of actions the plugin wants System Settings to display in the toolbar when
     * it is loaded. This function does not need to be implemented if adding actions to the toolbar
     * is not required.
     *
     * @returns The list of actions the plugin provides.
     */
    virtual QList<QAction*>& actionsList() const;

    /**
     * Provides the service object, which is used to retrieve information for the configuration dialog.
     *
     * @returns the service object of the plugin.
     */
    const KService::Ptr& service() const;

public Q_SLOTS:
    /**
     * Called when the text in the search box changes allowing the display to be filtered.
     *
     * @warning Search will not work in the view if this function is not implemented.
     */
    virtual void searchChanged( const QString& text );

    /**
     * Allows views to add custom configuration pages to the System Settings configure dialog
     *
     * @warning Deleting the config object will cause System Settings to crash
     */
    virtual void addConfiguration( KConfigDialog * config );

    /**
     * Allows views to load their configuration before the configuration dialog is shown
     * Views should revert any changes that have not been saved
     */
    virtual void loadConfiguration();

    /**
     * Should be implmented to ensure that views settings are saved when the user confirms their changes
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
     * @warning Failure to emit this will result in inconsistent application headers and change state.
     */
    void viewChanged( bool state );

    /**
     * Causes System Settings to hide / show the toolbar items specified.
     * This is used to control the display of the Configure and Search actions
     *
     * @param items The items that are wanted in the toolbar
     */
    void changeToolBarItems( BaseMode::ToolBarItems items );

protected:
    /**
     * Returns the root item of the list of categorised modules.
     * This is usually passed to the constructor of MenuModel.
     *
     * @warning This is shared between all views, and should not be deleted manually.
     * @returns The root menu item as provided by System Settings.
     */
    MenuItem * rootItem() const;

    /**
     * Provides access to the configuration for the plugin.
     *
     * @returns The configuration group for the plugin.
     */
    KConfigGroup& config() const;

    /**
     * Provides access to item views used by the plugin.
     * This is currently used to show the enhanced tooltips.
     *
     * @returns A list of pointers to item views used by the mode.
     *          The list can be empty.
     */
    virtual QList<QAbstractItemView*> views() const;

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BaseMode::ToolBarItems)

#endif
