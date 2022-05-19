/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@informatik.uni-bremen.de>
 *   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

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
#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/Action>
#include <KAuth/ObjectDecorator>
#else
#include <KAuthAction>
#include <KAuthObjectDecorator>
#endif
#include <KAuthorized>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KColorScheme>
#include <KMessageBox>
#include <KPageWidget>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KTitleWidget>
#include <Kirigami/Units>

#include <KActivities/ResourceInstance>

#include <cmath>
#include <kpluginmetadata.h>

#include "MenuItem.h"

class CustomTitle : public KTitleWidget
{
public:
    explicit CustomTitle(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void colorsChanged();
};

CustomTitle::CustomTitle(QWidget *parent)
    : KTitleWidget(parent)
{
    // Use the same left margin as QML titles for consistency (Kirigami/AbstractPageHeader.qml)
    setContentsMargins(Kirigami::Units().gridUnit(),
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
    QMap<KPageWidgetItem *, QString> mPagesPluginIdMap;
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
    KCModule::Buttons mButtonMask = ~KCModule::Buttons(KCModule::NoAdditionalButton);
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
    d->mCustomHeader->setVisible(false);

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

QString ModuleView::activeModuleName() const
{
    return d->mPageWidget->currentPage() ? d->mPageWidget->currentPage()->name() : QString();
}

void ModuleView::loadModule(const QModelIndex &menuItem, const QStringList &args)
{
    if (!menuItem.isValid()) {
        return;
    }

    MenuItem *item = menuItem.data(Qt::UserRole).value<MenuItem *>();

    // if module has a main page (like in Appearance > Global Theme) we'll load that
    if (item->isLibrary() || item->isExternalAppModule()) {
        addModule(item, args);
    }
    // if module doesn't have a main page, we'll load the first subpage
    else if (menuItem.model()->rowCount(menuItem) > 0) {
        MenuItem *subpageItem = menuItem.model()->index(0, 0, menuItem).data(Qt::UserRole).value<MenuItem *>();
        addModule(subpageItem, args);
    }
}

void ModuleView::addModule(MenuItem *item, const QStringList &args)
{
    const KPluginMetaData data = item->metaData();
    if (!KAuthorized::authorizeControlModule(data.pluginId())) {
        qWarning() << "Not authorised to load module";
        return;
    }
    if (data.isHidden()) {
        return;
    }

    if (KPageWidgetItem *page = d->mPagesPluginIdMap.key(data.name())) {
        activeModuleChanged(page, d->mPageWidget->currentPage());
        return;
    }

    // Create the scroller
    auto *moduleScroll = new QScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);
    // Create the page
    auto *page = new KPageWidgetItem(moduleScroll, data.name());
    // Provide information to the users

    if (item->isExternalAppModule()) {
        auto *externalWidget = new ExternalAppModule(this, KService::Ptr(new KService(item->metaData().metaDataFileName())));
        moduleScroll->setWidget(externalWidget);
        d->mCustomHeader->setText(item->metaData().name()); // We have to set this manually, BUG: 448672
        page->setName(QString());
    } else { // It must be a normal module then
        auto *moduleProxy = new KCModuleProxy(data, moduleScroll, args);
        moduleScroll->setWidget(moduleProxy);
        moduleProxy->setAutoFillBackground(false);
        connect(moduleProxy, &KCModuleProxy::changed, this, &ModuleView::stateChanged);
        d->mPages.insert(page, moduleProxy);
    }

    d->mPagesPluginIdMap.insert(page, data.name());
    updatePageIconHeader(page);
    // Add the new page
    d->mPageWidget->addPage(page);
}

void ModuleView::updatePageIconHeader(KPageWidgetItem *page)
{
    if (!page) {
        // Page is invalid. Probably means we have a race condition during closure of everyone so do nothing
        return;
    }

    KCModuleProxy *moduleProxy = d->mPages.value(page);
    if (!moduleProxy || !moduleProxy->metaData().isValid()) {
        // Seems like we have some form of a race condition going on here...
        return;
    }

    const QString moduleName = moduleProxy->metaData().name();
    page->setHeader(moduleName);
    page->setIcon(QIcon::fromTheme(moduleProxy->metaData().iconName()));

    const bool isQml = moduleProxy->realModule() && moduleProxy->realModule()->inherits("KCModuleQml");
    const bool isSidebar = faceType() == KPageView::Plain;

    // Use the module's header only for QWidgets KCMs on Icons mode
    page->setHeaderVisible(!isQml && !isSidebar);

    // Use the custom header only for QWidgets KCMs on Sidebar mode
    // Only affect visibility if it's the current page
    if (d->mPageWidget->currentPage() == page) {
        d->mCustomHeader->setVisible(!isQml && isSidebar);
        // KTitleWidget->setText() would set the titlebar visible again
        if (d->mCustomHeader->isVisible()) {
            d->mCustomHeader->setText(moduleName);
        }
    }
}

bool ModuleView::resolveChanges()
{
    KCModuleProxy *currentProxy = d->mPages.value(d->mPageWidget->currentPage());
    return resolveChanges(currentProxy);
}

bool ModuleView::resolveChanges(KCModuleProxy *currentProxy)
{
    if (!currentProxy || !currentProxy->isChanged()) {
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
    for (auto page = d->mPagesPluginIdMap.cbegin(); page != d->mPagesPluginIdMap.cend(); ++page) {
        d->mPageWidget->removePage(page.key());
    }

    d->mPages.clear();
    d->mPagesPluginIdMap.clear();
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
    Q_EMIT moduleSaved();
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
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return;
    }

    const QString docPath = activeModule->metaData().value(QStringLiteral("X-DocPath"));
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

    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return;
    }

    // TODO: if we'll ever need statistics for kinfocenter modules, save them with an URL like "kinfo:"
    if (d->mSaveStatistics && activeModule->metaData().pluginId() != QStringLiteral("kcm_landingpage")) {
        KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("kcm:") + activeModule->metaData().pluginId()),
                                                      QStringLiteral("org.kde.systemsettings"));
    }

    d->mLayout->setContentsMargins(0, 0, 0, 0);
    d->mLayout->setSpacing(0);
    d->mButtons->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                    0, // Remove extra space between KCM content and bottom buttons
                                    style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                    style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    d->mPageWidget->layout()->setSpacing(0);
    if (auto titleWidget = qobject_cast<KTitleWidget *>(d->mPageWidget->pageHeader())) {
        titleWidget->layout()->setContentsMargins(Kirigami::Units().gridUnit(),
                                                  style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                                  style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                                  style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    }

    updatePageIconHeader(current);
    moduleShowDefaultsIndicators(d->mDefaultsIndicatorsVisible);
}

