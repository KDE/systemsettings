/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef BASEDATA_H
#define BASEDATA_H

#include <QtCore/QObject>
#include "systemsettingsview_export.h"

class QString;
class MenuItem;
class KConfigGroup;

/**
 * @brief Provides a interface sharing common data between modules in System Settings
 *
 * BaseData is a standard interface in System Settings to retrieve information that is shared between all modules.
 * It is a singleton, and will be automatically cleaned up. 
 *
 * @author Ben Cooksley <ben@eclipse.endoftheinternet.org>
*/
class SYSTEMSETTINGSVIEW_EXPORT BaseData : public QObject 
{
    Q_OBJECT
    Q_DISABLE_COPY(BaseData)

private:
    explicit BaseData();

public:
    /**
     * Provides a pointer to access the shared BaseData instance in order to retrieve data.
     *
     * @returns Access to the shared instance of BaseData.
     */
    static BaseData* instance();

    /**
    * Normal destructor that handles cleanup. Any objects created through BaseData must be assumed
    * to be invalid afterwards.
    */
    ~BaseData();

    /**
    * Provides the shared MenuItem which lists all categories and modules, for use with MenuModel.
    *
    * @returns the shared MenuItem.
    */
    MenuItem * menuItem();

    /**
    * Sets the MenuItem which the Singleton will return.
    * For internal use only.
    *
    * @param item A pointer to the MenuItem object
    */
    void setMenuItem( MenuItem * item );

    /**
    * Returns the configuration group by the name provided in the current applications configuration file.
    *
    * @param pluginName the name of the group that is required.
    * @returns The configuration group that is required.
    */
    KConfigGroup configGroup( const QString& pluginName );

private:
    MenuItem * rootMenu;
};

#endif

