/*
 *   SPDX-FileCopyrightText: 2009 Rafael Fernández López <ereslibre@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "CategorizedView.h"

#include <KFileItemDelegate>

#include <QScrollBar>

CategorizedView::CategorizedView(QWidget *parent)
    : KCategorizedView(parent)
{
    setStyle(new ActivateItemOnSingleClickStyle(style()));
    setWordWrap(true);
}

void CategorizedView::setModel(QAbstractItemModel *model)
{
    KCategorizedView::setModel(model);
    int maxWidth = -1;
    int maxHeight = -1;
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i, modelColumn(), rootIndex());
        const QSize size = sizeHintForIndex(index);
        maxWidth = qMax(maxWidth, size.width());
        maxHeight = qMax(maxHeight, size.height());
    }
    setGridSize(QSize(maxWidth, maxHeight));
    static_cast<KFileItemDelegate *>(itemDelegate())->setMaximumSize(QSize(maxWidth, maxHeight));
}

void CategorizedView::wheelEvent(QWheelEvent *event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    // https://bugreports.qt-project.org/browse/QTBUG-7232
    verticalScrollBar()->setSingleStep(10);
    KCategorizedView::wheelEvent(event);
}
