/*
 *   SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CATEGORYDRAWER_H
#define CATEGORYDRAWER_H

#include <KCategoryDrawer>

class QPainter;
class QModelIndex;
class QStyleOption;

class CategoryDrawer : public KCategoryDrawer
{
    Q_OBJECT
public:
    explicit CategoryDrawer(KCategorizedView *view);

    void drawCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter) const override;
};

#endif
