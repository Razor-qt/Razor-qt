#ifndef SHORTCUTEDITOR_H
#define SHORTCUTEDITOR_H

/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Aaron Lewis <the.warl0ck.1989@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include <QtGui/QTreeWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QItemDelegate>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QCursor>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QMouseEvent>
#include <QSettings>
#include <QAction>

#include "../../libraries/razorqt/razorshortcutbutton.h"
#include <razorqt/razorsettings.h>

#include <QDebug>
///
class ShortCutDelegate : public QItemDelegate
{
	Q_OBJECT

	public:
		ShortCutDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
				const QModelIndex &index) const;

		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
				const QModelIndex &index) const;

		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

		void updateEditorGeometry(QWidget *editor,
				const QStyleOptionViewItem &option, const QModelIndex &index) const;

	private:

		private slots:

};
///

class ShortcutEditor : public QWidget
{
	Q_OBJECT

	public:
		ShortcutEditor(QWidget *parent = 0);
		~ShortcutEditor();

	private:
		QTreeWidget *mTreeWidget;
		QMenu *mMenu;

		/// mapping between key sequence and the item holding it
		QMap<QString,QTreeWidgetItem*> mapping;
		/// load shortcut settings (auto filter duplicate)
		void loadSettings ();

		private slots:
			/// add new group
			void addGroup ();
		/// add new shortcut
		void addEmpty ();
		/// remove current shortcut
		void removeCurrent ();
		/// reset shortcuts
		void resetAll ();

		/// show context menu
		void showMenu ();

		/// shortcut changed , remove old one
		void shortcutChanged (QTreeWidgetItem *item , int col);

	protected:

};

#endif // SHORTCUTEDITOR_H
