/*
 *   SPDX-FileCopyrightText: 2008 Konstantin Heil <konst.heil@stud.uni-heidelberg.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tooltipmanager.h"

#include "MenuItem.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QRect>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>

#include <KColorScheme>
#include <KConfigGroup>
#include <KDirWatch>
#include <KIconLoader>
#include <KLocalizedString>
#include <KToolTipWidget>

class IconLoaderSingleton
{
public:
    IconLoaderSingleton() = default;

    KIconLoader self;
};

Q_GLOBAL_STATIC(IconLoaderSingleton, privateIconLoaderSelf)

class ToolTipManager::Private
{
public:
    Private()
        : tooltip(nullptr)
        , view(nullptr)
        , timer(nullptr)
        , tooltipsEnabledGlobally(true)
        , delay(QApplication::style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay))
    {
    }

    KToolTipWidget *tooltip = nullptr;
    QWidget *view = nullptr;
    const QAbstractItemModel *model;
    QTimer *timer = nullptr;
    bool tooltipsEnabledGlobally;
    QPersistentModelIndex item;
    QRect itemRect;
    int delay;
    ToolTipPosition toolTipPosition;
};

ToolTipManager::ToolTipManager(const QAbstractItemModel *model, QWidget *parent, ToolTipPosition toolTipPosition)
    : QObject(parent)
    , d(new ToolTipManager::Private)
{
    d->view = parent;
    d->model = model;
    d->toolTipPosition = toolTipPosition;
    d->tooltip = new KToolTipWidget(d->view);
    d->tooltip->setHideDelay(0);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, &QTimer::timeout, this, &ToolTipManager::prepareToolTip);

    d->view->installEventFilter(this);

    // Load the tooltip configuration
    loadSettings();
    // Monitor the configuration file and reload the setting
    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(configFile);
    connect(KDirWatch::self(), &KDirWatch::created, this, &ToolTipManager::slotSettingsChanged);
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &ToolTipManager::slotSettingsChanged);
}

ToolTipManager::~ToolTipManager()
{
    delete d;
}

void ToolTipManager::loadSettings()
{
    // Check if the display of informational tooltip is disabled, Delay = -1
    d->tooltipsEnabledGlobally = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), "PlasmaToolTips").readEntry("Delay", d->delay) > 0;
}

void ToolTipManager::slotSettingsChanged(const QString &file)
{
    if (!file.endsWith(QLatin1String("plasmarc"))) {
        return;
    }

    KSharedConfig::openConfig(QStringLiteral("plasmarc"))->reparseConfiguration();
    loadSettings();
}

void ToolTipManager::setModel(const QAbstractItemModel *model)
{
    d->model = model;
}

const QAbstractItemModel *ToolTipManager::model() const
{
    return d->model;
}

bool ToolTipManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->view) {
        switch (event->type()) {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
            hideToolTip();
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ToolTipManager::requestToolTip(const QModelIndex &index, const QRect &rect)
{
    // only request a tooltip for the name column and when no selection or
    // drag & drop operation is done (indicated by the left mouse button)
    if (!(QApplication::mouseButtons() & Qt::LeftButton)) {
        d->tooltip->hide();

        d->itemRect = rect;
        const QPoint pos = d->view->mapToGlobal(d->itemRect.topLeft());
        d->itemRect.moveTo(pos);
        d->item = index;
        d->timer->start(d->delay);
    } else {
        hideToolTip();
    }
}

void ToolTipManager::hideToolTip()
{
    d->timer->stop();
    d->tooltip->hideLater();
}

void ToolTipManager::prepareToolTip()
{
    // item may have gone away since we're triggered by a timer
    MenuItem *menuItem = d->model->data(d->item, Qt::UserRole).value<MenuItem *>();
    if (menuItem) {
        showToolTip(d->item);
    }
}

void ToolTipManager::showToolTip(const QModelIndex &menuItem)
{
    // Do not show the tooltips if the display of informational tooltips is disabled
    if (!d->tooltipsEnabledGlobally) {
        return;
    }

    if (QApplication::mouseButtons() & Qt::LeftButton) {
        return;
    }

    QWidget *tip = createTipContent(menuItem);
    if (d->toolTipPosition == ToolTipPosition::BottomCenter) {
        d->tooltip->showBelow(d->itemRect, tip, d->view->nativeParentWidget()->windowHandle());
    } else {
        if (qApp->isRightToLeft()) {
            d->tooltip->showAt(d->itemRect.topLeft() - QPoint(d->tooltip->width(), 0), tip, d->view->nativeParentWidget()->windowHandle());
        } else {
            d->tooltip->showAt(d->itemRect.topRight(), tip, d->view->nativeParentWidget()->windowHandle());
        }
    }
    connect(d->tooltip, &KToolTipWidget::hidden, tip, &QObject::deleteLater);
}

QWidget *ToolTipManager::createTipContent(QModelIndex item)
{
    auto loader = KIconLoader::global();
    const QSize dialogIconSize = QSize(loader->currentSize(KIconLoader::Dialog), loader->currentSize(KIconLoader::Dialog));
    const QSize toolbarIconSize = QSize(loader->currentSize(KIconLoader::MainToolbar), loader->currentSize(KIconLoader::MainToolbar));

    QWidget *tipContent = new QWidget();
    QGridLayout *tipLayout = new QGridLayout();
    tipLayout->setAlignment(Qt::AlignLeft);

    QLayout *primaryLine = generateToolTipLine(&item, tipContent, dialogIconSize, true);
    primaryLine->setAlignment(Qt::AlignLeft);
    tipLayout->addLayout(primaryLine, 0, 0, Qt::AlignLeft);

    for (int done = 0; d->model->rowCount(item) > done; done = 1 + done) {
        QModelIndex childItem = d->model->index(done, 0, item);
        QLayout *subLine = generateToolTipLine(&childItem, tipContent, toolbarIconSize, false);
        subLine->setAlignment(Qt::AlignLeft);
        tipLayout->addLayout(subLine, done + 2, 0, Qt::AlignLeft);
    }

    tipLayout->setVerticalSpacing(tipContent->fontMetrics().height() / 3);
    tipContent->setLayout(tipLayout);

    if (d->model->rowCount(item) > 0) {
        QFrame *separatorLine = new QFrame(tipContent);
        separatorLine->setFrameStyle(QFrame::HLine);
        tipLayout->addWidget(separatorLine, 1, 0);
    }

    return tipContent;
}

QLayout *ToolTipManager::generateToolTipLine(QModelIndex *item, QWidget *toolTip, QSize iconSize, bool comment)
{
    // Get MenuItem
    MenuItem *menuItem = d->model->data(*item, Qt::UserRole).value<MenuItem *>();

    QString text = menuItem->name();
    if (comment) {
        text = QStringLiteral("<b>%1</b>").arg(menuItem->name());

        // Generate text
        text += QStringLiteral("<br />");
        if (!menuItem->comment().isEmpty()) {
            text += menuItem->comment();
        } else {
            int childCount = d->model->rowCount(*item);
            text += i18np("<i>Contains 1 item</i>", "<i>Contains %1 items</i>", childCount);
        }
    }
    QLabel *textLabel = new QLabel(toolTip);
    textLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    textLabel->setForegroundRole(QPalette::ToolTipText);
    textLabel->setText(text);

    // Get icon
    QPalette pal = textLabel->palette();
    for (auto state : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        pal.setBrush(state, QPalette::WindowText, pal.toolTipText());
        pal.setBrush(state, QPalette::Window, pal.toolTipBase());
    }

    privateIconLoaderSelf->self.setCustomPalette(pal);

    QIcon icon = KDE::icon(menuItem->iconName(), &privateIconLoaderSelf->self);
    QLabel *iconLabel = new QLabel(toolTip);
    iconLabel->setPixmap(icon.pixmap(iconSize));
    iconLabel->setMaximumSize(iconSize);

    // Generate layout
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(textLabel->fontMetrics().height() / 3);
    layout->setAlignment(Qt::AlignLeft);
    layout->addWidget(iconLabel, Qt::AlignLeft);
    layout->addWidget(textLabel, Qt::AlignLeft);

    return layout;
}
