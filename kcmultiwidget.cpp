/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>
   Copyright (c) 2004 Frans Englich <englich@kde.org>

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

KCMultiWidget::KCMultiWidget(QWidget *parent, Qt::WindowModality modality)
    : KPageDialog( parent ),
    d( new KCMultiWidgetPrivate )
{
    InitKIconDialog(i18n("Configure"), modality);
    init();
}

// Maybe move into init()?
void KCMultiWidget::InitKIconDialog(const QString& caption,
                                    Qt::WindowModality modality)
{
  setCaption(caption);
  setButtons(KDialog::Help |
             KDialog::Default |
             KDialog::Cancel |
             KDialog::Apply |
             KDialog::Ok |
             KDialog::Reset |
             KDialog::User1);
  setDefaultButton(KDialog::Ok);
  setButtonGuiItem(KDialog::User1, KStandardGuiItem::close());

  setWindowModality(modality);
}

inline void KCMultiWidget::init()
{
    // A bit hackish: KCMultiWidget inherits from KPageDialog, but it really is
    // a widget...
    setWindowFlags(Qt::Widget);

    connect( this, SIGNAL( finished()), SLOT( dialogClosed()));
    showButton( Ok, false );
    showButton( Cancel, false );
    showButton( Reset, true );     // Reset button
    showButton( User1, false );    // Close button.

    enableButton(Apply, false);
    enableButton(Reset, false);
    enableButton(Default, false);
    enableButton(Help, false);

    connect( this, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)), this, SLOT(slotAboutToShow(KPageWidgetItem*, KPageWidgetItem* )) );
    setInitialSize(QSize(640,480));
    setFaceType( Auto );
    connect( this, SIGNAL(helpClicked()), this, SLOT(slotHelp()) );
    connect( this, SIGNAL(defaultClicked()), this, SLOT(slotDefault()) );
    connect( this, SIGNAL(applyClicked()), this, SLOT(slotApply()) );
    connect( this, SIGNAL(resetClicked()), this, SLOT(slotReset()) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotClose()) );
}

KCMultiWidget::~KCMultiWidget()
{
    qDeleteAll( m_orphanModules );
    delete d;
}

void KCMultiWidget::slotDefault()
{
    currentModule()->defaults();
    clientChanged( true );
    return;
}

void KCMultiWidget::slotReset()
{
    ModuleList::Iterator end = m_modules.end();
    currentModule()->load();
    clientChanged( false );
}

void KCMultiWidget::apply()
{
    QStringList updatedModules;
    foreach( const CreatedModule &it, m_modules )
    {
        KCModuleProxy * m = it.kcm;
        if( m && m->changed() )
        {
            m->save();
            QStringList names = moduleParentComponents[ m ];

            foreach ( const QString &name , names )
            {
                if ( updatedModules.indexOf(name) == -1 )
                    updatedModules.append(name);
            }
        }
    }
    foreach( const QString &it, updatedModules )
    {
        kDebug() << it << " " << ( it );
        emit configCommitted( it.toLatin1() );
    }
    emit configCommitted();
}

void KCMultiWidget::slotApply()
{
    apply();
}

