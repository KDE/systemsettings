/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>
   Copyright (c) 2004 Frans Englich <englich@kde.org>
   Copyright (c) 2008 Michael Jansen <kde@michael-jansen.biz>
   Copyright (c) 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kcmultiwidget.h"

#include <QLayout>
#include <QProcess>
#include <QScrollArea>

#include <QtGui/QKeyEvent>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWhatsThis>

#include <kdebug.h>
#include <kiconloader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kstandardguiitem.h>
#include <kuser.h>
#include <kauthorized.h>
#include <ktoolinvocation.h>

#include <kpushbutton.h>
#include <kdialogbuttonbox.h>

#include "kcmoduleloader.h"
#include "kcmoduleproxy.h"

/*
Button usage:

    User1 => Close
    User2 => Admin (dead in KDE 4)
*/

class KCMultiWidget::KCMultiWidgetPrivate
{
    public:
        KCMultiWidgetPrivate()
            : hasRootKCM( false )
        {}

        bool hasRootKCM;
};

KCMultiWidget::KCMultiWidget(QWidget *parent)
    : QWidget( parent ),
    d( new KCMultiWidgetPrivate )
{
    QVBoxLayout * mainLayout = new QVBoxLayout();
    setLayout( mainLayout );
    
    m_pageWidget = new KPageWidget( this );
    mainLayout->addWidget( m_pageWidget );
    
    connect( m_pageWidget, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)),
        this, SLOT(slotAboutToShow(KPageWidgetItem*, KPageWidgetItem* )) );
    m_pageWidget->setFaceType( KPageWidget::Auto );
    
    m_buttonBox = new KDialogButtonBox( this, Qt::Horizontal );
    mainLayout->addWidget( m_buttonBox );
    setupButtonBox();
}

KCMultiWidget::~KCMultiWidget()
{
    delete d;
    delete m_pageWidget;
}

void KCMultiWidget::setupButtonBox()
{
    //Set up buttons
    m_helpButton = m_buttonBox->addButton( KStandardGuiItem::help(), QDialogButtonBox::HelpRole );
    m_defaultsButton = m_buttonBox->addButton( KStandardGuiItem::defaults(), QDialogButtonBox::ResetRole );
    m_applyButton = m_buttonBox->addButton( KStandardGuiItem::apply(), QDialogButtonBox::ApplyRole );
    m_resetButton = m_buttonBox->addButton( KStandardGuiItem::reset(), QDialogButtonBox::ResetRole );
    
    m_helpButton->setEnabled( false );
    m_defaultsButton->setEnabled( false );
    m_applyButton->setEnabled( false );
    m_resetButton->setEnabled( false );
    
    connect( m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()) );
    connect( m_defaultsButton, SIGNAL(clicked()), this, SLOT(slotDefault()) );
    connect( m_applyButton, SIGNAL(clicked()), this, SLOT(slotApply()) );
    connect( m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()) );
}

