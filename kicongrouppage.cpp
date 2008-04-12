/*
   kicongrouppage.cpp

   Copyright (c) 2007 Michael D. Stemle, Jr. <manchicken@notsosoft.net>

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

#include "kicongrouppage.h"

#include <QtAlgorithms>
#include <QStyle>

/*********** KIconGroupPage **************/
KIconGroupPage::KIconGroupPage(QWidget* parent) : QFrame(parent) {
	render();
}

KIconGroupPage::~KIconGroupPage() {
	// Delete the rows...
	qDeleteAll(m_rows.values().begin(), m_rows.values().end());
	m_rows.clear();
}

const KIconGroupRow* KIconGroupPage::appendGroup(QString name) {
	KIconGroupRow* group = new KIconGroupRow(name, parentWidget());

	m_rows.insert(name, group);

	render();

	return group;
}

const KIconGroupItem* KIconGroupPage::appendIconToGroup(const QString& group,
		const QIcon& icon,
		const QString& label) {
	if (!m_rows.contains(group)) {
		appendGroup(group);
	}

	return m_rows[group]->appendIcon(icon, label);
}

void KIconGroupPage::render() {
	m_layout = new QBoxLayout(QBoxLayout::TopToBottom, parentWidget());
}


/*********** KIconGroupRow **************/
KIconGroupRow::KIconGroupRow(QWidget* parent) : QBoxLayout(QBoxLayout::LeftToRight, parent) {
	render();
}

KIconGroupRow::KIconGroupRow(QString& name, QWidget* parent) : QBoxLayout(QBoxLayout::LeftToRight, parent) {
	setGroupName(name);
	render();
}

KIconGroupRow::~KIconGroupRow() {
	// Let's delete our icons
	qDeleteAll(m_icons.begin(), m_icons.end());
	m_icons.clear();
}

const KIconGroupItem* KIconGroupRow::appendIcon( const QIcon& icon, const QString& label ) {
	KIconGroupItem* item = new KIconGroupItem(parentWidget(),icon,label);
	item->setParent(parentWidget());

	m_icons.append(item);

	return item;
}

void KIconGroupRow::render() {
}


/*********** KIconGroupItem **************/
KIconGroupItem::KIconGroupItem(QWidget* parent,
		const QIcon& icon,
		const QString& label) : QLabel(parent) {
	m_parent = parent;
	m_icon = icon;
	m_text = label; // Lets keep our own copy...

	render();
}

void KIconGroupItem::render() {
	QStyle *stl = 0;

	// If there's no parent, we've got no style, can't draw an icon.
	if (!m_parent) {
		return;
	}

	setPixmap(static_cast<const QPixmap&>(m_icon.pixmap(m_parent->style()->pixelMetric(QStyle::PM_IconViewIconSize))));
	setText(m_text);
}

void KIconGroupItem::setParent(QWidget* parent) {
	m_parent = parent;

	render();
}

void KIconGroupItem::setIcon(const QIcon& icon) {
	m_icon = icon;

	render();
}

void KIconGroupItem::setLabel(const QString& label) {
	m_text = label;

	render();
}

#include "kicongrouppage.moc"
