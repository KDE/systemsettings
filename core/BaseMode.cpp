/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "BaseMode.h"

#include <QAbstractItemView>
#include <QAction>
#include <QList>

#include <KConfigDialog>
#include <KConfigGroup>

#include "BaseData.h"
#include "MenuItem.h"
#include "ModuleView.h"

class BaseMode::Private
{
public:
    Private()
    {
    }

    QList<QAction *> actionsList;
    KPluginMetaData metaData;
    MenuItem *rootItem = nullptr;
    MenuItem *homeItem = nullptr;
    QString startupModule;
    QStringList startupModuleArgs;
    KConfigGroup config;
    bool showToolTips = true;
    BaseMode::ApplicationMode applicationMode = BaseMode::SystemSettings;
};

BaseMode::BaseMode(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new Private())
{
    if (args.count() >= 1 && args.first().canConvert<ApplicationMode>()) {
        d->applicationMode = args.first().value<ApplicationMode>();
    }
    if (args.count() >= 2 && args[1].canConvert<QString>()) {
        d->startupModule = args[1].toString();
    }
    if (args.count() >= 3 && args[2].canConvert<QStringList>()) {
        d->startupModuleArgs = args[2].toStringList();
    }
}

void BaseMode::init()
{
    d->rootItem = BaseData::instance()->menuItem();
    d->homeItem = BaseData::instance()->homeItem();
    d->config = BaseData::instance()->configGroup(QStringLiteral("systemsettings_sidebar_mode"));
    initEvent();
    connect(moduleView(), &ModuleView::moduleChanged, this, &BaseMode::viewChanged);
}

BaseMode::~BaseMode()
{
    delete d;
}

void BaseMode::initEvent()
{
}

QWidget *BaseMode::mainWidget()
{
    return nullptr;
}

BaseMode::ApplicationMode BaseMode::applicationMode() const
{
    return d->applicationMode;
}

ModuleView *BaseMode::moduleView() const
{
    return nullptr;
}

QList<QAction *> &BaseMode::actionsList() const
{
    return d->actionsList;
}

const KPluginMetaData &BaseMode::metaData() const
{
    return d->metaData;
}

void BaseMode::setStartupModule(const QString &startupModule)
{
    d->startupModule = startupModule;
}

QString BaseMode::startupModule() const
{
    return d->startupModule;
}

void BaseMode::setStartupModuleArgs(const QStringList &startupModuleArgs)
{
    d->startupModuleArgs = startupModuleArgs;
}

QStringList BaseMode::startupModuleArgs() const
{
    return d->startupModuleArgs;
}

void BaseMode::searchChanged(const QString &text)
{
    Q_UNUSED(text);
}

void BaseMode::saveState()
{
}

void BaseMode::giveFocus()
{
}

void BaseMode::addConfiguration(KConfigDialog *config)
{
    Q_UNUSED(config);
}

void BaseMode::loadConfiguration()
{
}

void BaseMode::saveConfiguration()
{
}

MenuItem *BaseMode::rootItem() const
{
    return d->rootItem;
}

MenuItem *BaseMode::homeItem() const
{
    return d->homeItem;
}

KConfigGroup &BaseMode::config() const
{
    return d->config;
}

QList<QAbstractItemView *> BaseMode::views() const
{
    return {};
}

bool BaseMode::defaultsIndicatorsVisible() const
{
    return false;
}

void BaseMode::toggleDefaultsIndicatorsVisibility()
{
}

#include "moc_BaseMode.cpp"
