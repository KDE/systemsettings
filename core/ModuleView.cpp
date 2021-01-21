/*****************************************************************************
 *   Copyright (C) 2009 Ben Cooksley <bcooksley@kde.org>                     *
 *   Copyright (C) 2009 by Mathias Soeken <msoeken@informatik.uni-bremen.de> *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA            *
 *****************************************************************************/

#include "ModuleView.h"
#include "ExternalAppModule.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <KAboutData>
#include <KAuthAction>
#include <KAuthObjectDecorator>
#include <KAuthorized>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KColorScheme>
#include <KMessageBox>
#include <KPageWidget>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KTitleWidget>

#include <KActivities/ResourceInstance>

#include <cmath>

#include "MenuItem.h"

class CustomTitle : public KTitleWidget
{
public:
    CustomTitle(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
    void colorsChanged();
};

CustomTitle::CustomTitle(QWidget *parent)
    : KTitleWidget(parent)
{
    setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                       style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                       style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                       style()->pixelMetric(QStyle::PM_LayoutBottomMargin));

    colorsChanged();
    connect(qApp, &QApplication::paletteChanged, this, &CustomTitle::colorsChanged);
}

void CustomTitle::colorsChanged()
{
    auto config = KSharedConfig::openConfig();
    auto active = KColorScheme(QPalette::Active, KColorScheme::Header, config);
    auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
    auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, config);

    QPalette palette = KColorScheme::createApplicationPalette(config);

    palette.setBrush(QPalette::Active, QPalette::Window, active.background());
    palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
    palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
    palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
    palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

    setPalette(palette);
}

void CustomTitle::paintEvent(QPaintEvent *event)
{
    KTitleWidget::paintEvent(event);

    auto linearlyInterpolateDouble = [](double one, double two, double factor) {
        return one + (two - one) * factor;
    };

    QPainter p(this);

    const QColor window = palette().color(QPalette::Window);
    const QColor text = palette().color(QPalette::Text);
    const qreal balance = 0.2;

    const QColor separator = QColor::fromHsv(std::fmod(linearlyInterpolateDouble(window.hue(), text.hue(), balance), 360.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.saturation(), text.saturation(), balance), 255.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.value(), text.value(), balance), 255.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.alpha(), text.alpha(), balance), 255.0));
    p.fillRect(event->rect(), window);
    p.fillRect(QRect(QPoint(0, height() - 1), QSize(width(), 1)), separator);
}

class ModuleView::Private
{
public:
    Private()
    {
    }
    QMap<KPageWidgetItem *, KCModuleProxy *> mPages;
    QMap<KPageWidgetItem *, KCModuleInfo *> mModules;
    KPageWidget *mPageWidget = nullptr;
    CustomTitle *mCustomHeader = nullptr;
    QVBoxLayout *mLayout = nullptr;
    QDialogButtonBox *mButtons = nullptr;
    KAuth::ObjectDecorator *mApplyAuthorize = nullptr;
    QPushButton *mApply = nullptr;
    QPushButton *mReset = nullptr;
    QPushButton *mDefault = nullptr;
    QPushButton *mHelp = nullptr;
    bool pageChangeSupressed = false;
    bool mSaveStatistics = true;
    bool mDefaultsIndicatorsVisible = false;
};

