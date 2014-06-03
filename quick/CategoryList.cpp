/*
 Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>
 Copyright (c) 2009 Ben Cooksley <bcooksley@kde.org>
 Copyright 2014 Sebastian Kügler <sebas@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "CategoryList.h"

#include "MenuItem.h"
#include "MenuProxyModel.h"
#include "host.h"

#include <QFile>
#include <QModelIndex>
#include <QTextStream>
#include <QQmlContext>

#include <KCModuleInfo>
#include <KIconLoader>
#include <KLocalizedString>

#include <QDebug>

static const char kcc_infotext[] = I18N_NOOP("System Settings");
static const char title_infotext[] = I18N_NOOP("Configure your system");
static const char intro_infotext[] = I18N_NOOP("Welcome to \"System Settings\", "
                                     "a central place to configure your computer system.");

class CategoryList::Private
{
public:
    Private() {}

    QModelIndex categoryMenu;
    QAbstractItemModel *itemModel;
    QMap<QString, QModelIndex> itemMap;
};

CategoryList::CategoryList(const QString &path, QWidget *parent)
    : QQuickWidget(parent), d(new Private())
{
    d->itemModel = Host::self()->categoriesModel();

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    //setStyleSheet(QString("background:transparent;"));
    //setAutoFillBackground(false);

    ///*
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::transparent);
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Text, Qt::transparent);
    setPalette(p);
    //*/
    setMinimumSize(400, 400);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("menuModel", d->itemModel);
    rootContext()->setContextProperty("host", Host::self());

    setSource(path);

//     connect( d->categoryView->browserExtension(),
//              SIGNAL( openUrlRequest( const QUrl&,
//                                      const KParts::OpenUrlArguments&,
//                                      const KParts::BrowserArguments& ) ),
//              this, SLOT(slotModuleLinkClicked(QUrl)) );
}

CategoryList::~CategoryList()
{
    delete d;
}

void CategoryList::changeModule(QModelIndex newItem)
{
    d->categoryMenu = newItem;
}

void CategoryList::slotModuleLinkClicked(const QUrl &moduleName)
{
    QModelIndex module = d->itemMap.value(moduleName.url());
    qDebug() << "Link name: " + moduleName.url();
    emit moduleSelected(module);
}

#include "moc_CategoryList.cpp"
