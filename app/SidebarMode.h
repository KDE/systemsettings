/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SIDEBARMODE_H
#define SIDEBARMODE_H

#include <KSelectionProxyModel>
#include <QIcon>
#include <QWidget>
#include <qqmlregistration.h>

class ModuleView;
class KAboutData;
class QModelIndex;
class QAbstractItemView;
class QAbstractItemModel;
class QAction;
class SidebarMode;
class MenuItem;

class FocusHackWidget : public QWidget
{
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
    QAbstractItemModel *m_parentModel;
    SidebarMode *m_sidebarMode;
    QPersistentModelIndex m_activeModuleIndex;
};

class SidebarMode : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Not creatable, use the systemsettings attached property")

    Q_PROPERTY(QAbstractItemModel *categoryModel READ categoryModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *searchModel READ searchModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *subCategoryModel READ subCategoryModel CONSTANT)
    Q_PROPERTY(int activeCategoryRow READ activeCategoryRow NOTIFY activeCategoryRowChanged)
    Q_PROPERTY(int activeSearchRow READ activeSearchRow NOTIFY activeSearchRowChanged)
    Q_PROPERTY(int activeSubCategoryRow READ activeSubCategoryRow NOTIFY activeSubCategoryRowChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(bool actionMenuVisible READ actionMenuVisible NOTIFY actionMenuVisibleChanged)
    Q_PROPERTY(bool introPageVisible READ introPageVisible WRITE setIntroPageVisible NOTIFY introPageVisibleChanged)
    Q_PROPERTY(bool defaultsIndicatorsVisible READ defaultsIndicatorsVisible NOTIFY defaultsIndicatorsVisibleChanged)
    Q_PROPERTY(qreal headerHeight READ headerHeight WRITE setHeaderHeight NOTIFY headerHeightChanged)

public:
    enum ApplicationMode {
        SystemSettings = 0,
        InfoCenter,
    };
    Q_ENUM(ApplicationMode)

    enum ToolBarItemsFlags {
        NoItems = 0x1, /**< The Toolbar will not have any items added by System Settings */
        Search = 0x2, /**< The Toolbar will have the search bar added by System Settings */
        Configure = 0x4, /**< The Toolbar will have configure added by System Settings */
        Quit = 0x8, /**< The toolbar will have exit added by System Settings */
    };
    Q_DECLARE_FLAGS(ToolBarItems, ToolBarItemsFlags)

    SidebarMode(QObject *parent, const QVariantList &args);
    ~SidebarMode() override;
    QWidget *mainWidget();
    void initEvent();
    void giveFocus();
    ModuleView *moduleView() const;
    void reloadStartupModule();
    QList<QAction *> &actionsList() const;

    QAbstractItemModel *categoryModel() const;
    QAbstractItemModel *searchModel() const;
    QAbstractItemModel *subCategoryModel() const;

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
    void toggleDefaultsIndicatorsVisibility();

    Q_INVOKABLE QAction *action(const QString &name) const;
    // QML doesn't understand QIcon, otherwise we could get it from the QAction itself
    Q_INVOKABLE QString actionIconName(const QString &name) const;
    Q_INVOKABLE void showActionMenu(const QPoint &position);

    Q_INVOKABLE void loadModule(const QModelIndex &activeModule, const QStringList &args = QStringList());

    /**
     * Helper function to move focus to the next/previous QQuickWidget
     */
    Q_INVOKABLE void focusNext();
    Q_INVOKABLE void focusPrevious();

    QList<QAbstractItemView *> views() const;

    void setStartupModule(const QString &startupModule);
    QString startupModule() const;

    void setStartupModuleArgs(const QStringList &startupModuleArgs);
    QStringList startupModuleArgs() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void moduleLoaded();
    void updateDefaults();
    void initWidget();

private:
    void initPlaceHolderWidget();
    void updateModelMenuItem(MenuItem *item);
    void updateCategoryModel(const QModelIndex &categoryIdx);
    void refreshDefaults();
    void setActionMenuVisible(bool visible);
    MenuItem *rootItem() const;
    MenuItem *homeItem() const;

Q_SIGNALS:
    void activeCategoryRowChanged();
    void activeSubCategoryRowChanged();
    void activeSearchRowChanged();
    void widthChanged();
    void actionMenuVisibleChanged();
    void introPageVisibleChanged();
    void headerHeightChanged();
    void defaultsIndicatorsVisibleChanged();
    void actionsChanged();
    void viewChanged(bool state);
    void changeToolBarItems(ToolBarItems items);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SidebarMode::ToolBarItems)

#endif
