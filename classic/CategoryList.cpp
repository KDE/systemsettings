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
#include "systemsettings_classic_debug.h"
#include "MenuItem.h"

#include <QFile>
#include <QModelIndex>
#include <QTextStream>
#include <QIcon>
#include <QBuffer>

#include <KHTMLPart>

#include <KIconLoader>
#include <QUrl>
#include <QHBoxLayout>
#include <KHTMLView>
#include <KCModuleInfo>
#include <QStandardPaths>
#include <QApplication>

static const char kcc_infotext[]= I18N_NOOP("System Settings");
static const char title_infotext[]= I18N_NOOP("Configure your system");
static const char intro_infotext[]= I18N_NOOP("Welcome to \"System Settings\", "
    "a central place to configure your computer system.");

class CategoryList::Private {
public:
    Private() {}

    KHTMLPart * categoryView = nullptr;
    QModelIndex categoryMenu;
    QAbstractItemModel * itemModel = nullptr;
    QMap<QString, QModelIndex> itemMap;
};

CategoryList::CategoryList( QWidget *parent, QAbstractItemModel *model )
    : QWidget(parent), d( new Private() )
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    setMinimumSize( 400, 400 );
    d->itemModel = model;

    // set what's this help
    this->setWhatsThis( i18n( intro_infotext ) );
    d->categoryView = new KHTMLPart( this );
    mainLayout->addWidget(d->categoryView->view());
    d->categoryView->view()->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    d->categoryView->widget()->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    connect( d->categoryView->browserExtension(),
             SIGNAL( openUrlRequest( const QUrl&,
                                     const KParts::OpenUrlArguments&,
                                     const KParts::BrowserArguments& ) ),
             this, SLOT(slotModuleLinkClicked(QUrl)) );
}

CategoryList::~CategoryList()
{
    delete d;
}

void CategoryList::updatePixmap()
{
    QString content;
    QString moduleName;
    d->itemMap.clear();

    const QString templatePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("systemsettings/classic/main.html") );
    QFile templateFile( templatePath );
    templateFile.open( QIODevice::ReadOnly );
    QTextStream templateText( &templateFile );
    QString templateString = templateText.readAll();
    templateString = templateString.arg( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/infopage/kde_infopage.css") ) );
    if ( qApp->layoutDirection() == Qt::RightToLeft ) {
        templateString = templateString.arg( QStringLiteral("@import \"%1\";") ).arg( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/infopage/kde_infopage_rtl.css") ) );
    } else {
        templateString = templateString.arg( QString() );
    }
    templateString = templateString.arg( i18n( kcc_infotext ) );
    templateString = templateString.arg( i18n( title_infotext ) );
    templateString = templateString.arg( i18n( intro_infotext ) );
    if ( d->categoryMenu.isValid() ) {
        moduleName = d->itemModel->data( d->categoryMenu, Qt::DisplayRole ).toString();
    }
    content += QLatin1String("<div id=\"tableTitle\">") + moduleName + QLatin1String("</div>");
    content += QStringLiteral("<table class=\"kc_table\">\n");
    for( int done = 0;  d->itemModel->rowCount( d->categoryMenu ) > done; ++done ) {
        QModelIndex childIndex = d->itemModel->index( done, 0, d->categoryMenu );
        MenuItem *childItem = d->itemModel->data( childIndex, Qt::UserRole ).value<MenuItem*>();
        const QString url = QLatin1String("kcm:///") + childItem->item().fileName();
        QUrl link(url);
        const QString szLink = QLatin1String("<a href=\"") + link.url() + QLatin1String("\" >");
        content += QLatin1String("<tr><td class=\"kc_leftcol\">") + szLink + QLatin1String("<img src=\"%1\" width=\"24\" height=\"24\"></a></td><td class=\"kc_middlecol\">");
        const QString szName = childItem->name();
        const QString szComment = childItem->service()->comment();
        content += szLink + szName + QLatin1String("</a></td><td class=\"kc_rightcol\">") + szLink + szComment + QLatin1String("</a>");

        //passing just the path is insufficient as some icon sets (breeze) only provide SVGs
        //instead pass data inline

        QIcon icon = QIcon::fromTheme(childItem->service()->icon());
        QImage image(icon.pixmap(24).toImage()); //icons are hardcoded to size 24 above
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        image.save(&buffer, "PNG"); // writes the image in PNG format inside the buffer
        QString iconBase64 = QString::fromLatin1(byteArray.toBase64().data());

        content = content.arg(QLatin1String("data:image/png;base64,") + iconBase64);


        d->itemMap.insert( link.url(), childIndex );
        content += QStringLiteral("</td></tr>\n");
    }
    content += QStringLiteral("</table>");
    d->categoryView->begin( QUrl::fromLocalFile( templatePath ) );
    d->categoryView->write( templateString.arg( content ) );
    d->categoryView->end();
}

void CategoryList::changeModule( const QModelIndex &newItem )
{
    d->categoryMenu = newItem;
    updatePixmap();
}

void CategoryList::slotModuleLinkClicked( const QUrl& moduleName )
{
    QModelIndex module = d->itemMap.value( moduleName.url() );
    qCDebug(SYSTEMSETTINGS_CLASSIC_LOG) << "Link name: " << moduleName.url();
    emit moduleSelected( module );
}

#include "moc_CategoryList.cpp"
