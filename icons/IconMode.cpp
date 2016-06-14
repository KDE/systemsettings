/**************************************************************************
 * Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                 *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/

#include "IconMode.h"
#include "CategoryDrawer.h"
#include "CategorizedView.h"

#include "MenuItem.h"
#include "MenuModel.h"
#include "ModuleView.h"
#include "MenuProxyModel.h"

#include <QStackedWidget>
#include <QAction>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QLabel>
#include <QDebug>
#include <qfontdatabase.h>

#include <KAboutData>
#include <KStandardAction>
#include <KFileItemDelegate>
#include <KLocalizedString>
#include <KWidgetItemDelegate>

K_PLUGIN_FACTORY( IconModeFactory, registerPlugin<IconMode>(); )

class DaveDelegate : public KWidgetItemDelegate
{
public:
    explicit DaveDelegate(QAbstractItemView* parent = 0);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QList<QWidget *> createItemWidgets(const QModelIndex & index) const override;
    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem & option, const QPersistentModelIndex & index) const override;

};

DaveDelegate::DaveDelegate(QAbstractItemView* parent):
    KWidgetItemDelegate(parent)
{
}

QSize DaveDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QRect maxSize(0, 0, 200, 300);

    QFontMetrics metrics = option.fontMetrics;
    QRect titleRect = metrics.boundingRect(maxSize, Qt::AlignTop | Qt::TextWordWrap, index.data(Qt::DisplayRole).toString());

    QString subItemsText;
    for(int i=0; i< index.model()->rowCount(index); i++) {
        if (! subItemsText.isEmpty()) {
            subItemsText += ", "; // TODO i18n/locale
        }
        subItemsText += index.child(i,0).data().toString();
    }

    QFontMetrics tooltipFontMetrics(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    QRect subTextRect = tooltipFontMetrics.boundingRect(maxSize, Qt::AlignTop | Qt::TextWordWrap, subItemsText);

    QSize size(maxSize.width(), titleRect.height() + subTextRect.height());

    return size + QSize(32 + 8 + 8, 10);
}

QList<QWidget *> DaveDelegate::createItemWidgets(const QModelIndex & index) const
{
    auto label = new QLabel();
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    setBlockedEventTypes(label, QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease);

    QString subItemsText;
    const int subItemsCount = index.model()->rowCount(index);
    if (subItemsCount > 1) {
        for (int i = 0; i < subItemsCount; ++i) {
            if (!subItemsText.isEmpty()) {
                subItemsText += ", "; // TODO i18n/locale
            }
            subItemsText += "<a href=\""+QString::number(i) + "\" style=\"text-decoration:none\">";
            subItemsText += index.child(i,0).data().toString();
            subItemsText += "</a>";
        }
    }
    label->setText(subItemsText);

    QPersistentModelIndex persistentIndex(index);
    connect(label, &QLabel::linkActivated, [=](const QString &link) {
        bool ok;
        int childRow = link.toInt(&ok);
        if (!ok) {
            return;
        }
        itemView()->activated(persistentIndex.child(childRow, 0));
    });

    return QList<QWidget* >() << label;
}

void DaveDelegate::updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const
{
    Q_ASSERT(widgets.size() == 1);
    QLabel* label = qobject_cast<QLabel*>(widgets.first());

    label->setGeometry(QRect(44,20,option.rect.width()-48, option.rect.height()-24));
}


void DaveDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
//     IconSize(KIconLoader::Dialog)
    auto style = qApp->style();
    painter->save();

    style->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    QRect innerRect = option.rect.adjusted(4,4,-4,-4);

    painter->drawPixmap(innerRect.topLeft(), index.data(Qt::DecorationRole).value<QIcon>().pixmap(32,32));

    const QRect textArea = innerRect.adjusted(32 + 8, 0, 0, 0);

    QFont titleFont = option.font;
    titleFont.setBold(true);
    painter->setFont(titleFont);

    QRect titleRectBounds;
    painter->drawText(textArea, Qt::AlignTop | Qt::TextWordWrap, index.data(Qt::DisplayRole).toString(), &titleRectBounds);

//     QString subItemsText;
//     for(int i=0; i< index.model()->rowCount(index); i++) {
//         if (! subItemsText.isEmpty()) {
//             subItemsText += ", "; // TODO i18n/locale
//         }
//         subItemsText += index.child(i,0).data().toString();
//     }

//     const QRect toolTipTextArea = textArea.adjusted(0, titleRectBounds.height(), 0,0);
//     auto tooltipFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
//     painter->setFont(tooltipFont);
//     painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
//     painter->drawText(toolTipTextArea, subItemsText);

    painter->restore();
}

class IconMode::Private {
public:
    Private() : categoryDrawer( 0 ),  categoryView( 0 ), moduleView( 0 ) {}
    virtual ~Private() {
        delete aboutIcon;
    }

    KCategoryDrawer * categoryDrawer;
    KCategorizedView * categoryView;
    QStackedWidget * mainWidget;
    MenuProxyModel * proxyModel;
    KAboutData * aboutIcon;
    ModuleView * moduleView;
    QAction * backAction;
};

IconMode::IconMode( QObject *parent, const QVariantList& )
    : BaseMode( parent )
    , d( new Private() )
{
    d->aboutIcon = new KAboutData( "IconView", i18n( "Icon View" ),
                                 "1.0", i18n( "Provides a categorized icons view of control modules." ),
                                 KAboutLicense::GPL, i18n( "(c) 2009, Ben Cooksley" ) );
    d->aboutIcon->addAuthor( i18n( "Ben Cooksley" ), i18n( "Author" ), "bcooksley@kde.org" );
    d->aboutIcon->addAuthor( i18n( "Mathias Soeken" ), i18n( "Developer" ), "msoeken@informatik.uni-bremen.de" );
    d->aboutIcon->setProgramIconName( "view-list-icons" );

    d->backAction = KStandardAction::back( this, SLOT(backToOverview()), this );
    d->backAction->setText( i18n( "Overview" ) );
    d->backAction->setToolTip( i18n("Keyboard Shortcut: %1", d->backAction->shortcut().toString( QKeySequence::NativeText )) );
    d->backAction->setEnabled( false );
    actionsList() << d->backAction;
}

IconMode::~IconMode()
{
    delete d;
}

KAboutData * IconMode::aboutData()
{
    return d->aboutIcon;
}

ModuleView * IconMode::moduleView() const
{
    return d->moduleView;
}

QWidget * IconMode::mainWidget()
{
    if( !d->categoryView ) {
        initWidget();
    }
    return d->mainWidget;
}

QList<QAbstractItemView*> IconMode::views() const
{
    QList<QAbstractItemView*> list;
    list.append( d->categoryView );
    return list;
}

void IconMode::initEvent()
{
    MenuModel * model = new MenuModel( rootItem(), this );
    foreach( MenuItem * child, rootItem()->children() ) {
        model->addException( child );
    }

    d->proxyModel = new MenuProxyModel( this );
    d->proxyModel->setCategorizedModel( true );
    d->proxyModel->setSourceModel( model );
    d->proxyModel->sort( 0 );

    d->mainWidget = new QStackedWidget();
    d->moduleView = new ModuleView( d->mainWidget );
    connect( d->moduleView, SIGNAL(moduleChanged(bool)), this, SLOT(moduleLoaded()) );
    connect( d->moduleView, SIGNAL(closeRequest()), this, SLOT(backToOverview()) );
    d->categoryView = 0;
}

void IconMode::searchChanged( const QString& text )
{
    d->proxyModel->setFilterRegExp( text );
    if ( d->categoryView ) {
        QAbstractItemModel *model = d->categoryView->model();
        const int column = d->categoryView->modelColumn();
        const QModelIndex root = d->categoryView->rootIndex();
        for ( int i = 0; i < model->rowCount(); ++i ) {
            const QModelIndex index = model->index( i, column, root );
            if ( model->flags( index ) & Qt::ItemIsEnabled ) {
                d->categoryView->scrollTo( index );
                break;
            }
        }
    }
}

void IconMode::changeModule( const QModelIndex& activeModule )
{
    d->moduleView->closeModules();
    d->mainWidget->setCurrentWidget( d->moduleView );
    d->moduleView->loadModule( activeModule );
}

void IconMode::moduleLoaded()
{
    d->backAction->setEnabled( true );
    emit changeToolBarItems(BaseMode::NoItems);
}

void IconMode::backToOverview()
{
    if( d->moduleView->resolveChanges() ) {
        d->mainWidget->setCurrentWidget( d->categoryView );
        d->moduleView->closeModules();
        d->backAction->setEnabled( false );
        emit changeToolBarItems( BaseMode::Search | BaseMode::Configure | BaseMode::Quit );
        emit viewChanged( false );
    }
}

void IconMode::initWidget()
{
    // Create the widget
    d->categoryView = new CategorizedView( d->mainWidget );
    d->categoryDrawer = new CategoryDrawer(d->categoryView);

    d->categoryView->setSelectionMode( QAbstractItemView::SingleSelection );
    //PORT QT5 d->categoryView->setSpacing( KDialog::spacingHint() );
    d->categoryView->setCategoryDrawer( d->categoryDrawer );
    d->categoryView->setViewMode( QListView::IconMode );
    d->categoryView->setMouseTracking( true );
//     d->categoryView->viewport()->setAttribute( Qt::WA_Hover );

    auto delegate = new DaveDelegate( d->categoryView );
    d->categoryView->setItemDelegate( delegate );

    d->categoryView->setFrameShape( QFrame::NoFrame );
    d->categoryView->setModel( d->proxyModel );
    connect( d->categoryView, SIGNAL(activated(QModelIndex)),
             this, SLOT(changeModule(QModelIndex)) );

    d->mainWidget->addWidget( d->categoryView );
    d->mainWidget->addWidget( d->moduleView );
    d->mainWidget->setCurrentWidget( d->categoryView );
}

void IconMode::leaveModuleView()
{
    d->moduleView->closeModules(); // We have to force it here
    backToOverview();
}

void IconMode::giveFocus()
{
    d->categoryView->setFocus();
}

#include "IconMode.moc"
