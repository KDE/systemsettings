/*
 Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>
 Copyright (c) 2009 Ben Cooksley <bcooksley@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "CategoryList.h"

#include "MenuItem.h"

#include <QFile>
#include <QModelIndex>
#include <QTextStream>

#include <KDebug>
#include <KLocale>
#include <KCursor>
#include <KHTMLPart>
#include <KHTMLView>
#include <KApplication>
#include <KCModuleInfo>
#include <KStandardDirs>
#include <KGlobalSettings>

static const char kcc_infotext[]= I18N_NOOP("System Settings");
static const char title_infotext[]= I18N_NOOP("Configure your system");
static const char intro_infotext[]= I18N_NOOP("Welcome to \"System Settings\", "
    "a central place to configure your computer system.");

class CategoryList::Private {
public:
    Private() {}

    KHTMLPart * categoryView;
    QModelIndex categoryMenu;
    QAbstractItemModel * itemModel;
    QMap<QString, QModelIndex> itemMap;
};

CategoryList::CategoryList( QWidget *parent, QAbstractItemModel *model )
    : KHBox(parent), d( new Private() )
{
    setMinimumSize( 400, 400 );
    d->itemModel = model;

    // set what's this help
    this->setWhatsThis( i18n( intro_infotext ) );
    d->categoryView = new KHTMLPart( this );
    d->categoryView->view()->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    d->categoryView->widget()->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    connect( d->categoryView->browserExtension(),
             SIGNAL( openUrlRequest( const KUrl&,
                                     const KParts::OpenUrlArguments&,
                                     const KParts::BrowserArguments& ) ),
             this, SLOT(slotModuleLinkClicked(KUrl)) );
}

CategoryList::~CategoryList()
{
    delete d;
}

void CategoryList::updatePixmap()
{
    QString content;
    QString moduleName;
    KIconLoader * iconL = KIconLoader::global();
    d->itemMap.clear();

    const QString templatePath = KStandardDirs::locate( "data", "systemsettings/classic/main.html" );
    QFile templateFile( templatePath );
    templateFile.open( QIODevice::ReadOnly );
    QTextStream templateText( &templateFile );
    QString templateString = templateText.readAll();
    templateString = templateString.arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage.css" ) );
    if ( kapp->layoutDirection() == Qt::RightToLeft ) {
        templateString = templateString.arg( "@import \"%1\";" ).arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
    } else {
        templateString = templateString.arg( QString() );
    }
    templateString = templateString.arg( i18n( kcc_infotext ) );
    templateString = templateString.arg( i18n( title_infotext ) );
    templateString = templateString.arg( i18n( intro_infotext ) );
    if ( d->categoryMenu.isValid() ) {
        moduleName = d->itemModel->data( d->categoryMenu, Qt::DisplayRole ).toString();
    }
    content += "<div id=\"tableTitle\">" + moduleName + "</div>";
    content += "<table class=\"kc_table\">\n";
    for( int done = 0;  d->itemModel->rowCount( d->categoryMenu ) > done; ++done ) {
        QModelIndex childIndex = d->itemModel->index( done, 0, d->categoryMenu );
        MenuItem *childItem = d->itemModel->data( childIndex, Qt::UserRole ).value<MenuItem*>();
        KUrl link( "kcm://" );
        link.setFileName( childItem->item().fileName() );
        kDebug() << childItem->name() << childItem->item().fileName() << link.url();
        const QString szLink = "<a href=\"" + link.url() + "\" >";
        content += "<tr><td class=\"kc_leftcol\">" + szLink + "<img src=\"%1\" width=\"24\" height=\"24\"></a></td><td class=\"kc_middlecol\">";
        const QString szName = childItem->name();
        const QString szComment = childItem->service()->comment();
        content += szLink + szName + "</a></td><td class=\"kc_rightcol\">" + szLink + szComment + "</a>";
        content = content.arg( iconL->iconPath(childItem->service()->icon(), - KIconLoader::SizeSmallMedium ) );
        d->itemMap.insert( link.url(), childIndex );
        content += "</td></tr>\n";
    }
    content += "</table>";
    d->categoryView->begin( KUrl( templatePath ) );
    d->categoryView->write( templateString.arg( content ) );
    d->categoryView->end();
}

void CategoryList::changeModule( QModelIndex newItem )
{
    d->categoryMenu = newItem;
    updatePixmap();
}

void CategoryList::slotModuleLinkClicked( const KUrl& moduleName ) 
{
    QModelIndex module = d->itemMap.value( moduleName.url() );
    kDebug() << "Link name: " + moduleName.url();
    emit moduleSelected( module );
}

#include "CategoryList.moc"
