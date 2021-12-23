/*
 *   SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CATEGORIZEDVIEW_H
#define CATEGORIZEDVIEW_H

#include <QProxyStyle>

#include <KCategorizedView>

class CategorizedView : public KCategorizedView
{
public:
    explicit CategorizedView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;

protected:
    void wheelEvent(QWheelEvent *) override;
};

class ActivateItemOnSingleClickStyle : public QProxyStyle
{
public:
    explicit ActivateItemOnSingleClickStyle(QStyle *style)
        : QProxyStyle(style)
    {
    }

    int styleHint(QStyle::StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override
    {
        if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick) {
            return 1;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

#endif
