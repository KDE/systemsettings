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

#include "BaseMode.h"

#include <QList>
#include <QAction>
#include <QAbstractItemView>

#include <KConfigGroup>
#include <KConfigDialog>
#include <KServiceTypeTrader>

#include "MenuItem.h"
#include "BaseData.h"
#include "ModuleView.h"

class BaseMode::Private {
public:
    Private() {}

    QList<QAction*> actionsList;
    KService::Ptr service;
    MenuItem *rootItem;
    KConfigGroup config;
};

BaseMode::BaseMode( QObject* parent )
    : QObject( parent )
    , d( new Private() )
{
}

BaseMode::~BaseMode()
{
    delete d;
}

void BaseMode::init( const KService::Ptr modeService )
{
    d->rootItem = BaseData::instance()->menuItem();
    d->service = modeService;
    d->config = BaseData::instance()->configGroup( modeService->library() );
    initEvent();
    connect( moduleView(), SIGNAL(moduleChanged(bool)), this, SIGNAL(viewChanged(bool)) );
}

void BaseMode::initEvent()
{
}

QWidget * BaseMode::mainWidget()
{
    return 0;
}

KAboutData * BaseMode::aboutData()
{
    return 0;
}

ModuleView * BaseMode::moduleView() const
{
    return 0;
}

QList<QAction*>& BaseMode::actionsList() const
{
    return d->actionsList;
}

const KService::Ptr& BaseMode::service() const
{
    return d->service;
}

void BaseMode::searchChanged( const QString& text )
{
    Q_UNUSED( text );
}

void BaseMode::saveState()
{
}

void BaseMode::leaveModuleView()
{
}

void BaseMode::giveFocus()
{
}

void BaseMode::addConfiguration( KConfigDialog * config )
{
    Q_UNUSED( config );
}

void BaseMode::loadConfiguration()
{
}

void BaseMode::saveConfiguration()
{
}

MenuItem * BaseMode::rootItem() const
{
    return d->rootItem;
}

KConfigGroup& BaseMode::config() const
{
    return d->config;
}

QList<QAbstractItemView*> BaseMode::views() const
{
    return QList<QAbstractItemView*>();
}

#include "BaseMode.moc"
