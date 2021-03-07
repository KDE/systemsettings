/*
 *   SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CATEGORIZEDVIEW_H
#define CATEGORIZEDVIEW_H

#include <KCategorizedView>

class CategorizedView : public KCategorizedView
{
public:
    explicit CategorizedView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;

protected:
    void wheelEvent(QWheelEvent *) override;
};

#endif