void KCMultiWidget::keyPressEvent ( QKeyEvent * event )
{
    //Mimic part of the KDialog behaviour
    if ( event->modifiers() == 0 ) {
        if ( event->key() == Qt::Key_F1 ) {
            if ( m_helpButton->isVisible() && m_helpButton->isEnabled() ) {
                m_helpButton->animateClick();
                event->accept();
                return;
            }
        }
        if ( event->key() == Qt::Key_Escape ) {
            event->accept();
            if( queryClose() ) //Close modules
                emit finished();
            return;
        }
    } else if ( event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier ) {
        QWhatsThis::enterWhatsThisMode();
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent( event );
}

void KCMultiWidget::slotDefault()
{
    defaults(currentModule());
}


void KCMultiWidget::slotReset()
{
    reset(currentModule());
}


void KCMultiWidget::slotApply()
{
    apply(currentModule());
}


void KCMultiWidget::slotHelp()
{
    QString docPath = currentModule()->moduleInfo().docPath();
    if(docPath.isEmpty())
       return;
    KUrl url( KUrl("help:/"), docPath );

    if (url.protocol() == "help" || url.protocol() == "man" || url.protocol() == "info") {
        QProcess::startDetached("khelpcenter", QStringList() << url.url());
    } else {
        KToolInvocation::invokeBrowser( url.url() );
    }
}


void KCMultiWidget::clientChanged(bool state)
{
    kDebug( 710 ) << state;
    foreach( const CreatedModule &it, m_modules )
        if( it.kcm->changed() ) {
            m_applyButton->setEnabled( true );
            m_resetButton->setEnabled( true );
            return;
        }
    m_applyButton->setEnabled( false );
    m_resetButton->setEnabled( false );
}


void KCMultiWidget::addModule(const KCModuleInfo& moduleinfo)
{
    if( !moduleinfo.service() ) {
        kWarning() << "ModuleInfo has no associated KService" ;
        return;
    }

    if ( !KAuthorized::authorizeControlModule( moduleinfo.service()->menuId() )) {
        kWarning() << "Not authorised to load module" ;
        return;
    }

    if(moduleinfo.service()->noDisplay()) {
        return;
    }

    QScrollArea* moduleScrollArea = new QScrollArea( m_pageWidget );
    KCModuleProxy *module = new KCModuleProxy( moduleinfo, moduleScrollArea );
    moduleScrollArea->setWidget( module );
    moduleScrollArea->setWidgetResizable( true );
    moduleScrollArea->setFrameStyle( QFrame::NoFrame );
    moduleScrollArea->viewport()->setAutoFillBackground(false);
    module->setAutoFillBackground(false);
    QStringList parentComponents = moduleinfo.service()->property(
            "X-KDE-System-Settings-Parent-Category" ).toStringList();
    moduleParentComponents.insert( module, parentComponents );

    connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

    CreatedModule cm;
    cm.kcm = module;
    cm.service = moduleinfo.service();
    cm.adminmode = false;

    // "root KCMs are gone" says KControl
    //	if ( moduleinfo.needsRootPrivileges() && !d->hasRootKCM &&
    //			!KUser().isSuperUser() ) {/* If we're embedded, it's true */
    // 		d->hasRootKCM = true;
    // 		cm.adminmode = true;
    // 		m_modules.append( cm );
    // 		if( dialogface==Plain ) {
    // 			slotAboutToShow( page ); // Won't be called otherwise, necessary for adminMode button
    //                }
    // 	} else {
    // 		m_modules.append( cm );
    // 	}

    m_modules.append( cm );
    if( m_modules.count() == 1 ) {
        slotAboutToShow( module );
    }
    KPageWidgetItem* page = m_pageWidget->addPage(moduleScrollArea, moduleinfo.moduleName());
    page->setIcon( KIcon(moduleinfo.icon()) );
    page->setHeader(moduleinfo.comment());
}


KCModuleProxy* KCMultiWidget::currentModule() 
{
    KPageWidgetItem *pageWidget = m_pageWidget->currentPage();
    if ( pageWidget == 0 )
        return 0;

    QScrollArea *scrollArea = qobject_cast<QScrollArea*>( pageWidget->widget() );
    KCModuleProxy *module = qobject_cast<KCModuleProxy*>( scrollArea->widget() );

    return module;
}


void KCMultiWidget::slotAboutToShow(KPageWidgetItem* current, KPageWidgetItem* before)
 {
    if( before != 0 ) {
        QScrollArea *scrollArea = qobject_cast<QScrollArea*>( before->widget() );
        KCModuleProxy *module = qobject_cast<KCModuleProxy*>( scrollArea->widget() );
        if (!queryClose(module)) {
            m_pageWidget->setCurrentPage(before);
            return;
        }
    }

    QWidget* sendingWidget = current->widget();
    slotAboutToShow(sendingWidget);
}


void KCMultiWidget::slotAboutToShow(QWidget *page)
{
    QList<KCModuleProxy*> objects = page->findChildren<KCModuleProxy*>();

    // add fall back
    objects.append( qobject_cast<KCModuleProxy*>(page) );

    KCModuleProxy *module = objects.first();
    if( ! module ) {
        return;
    }

    emit ( aboutToShow( module ) );

    int buttons = 0;
    bool found = false;
    foreach( const CreatedModule &it, m_modules ) {
        if( it.kcm==module) {
            //showButton(User2, it.adminmode);
            buttons = it.kcm->buttons();
            found = true;
        }
    }
    if (!found) {
        buttons = module->buttons();
    }

    m_applyButton->setVisible( buttons & KCModule::Apply );
    m_resetButton->setVisible( buttons & KCModule::Apply );
    
    m_helpButton->setEnabled( buttons & KCModule::Help );
    m_defaultsButton->setEnabled( buttons & KCModule::Help );
    
    //disconnect( this, SIGNAL(user3Clicked()), 0, 0 );

    // 	if (module->moduleInfo().needsRootPrivileges() &&
    // 			!module->rootMode() )
    // 	{ /* Enable the Admin Mode button */
    // 		enableButton( User2, true );
    // 		connect( this, SIGNAL(user3Clicked()), module, SLOT( runAsRoot() ));
    // 		connect( this, SIGNAL(user3Clicked()), SLOT( disableRModeButton() ));
    // 	} else {
    // 		enableButton( User2, false );
    // 	}
}

// Currently unused. Whenever root mode comes back
#if 0
void KCMultiWidget::rootExit()
{
    enableButton( User2, true);
}

void KCMultiWidget::disableRModeButton()
{
    enableButton( User2, false );
    connect ( currentModule(), SIGNAL( childClosed() ), SLOT( rootExit() ) );
}
#endif


void KCMultiWidget::apply(KCModuleProxy *module)
{
    module->save();
    emit configCommitted();

        // TODO: check what that stuff does! I think it's not needed
        QStringList updatedModules;
        QStringList names = moduleParentComponents[ module ];
        foreach ( const QString &name , names )
        {
            if ( updatedModules.indexOf(name) == -1 )
                updatedModules.append(name);
        }

        foreach( const QString &it, updatedModules )
        {
            emit configCommitted( it.toLatin1() );
        }

    clientChanged(false);
}


void KCMultiWidget::defaults(KCModuleProxy *module)
{
    module->defaults();
    clientChanged( true );
}


bool KCMultiWidget::queryClose()
{
    return queryClose(currentModule());
}


bool KCMultiWidget::queryClose(KCModuleProxy *module)
{
    if( !module || !module->changed() )
        return true;

    // Let the user decide
    int res = KMessageBox::warningYesNoCancel(
        this,
        i18n("The current page has been modified.\n"
             "Do you want to apply the changes or discard them?"),
        i18n("Apply Settings"),
        KStandardGuiItem::apply(),
        KStandardGuiItem::discard(),
        KStandardGuiItem::cancel() );

    switch (res) {

    case KMessageBox::Yes:
        apply(module);
        return true;

    case KMessageBox::No:
        reset(module);
        return true;

    case KMessageBox::Cancel:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}


void KCMultiWidget::reset(KCModuleProxy *module)
{
    module->load();
    clientChanged(false);
}


#include "kcmultiwidget.moc"
