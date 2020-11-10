/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef SIDEBARMODE_H
#define SIDEBARMODE_H

#include "BaseMode.h"
#include <QWidget>
#include <KSelectionProxyModel>
#include <QIcon>

class ModuleView;
class KAboutData;
class QModelIndex;
class QAbstractItemView;
class QAbstractItemModel;
class QAction;
class SidebarMode;

class FocusHackWidget : public QWidget {
    Q_OBJECT
public:
    explicit FocusHackWidget(QWidget *parent = nullptr);
    ~FocusHackWidget() override;

public Q_SLOTS:
    void focusNext();
    void focusPrevious();
};

class SubcategoryModel : public KSelectionProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool categoryOwnedByKCM READ categoryOwnedByKCM NOTIFY categoryOwnedByKCMChanged)

public:
    explicit SubcategoryModel(QAbstractItemModel *parentModel, SidebarMode *parent = nullptr);

    QString title() const;
    QIcon icon() const;
    bool categoryOwnedByKCM() const;

    void setParentIndex(const QModelIndex &activeModule);

    Q_INVOKABLE void loadParentCategoryModule();

Q_SIGNALS:
    void titleChanged();
    void iconChanged();
    void categoryOwnedByKCMChanged();

private:
    SidebarMode *m_sidebarMode;
    QAbstractItemModel *m_parentModel;
    QPersistentModelIndex m_activeModuleIndex;
};

class SidebarMode : public BaseMode
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *categoryModel READ categoryModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *searchModel READ searchModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *subCategoryModel READ subCategoryModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *mostUsedModel READ mostUsedModel CONSTANT)
    Q_PROPERTY(int activeCategoryRow READ activeCategoryRow NOTIFY activeCategoryRowChanged)
    Q_PROPERTY(int activeSearchRow READ activeSearchRow NOTIFY activeSearchRowChanged)
    Q_PROPERTY(int activeSubCategoryRow READ activeSubCategoryRow NOTIFY activeSubCategoryRowChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(bool actionMenuVisible READ actionMenuVisible NOTIFY actionMenuVisibleChanged)
    Q_PROPERTY(bool introPageVisible READ introPageVisible WRITE setIntroPageVisible NOTIFY introPageVisibleChanged)
    Q_PROPERTY(bool defaultsIndicatorsVisible READ defaultsIndicatorsVisible NOTIFY defaultsIndicatorsVisibleChanged)
    Q_PROPERTY(qreal headerHeight READ headerHeight WRITE setHeaderHeight NOTIFY headerHeightChanged)

public:
    SidebarMode(QObject * parent, const QVariantList &args );
    ~SidebarMode() override;
    QWidget * mainWidget() override;
    void initEvent() override;
    void giveFocus() override;
    KAboutData * aboutData() override;
    ModuleView * moduleView() const override;
    void reloadStartupModule() override;

    QAbstractItemModel *categoryModel() const;
    QAbstractItemModel *searchModel() const;
    QAbstractItemModel *subCategoryModel() const;
    QAbstractItemModel *mostUsedModel() const;

    int activeCategoryRow() const;
    int activeSubCategoryRow() const;
    int activeSearchRow() const;

    int width() const;

    bool actionMenuVisible() const;

    bool introPageVisible() const;
    void setIntroPageVisible(const bool &introPageVisible);

    qreal headerHeight() const;
    void setHeaderHeight(qreal height);

    bool defaultsIndicatorsVisible() const;
    Q_INVOKABLE void toggleDefaultsIndicatorsVisibility();

    Q_INVOKABLE QAction *action(const QString &name) const;
    // QML doesn't understand QIcon, otherwise we could get it from the QAction itself
    Q_INVOKABLE QString actionIconName(const QString &name) const;
    Q_INVOKABLE void requestToolTip(const QModelIndex &index, const QRectF &rect);
    Q_INVOKABLE void requestMostUsedToolTip(int index, const QRectF &rect);
    Q_INVOKABLE void hideToolTip();
    Q_INVOKABLE void hideMostUsedToolTip();
    Q_INVOKABLE void showActionMenu(const QPoint &position);

    Q_INVOKABLE void loadModule(const QModelIndex& activeModule, const QStringList &args = QStringList());

protected:
    QList<QAbstractItemView*> views() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void moduleLoaded();
    void updateDefaults();
    void initWidget();

Q_SIGNALS:
    void activeCategoryRowChanged();
    void activeSubCategoryRowChanged();
    void activeSearchRowChanged();
    void widthChanged();
    void actionMenuVisibleChanged();
    void introPageVisibleChanged();
    void headerHeightChanged();
    void defaultsIndicatorsVisibleChanged();

private:
    class Private;
    Private *const d;
};

#endif
