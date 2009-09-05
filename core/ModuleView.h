/*****************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org>   *
 *   Copyright (C) 2009 by Mathias Soeken <msoeken@informatik.uni-bremen.de> *
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

#ifndef MODULE_VIEW_H
#define MODULE_VIEW_H

#include <QtGui/QWidget>
#include "systemsettingsview_export.h"

class MenuItem;
class KAboutData;
class KCModuleInfo;
class KCModuleProxy;
class KPageWidgetItem;

/**
 * @brief Provides a convienent way to display modules
 *
 * Provides a standardised interface for displaying multiple modules simultaneously
 * and provides facilities to access the current module, and to load its help, restore
 * default settings, save new settings and revert changes to settings
 *
 * It also provides checking for when a module has changed its configuration, and will prompt
 * if the user tries to change module or view if BaseMode is reimplemented correctly
 *
 * It also provides signals for active module changes, configuration changes and for when it has
 * been requested to close by button press
 *
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
 * @author Ben Cooksley <ben@eclipse.endoftheinternet.org>
 */
class SYSTEMSETTINGSVIEW_EXPORT ModuleView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a ModuleView, with the parent specified.
     */
    explicit ModuleView(QWidget * parent = 0);

    /**
     * Destroys the module view, along with all modules loaded, and any changes present in them.
     *
     * @warning The user will not be prompted to save changes if any exist.\n
     */
    ~ModuleView();

    /**
     * Provides the module information, which is used to set the caption of the window when either the
     * active module or configuration changes.
     */
    KCModuleInfo * activeModule() const;

    /**
     * Provides the about data of the active module, used for the about dialog.
     */
    const KAboutData * aboutData() const;

    /**
     * Resolves any changes in the currently active module by prompting the user if they exist.
     *
     * @returns true if the user saved or discarded changes, or there were no changes at all.
     * @returns false if the user canceled the module change.
     */
    bool resolveChanges();

    /**
     * Closes all active modules, after checking there are no active changes.
     *
     * @warning This forces all modules to be destroyed regardless of if changes exist or not
     * If possible, always check with resolveChanges() first.
     */
    void closeModules();

public Q_SLOTS:
    /**
     * Loads the module specified by menuItem.\n
     * If the module has children, they will all be loaded instead of the module.
     *
     * @param menuItem the MenuItem that you want to load.
     */
    void loadModule( MenuItem *menuItem );

    /**
     * Will open KHelpCenter, and load the help for the active module.
     */
    void moduleHelp();

    /**
     * Causes the active module to reload its configuration, reverting all changes.
     */
    void moduleLoad();

    /**
     * Causes the active module to save its configuration, applying all changes.
     */
    bool moduleSave();

    /**
     * Causes the active module to revert all changes to the configuration, and return to defaults.
     */
    void moduleDefaults();

    /**
     * Reimplemented for internal reasons.\n
     */
    void keyPressEvent( QKeyEvent * event );

private:
    bool resolveChanges( KCModuleProxy *currentProxy );
    void addModule( KCModuleInfo *module );
    bool moduleSave( KCModuleProxy *module );
    void updatePageIconHeader( KPageWidgetItem * page );

private Q_SLOTS:
    void activeModuleChanged( KPageWidgetItem* current, KPageWidgetItem* previous);
    void updateButtons();
    void stateChanged();

Q_SIGNALS:
    /**
     * Emitted when the currently active module is changed. This occurs whenever the active module or
     * its configuration changes. This causes the window caption to update.
     */
    void moduleChanged( bool state );

    /**
     * Emitted when the ModuleView is asked to close.\n
     */
    void closeRequest();

private:
    class Private;
    Private *const d;
};

#endif
