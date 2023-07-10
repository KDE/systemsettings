/*
 *   SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CategoryDrawer.h"

#include "MenuProxyModel.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOption>

CategoryDrawer::CategoryDrawer(KCategorizedView *view)
    : KCategoryDrawer(view)
{
}

void CategoryDrawer::drawCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter) const
{
    QStyleOption copy = option;
    copy.palette.setBrush(QPalette::Window, option.palette.base());
    KCategoryDrawer::drawCategory(index, sortRole, copy, painter);
}

#include "moc_CategoryDrawer.cpp"
