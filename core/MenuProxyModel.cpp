/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MenuProxyModel.h"

#include "MenuItem.h"
#include "MenuModel.h"

#include <KCModuleInfo>

MenuProxyModel::MenuProxyModel(QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , m_filterHighlightsEntries(true)
{
    setSortRole(MenuModel::UserSortRole);
    setFilterRole(MenuModel::UserFilterRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QHash<int, QByteArray> MenuProxyModel::roleNames() const
{
    QHash<int, QByteArray> names = KCategorizedSortFilterProxyModel::roleNames();
    names[KCategorizedSortFilterProxyModel::CategoryDisplayRole] = "categoryDisplayRole";
    return names;
}

bool MenuProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (isCategorizedModel()) {
        return KCategorizedSortFilterProxyModel::lessThan(left, right);
    }

    QVariant leftWeight = left.data(MenuModel::UserSortRole);
    QVariant rightWeight = right.data(MenuModel::UserSortRole);

    if (leftWeight.toInt() == rightWeight.toInt()) {
        return left.data().toString() < right.data().toString();
    }

    return leftWeight.toInt() < rightWeight.toInt();
}

bool MenuProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (isCategorizedModel()) {
        QVariant leftWeight = left.data(MenuModel::UserSortRole);
        QVariant rightWeight = right.data(MenuModel::UserSortRole);

        if (!leftWeight.isValid() || !rightWeight.isValid()) {
            return KCategorizedSortFilterProxyModel::subSortLessThan(left, right);
        } else {
            if (leftWeight.toInt() == rightWeight.toInt()) {
                return left.data().toString() < right.data().toString();
            } else {
                return leftWeight.toInt() < rightWeight.toInt();
            }
        }
    }
    return KCategorizedSortFilterProxyModel::subSortLessThan(left, right);
}

bool MenuProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!m_filterHighlightsEntries) {
        // Don't show empty categories
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        MenuItem *mItem = index.data(Qt::UserRole).value<MenuItem *>();
        if (mItem->menu() && mItem->children().isEmpty()) {
            return false;
        }
        if (mItem->metaData().pluginId() == QLatin1String("kcm_landingpage")) {
            return false;
        } else {
            return KCategorizedSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        }
    }

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    MenuItem *mItem = index.data(Qt::UserRole).value<MenuItem *>();

    if (mItem->metaData().pluginId() == QLatin1String("kcm_landingpage")) {
        return false;
    }

    // accept only systemsettings categories that have children
    if (mItem->children().isEmpty() && mItem->isSystemsettingsCategory()) {
        return false;
    } else {
        return true; // Items matching the regexp are disabled, not hidden
    }
}

void MenuProxyModel::setFilterHighlightsEntries(bool highlight)
{
    m_filterHighlightsEntries = highlight;
}

bool MenuProxyModel::filterHighlightsEntries() const
{
    return m_filterHighlightsEntries;
}

Qt::ItemFlags MenuProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    QString matchText = index.data(MenuModel::UserFilterRole).toString();
    if (!matchText.contains(filterRegExp())) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

void MenuProxyModel::setFilterRegExp(const QString &pattern)
{
    if (pattern == filterRegExp()) {
        return;
    }
    Q_EMIT layoutAboutToBeChanged();
    KCategorizedSortFilterProxyModel::setFilterRegExp(pattern);
    Q_EMIT layoutChanged();
    Q_EMIT filterRegExpChanged();
}

QString MenuProxyModel::filterRegExp() const
{
    return KCategorizedSortFilterProxyModel::filterRegExp().pattern();
}

void MenuProxyModel::setFilterRegExp(const QRegExp &regExp)
{
    Q_EMIT layoutAboutToBeChanged();
    KCategorizedSortFilterProxyModel::setFilterRegExp(regExp);
    Q_EMIT layoutChanged();
}
