/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MenuItem.h"

#include <QList>
#include <QString>

#include <KCMUtils/KCModuleLoader>
#include <KCModuleInfo>

static bool childIsLessThan(MenuItem *left, MenuItem *right)
{
    return left->weight() < right->weight();
}

class MenuItem::Private
{
public:
    Private()
    {
    }

    MenuItem *parent;
    QList<MenuItem *> children;
    bool menu;
    QString name;
    QString category;
    int weight;
    KService::Ptr service;
    KCModuleInfo item;
    bool showDefaultIndicator;
};

MenuItem::MenuItem(bool isMenu, MenuItem *itsParent)
    : d(new Private())
{
    d->parent = itsParent;
    d->menu = isMenu;
    d->showDefaultIndicator = false;

    if (d->parent) {
        d->parent->children().append(this);
    }
}

MenuItem::~MenuItem()
{
    qDeleteAll(d->children);
    delete d;
}

void MenuItem::sortChildrenByWeight()
{
    std::sort(d->children.begin(), d->children.end(), childIsLessThan);
}

MenuItem *MenuItem::child(int index)
{
    return d->children.at(index);
}

QStringList MenuItem::keywords()
{
    QStringList listOfKeywords;

    listOfKeywords << d->item.keywords() << d->name;
    foreach (MenuItem *child, d->children) {
        listOfKeywords += child->keywords();
    }
    return listOfKeywords;
}

MenuItem *MenuItem::parent() const
{
    return d->parent;
}

QList<MenuItem *> &MenuItem::children() const
{
    return d->children;
}

KService::Ptr &MenuItem::service() const
{
    return d->service;
}

KCModuleInfo &MenuItem::item() const
{
    return d->item;
}

QString &MenuItem::name() const
{
    return d->name;
}

QString &MenuItem::category() const
{
    return d->category;
}

int MenuItem::weight()
{
    return d->weight;
}

bool MenuItem::menu() const
{
    return d->menu;
}

void MenuItem::setService(const KService::Ptr &service)
{
    d->service = service;
    d->category = service->property(QStringLiteral("X-KDE-System-Settings-Category")).toString();
    if (d->category.isEmpty()) {
        d->category = service->property(QStringLiteral("X-KDE-KInfoCenter-Category")).toString();
    }
    d->name = service->name();
    d->item = KCModuleInfo(service);
    const QVariant itemWeight = service->property(QStringLiteral("X-KDE-Weight"), QVariant::Int);
    if (itemWeight.isValid()) {
        d->weight = itemWeight.toInt();
    } else {
        d->weight = 100;
    }
}

bool MenuItem::showDefaultIndicator() const
{
    return d->showDefaultIndicator;
}

void MenuItem::setItem(const KCModuleInfo &item)
{
    // d->name = item.moduleName();
    d->item = item;
}

void MenuItem::updateDefaultIndicator()
{
    d->showDefaultIndicator = !KCModuleLoader::isDefaults(d->item);
    if (menu()) {
        for (auto child : children()) {
            d->showDefaultIndicator |= child->showDefaultIndicator();
        }
    }
    if (d->parent) {
        d->parent->updateDefaultIndicator();
    }
}

MenuItem *MenuItem::descendantForModule(const QString &moduleName)
{
    if (d->service) {
        if (d->service->desktopEntryName() == moduleName) {
            return this;
        } else if (item().fileName().length() > 0 && item().fileName().split(QLatin1Char('.'), QString::SkipEmptyParts).first() == moduleName) {
            return this;
        }
    }

    for (auto *child : d->children) {
        MenuItem *candidate = child->descendantForModule(moduleName);
        if (candidate) {
            return candidate;
        }
    }

    return nullptr;
}
