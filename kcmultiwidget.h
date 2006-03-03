/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#ifndef KCMULTIDIALOG_H
#define KCMULTIDIALOG_H

#include <qptrdict.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kservice.h>

class KCModuleProxy;
class KCModuleInfo;
class KCModule;

/**
 * @short A method that offers a KDialogBase containing arbitrary
 *   Control Modules.
 */
class KUTILS_EXPORT KCMultiWidget : public KDialogBase
{
	Q_OBJECT

public:
	/**
	 * Constructs a new KCMultiWidget
	 *
	 * @param parent The parent widget
	 * @param name The widget name
	 * @param modal If you pass true here, the dialog will be modal
	 **/
	KCMultiWidget( QWidget *parent=0, const char *name=0, bool modal=false );

	/**
	 * Construct a personalized KCMultiWidget.
	 *
	 * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or
	 *		IconList.
	 * @param parent Parent of the dialog.
	 * @param name Dialog name (for internal use only).
	 * @param modal Controls dialog modality. If @p false, the rest of the
	 *		program interface (example: other dialogs) is accessible while
	 *		the dialog is open.
	 */
	KCMultiWidget( int dialogFace, QWidget * parent = 0, 
			const char * name = 0, bool modal = false );


	/**
	 * Destructor
	 **/
   virtual ~KCMultiWidget();

	/**
	 * Add a module.
	 *
	 * @param module Specify the name of the module that is to be added
	 *			   to the list of modules the dialog will show.
	 *
	 * @param withfallback Try harder to load the module. Might result
	 *					 in the module appearing outside the dialog.
	 **/
	void addModule(const QString& module, bool withfallback=true);

	/**
	 * Add a module.
	 *
	 * @param moduleinfo Pass a KCModuleInfo object which will be
	 *				   used for creating the module. It will be added
	 *				   to the list of modules the dialog will show.
	 *
	 * @param parentmodulenames The names of the modules that should appear as
	 *						  parents in the TreeList. Look at the
	 *						  KDialogBase::addPage documentation for more info
	 *						  on this.
	 *
	 * @param withfallback Try harder to load the module. Might result
	 *					 in the module appearing outside the dialog.
	 **/
	void addModule(const KCModuleInfo& moduleinfo, QStringList
			parentmodulenames = QStringList(), bool withfallback=false);

	/**
	 * @return the current module that is being shown.
	 */
	KCModuleProxy * currentModule();
	
signals:
	/**
	 * Emitted after all KCModules have been told to save their configuration.
	 *
	 * The applyClicked and okClicked signals are emitted before the
	 * configuration is saved.
	 */
	void configCommitted();

	/**
	 * Emitted after the KCModules have been told to save their configuration.
	 * It is emitted once for every instance the KCMs that were changed belong
	 * to.
	 *
	 * You can make use of this if you have more than one component in your
	 * application. instanceName tells you the instance that has to reload its
	 * configuration.
	 *
	 * The applyClicked and okClicked signals are emitted before the
	 * configuration is saved.
	 *
	 * @param instanceName The name of the instance that needs to reload its
	 *					 configuration.
	 */
	void configCommitted( const QCString & instanceName );

	/**
	 * Emitted right before a module is shown.
	 * Usefull for updating caption, help, etc
	 *
	 * @param moduleinfo The module that is about to be shown.
	 * @sense 3.4
	 */
	void aboutToShow( KCModuleProxy *moduleProxy );

protected slots:
	/**
	 * This slot is called when the user presses the "Default" Button.
	 * You can reimplement it if needed.
	 *
	 * @note Make sure you call the original implementation.
	 **/
	virtual void slotDefault();

	/**
	 * This slot is called when the user presses the "Reset" Button.
	 * You can reimplement it if needed.
	 *
	 * @note Make sure you call the original implementation.
	 */
	virtual void slotUser1();

	/**
	 * This slot is called when the user presses the "Apply" Button.
	 * You can reimplement it if needed.
	 *
	 * @note Make sure you call the original implementation.
	 **/
	virtual void slotApply();

	/**
	 * This slot is called when the user presses the "OK" Button.
	 * You can reimplement it if needed.
	 *
	 * @note Make sure you call the original implementation.
	 **/
	virtual void slotOk();

	/**
	 * This slot is called when the user presses the "Help" Button.
	 * It reads the DocPath field of the currently selected KControl
	 * module's .desktop file to find the path to the documentation,
	 * which it then attempts to load.
	 * 
	 * You can reimplement this slot if needed.
	 *
	 * @note Make sure you call the original implementation.
	 **/
	virtual void slotHelp();

private slots:

	void slotAboutToShow(QWidget *);

	void clientChanged(bool state);

	/**
	 * Called when entering root mode, and disables
	 * the Admin Mode button such that the user doesn't do it
	 * twice.
	 *
	 * @since 3.4
	 */
	void disableRModeButton();

	/**
	 * Called when the current module exits from root 
	 * mode. Enables the Administrator Mode button, again.
	 *
	 * @since 3.4
	 */
	void rootExit();

	/**
	 *
	 * Called when the dialog is hidden. It unregisters the modules,
	 * such that they don't hinder the same modules to be opened in 
	 * another application.
	 *
	 * @since 3.4
	 */
	void dialogClosed();

private:

	void applyOrRevert(KCModuleProxy * module);

	void init();
	void apply();

	struct CreatedModule
	{
		KCModuleProxy * kcm;
		KService::Ptr service;
		/* KDE 4 Move to Private class */
	};
	typedef QValueList<CreatedModule> ModuleList;
	ModuleList m_modules;

	typedef QMap<KService::Ptr, KCModuleProxy*> OrphanMap;
	OrphanMap m_orphanModules;

	QPtrDict<QStringList> moduleParentComponents;
	QString _docPath;
	int dialogface;

	class KCMultiWidgetPrivate;
	KCMultiWidgetPrivate *d;
};

#endif //KCMULTIDIALOG_H

