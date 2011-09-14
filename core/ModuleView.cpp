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

#include <QMap>
#include <QList>
#include <QProcess>
#include <QKeyEvent>
#include <QWhatsThis>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QAbstractItemModel>

#include <KDebug>
#include <KDialog>
#include <KAboutData>
#include <KPageWidget>
#include <KPushButton>
#include <KAuthorized>
#include <KMessageBox>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KStandardGuiItem>
#include <KDialogButtonBox>
#include <kauthaction.h>

#include "MenuItem.h"

class ModuleView::Private {
public:
    Private() { }
    QMap<KPageWidgetItem*, KCModuleProxy*> mPages;
    QMap<KPageWidgetItem*, KCModuleInfo*> mModules;
    KPageWidget* mPageWidget;
    QVBoxLayout* mLayout;
    KDialogButtonBox* mButtons;
    KPushButton* mApply;
    KPushButton* mReset;
    KPushButton* mDefault;
    KPushButton* mHelp;
    bool pageChangeSupressed;
};

ModuleView::ModuleView( QWidget * parent )
    : QWidget( parent )
    , d( new Private() )
{
    // Configure a layout first
    d->mLayout = new QVBoxLayout(this);
    // Create the Page Widget
    d->mPageWidget = new KPageWidget(this);
    d->mPageWidget->layout()->setMargin(0);
    d->mLayout->addWidget(d->mPageWidget);
    // Create the dialog
    d->mButtons = new KDialogButtonBox( this, Qt::Horizontal );
    d->mLayout->addWidget(d->mButtons);

    // Create the buttons in it
    d->mApply = d->mButtons->addButton( KStandardGuiItem::apply(), QDialogButtonBox::ApplyRole );
    d->mDefault = d->mButtons->addButton( KStandardGuiItem::defaults(), QDialogButtonBox::ResetRole );
    d->mReset = d->mButtons->addButton( KStandardGuiItem::reset(), QDialogButtonBox::ResetRole );
    d->mHelp = d->mButtons->addButton( KStandardGuiItem::help(), QDialogButtonBox::HelpRole );
    // Set some more sensible tooltips
    d->mReset->setToolTip( i18n("Reset all current changes to previous values") );
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
    connect( d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()) );
    connect( d->mReset, SIGNAL(clicked()), this, SLOT(moduleLoad()) );
    connect( d->mHelp, SIGNAL(clicked()), this, SLOT(moduleHelp()) );
    connect( d->mDefault, SIGNAL(clicked()), this, SLOT(moduleDefaults()) );
    connect( d->mPageWidget, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
             this, SLOT(activeModuleChanged(KPageWidgetItem*,KPageWidgetItem*)) );
    connect( this, SIGNAL(moduleChanged(bool)), this, SLOT(updateButtons()) );
}

ModuleView::~ModuleView()
{
    delete d;
}

KCModuleInfo * ModuleView::activeModule() const
{
    return d->mModules.value( d->mPageWidget->currentPage() );
}

const KAboutData * ModuleView::aboutData() const
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    KAboutData * aboutData = 0;
    if( activeModule ) {
        aboutData = const_cast<KAboutData*>( activeModule->aboutData() );
    }
    if ( aboutData ) {
        aboutData->setProgramIconName( activeModule->moduleInfo().service()->icon() );
        return aboutData;
    }
    return 0;
}

void ModuleView::loadModule( QModelIndex menuItem )
{
    if ( !menuItem.isValid() ) {
        return;
    }

    QList<QModelIndex> indexes;
    for ( int done = 0; menuItem.model()->rowCount( menuItem ) > done; done = 1 + done ) {
        indexes << menuItem.model()->index( done, 0, menuItem );
    }
    if ( indexes.empty() ) {
        indexes << menuItem;
    }

    foreach ( QModelIndex module, indexes ) {
        MenuItem *menuItem = module.data( Qt::UserRole ).value<MenuItem*>();
        addModule( &menuItem->item() );
    }
    // changing state is not needed here as the adding / changing of pages does it
}

void ModuleView::addModule( KCModuleInfo *module )
{
    if( !module ) {
        return;
    }
    if( !module->service() ) {
        kWarning() << "ModuleInfo has no associated KService" ;
        return;
    }
    if ( !KAuthorized::authorizeControlModule( module->service()->menuId() ) ) {
        kWarning() << "Not authorised to load module" ;
        return;
    }
    if( module->service()->noDisplay() ) {
        return;
    }

    // Create the scroller
    QScrollArea * moduleScroll = new QScrollArea( this );
    // Prepare the scroll area
    moduleScroll->setWidgetResizable( true );
    moduleScroll->setFrameStyle( QFrame::NoFrame );
    moduleScroll->viewport()->setAutoFillBackground( false );
    // Create the page
    KPageWidgetItem *page = new KPageWidgetItem( moduleScroll, module->moduleName() );
    // Provide information to the users

    if( module->service()->hasServiceType("SystemSettingsExternalApp") ||  // Is it an external app?
            module->service()->substituteUid() ) { // ...or does it require UID substituion?
        QWidget * externalWidget = new ExternalAppModule( this, module );
        moduleScroll->setWidget( externalWidget );
    } else { // It must be a normal module then
        KCModuleProxy * moduleProxy = new KCModuleProxy( *module, moduleScroll );
        moduleScroll->setWidget( moduleProxy );
        moduleProxy->setAutoFillBackground( false );
        connect( moduleProxy, SIGNAL(changed(bool)), this, SLOT(stateChanged()));
        d->mPages.insert( page, moduleProxy );
    }

    d->mModules.insert( page, module );
    updatePageIconHeader( page, true );
    // Add the new page
    d->mPageWidget->addPage( page );
}