ModuleView::ModuleView(QWidget *parent)
    : QWidget(parent)
    , d(new Private())
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    // Configure a layout first
    d->mLayout = new QVBoxLayout();
    // Create the Page Widget
    d->mPageWidget = new KPageWidget(this);
    d->mCustomHeader = new CustomTitle(this);

    rootLayout->addWidget(d->mCustomHeader);
    rootLayout->addItem(d->mLayout);
    // d->mPageWidget->setPageHeader(d->mCustomHeader);
    d->mPageWidget->layout()->setContentsMargins(0, 0, 0, 0);

    // Zero out only the horizontal spacing (the vertical spacing is fine)
    QGridLayout *gridLayout = static_cast<QGridLayout *>(d->mPageWidget->layout());

    gridLayout->setHorizontalSpacing(0);

    d->mLayout->addWidget(d->mPageWidget);
    // Create the dialog
    d->mButtons = new QDialogButtonBox(Qt::Horizontal, this);
    d->mLayout->addWidget(d->mButtons);

    // Create the buttons in it
    d->mApply = d->mButtons->addButton(QDialogButtonBox::Apply);
    KGuiItem::assign(d->mApply, KStandardGuiItem::apply());
    d->mDefault = d->mButtons->addButton(QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(d->mDefault, KStandardGuiItem::defaults());
    d->mReset = d->mButtons->addButton(QDialogButtonBox::Reset);
    KGuiItem::assign(d->mReset, KStandardGuiItem::reset());
    d->mHelp = d->mButtons->addButton(QDialogButtonBox::Help);
    KGuiItem::assign(d->mHelp, KStandardGuiItem::help());
    // Set some more sensible tooltips
    d->mReset->setToolTip(i18n("Reset all current changes to previous values"));
    // Set Auto-Default mode ( KDE Bug #211187 )
    d->mApply->setAutoDefault(true);
    d->mDefault->setAutoDefault(true);
    d->mReset->setAutoDefault(true);
    d->mHelp->setAutoDefault(true);
    // Prevent the buttons from being used
    d->mApply->setEnabled(false);
    d->mDefault->setEnabled(false);
    d->mReset->setEnabled(false);
    d->mHelp->setEnabled(false);
    // Connect up the buttons
    connect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
    connect(d->mReset, &QAbstractButton::clicked, this, &ModuleView::moduleLoad);
    connect(d->mHelp, &QAbstractButton::clicked, this, &ModuleView::moduleHelp);
    connect(d->mDefault, &QAbstractButton::clicked, this, &ModuleView::moduleDefaults);
    // clang-format off
    connect(d->mPageWidget, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
             this, SLOT(activeModuleChanged(KPageWidgetItem*,KPageWidgetItem*)));
    // clang-format on
    d->mApplyAuthorize = new KAuth::ObjectDecorator(d->mApply);
    d->mApplyAuthorize->setAuthAction(KAuth::Action());
}

ModuleView::~ModuleView()
{
    delete d;
}

KCModuleInfo *ModuleView::activeModule() const
{
    return d->mModules.value(d->mPageWidget->currentPage());
}

const KAboutData *ModuleView::aboutData() const
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    KAboutData *aboutData = nullptr;
    if (activeModule) {
        aboutData = const_cast<KAboutData *>(activeModule->aboutData());
    }
    return aboutData;
}

void ModuleView::loadModule(const QModelIndex &menuItem, const QStringList &args)
{
    if (!menuItem.isValid()) {
        return;
    }

    QList<QModelIndex> indexes;

    MenuItem *item = menuItem.data(Qt::UserRole).value<MenuItem *>();

    if (!item->item().library().isEmpty() || !item->service()->exec().isEmpty()) {
        indexes << menuItem;
    }

    for (int done = 0; menuItem.model()->rowCount(menuItem) > done; done = 1 + done) {
        indexes << menuItem.model()->index(done, 0, menuItem);
    }

    foreach (const QModelIndex &module, indexes) {
        MenuItem *newMenuItem = module.data(Qt::UserRole).value<MenuItem *>();
        addModule(&newMenuItem->item(), args);
    }
    // changing state is not needed here as the adding / changing of pages does it
}

void ModuleView::addModule(KCModuleInfo *module, const QStringList &args)
{
    if (!module || !module->service()->isValid()) {
        return;
    }
    if (!module->service()) {
        qWarning() << "ModuleInfo has no associated KService";
        return;
    }
    if (!KAuthorized::authorizeControlModule(module->service()->menuId())) {
        qWarning() << "Not authorised to load module";
        return;
    }
    if (module->service()->noDisplay()) {
        return;
    }

    if (KPageWidgetItem *page = d->mModules.key(module)) {
        activeModuleChanged(page, d->mPageWidget->currentPage());
        return;
    }

    // Create the scroller
    QScrollArea *moduleScroll = new QScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);
    // Create the page
    KPageWidgetItem *page = new KPageWidgetItem(moduleScroll, module->moduleName());
    // Provide information to the users

    if (module->service()->hasServiceType(QStringLiteral("SystemSettingsExternalApp")) || // Is it an external app?
        module->service()->substituteUid()) { // ...or does it require UID substitution?
        QWidget *externalWidget = new ExternalAppModule(this, module);
        moduleScroll->setWidget(externalWidget);
    } else { // It must be a normal module then
        KCModuleProxy *moduleProxy = new KCModuleProxy(*module, moduleScroll, args);
        moduleScroll->setWidget(moduleProxy);
        moduleProxy->setAutoFillBackground(false);
        connect(moduleProxy, SIGNAL(changed(bool)), this, SLOT(stateChanged()));
        d->mPages.insert(page, moduleProxy);
    }

    d->mModules.insert(page, module);
    updatePageIconHeader(page, true);
    // Add the new page
    d->mPageWidget->addPage(page);
}

