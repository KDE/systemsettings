/*
 *   SPDX-FileCopyrightText: 2008 Konstantin Heil <konst.heil@stud.uni-heidelberg.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include <QModelIndex>
#include <QObject>
#include <QQuickItem>

class QLayout;

/**
 * @brief Manages the tooltips for an item view.
 *
 * When hovering an item, a tooltip is shown after
 * a short timeout. The tooltip is hidden again when the
 * viewport is hovered or the item view has been left.
 */
class ToolTipManager : public QObject
{
    Q_OBJECT

public:
    enum ToolTipPosition {
        BottomCenter,
        Right,
    };

    /**
     * Standard constructor. The ToolTipManager will start handling ToolTip events on the provided
     * view immediately.
     *
     * @param parent The view which will have the tooltips displayed for.
     * @param toolTipPosition The position of the tooltip.
     */
    explicit ToolTipManager(const QAbstractItemModel *model, QWidget *parent, ToolTipManager::ToolTipPosition toolTipPosition);
    ~ToolTipManager() override;

    void setModel(const QAbstractItemModel *model);
    const QAbstractItemModel *model() const;

public Q_SLOTS:
    /**
     * Hides the currently shown tooltip. Invoking this method is
     * only needed when the tooltip should be hidden although
     * an item is hovered.
     */
    void hideToolTip();
    void requestToolTip(const QModelIndex &index, const QRect &rect);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void prepareToolTip();
    void slotSettingsChanged(const QString &file);

private:
    void showToolTip(const QModelIndex &menuItem);
    QWidget *createTipContent(QModelIndex item);
    QLayout *generateToolTipLine(QModelIndex *item, QWidget *toolTip, QSize iconSize, bool comment);
    void loadSettings();

    class Private;
    ToolTipManager::Private *d;
};

#endif
