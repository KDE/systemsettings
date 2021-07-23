/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@informatik.uni-bremen.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MODULE_VIEW_H
#define MODULE_VIEW_H

#include <KPageView>
#include <QModelIndex>
#include <QWidget>

#include "systemsettingsview_export.h"

class KCModuleInfo;
class KCModuleProxy;
class KPageWidgetItem;

/**
 * @brief Provides a convenient way to display modules
 *
 * Provides a standardised interface for displaying multiple modules simultaneously
 * and provides facilities to access the current module, and to load its help, restore
 * default settings, save new settings and revert changes to settings
 *
 * It also provides checking for when a module has changed its configuration, and will prompt
 * if the user tries to change module or view if BaseMode is reimplemented correctly
 *
 * It also provides signals for active module changes, configuration changes and for when it has
 * been requested to close by button press
 *
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
 * @author Ben Cooksley <bcooksley@kde.org>
 */
class SYSTEMSETTINGSVIEW_EXPORT ModuleView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a ModuleView, with the parent specified.
     */
    explicit ModuleView(QWidget *parent = nullptr);

    /**
     * Destroys the module view, along with all modules loaded, and any changes present in them.
     *
     * @warning The user will not be prompted to save changes if any exist.
     */
    ~ModuleView() override;

    /**
     * Provides the module information, which is used to set the caption of the window when either the
     * active module or configuration changes.
     */
    KCModuleInfo *activeModule() const;

    /**
     * Resolves any changes in the currently active module by prompting the user if they exist.
     *
     * @returns true if the user saved or discarded changes, or there were no changes at all.
     * @returns false if the user canceled the module change.
     */
    bool resolveChanges();

    /**
     * Closes all active modules, after checking there are no active changes.
     *
     * @warning This forces all modules to be destroyed regardless of if changes exist or not
     * If possible, always check with resolveChanges() first.
     */
    void closeModules();

    void setFaceType(KPageView::FaceType type);

    KPageView::FaceType faceType() const;

    /**
     * Sets whether Systemsettings should save statisctics about
     * most used modules using KActivities::Stats
     */
    void setSaveStatistics(bool save);

    /**
     * @returns whether Systemsettings should save statisctics about
     * most used module
     */
    bool saveStatistics() const;

    /**
     * Shows or hides the Apply button.
     */
    void setApplyVisible(bool visible);

    /**
     * @returns True if the Apply button is visible.
     */
    bool isApplyVisible() const;

    /**
     * Shows or hides the Defaults button.
     */
    void setDefaultsVisible(bool visible);

    /**
     * @returns True if the Defaults button is visible.
     */
    bool isDefaultsVisible() const;

    /**
     * Shows or hides the Reset button.
     */
    void setResetVisible(bool visible);

    /**
     * @returns True if the Reset button is visible.
     */
    bool isResetVisible() const;

    /**
     * Show or hide defaults indicators (field level)
     */
    void moduleShowDefaultsIndicators(bool show);

    qreal headerHeight() const;
    void setHeaderHeight(qreal height);

public Q_SLOTS:
    /**
     * Loads the module specified by menuItem.\n
     * If the module has children, they will all be loaded instead of the module.
     *
     * @param menuItem the QModelIndex that you want to load. Must be sourced from either MenuModel or MenuProxyModel
     */
    void loadModule(const QModelIndex &menuItem, const QStringList &args);

    /**
     * Will open KHelpCenter, and load the help for the active module.
     */
    void moduleHelp();

    /**
     * Causes the active module to reload its configuration, reverting all changes.
     */
    void moduleLoad();

    /**
     * Causes the active module to save its configuration, applying all changes.
     */
    bool moduleSave();

    /**
     * Causes the active module to revert all changes to the configuration, and return to defaults.
     */
    void moduleDefaults();

    /**
     * Reimplemented for internal reasons.\n
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool resolveChanges(KCModuleProxy *currentProxy);
    void addModule(KCModuleInfo *module, const QStringList &args);
    bool moduleSave(KCModuleProxy *module);
    void updatePageIconHeader(KPageWidgetItem *page, bool light = false);

private Q_SLOTS:
    void activeModuleChanged(KPageWidgetItem *current, KPageWidgetItem *previous);
    void stateChanged();

Q_SIGNALS:
    /**
     * Emitted when the currently active module is changed. This occurs whenever the active module or
     * its configuration changes. This causes the window caption to update.
     */
    void moduleChanged(bool state);

    /**
     * Emitted after the currently active module was saved.
     */
    void moduleSaved();

    /**
     * Emitted when the ModuleView is asked to close.\n
     */
    void closeRequest();

    /**
     * Emmitted when showDefaultsIndicators state changed
     */
    void showDefaultsIndicatorsChanged(bool show);

private:
    class Private;
    Private *const d;
};

#endif