void ModuleView::updatePageIconHeader(KPageWidgetItem *page, bool light)
{
    if (!page) {
        // Page is invalid. Probably means we have a race condition during closure of everyone so do nothing
        return;
    }

    KCModuleProxy *moduleProxy = d->mPages.value(page);
    KCModuleInfo *moduleInfo = d->mModules.value(page);

    if (!moduleInfo) {
        // Seems like we have some form of a race condition going on here...
        return;
    }

    page->setHeader(moduleInfo->moduleName());
    page->setIcon(QIcon::fromTheme(moduleInfo->icon()));

    d->mCustomHeader->setVisible(false);
    page->setHeaderVisible(false);

    if (light) {
        return;
    }

    if (moduleProxy && moduleProxy->realModule()->useRootOnlyMessage()) {
        page->setHeader(moduleInfo->moduleName() + QStringLiteral("<br><small>") + moduleProxy->realModule()->rootOnlyMessage() + QStringLiteral("</small>"));
        d->mCustomHeader->setText(moduleInfo->moduleName() + QStringLiteral("<br><small>") + moduleProxy->realModule()->rootOnlyMessage()
                                  + QStringLiteral("</small>"));
    }
}

bool ModuleView::resolveChanges()
{
    KCModuleProxy *currentProxy = d->mPages.value(d->mPageWidget->currentPage());
    return resolveChanges(currentProxy);
}

bool ModuleView::resolveChanges(KCModuleProxy *currentProxy)
{
    if (!currentProxy || !currentProxy->changed()) {
        return true;
    }

    // Let the user decide
    const int queryUser = KMessageBox::warningYesNoCancel(this,
                                                          i18n("The settings of the current module have changed.\n"
                                                               "Do you want to apply the changes or discard them?"),
                                                          i18n("Apply Settings"),
                                                          KStandardGuiItem::apply(),
                                                          KStandardGuiItem::discard(),
                                                          KStandardGuiItem::cancel());

    switch (queryUser) {
    case KMessageBox::Yes:
        return moduleSave(currentProxy);
    case KMessageBox::No:
        currentProxy->load();
        return true;
    case KMessageBox::Cancel:
        return false;
    default:
        Q_ASSERT(false);
        return false;
    }
}

void ModuleView::closeModules()
{
    d->pageChangeSupressed = true;
    d->mApplyAuthorize->setAuthAction(KAuth::Action()); // Ensure KAuth knows that authentication is now pointless...
    QMap<KPageWidgetItem *, KCModuleInfo *>::iterator page = d->mModules.begin();
    QMap<KPageWidgetItem *, KCModuleInfo *>::iterator pageEnd = d->mModules.end();
    for (; page != pageEnd; ++page) {
        d->mPageWidget->removePage(page.key());
    }

    d->mPages.clear();
    d->mModules.clear();
    d->pageChangeSupressed = false;
}

bool ModuleView::moduleSave()
{
    KCModuleProxy *moduleProxy = d->mPages.value(d->mPageWidget->currentPage());
    return moduleSave(moduleProxy);
}

bool ModuleView::moduleSave(KCModuleProxy *module)
{
    if (!module) {
        return false;
    }

    module->save();
    emit moduleSaved();
    return true;
}

void ModuleView::moduleLoad()
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->load();
    }
}

void ModuleView::moduleDefaults()
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->defaults();
    }
}

void ModuleView::moduleHelp()
{
    KCModuleInfo *activeModule = d->mModules.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return;
    }

    QString docPath = activeModule->docPath();
    if (docPath.isEmpty()) {
        return;
    }

    // UrlHandler from KGUIAddons sets a handler for help:/ urls, which opens khelpcenter
    // if it's available or falls back to opening the relevant page at docs.kde.org
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/") + docPath));
}

