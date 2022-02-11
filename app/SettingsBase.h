/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SETTINGS_BASE_H
#define SETTINGS_BASE_H

#include "BaseMode.h"
#include "MenuItem.h"
#include "tooltipmanager.h"
#include <QButtonGroup>
#include <QMap>
#include <QStackedWidget>

#include <KAboutApplicationDialog>
#include <KActionMenu>
#include <KConfigDialog>
#include <KLineEdit>
#include <KXmlGuiWindow>

class SettingsBase : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit SettingsBase(BaseMode::ApplicationMode mode, QWidget *parent = nullptr);
    ~SettingsBase() override;
    bool queryClose() override;

    void setStartupModule(const QString &startupModule);
    void setStartupModuleArgs(const QStringList &startupModuleArgs);
    void reloadStartupModule();

protected:
    QSize sizeHint() const override;

private Q_SLOTS:
    void initApplication();
    void initToolBar();
    void initHelpMenu();
    void initMenuList(MenuItem *parent);
    void about();
    void changePlugin();
    void viewChange(bool state);
    void updateViewActions();
    void changeToolBar(BaseMode::ToolBarItems toolbar);
    void changeAboutMenu(const KAboutData *menuAbout, QAction *menuItem, const QString &fallback);

private:
    /**
     * @return the plugin controller if the current view is found in the plugin list and successfully loaded,
     *         @c nullptr otherwise
     */
    BaseMode *loadCurrentView();

    // The plugins
    QVector<KPluginMetaData> m_plugins;
    QMap<QString, BaseMode *> m_loadedViews;
    QList<ToolTipManager *> tooltipManagers;
    BaseMode *activeView = nullptr;
    // The search bar
    KLineEdit *searchText = nullptr;
    QWidget *spacerWidget = nullptr;
    // The toolbar
    QWidgetAction *searchAction = nullptr;
    QWidgetAction *spacerAction = nullptr;
    QAction *switchToIconAction = nullptr;
    QAction *switchToSidebarAction = nullptr;
    QAction *quitAction = nullptr;
    // The help menu
    QAction *aboutViewAction = nullptr;
    KActionMenu *helpActionMenu = nullptr;
    // The control module
    QStackedWidget *stackedWidget = nullptr;
    // The module list
    MenuItem *rootModule = nullptr;
    MenuItem *homeModule = nullptr;
    MenuItem *lostFound = nullptr;
    QStringList categories;
    QList<KPluginMetaData> pluginModules;
    // The about dialog
    KAboutApplicationDialog *aboutDialog = nullptr;
    BaseMode::ApplicationMode m_mode = BaseMode::SystemSettings;
    QString m_startupModule;
    QStringList m_startupModuleArgs;
};
#endif
