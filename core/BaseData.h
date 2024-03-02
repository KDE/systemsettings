/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BASEDATA_H
#define BASEDATA_H

#include <QObject>

#include "systemsettingsview_export.h"

class QQmlEngine;
class QString;
class MenuItem;
class KConfigGroup;

/**
 * @brief Provides a interface sharing common data between modules in System Settings
 *
 * BaseData is a standard interface in System Settings to retrieve information that is shared between all modules.
 * It is a singleton, and will be automatically cleaned up.
 *
 * @author Ben Cooksley <bcooksley@kde.org>
 */
class SYSTEMSETTINGSVIEW_EXPORT BaseData : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BaseData)

private:
    explicit BaseData();

public:
    /**
     * Provides a pointer to access the shared BaseData instance in order to retrieve data.
     *
     * @returns Access to the shared instance of BaseData.
     */
    static BaseData *instance();

    /**
     * Normal destructor that handles cleanup. Any objects created through BaseData must be assumed
     * to be invalid afterwards.
     */
    ~BaseData() override;

    /**
     * Provides the shared MenuItem which lists all categories and modules, for use with MenuModel.
     *
     * @returns the shared MenuItem.
     */
    MenuItem *menuItem();

    /**
     * Sets the MenuItem which the Singleton will return.
     * For internal use only.
     *
     * @param item A pointer to the MenuItem object
     */
    void setMenuItem(MenuItem *item);

    /**
     * Provides the shared MenuItem that corresponds to a KCM which should be used as startup page.
     *
     * @returns the shared MenuItem. It may be nullptr.
     */
    MenuItem *homeItem();

    /**
     * Sets the homescreen MenuItem which the Singleton will return.
     * For internal use only.
     *
     * @param item A pointer to the MenuItem object
     */
    void setHomeItem(MenuItem *item);

    /**
     * Returns the configuration group by the name provided in the current applications configuration file.
     *
     * @param pluginName the name of the group that is required.
     * @returns The configuration group that is required.
     */
    KConfigGroup configGroup(const QString &pluginName);

    std::shared_ptr<QQmlEngine> qmlEngine();

private:
    MenuItem *rootMenu;
    MenuItem *m_homeItem;
    std::weak_ptr<QQmlEngine> m_engine;
};

#endif