void ModuleView::activeModuleChanged(KPageWidgetItem *current, KPageWidgetItem *previous)
{
    d->mPageWidget->blockSignals(true);
    d->mPageWidget->setCurrentPage(previous);
    KCModuleProxy *previousModule = d->mPages.value(previous);
    if (resolveChanges(previousModule)) {
        d->mPageWidget->setCurrentPage(current);
    }
    d->mPageWidget->blockSignals(false);
    if (d->pageChangeSupressed) {
        return;
    }
    // We need to get the state of the now active module
    stateChanged();

    KCModuleInfo *activeModuleInfo = activeModule();
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule || activeModuleInfo) {
        // TODO: if we'll ever need statistics for kinfocenter modules, save them with an URL like "kinfo:"
        if (activeModule && d->mSaveStatistics) {
            KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("kcm:") + activeModule->moduleInfo().service()->storageId()),
                                                          QStringLiteral("org.kde.systemsettings"));
        }

        d->mCustomHeader->setText(activeModuleInfo->moduleName());

        const bool isQml = (activeModule && activeModule->realModule() && activeModule->realModule()->inherits("KCModuleQml"));
        d->mCustomHeader->setVisible(!isQml);
        current->setHeaderVisible(!isQml);

        QGridLayout *gridLayout = static_cast<QGridLayout *>(d->mPageWidget->layout());

        // QML KCM
        if (isQml) {
            gridLayout->setHorizontalSpacing(0);
            d->mCustomHeader->setVisible(false);
            current->setHeaderVisible(false);

            d->mButtons->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                            0, // Remove extra space between KCM content and bottom buttons
                                            style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                            style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
            d->mLayout->setContentsMargins(0, 0, 0, 0);
            d->mLayout->setSpacing(0);

            // QWidget KCM
        } else {
            // Sidebar mode
            if (faceType() == KPageView::Plain) {
                d->mCustomHeader->setVisible(true);
                current->setHeaderVisible(false);
                gridLayout->setHorizontalSpacing(0);

                d->mLayout->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                               style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                               style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                               style()->pixelMetric(QStyle::PM_LayoutBottomMargin));

                // Icons mode
            } else {
                d->mCustomHeader->setVisible(false);
                current->setHeaderVisible(true);
                gridLayout->setHorizontalSpacing(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
                d->mLayout->setContentsMargins(0, 0, 0, 0);
            }

            d->mButtons->setContentsMargins(0, 0, 0, 0);
            d->mLayout->setSpacing(style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));
        }
        moduleShowDefaultsIndicators(d->mDefaultsIndicatorsVisible);
    }
}

void ModuleView::stateChanged()
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    KAuth::Action moduleAction;
    bool change = false;
    bool defaulted = false;
    KCModule::Buttons buttons = KCModule::NoAdditionalButton;
    if (activeModule) {
        buttons = activeModule->buttons();
        change = activeModule->changed();
        defaulted = activeModule->defaulted();

        disconnect(d->mApplyAuthorize, SIGNAL(authorized(KAuth::Action)), this, SLOT(moduleSave()));
        disconnect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
        if (activeModule->realModule()->authAction().isValid()) {
            connect(d->mApplyAuthorize, SIGNAL(authorized(KAuth::Action)), this, SLOT(moduleSave()));
            moduleAction = activeModule->realModule()->authAction();
        } else {
            connect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
        }
    }

    updatePageIconHeader(d->mPageWidget->currentPage());

    KCModuleProxy *moduleProxy = d->mPages.value(d->mPageWidget->currentPage());
    d->mCustomHeader->setVisible(!moduleProxy || !moduleProxy->realModule()->inherits("KCModuleQml"));

    d->mApplyAuthorize->setAuthAction(moduleAction);
    d->mDefault->setEnabled(!defaulted);
    d->mDefault->setVisible(buttons & KCModule::Default);
    d->mApply->setEnabled(change);
    d->mApply->setVisible(buttons & KCModule::Apply);
    d->mReset->setEnabled(change);
    d->mReset->setVisible(buttons & KCModule::Apply);
    d->mHelp->setEnabled(buttons & KCModule::Help);
    d->mHelp->setVisible(buttons & KCModule::Help);
    emit moduleChanged(change);
}

void ModuleView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1 && d->mHelp->isVisible() && d->mHelp->isEnabled()) {
        d->mHelp->animateClick();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        emit closeRequest();
        return;
    } else if (event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier) {
        QWhatsThis::enterWhatsThisMode();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void ModuleView::setFaceType(KPageView::FaceType type)
{
    d->mPageWidget->setFaceType(type);
}

KPageView::FaceType ModuleView::faceType() const
{
    return d->mPageWidget->faceType();
}

void ModuleView::setSaveStatistics(bool save)
{
    d->mSaveStatistics = save;
}

bool ModuleView::saveStatistics() const
{
    return d->mSaveStatistics;
}

void ModuleView::setApplyVisible(bool visible)
{
    d->mApply->setVisible(visible);
}

bool ModuleView::isApplyVisible() const
{
    return d->mApply->isVisible();
}

void ModuleView::setDefaultsVisible(bool visible)
{
    d->mDefault->setVisible(visible);
}

bool ModuleView::isDefaultsVisible() const
{
    return d->mDefault->isVisible();
}

void ModuleView::setResetVisible(bool visible)
{
    d->mReset->setVisible(visible);
}

bool ModuleView::isResetVisible() const
{
    return d->mReset->isVisible();
}

void ModuleView::moduleShowDefaultsIndicators(bool show)
{
    d->mDefaultsIndicatorsVisible = show;
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->setDefaultsIndicatorsVisible(show);
    }
}

void ModuleView::setHeaderHeight(qreal height)
{
    if (height == d->mCustomHeader->minimumHeight()) {
        return;
    }

    d->mCustomHeader->setMinimumHeight(height);
}

qreal ModuleView::headerHeight() const
{
    return d->mCustomHeader->minimumHeight();
}
