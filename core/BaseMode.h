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

#include <QObject>
#include "kcontrolview_export.h"

#include <KService>

class QAction;
class MenuItem;
class ModuleView;
class QAbstractItemView;
template<typename T> class QList;

/**
 * @brief Provides a interface for KControl views
 *
 * BaseMode is a standard interface for all plugins to KControl to allow them to provide
 * their own interface to KDE control modules.\n
 *
 * The developer need only ensure that they perform all initialization of their plugin in
 * initEvent() to ensure that the plugin is displayed, and initial actions are loaded. 
 *
 * @author Ben Cooksley <ben@eclipse.endoftheinternet.org>
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
*/
class KCONTROLVIEW_EXPORT BaseMode : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a BaseMode for use in KControl.\n
     * Plugin developers should perform all initialisation in initEvent() not here
     *
     * @param parent The parent of this BaseMode
     */
    explicit BaseMode( QObject * parent );
    /**
     * Normal destructor. Plugin developers only need destroy what they created
     * not what is provided by BaseMode itself
     */
    virtual ~BaseMode();

    /**
     * Performs internal setup.\n
     * Plugin developers should perform initialisation in initEvent() not here
     *
     * @param modeService BaseMode's service object, used for providing extra information to KControl
     */
    void init( const KService::Ptr modeService );

    /**
     * Prepares the BaseMode for use.\n
     * Plugin developers should perform initialisation here, creating the View and preparing to
     * be displayed.
     */
    virtual void initEvent();

    /**
     * Returns the widget to be displayed in the center of KControl.\n
     * The widget should not be created here, it should be created in initEvent()
     * since this function is called multiple times
     *
     * @returns The main widget of the BaseMode
     */
    virtual QWidget * mainWidget();

    /**
     * Provides information about the BaseMode used in the About dialog of KControl.\n
     * This does not need to be implemented, and need only be implemented if the author
     * wants information about the view displayed in the About dialog
     *
     * @returns The about data of the BaseMode
     */
    virtual KAboutData * aboutData();

    /**
     * The state of the plugin ( position of the splitter for instance ) should be saved
     * to the configuration object when this is called.
     */
    virtual void saveState();

    /**
     * Provides access to the ModuleView the application uses to display control modules.\n
     * Failure to reimplement will cause modules not to be checked for configuration
     * changes, and for the module to not be displayed in the About dialog
     *
     * @returns The ModuleView used by the BaseMode for handling modules
     */
    virtual ModuleView * moduleView() const;

    /**
     * Provides the list of actions the BaseMode wants KControl to display in the toolbar when
     * it is loaded. This function does not need to be implemented if adding actions to the toolbar
     * is not required
     *
     * @returns The list of actions the BaseMode provides
     */
    virtual QList<QAction*>& actionsList() const;

    /**
     * Provides the service object, which is used to retrieve information for the configuration dialog
     *
     * @returns the service object of the BaseMode
     */
    const KService::Ptr& service() const;

    /**
     * Returns whether the enhanced tooltip is enabled. This tooltip
     * can only be enabled when an item view is provided by the child
     * class.
     *
     * @returns true if the enhanced tooltip is enabled
     */
    bool isEnhancedTooltipEnabled() const;

public Q_SLOTS:
    /**
     * Called when the text in the search box changes allowing the display to be filtered.\n
     * Search will not work in the plugin if this function is not implemented.
     */
    virtual void searchChanged( const QString& text );

    /**
     * Enables or disables the enhanced tooltip. This is only possible
     * when the child class provides an item view. The normal tooltip will be shown if this is disabled
     */
    void setEnhancedTooltipEnabled( bool enable );

Q_SIGNALS:
    /**
     * Should be emitted to signal the application to reload the actions
     * Actions previously contained in the list must be destroyed after this has been emitted
     */
    void actionsChanged();

    /**
     * Should be emitted when the type ( list of modules / display of module )
     * of displayed view is changed. Failure to do so will result in inconsistent headers
     * and search not being disabled when a module is displayed
     */ 
    void viewChanged();

protected:
    /**
     * Returns the root item of the list of categorised modules
     * This is usually passed to the constructor of MenuModel
     *
     * @returns The root menu item as provided by KControl
     */
    MenuItem * rootItem() const;

    /**
     * Provides access to the configuration for the BaseMode
     *
     * @returns The configuration group for the BaseMode
     */
    KConfigGroup& config() const;

    /**
     * Provides access to a item views if there are used
     * any. This could be used by other methods from this class
     * for example displaying and hiding tooltips.
     *
     * @returns A list of pointers to item views used by the mode.
     *          The list can be empty.
     */
    virtual QList<QAbstractItemView*> views() const;

private:
    class Private;
    Private *const d;
};

#endif
