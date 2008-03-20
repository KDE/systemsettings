/* This file is part of the KDE project
   Copyright 2007 Will Stephenson <wstephenson@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy 
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "moduletab.h"

#include <QTabBar>
#include <QFont>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <kdebug.h>

#include "moduletabbar.h"

class ModuleTabPrivate {
public:
	ModuleTabBar* tabBar;
	
	QStackedWidget* stackedWidget;
	
};

ModuleTab::ModuleTab ( QWidget *parent )
    : QWidget( parent ), d(new ModuleTabPrivate())
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(0);
	
	d->tabBar = new ModuleTabBar(this);
	d->tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	d->tabBar->setShape(QTabBar::TriangularNorth);
	mainLayout->addWidget(d->tabBar);
	
    QFont font = d->tabBar->font();
    font.setBold(true);
    font.setPointSize( font.pointSize() * 1.2 );
    d->tabBar->setFont(font);
    
    d->stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(d->stackedWidget);
    
    connect(d->tabBar, SIGNAL(currentChanged( int )), d->stackedWidget, SLOT(setCurrentIndex(int)));
    connect(d->tabBar, SIGNAL(currentChanged( int )), this, SIGNAL(currentChanged(int)));

}

ModuleTab::~ModuleTab()
{
	delete d;
}

int ModuleTab::count() const 
{
	return d->stackedWidget->count();
}


QWidget* ModuleTab::currentWidget() const
{
	return d->stackedWidget->currentWidget();
}

void ModuleTab::addTab(QWidget* view, const QString& name)
{
	d->tabBar->addTab(name);
	d->stackedWidget->addWidget(view);
}


#include "moduletab.moc"