void ModuleView::updatePageIconHeader( KPageWidgetItem * page, bool light )
{
    if( !page ) {
        // Page is invalid. Probably means we have a race condition during closure of everyone so do nothing
        return;
    }

    KCModuleProxy * moduleProxy = d->mPages.value( page );
    KCModuleInfo * moduleInfo = d->mModules.value( page );

    if( !moduleInfo ) {
        // Seems like we have some form of a race condition going on here...
        return; 
    }

    page->setHeader( moduleInfo->comment() );
    page->setIcon( KIcon( moduleInfo->icon() ) );
    if( light ) {
        return;
    }

    if( moduleProxy && moduleProxy->realModule()->useRootOnlyMessage() ) {
        page->setHeader( "<b>" + moduleInfo->comment() + "</b><br><i>" + moduleProxy->rootOnlyMessage() + "</i>" );
        page->setIcon( KIcon( moduleInfo->icon(), 0, QStringList() << "dialog-warning" ) );
    }
}

bool ModuleView::resolveChanges()
{
    KCModuleProxy * currentProxy = d->mPages.value( d->mPageWidget->currentPage() );
    return resolveChanges(currentProxy);
}

bool ModuleView::resolveChanges(KCModuleProxy * currentProxy)
{
    if( !currentProxy || !currentProxy->changed() ) {
        return true;
    }

    // Let the user decide
    KGuiItem applyItem = KStandardGuiItem::apply();
    applyItem.setIcon( KIcon(d->mApply->icon()) );
    const int queryUser = KMessageBox::warningYesNoCancel(
        this,
        i18n("The settings of the current module have changed.\n"
             "Do you want to apply the changes or discard them?"),
        i18n("Apply Settings"),
        applyItem,
        KStandardGuiItem::discard(),
        KStandardGuiItem::cancel() );

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
    d->mApply->setAuthAction( 0 ); // Ensure KAuth knows that authentication is now pointless...
    QMap<KPageWidgetItem*, KCModuleInfo*>::iterator page = d->mModules.begin();
    QMap<KPageWidgetItem*, KCModuleInfo*>::iterator pageEnd = d->mModules.end();
    for ( ; page != pageEnd; ++page ) {
        d->mPageWidget->removePage( page.key() );
    }

    d->mPages.clear();
    d->mModules.clear();
    d->pageChangeSupressed = false;
}

bool ModuleView::moduleSave()
{
    KCModuleProxy * moduleProxy = d->mPages.value( d->mPageWidget->currentPage() );
    return moduleSave( moduleProxy );
}

bool ModuleView::moduleSave(KCModuleProxy *module)
{
    if( !module ) {
        return false;
    }

    module->save();
    return true;
}

void ModuleView::moduleLoad()
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( activeModule ) {
        activeModule->load();
    }
}

void ModuleView::moduleDefaults()
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( activeModule ) {
        activeModule->defaults();
    }
}

void ModuleView::moduleHelp()
{
    KCModuleInfo * activeModule = d->mModules.value( d->mPageWidget->currentPage() );
    if( !activeModule ) {
        return;
    }

    QString docPath = activeModule->docPath();
    if( docPath.isEmpty() ) {
        return;
    }
    KUrl url( KUrl("help:/"), docPath );
    QProcess::startDetached("khelpcenter", QStringList() << url.url());
}

void ModuleView::activeModuleChanged(KPageWidgetItem * current, KPageWidgetItem * previous)
{
    d->mPageWidget->blockSignals(true);
    d->mPageWidget->setCurrentPage(previous);
    KCModuleProxy * previousModule = d->mPages.value(previous);
    if( resolveChanges(previousModule) ) {
        d->mPageWidget->setCurrentPage(current);
    }
    d->mPageWidget->blockSignals(false);
    if( d->pageChangeSupressed ) {
        return;
    }
    // We need to get the state of the now active module
    stateChanged();
}

void ModuleView::stateChanged()
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    KAuth::Action * moduleAction = 0;
    bool change = false;
    if( activeModule ) {
        change = activeModule->changed();

        disconnect( d->mApply, SIGNAL(authorized(KAuth::Action*)), this, SLOT(moduleSave()) );
        disconnect( d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()) );
        if( activeModule->realModule()->authAction() ) {
            connect( d->mApply, SIGNAL(authorized(KAuth::Action*)), this, SLOT(moduleSave()) );
            moduleAction = activeModule->realModule()->authAction();
        } else {
            connect( d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()) );
        }
    }

    updatePageIconHeader( d->mPageWidget->currentPage() );
    d->mApply->setAuthAction( moduleAction );
    d->mApply->setEnabled( change );
    d->mReset->setEnabled( change );
    emit moduleChanged( change );
}

void ModuleView::keyPressEvent ( QKeyEvent * event )
{
    if ( event->key() == Qt::Key_F1 && d->mHelp->isVisible() && d->mHelp->isEnabled()) {
        d->mHelp->animateClick();
        event->accept();
        return;
    } else if ( event->key() == Qt::Key_Escape ) {
        event->accept();
        emit closeRequest();
        return;
    } else if ( event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier ) {
        QWhatsThis::enterWhatsThisMode();
        event->accept();
        return;
    }

    QWidget::keyPressEvent( event );
}

void ModuleView::updateButtons()
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( !activeModule ) {
        return;
    }

    const int buttons = activeModule->buttons();

    d->mApply->setShown(buttons & KCModule::Apply );
    d->mReset->setShown(buttons & KCModule::Apply );

    d->mHelp->setEnabled(buttons & KCModule::Help );
    d->mDefault->setEnabled(buttons & KCModule::Default );
}

#include "ModuleView.moc"