void ModuleView::stateChanged()
{
    updatePageIconHeader(d->mPageWidget->currentPage());
    updateButtons();

    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    Q_EMIT moduleChanged(activeModule && activeModule->isChanged());
}

void ModuleView::updateButtons()
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    KAuth::Action moduleAction;
    bool change = false;
    bool defaulted = false;
    KCModule::Buttons buttons = KCModule::NoAdditionalButton;

    if (activeModule) {
        buttons = activeModule->buttons() & d->mButtonMask;
        change = activeModule->isChanged();
        defaulted = activeModule->defaulted();

        // Do not display Help button if there is no docPath available
        if (activeModule->metaData().value(QStringLiteral("X-DocPath")).isEmpty()) {
            buttons &= ~KCModule::Help;
        }

        disconnect(d->mApplyAuthorize, SIGNAL(authorized(KAuth::Action)), this, SLOT(moduleSave()));
        disconnect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
        if (activeModule->realModule()->authAction().isValid()) {
            connect(d->mApplyAuthorize, SIGNAL(authorized(KAuth::Action)), this, SLOT(moduleSave()));
            moduleAction = activeModule->realModule()->authAction();
        } else {
            connect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
        }
    }

    d->mApplyAuthorize->setAuthAction(moduleAction);
    d->mDefault->setEnabled(!defaulted);
    d->mDefault->setVisible(buttons & KCModule::Default);
    d->mApply->setEnabled(change);
    d->mApply->setVisible(buttons & KCModule::Apply);
    d->mReset->setEnabled(change);
    d->mReset->setVisible(buttons & KCModule::Apply);
    d->mHelp->setEnabled(buttons & KCModule::Help);
    d->mHelp->setVisible(buttons & KCModule::Help);

    d->mButtons->setVisible(buttons != KCModule::NoAdditionalButton);
}

void ModuleView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1 && d->mHelp->isVisible() && d->mHelp->isEnabled()) {
        d->mHelp->animateClick();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        Q_EMIT closeRequest();
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
    d->mButtonMask.setFlag(KCModule::Apply, visible);
    updateButtons();
}

bool ModuleView::isApplyVisible() const
{
    return d->mApply->isVisible();
}

void ModuleView::setDefaultsVisible(bool visible)
{
    d->mButtonMask.setFlag(KCModule::Default, visible);
    updateButtons();
}

bool ModuleView::isDefaultsVisible() const
{
    return d->mDefault->isVisible();
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

void ModuleView::setActiveModule(const QString &moduleName)
{
    const auto pageList = d->mPagesPluginIdMap.keys();
    for (const auto page : pageList) {
        if (d->mPagesPluginIdMap.value(page) == moduleName) {
            d->mPageWidget->setCurrentPage(page);
            break;
        }
    }
}

KPluginMetaData ModuleView::activeModuleMetadata() const
{
    KCModuleProxy *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return {};
    }
    return activeModule->metaData();
}