void KCMultiWidget::slotOk()
{
    emit okClicked();
    apply();
    accept();
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

// Close button
void KCMultiWidget::slotClose() {
    emit close();
}

void KCMultiWidget::clientChanged(bool state)
{
    kDebug( 710 ) << state;
    foreach( const CreatedModule &it, m_modules )
        if( it.kcm->changed() ) {
            enableButton( Apply, true );
            enableButton( Reset, true);
            return;
        }
    enableButton( Apply, false );
    enableButton( Reset, false);
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
    KCModuleProxy * module;
    QScrollArea * moduleScrollArea;
    if( m_orphanModules.contains( moduleinfo.service() ) )
    {
        // the KCModule already exists - it was removed from the dialog in
        // removeAllModules
        module = m_orphanModules[ moduleinfo.service() ];
        m_orphanModules.remove( moduleinfo.service() );
        kDebug( 710 ) << "Use KCModule from the list of orphans for " <<
            moduleinfo.moduleName() << ": " << module << endl;

        if( module->changed() ) {
            clientChanged( true );
        }

        moduleScrollArea = static_cast<QScrollArea*>( module->parentWidget() );
    }
    else
    {
        moduleScrollArea = new QScrollArea( this );
        module = new KCModuleProxy( moduleinfo, moduleScrollArea );
        moduleScrollArea->setWidget( module );
        moduleScrollArea->setWidgetResizable( true );
        moduleScrollArea->setFrameStyle( QFrame::NoFrame );
        moduleScrollArea->viewport()->setAutoFillBackground(false);
        module->setAutoFillBackground(false);
        QStringList parentComponents = moduleinfo.service()->property(
                "X-KDE-System-Settings-Parent-Category" ).toStringList();
        moduleParentComponents.insert( module,
                parentComponents );

        connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
    }

    CreatedModule cm;
    cm.kcm = module;
    cm.service = moduleinfo.service();
    cm.adminmode = false;
    cm.buttons = module->buttons();
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
    KPageWidgetItem* page = addPage(moduleScrollArea, moduleinfo.moduleName());
    page->setIcon( KIcon(moduleinfo.icon()) );
    page->setHeader(moduleinfo.comment());
}

KCModuleProxy* KCMultiWidget::currentModule() 
{
    KPageWidgetItem *pageWidget = currentPage();
    if ( pageWidget == 0 )
        return 0;

    QScrollArea *scrollArea = qobject_cast<QScrollArea*>( pageWidget->widget() );
    KCModuleProxy *module = qobject_cast<KCModuleProxy*>( scrollArea->widget() );

    return module;
}

void KCMultiWidget::applyOrRevert(KCModuleProxy * module)
{
    if( !module || !module->changed() )
        return;

    int res = KMessageBox::warningYesNo(this,
            i18n("There are unsaved changes in the active module.\n"
                "Do you want to apply the changes or discard them?"),
            i18n("Unsaved Changes"),
            KStandardGuiItem::apply(),
            KStandardGuiItem::discard());
    if (res == KMessageBox::Yes) {
        slotApply();
    } else {
        module->load();
        clientChanged( false );
    }
}

void KCMultiWidget::slotAboutToShow(KPageWidgetItem* current, KPageWidgetItem* before)
 {
    if( before != 0 ) {
        QScrollArea *scrollArea = qobject_cast<QScrollArea*>( before->widget() );
        KCModuleProxy *module = qobject_cast<KCModuleProxy*>( scrollArea->widget() );
        applyOrRevert( module );
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
            showButton(User2, it.adminmode);
            buttons = it.buttons;
            found = true;
        }
    }
    if (!found) {
        buttons = module->buttons();
    }
    //Q_ASSERT(found);

    showButton(Apply, buttons & KCModule::Apply);
    showButton(Reset, buttons & KCModule::Apply);

    // Close button. No Apply button implies a Close button.
    showButton(User1, (buttons & KCModule::Apply)==0);

    enableButton( KDialog::Help, buttons & KCModule::Help );
    enableButton( KDialog::Default, buttons & KCModule::Default );

    disconnect( this, SIGNAL(user3Clicked()), 0, 0 );

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

void KCMultiWidget::rootExit()
{
    enableButton( User2, true);
}

void KCMultiWidget::disableRModeButton()
{
    enableButton( User2, false );
    connect ( currentModule(), SIGNAL( childClosed() ), SLOT( rootExit() ) );
}

void KCMultiWidget::slotCancel() {
    dialogClosed();
}

void KCMultiWidget::dialogClosed()
{
    applyOrRevert( currentModule() );
}

#include "kcmultiwidget.moc"
