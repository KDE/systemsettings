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

#include "ExternalAppModule.h"

#include <QtGui/QApplication>

#include <KCModuleInfo>
#include <KRun>

ExternalAppModule::ExternalAppModule(QWidget * parent, KCModuleInfo * module)
{
    Q_UNUSED(parent)

    firstShow = true;
    moduleInfo = module;
    externalModule.setupUi( this );
    externalModule.LblText->setText( i18n("%1 is an external application and has been automatically launched", module->moduleName() ) );
    externalModule.PbRelaunch->setText( i18n("Relaunch %1", module->moduleName()) );
    connect( externalModule.PbRelaunch, SIGNAL(clicked()), this, SLOT(runExternal()) );
}

ExternalAppModule::~ExternalAppModule()
{
}

void ExternalAppModule::showEvent(QShowEvent * event)
{
    if( firstShow ) {
        runExternal();
        firstShow = false;
    }
    QWidget::showEvent(event);
}

void ExternalAppModule::runExternal()
{
    KRun::run( *(moduleInfo->service()), KUrl::List(), qApp->activeWindow() ); // Launch it!
}

#include "ExternalAppModule.moc"
