/*****************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org>   *
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

#include <QMap>
#include <QList>
#include <QProcess>
#include <QKeyEvent>
#include <QWhatsThis>
#include <QScrollArea>
#include <QVBoxLayout>

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

#include "MenuItem.h"

class ModuleView::Private {
public:
    Private() { }
    QMap<KPageWidgetItem*, KCModuleProxy*> mPages;
    QMap<KCModuleProxy*, KCModuleInfo*> mModules;
    KPageWidget* mPageWidget;
    QVBoxLayout* mLayout;
    KDialogButtonBox* mButtons;
    KPushButton* mApply;
    KPushButton* mReset;
    KPushButton* mDefault;
    KPushButton* mHelp;
};

ModuleView::ModuleView( QWidget * parent ) : QWidget( parent ), d( new Private() )
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
    connect( d->mPageWidget, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)),
             this, SLOT(activeModuleChanged(KPageWidgetItem*, KPageWidgetItem*)) );
    connect( this, SIGNAL(moduleChanged(bool)), this, SLOT(updateButtons()) );
}

ModuleView::~ModuleView()
{
    delete d;
}

KCModuleInfo * ModuleView::activeModule() const
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( activeModule ) {
        return d->mModules.value(activeModule);
    } else {
        return 0;
    }
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

void ModuleView::loadModule( MenuItem *menuItem )
{
    if ( !menuItem ) {
      return;
    }

    QList<KCModuleInfo*> modules;
    if ( menuItem->children().empty() ) {
        modules << &menuItem->item();
    } else {
        foreach ( MenuItem *child, menuItem->children() ) {
            modules << &child->item();
        }
    }

    foreach ( KCModuleInfo *module, modules ) {
        addModule(module);
    }
    emit moduleChanged( false );
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

    // Create the items
    QScrollArea * moduleScroll = new QScrollArea( this );
    KCModuleProxy * moduleProxy = new KCModuleProxy( *module, moduleScroll );
    // Prepare the module
    moduleProxy->setAutoFillBackground( false );
    // Prepare the scroll area
    moduleScroll->setWidgetResizable( true );
    moduleScroll->setFrameStyle( QFrame::NoFrame );
    moduleScroll->viewport()->setAutoFillBackground( false );
    moduleScroll->setWidget( moduleProxy );
    // Create the page
    KPageWidgetItem *page = new KPageWidgetItem( moduleScroll, module->moduleName() );
    // Provide information to the users
    page->setIcon( KIcon( module->service()->icon() ) );
    page->setHeader( module->service()->comment() );
    // Allow it to signal properly
    connect( moduleProxy, SIGNAL(changed(bool)), this, SIGNAL(moduleChanged(bool)));
    // Set it to be shown and signal that
    d->mPageWidget->addPage( page );
    d->mPages.insert( page, moduleProxy );
    d->mModules.insert( moduleProxy, module );
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
    int queryUser = KMessageBox::warningYesNoCancel(
        this,
        i18n("The settings of the current module have changed.\n"
             "Do you want to apply the changes or discard them?"),
        i18n("Apply Settings"),
        KStandardGuiItem::apply(),
        KStandardGuiItem::discard(),
        KStandardGuiItem::cancel() );

    switch (queryUser) {
        case KMessageBox::Yes:
            currentProxy->save();
            return true;

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

bool ModuleView::closeModules()
{
    if( !resolveChanges() ) {
        return false;
    }

    blockSignals(true);
    QMap<KPageWidgetItem*, KCModuleProxy*>::iterator pageIterator;
    QMap<KPageWidgetItem*, KCModuleProxy*>::iterator endIterator = d->mPages.end();
    // These two MUST be kept separate in order to ensure modules aren't loaded during the closing procedure
    for ( pageIterator = d->mPages.begin(); pageIterator != endIterator; pageIterator = pageIterator + 1 ) {
        delete pageIterator.value();
        pageIterator.value() = 0;
    }
    for ( pageIterator = d->mPages.begin(); pageIterator != endIterator; pageIterator = pageIterator + 1 ) {
        d->mPageWidget->removePage( pageIterator.key() );
    }
    d->mPages.clear();
    d->mModules.clear();
    blockSignals(false);
    return true;
}

void ModuleView::moduleSave()
{
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( activeModule ) {
        activeModule->save();
    }
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
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    if( !activeModule ) {
        return;
    }

    QString docPath = activeModule->moduleInfo().docPath();
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
    // We need to get the state of the now active module
    KCModuleProxy * activeModule = d->mPages.value( d->mPageWidget->currentPage() );
    bool change = false;
    if( activeModule ) {
        change = activeModule->changed();
    }
    d->mApply->setEnabled(change);
    d->mReset->setEnabled(change);
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

    int buttons = activeModule->buttons();

    d->mApply->setShown(buttons & KCModule::Apply );
    d->mReset->setShown(buttons & KCModule::Apply );

    d->mHelp->setEnabled(buttons & KCModule::Help );
    d->mDefault->setEnabled(buttons & KCModule::Default );
}

#include "ModuleView.moc"
