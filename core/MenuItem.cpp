/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MenuItem.h"

#include <QList>
#include <QString>

#include <KCMUtils/KCModuleLoader>
#include <KCModuleInfo>
#include <KConfigGroup>
#include <KDesktopFile>
#include <QFileInfo>

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
    bool showDefaultIndicator = false;
    bool isCategoryOwner = false;
    QString comment;
    QString iconName;
    QString systemsettingsCategoryModule;
    bool isSystemsettingsCategory = false;
    bool isSystemsettingsRootCategory = false;
    bool isExternalAppModule = false;
    KPluginMetaData metaData;
};

MenuItem::MenuItem(bool isMenu, MenuItem *itsParent)
    : d(new Private())
{
    d->parent = itsParent;
    d->menu = isMenu;

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
    listOfKeywords << KPluginMetaData::readTranslatedString(d->metaData.rawData(), QStringLiteral("X-KDE-Keywords")).split(QLatin1String(","));
    listOfKeywords << d->name;
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

QString MenuItem::comment() const
{
    return d->comment;
}

QString MenuItem::iconName() const
{
    return d->iconName;
}

bool MenuItem::isExternalAppModule() const
{
    return d->isExternalAppModule;
}

bool MenuItem::isSystemsettingsCategory() const
{
    return d->isSystemsettingsCategory;
}

QString MenuItem::systemsettingsCategoryModule() const
{
    return d->systemsettingsCategoryModule;
}

bool MenuItem::isSystemsettingsRootCategory() const
{
    return d->isSystemsettingsRootCategory;
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

void MenuItem::setCategoryConfig(const KDesktopFile &file)
{
    const KConfigGroup grp = file.desktopGroup();
    d->category = grp.readEntry("X-KDE-System-Settings-Category");
    if (d->category.isEmpty()) {
        d->category = grp.readEntry("X-KDE-KInfoCenter-Category");
    }
    d->name = grp.readEntry("Name");
    d->weight = grp.readEntry(QStringLiteral("X-KDE-Weight"), 100);
    d->comment = grp.readEntry("Comment");
    d->iconName = grp.readEntry("Icon");
    d->isSystemsettingsCategory = true;
    d->systemsettingsCategoryModule = grp.readEntry("X-KDE-System-Settings-Category-Module");
    d->isSystemsettingsRootCategory = QFileInfo(file.fileName()).fileName() == QLatin1String("settings-root-category.desktop");
}

void MenuItem::setMetaData(const KPluginMetaData &data)
{
    d->metaData = data;
    d->category = data.value(QStringLiteral("X-KDE-System-Settings-Category"));
    if (d->category.isEmpty()) {
        d->category = data.value(QStringLiteral("X-KDE-KInfoCenter-Category"));
    }
    d->name = data.name();
    const QVariant itemWeight = data.initialPreference();
    if (itemWeight.isValid()) {
        d->weight = itemWeight.toInt();
    } else {
        d->weight = 100;
    }
    d->comment = data.description();
    d->iconName = data.iconName();
    d->systemsettingsCategoryModule = data.value(QStringLiteral("X-KDE-System-Settings-Category-Module"));
    d->isExternalAppModule = (data.serviceTypes().contains(QStringLiteral("SystemSettingsExternalApp"))
        || data.serviceTypes().contains(QStringLiteral("SystemSettingsExternalApp")))
        && !data.value(QStringLiteral("Exec")).isEmpty(); // TODO load these without KServiceTypeTrader
}

KPluginMetaData MenuItem::metaData()
{
    return d->metaData;
}

bool MenuItem::showDefaultIndicator() const
{
    return d->showDefaultIndicator;
}

bool MenuItem::isCategoryOwner() const
{
    return d->isCategoryOwner;
}

void MenuItem::setCategoryOwner(bool owner)
{
    d->isCategoryOwner = owner;
}

void MenuItem::updateDefaultIndicator()
{
    std::unique_ptr<KCModuleData> moduleData(KPluginFactory::instantiatePlugin<KCModuleData>(d->metaData, nullptr).plugin);

    d->showDefaultIndicator = moduleData && !moduleData->isDefaults();
    if (menu()) {
        for (auto child : children()) {
            d->showDefaultIndicator |= child->showDefaultIndicator();
        }
    }
    if (d->parent) {
        d->parent->updateDefaultIndicator();
    }
}

void MenuItem::setDefaultIndicator(bool defaultIndicator)
{
    d->showDefaultIndicator = defaultIndicator;
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
        }
    }

    if (d->metaData.isValid() && d->metaData.pluginId() == moduleName) {
        return this;
    }

    for (auto *child : d->children) {
        MenuItem *candidate = child->descendantForModule(moduleName);
        if (candidate) {
            return candidate;
        }
    }

    return nullptr;
}

bool MenuItem::isLibrary()
{
    return d->metaData.isValid() && !isExternalAppModule();
}
