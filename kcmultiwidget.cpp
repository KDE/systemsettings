/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>
   Copyright (c) 2004 Frans Englich <frans.erglich.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <qcursor.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krun.h>
#include <kstdguiitem.h>
#include <kuser.h>

#include "kcmoduleloader.h"
#include "kcmoduleproxy.h"
#include "kcmultiwidget.h"
//#include "kcmultidialog.moc"

class KCMultiWidget::KCMultiWidgetPrivate
{
	public:
		KCMultiWidgetPrivate()
			: hasRootKCM( false ), currentModule( 0 )
		{}

		bool hasRootKCM;
		KCModuleProxy* currentModule;
};

 
KCMultiWidget::KCMultiWidget(QWidget *parent, const char *name, bool modal)
	: KDialogBase(IconList, i18n("Configure"), Help | Default |Cancel | Apply |
			Ok | User1 | User2 , Ok, parent, name, modal, true,
			KStdGuiItem::reset(), KStdGuiItem::adminMode())
	, dialogface( IconList ), d( new KCMultiWidgetPrivate )
{
	init();
}

KCMultiWidget::KCMultiWidget( int dialogFace, QWidget * parent, const char * name, bool modal )
	: KDialogBase( dialogFace, "Caption", Help | Default | Cancel | Apply | Ok |
			User1 | User2, Ok, parent, name, modal, true,
			KStdGuiItem::reset(), KStdGuiItem::adminMode())
	, dialogface( dialogFace ), d( new KCMultiWidgetPrivate )
{
	init();
}

inline void KCMultiWidget::init()
{
	connect( this, SIGNAL( finished()), SLOT( dialogClosed()));
	showButton( User1, true );
	showButton( Ok, false );
	showButton( Cancel, false );
	showButton( User2, false );
	enableButton(Apply, false);
	connect(this, SIGNAL(aboutToShowPage(QWidget *)), this, SLOT(slotAboutToShow(QWidget *)));
	setInitialSize(QSize(640,480));
	moduleParentComponents.setAutoDelete( true );
}
#include <kmessagebox.h>

KCMultiWidget::~KCMultiWidget()
{
	{
	OrphanMap::Iterator end2 = m_orphanModules.end();
	for( OrphanMap::Iterator it = m_orphanModules.begin(); it != end2; ++it ){
			}
	}
				
	OrphanMap::Iterator end2 = m_orphanModules.end();
	for( OrphanMap::Iterator it = m_orphanModules.begin(); it != end2; ++it )
		delete ( *it );
}

void KCMultiWidget::slotDefault()
{
	int curPageIndex = activePageIndex();

	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
		if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
		{
		  ( *it ).kcm->defaults();
		  clientChanged( true );
		  return;
		}
}

void KCMultiWidget::slotUser1()
{
	int curPageIndex = activePageIndex();

	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
		if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
		{
			( *it ).kcm->load();
			clientChanged( false );
			return;
		}
}

