/***************************************************************************
 *   Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                   *
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

#include "BaseData.h"

#include "MenuItem.h"

#include <KConfigGroup>

class DataHelper {

public:
    DataHelper() : object(0) {}
    ~DataHelper() {
        delete object;
    }
    BaseData * object;
};

K_GLOBAL_STATIC(DataHelper, internalInstance)

BaseData::BaseData()
{
    internalInstance->object = this;
}

BaseData::~BaseData()
{
}

BaseData *BaseData::instance()
{
    if( !internalInstance->object ) {
        new BaseData();
    }
    return internalInstance->object;
}

MenuItem * BaseData::menuItem()
{
    return rootMenu;
}

void BaseData::setMenuItem( MenuItem * item )
{
    rootMenu = item;
}

KConfigGroup BaseData::configGroup( const QString& pluginName )
{
    return KGlobal::config()->group( pluginName );
}

#include "BaseData.moc"
