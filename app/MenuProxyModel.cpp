/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MenuProxyModel.h"

#include "MenuItem.h"
#include "MenuModel.h"

#include <KPluginMetaData>

MenuProxyModel::MenuProxyModel(QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , m_filterHighlightsEntries(true)
    , m_showIrrelevantModules(false)
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
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (!m_showIrrelevantModules) {
        // Still find irrelevant modules when searching.
        const QRegularExpression filterRegExp = KCategorizedSortFilterProxyModel::filterRegularExpression();
        if (filterRegExp.pattern().isEmpty() && !index.data(MenuModel::IsRelevantRole).toBool()) {
            return false;
        }
    }

    if (!m_filterHighlightsEntries) {
        // Don't show empty categories
        auto mItem = index.data(Qt::UserRole).value<MenuItem *>();
        if (mItem->menu() && mItem->children().isEmpty()) {
            return false;
        }

        return KCategorizedSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    auto mItem = index.data(Qt::UserRole).value<MenuItem *>();

    // accept only systemsettings categories that have (relevant) children
    if (mItem->isSystemsettingsCategory()) {
        if (mItem->children().isEmpty()) {
            return false;
        }

        if (!m_showIrrelevantModules) {
            bool hasRelevantChildren = false;
            for (int i = 0; i < sourceModel()->rowCount(index); ++i) {
                const QModelIndex childIndex = sourceModel()->index(i, 0, index);
                if (childIndex.data(MenuModel::IsRelevantRole).toBool()) {
                    hasRelevantChildren = true;
                    break;
                }
            }

            return hasRelevantChildren;
        }
    }

    return true;
}

void MenuProxyModel::setFilterHighlightsEntries(bool highlight)
{
    m_filterHighlightsEntries = highlight;
}

bool MenuProxyModel::filterHighlightsEntries() const
{
    return m_filterHighlightsEntries;
}

void MenuProxyModel::setShowIrrelevantModules(bool show)
{
    if (m_showIrrelevantModules == show) {
        return;
    }
    beginFilterChange(); // Qt 6.9 API
    m_showIrrelevantModules = show;
    invalidateRowsFilter();
}

bool MenuProxyModel::showIrrelevantModules() const
{
    return m_showIrrelevantModules;
}

Qt::ItemFlags MenuProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    QString matchText = index.data(MenuModel::UserFilterRole).toString();
    QRegularExpression pattern = KCategorizedSortFilterProxyModel::filterRegularExpression();

    if (!matchText.contains(pattern)) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

void MenuProxyModel::setFilterRegularExpression(const QString &pattern)
{
    if (pattern == filterRegularExpression()) {
        return;
    }
    Q_EMIT layoutAboutToBeChanged();
    KCategorizedSortFilterProxyModel::setFilterRegularExpression(pattern);
    Q_EMIT layoutChanged();
    Q_EMIT filterRegularExpressionChanged();
}

QString MenuProxyModel::filterRegularExpression() const
{
    return KCategorizedSortFilterProxyModel::filterRegularExpression().pattern();
}

void MenuProxyModel::setFilterRegularExpression(const QRegularExpression &regExp)
{
    Q_EMIT layoutAboutToBeChanged();
    KCategorizedSortFilterProxyModel::setFilterRegularExpression(regExp);
    Q_EMIT layoutChanged();
}

#include "moc_MenuProxyModel.cpp"
