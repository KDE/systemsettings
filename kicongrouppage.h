/*
   kicongrouppage.h
  
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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#ifndef KICONGROUPPAGE_H

#define KICONGROUPPAGE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QBoxLayout>
#include <QIcon>
#include <QLabel>
#include <kicon.h>

class KIconGroupPage;
class KIconGroupRow;
class KIconGroupItem;

/* DESIGN
   VBox contains two rows: title and HBOX (maybe a third for a line?
   HBOX contains columns of VBOXes
   Those VBoxes contain two rows: icon and title

 +---------------Box-----------------+
 |                                   |
 | +-------------Box---------------+ |
 | |                               | |
 | | +-----------Box-------------+ | |
 | | |                           | | |
 | | | +--QLabel--+ +--QLabel--+ | | |
 | | | |          | |          | | | |
 | | | |   ICON   | |   ICON   | | | |
 | | | |  LABEL   | |  LABEL   | | | |
 | | | |          | |          | | | |
 | | | +----------+ +----------+ | | |
 | | |                           | | |
 | | +---------------------------+ | |
 | |                               | |
 | +-------------------------------+ |
 |                                   |
 +-----------------------------------+


 */


/**
 * A page of group icons
 **/
class KIconGroupPage : public QFrame {
  Q_OBJECT;

  /**
   * Property for the page name
   **/
  Q_PROPERTY(QString m_pageName READ pageName WRITE setPageName);

 public:

  /**
   * Constructor for KIconGroupPage
   *
   * @param parent The parent widget for the page..
   **/
  explicit KIconGroupPage(QWidget* parent = 0);

  ~KIconGroupPage();

  /**
   * Append a group to the page
   *
   * @param title The title of the group
   * @returns A constant pointer to the group that was added.
   **/
  void appendGroup(QString name);

  /**
   * Append an icon to a group
   *
   * @param group The group name to append the icon to
   * @param icon The icon to use
   * @param label The label to use with the icon
   * @returns A constant pointer to the item that was added.
   **/
  const KIconGroupItem* appendIconToGroup(const QString& name,
                                          const QIcon& icon,
                                          const QString& label);

  /**
   * Set the page name
   *
   * @param name The name to assign.
   **/
  void setPageName(QString name) { m_pageName = name; };

  /**
   * Get the page name
   *
   * @return Returns the page name
   **/
  QString pageName() const { return m_pageName; };

 private:
  void render();

  // Internal values
  QString m_pageName;
  QMap<QString, KIconGroupRow*> m_rows;
  QBoxLayout* m_layout;
};


/**
 * The row of icon groups
 **/
class KIconGroupRow : public QBoxLayout {
  Q_OBJECT;

  /**
   * Property for the icon group name
   **/
  Q_PROPERTY(QString m_groupName READ groupName WRITE setGroupName);

 public:

  /**
   * Constructor for KIconGroupRow
   *
   * @param parent The parent to stick the KIconGroupRow widget into.
   **/
  explicit KIconGroupRow(QWidget* parent = 0);

  /**
   * Constructor for KIconGroupRow
   *
   * @param parent The parent to stick the KIconGroupRow widget into.
   * @param name The name for the icon group row.
   **/
  explicit KIconGroupRow(QString& name, QWidget* parent = 0);

  /**
   * Deconstructor
   **/
  ~KIconGroupRow();

  /**
   * Append an icon
   *
   * @param icon The icon to add
   * @param label The label to add
   * @returns A constant pointer to the item that was just added.
   **/
  const KIconGroupItem* appendIcon( const QIcon& icon, const QString& label );

  /**
   * Set the group name
   *
   * @param name The name to assign
   **/
  void setGroupName(QString name) { m_groupName = name; };

  /**
   * Get the group name
   *
   * @return The group name.
   **/
  QString groupName() const { return m_groupName; };

 private:
  void render();

  QString m_groupName;
  QList<KIconGroupItem*> m_icons;
};


/**
 * KIconGroupItem
 *
 * @notes This is the low-level icon class for the icon groups.
 **/
class KIconGroupItem : public QLabel {
  Q_OBJECT;

 public:

  /**
   * A single icon in the group.
   * @param parent The parent widget
   * @param icon The icon to display
   * @param label The label to display for the icon.
   **/
  explicit KIconGroupItem(QWidget* parent, const QIcon& icon, const QString& label);

  /**
   * Set the parent widget
   *
   * @param parent The parent widget to set
   **/
  void setParent(QWidget* parent);

  /**
   * Set the icon
   *
   * @param icon The icon to add to the group item
   **/
  void setIcon(const QIcon& icon);

  /**
   * Get the icon
   *
   * @return Returns the currently set icon object
   **/
  const QIcon& icon() { return m_icon; };

  /**
   * Set the label
   *
   * @param label The label to set
   **/
  void setLabel(const QString& label);

  /**
   * Fetch the label
   **/
  const QString& label() { return m_text; };

 protected:
  void render();

 private:
 Q_DISABLE_COPY(KIconGroupItem);
 QWidget* m_parent;
 QIcon m_icon;
 QString m_text;
};

#endif // KICONGROUPPAGE_H
