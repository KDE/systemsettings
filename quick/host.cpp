/***************************************************************************
 *                                                                         *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "host.h"
#include "Category.h"
#include "MenuProxyModel.h"
#include "QuickMode.h"

#include <QGlobalStatic>
#include <QDebug>

Q_GLOBAL_STATIC(Host, staticHost)

class HostPrivate
{
public:
    HostPrivate(Host *host)
        : q(host),
          categoriesModel(0),
          rootCategory(0)
    {
    }

    Host *q;
    QuickMode *quickMode;
    bool moduleWidgetVisible;
    Category *currentCategory;
    MenuProxyModel *categoriesModel; // FIXME: replace by rootCategory().model()
    Category *rootCategory;
    QList<Category*> categories;
};

Host::Host(): QObject()
{
}

Host* Host::self()
{
    return staticHost;
}


void Host::setModel(MenuProxyModel *model)
{
    d = new HostPrivate(this);
    d->moduleWidgetVisible = false;
    d->currentCategory = 0;
    d->categoriesModel = model;
    d->rootCategory = new Category(model->index(0, 0), this);

    categories();
}

void Host::setQuickMode(QuickMode* quickmode)
{
    d->quickMode = quickmode;
}

Host::~Host()
{
    delete d;
}

QAbstractItemModel *Host::categoriesModel()
{
    return d->categoriesModel;
}

QQmlListProperty<Category> Host::categories()
{
    if (!d->categories.count()) {
        const int n = d->categoriesModel->rowCount(QModelIndex());
        for (int i = 0; i < n; i++) {
            QModelIndex index = d->categoriesModel->index(i, 0);
            d->categories.append(new Category(index, this));
        }
    }
    return QQmlListProperty<Category>(this, d->categories);
}

QQmlListProperty<Category> Host::modules()
{
    if (d->currentCategory) {
        //return QQmlListProperty<Category>(this, d->currentCategory->categories());
        return d->currentCategory->categories();
    }
    return QQmlListProperty<Category>();
}


void Host::categoryClicked(int ix)
{
    qDebug() << "Category: " << ix;
    QModelIndex index = d->categoriesModel->index(ix, 0);
    QString c = d->categoriesModel->data(index, Qt::DisplayRole).toString();
    qDebug() << " Cat from model: " << c;
}

void Host::resetModules()
{
    d->currentCategory = 0;
    emit modulesChanged();
}

void Host::moduleClicked(int ix)
{
    qDebug() << "Module: " << ix;
}

void Host::selectModule(Category *cat)
{
    if (d->categoriesModel->rowCount(cat->modelIndex()) > 0) {
//         d->stackedWidget->setCurrentWidget(d->classicCategory);
//         d->classicCategory->changeModule(activeModule);
        if (d->currentCategory != cat) {
            d->currentCategory = cat;
            qDebug() << "Updating Modules.";
            emit modulesChanged();
        }

        //emit viewChanged(false);
    }
    emit moduleSelected(cat->modelIndex());
}

void Host::setColumnWidth(int col, int colWidth)
{
    d->quickMode->setColumnWidth(col, colWidth);
}

void Host::setRowHeight(int row, int rowHeight)
{
    d->quickMode->setRowHeight(row, rowHeight);
}

bool Host::moduleWidgetVisible()
{
    return d->moduleWidgetVisible;
}

void Host::setModuleWidgetVisible(bool vis)
{
    if (d->moduleWidgetVisible != vis) {
        d->moduleWidgetVisible = vis;
        d->quickMode->setModuleWidgetVisible(vis, true);
        emit moduleWidgetVisibleChanged();
    }
}


#include "host.moc"
