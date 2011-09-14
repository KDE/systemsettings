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

#ifndef EXTERNALAPPMODULE_H
#define EXTERNALAPPMODULE_H

#include <QtGui/QWidget>

#include "ui_externalModule.h"

class QShowEvent;
class KCModuleInfo;

class ExternalAppModule : public QWidget
{
    Q_OBJECT

public:
    explicit ExternalAppModule(QWidget * parent = 0, KCModuleInfo * module = 0);
    ~ExternalAppModule();

protected:
    void showEvent(QShowEvent * event);

private Q_SLOTS:
    void runExternal();

private:
    KCModuleInfo * moduleInfo;
    Ui::ExternalModule externalModule;
    bool firstShow;
};

#endif