void KCMultiWidget::apply()
{
	QStringList updatedModules;
	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
	{
		KCModuleProxy * m = ( *it ).kcm;
		if( m && m->changed() )
		{
			m->save();
			QStringList * names = moduleParentComponents[ m ];
			kdDebug() << k_funcinfo << *names << " saved and added to the list" << endl;
			for( QStringList::ConstIterator it = names->begin(); it != names->end(); ++it )
				if( updatedModules.find( *it ) == updatedModules.end() )
					updatedModules.append( *it );
		}
	}
	for( QStringList::const_iterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
	{
		kdDebug() << k_funcinfo << *it << " " << ( *it ).latin1() << endl;
		emit configCommitted( ( *it ).latin1() );
	}
	emit configCommitted();
}

void KCMultiWidget::slotApply()
{
	QPushButton *button = actionButton(Apply);
	if (button)
		button->setFocus();
	emit applyClicked();
	apply();
}


void KCMultiWidget::slotOk()
{
	QPushButton *button = actionButton(Ok);
	if (button)
		button->setFocus();
	emit okClicked();
	apply();
	accept();
}

void KCMultiWidget::slotHelp()
{
	QString docPath;

	int curPageIndex = activePageIndex();
	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
		if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
		{
			docPath = ( *it ).kcm->moduleInfo().docPath();
			break;
		}

	KURL url( KURL("help:/"), docPath );

	if (url.protocol() == "help" || url.protocol() == "man" || url.protocol() == "info") {
		KProcess process;
		process << "khelpcenter"
				<< url.url();
		process.start(KProcess::DontCare);
		process.detach();
	} else {
		new KRun(url);
	}
}

void KCMultiWidget::clientChanged(bool state)
{
	kdDebug( 710 ) << k_funcinfo << state << endl;
	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
		if( ( *it ).kcm->changed() )
		{
			enableButton( Apply, true );
			return;
		}
	enableButton( Apply, false );
}

void KCMultiWidget::addModule(const QString& path, bool withfallback)
{
	QString complete = path;

	if( !path.endsWith( ".desktop" ))
		complete += ".desktop";

	KService::Ptr service = KService::serviceByStorageId( complete );

	addModule( KCModuleInfo( service ), QStringList(), withfallback);
}

void KCMultiWidget::addModule(const KCModuleInfo& moduleinfo,
		QStringList parentmodulenames, bool withfallback)
{
	kdDebug() << "KCMultiWidget::addModule " 
		<< moduleinfo.moduleName() << endl;

	if( !moduleinfo.service() )
		return;

	if ( !kapp->authorizeControlModule( moduleinfo.service()->menuId() ))
			return;

	if( !KCModuleLoader::testModule( moduleinfo ))
			return;

	QFrame* page = 0;
	if (!moduleinfo.service()->noDisplay())
		switch( dialogface )
		{
			case TreeList:
				parentmodulenames += moduleinfo.moduleName();
				page = addHBoxPage( parentmodulenames, moduleinfo.comment(),
						SmallIcon( moduleinfo.icon(),
							IconSize( KIcon::Small ) ) );
				break;
			case Tabbed:
			case IconList:
				page = addHBoxPage( moduleinfo.moduleName(),
						moduleinfo.comment(), DesktopIcon( moduleinfo.icon(),
							KIcon::SizeMedium ) );
				break;
			case Plain:
				page = plainPage();
				( new QHBoxLayout( page ) )->setAutoAdd( true );
				break;
			default:
				kdError( 710 ) << "unsupported dialog face for KCMultiWidget"
					<< endl;
				break;
		}
	if(!page) {
		KCModuleLoader::unloadModule(moduleinfo);
		return;
	}
	KCModuleProxy * module;
	if( m_orphanModules.contains( moduleinfo.service() ) )
	{
		// the KCModule already exists - it was removed from the dialog in
		// removeAllModules
		module = m_orphanModules[ moduleinfo.service() ];
		m_orphanModules.remove( moduleinfo.service() );
		kdDebug( 710 ) << "Use KCModule from the list of orphans for " <<
			moduleinfo.moduleName() << ": " << module << endl;

		module->reparent( page, 0, QPoint( 0, 0 ), true );

		if( module->changed() )
			clientChanged( true );

		if( activePageIndex() == -1 )
			showPage( pageIndex( page ) );
	}
	else
	{
		module = new KCModuleProxy( moduleinfo, withfallback, page );
		QStringList parentComponents = moduleinfo.service()->property(
				"X-KDE-ParentComponents" ).toStringList();
		moduleParentComponents.insert( module,
				new QStringList( parentComponents ) );

		connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

		if( m_modules.count() == 0 )
			aboutToShowPage( page );
	}
	CreatedModule cm;
	cm.kcm = module;
	cm.service = moduleinfo.service();
	m_modules.append( cm );
	if ( moduleinfo.needsRootPrivileges() && 
			!d->hasRootKCM &&
			!KUser().isSuperUser() ) /* If we're embedded, it's true */
	{
		d->hasRootKCM = true;
		showButton( User2, true );
		if( plainPage() ) // returns 0 if we're not a Plain dialog
			slotAboutToShow( page ); // Won't be called otherwise, necessary for adminMode button
	}
}

void KCMultiWidget::removeAllModules()
{
	kdDebug( 710 ) << k_funcinfo << endl;
	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
	{
		kdDebug( 710 ) << "remove 2" << endl;
		KCModuleProxy * kcm = ( *it ).kcm;
		QObject * page = kcm->parent();
		kcm->hide();
		if( page )
		{
			// I hate this
			kcm->reparent( 0, QPoint( 0, 0 ), false );
			delete page;
		}
		m_orphanModules[ ( *it ).service ] = kcm;
		kdDebug( 710 ) << "added KCModule to the list of orphans: " <<
			kcm << endl;
	}
	m_modules.clear();
	// all modules are gone, none can be changed
	clientChanged( false );
}

KCModuleProxy * KCMultiWidget::currentModule() {
	if(d)
		return d->currentModule;
	return NULL;
}

void KCMultiWidget::applyOrRevert(KCModuleProxy * module){
	if( !module || !module->changed() )
		return;
	
	int res = KMessageBox::warningYesNo(this,
				i18n("There are unsaved changes in the active module.\n"
    			 "Do you want to apply the changes or discard the changes?"),
                                          i18n("Unsaved Changes"),
                                          KStdGuiItem::apply(),
                                         KStdGuiItem::discard());
	if (res == KMessageBox::Yes)
		slotApply();
	else{
		module->load();
		clientChanged( false );
	}
}


void KCMultiWidget::slotAboutToShow(QWidget *page)
{
	kdDebug() << k_funcinfo << endl;

	QObject * obj = page->child( 0, "KCModuleProxy" );
	if( ! obj )
		return;

	KCModuleProxy * module = ( KCModuleProxy* )obj->qt_cast( "KCModuleProxy" );
	if( ! module )
		return;

	if( d && d->currentModule )
		applyOrRevert( d->currentModule );
	
	d->currentModule = module;
	emit ( aboutToShow( d->currentModule ) );
	
	enableButton( KDialogBase::Help,
			d->currentModule->buttons() & KCModule::Help );
	enableButton( KDialogBase::Default,
			d->currentModule->buttons() & KCModule::Default );

	disconnect( this, SIGNAL(user2Clicked()), 0, 0 );

	if (d->currentModule->moduleInfo().needsRootPrivileges() &&
			!d->currentModule->rootMode() )
	{ /* Enable the Admin Mode button */
		enableButton( User2, true );
		connect( this, SIGNAL(user2Clicked()), d->currentModule, SLOT( runAsRoot() ));
		connect( this, SIGNAL(user2Clicked()), SLOT( disableRModeButton() ));
	}
	else
		enableButton( User2, false );

}

void KCMultiWidget::rootExit()
{
	enableButton( User2, true);
}

void KCMultiWidget::disableRModeButton()
{
	enableButton( User2, false );
	connect ( d->currentModule, SIGNAL( childClosed() ), SLOT( rootExit() ) );
}

void KCMultiWidget::dialogClosed()
{
	if(d)
		applyOrRevert(d->currentModule);
	
	kdDebug() << k_funcinfo << endl;

	/* If we don't delete them, the DCOP registration stays, and trying to load the KCMs 
	 * in other situations will lead to "module already loaded in Foo," while to the user 
	 * doesn't appear so(the dialog is hidden) */
	ModuleList::Iterator end = m_modules.end();
	for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
		( *it ).kcm->deleteClient();
}


#include "kcmultiwidget.moc"
