/*
 *   This file is part of the KDE project
 *   SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MENUITEM_H
#define MENUITEM_H

#include <KService>

#include "systemsettingsview_export.h"

class QString;
class KCModuleInfo;
class KDesktopFile;
template<typename T> class QList;

/**
 * @brief Provides a specific item in the list of modules or categories
 *
 * This provides convenient access to the list of modules, providing information about them
 * such as name, module information and its service object.\n
 * This is created automatically by System Settings, and is shared among all plugins and so should not
 * be modified under any circumstances.\n
 *
 * System Settings creates it in a tree like manner, with categories containing subcategories and modules,
 * and subcategories repeating this.\n
 *
 * The service object must be set, unless it is the top level item, otherwise using applications
 * will crash when attempting to sort the children by weight
 *
 * @author Ben Cooksley <bcooksley@kde.org>
 * @author Will Stephenson <wstephenson@kde.org>
 */
class SYSTEMSETTINGSVIEW_EXPORT MenuItem
{
public:
    /**
     * Creates a MenuItem.
     * @note Will not provide keywords, name, or a module item until a service has been set.
     *
     * @param isMenu Specifies if it is a category or not.
     * @param parent The item it is parented to. Provide 0 for a top level item.
     */
    MenuItem(bool isMenu, MenuItem *parent);

    /**
     * Destroys a MenuItem, including all children, the service object and the module information.
     *
     * @warning Destroys the KService and KCModuleInfo objects provided by service() and item().
     */
    ~MenuItem();

    /**
     * Sorts the children depending on the value of "X-KDE-Weight" in the desktop files of the
     * category or module.
     */
    void sortChildrenByWeight();

    /**
     * Provides the MenuItem for the child at the specified index.
     *
     * @param index The index of the child.
     * @returns The MenuItem object of the specified child.
     */
    MenuItem *child(int index);

    /**
     * Returns the list of keywords, which is used for searching the list of categories and modules.
     *
     * @note The parent items share all the keywords of their children.
     * @returns The list of keywords the item has.
     */
    QStringList keywords();

    /**
     * Returns the parent of this item.
     *
     * @returns The MenuItem object of this items parent.
     */
    MenuItem *parent() const;

    /**
     * Provides a list of all the children of this item.
     *
     * @returns The list of children this has.
     */
    QList<MenuItem *> &children() const;

    QString comment() const;
    QString iconName() const;
    bool isSystemsettingsCategory() const;
    QString systemsettingsCategoryModule() const;
    bool isSystemsettingsRootCategory() const;

    /**
     * Provides the KDE control module information item, which can be used to load control modules
     * by the ModuleView.
     *
     * @returns The control module information object of the item, if the service object has been set.
     */
    KCModuleInfo &item() const;

    /**
     * Convenience function which provides the name of the current item.
     *
     * @returns The name of the item, if the service object has been set.
     */
    QString &name() const;

    /**
     * Convenience function which provides the System Settings category of the current item.
     *
     * @returns The category of the item, if the service object has been set.
     */
    QString &category() const;

    /**
     * Provides the weight of the current item, as determined by its service.
     * If the service does not specify a weight, it is 100
     *
     * @returns The weight of the service
     */
    int weight();

    /**
     * Provides information on which type the current item is.
     *
     * @returns true if it is a category.
     * @returns false if it is not a category.
     */
    bool menu() const;

    /**
     * Sets the service object, which is used to provide the module information, name and keywords
     * Applications will crash if it is not set, unless it is the top level item.
     *
     * @param service The service object to store.
     */
    void setService(const KService::Ptr &service);

    /**
     * Constructs an item which resembles a category using the given filename.
     * The properties are read using KConfig
     */
    void setCategoryConfig(const KDesktopFile &file);

    void setItem(const KCModuleInfo &item);

    MenuItem *descendantForModule(const QString &moduleName);

    bool showDefaultIndicator() const;

    void updateDefaultIndicator();

    void setDefaultIndicator(bool defaultIndicator);
    
    /**
     * If true this is the main module of this category and should appear in a more prominent way compared to the others
     */
    bool isCategoryOwner() const;
    void setCategoryOwner(bool owner);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_METATYPE(MenuItem *)

#endif
