/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "BaseData.h"

#include "MenuItem.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QQmlEngine>

class DataHelper
{
public:
    DataHelper()
    {
    }
    ~DataHelper()
    {
        delete object;
    }
    BaseData *object = nullptr;
};

Q_GLOBAL_STATIC(DataHelper, internalInstance)

BaseData::BaseData()
{
    internalInstance->object = this;
}

BaseData::~BaseData()
{
}

BaseData *BaseData::instance()
{
    if (!internalInstance->object) {
        new BaseData();
    }
    return internalInstance->object;
}

MenuItem *BaseData::menuItem()
{
    return rootMenu;
}

void BaseData::setMenuItem(MenuItem *item)
{
    rootMenu = item;
}

MenuItem *BaseData::homeItem()
{
    return m_homeItem;
}

void BaseData::setHomeItem(MenuItem *item)
{
    m_homeItem = item;
}

KConfigGroup BaseData::configGroup(const QString &pluginName)
{
    return KSharedConfig::openConfig()->group(pluginName);
}

std::shared_ptr<QQmlEngine> BaseData::qmlEngine()
{
    if (!m_engine) {
        m_engine.reset(new QQmlEngine(this));
    }
    return m_engine;
}

#include "moc_BaseData.cpp"
