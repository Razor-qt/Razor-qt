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

#include "shortcuteditor.h"

///
CommandFinder::CommandFinder(QWidget *parent):
    QWidget(parent),
    m_lineEdit(new QLineEdit(this)),
    m_button(new QPushButton(tr("...") , this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_button);
    // do not whring widgets inside
    layout->setContentsMargins(0, 0, 0, 0);

    connect(m_button , SIGNAL(clicked()) , SLOT(setCommand()));
}

void CommandFinder::setCommand()
{
    const QString &cmd = QFileDialog::getOpenFileName(this , tr("Find a command"));
    if (! cmd.isEmpty())
    {
        m_lineEdit->setText(cmd);
    }
}

///
QWidget *CommandFinderDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    return new CommandFinder(parent);
}

void CommandFinderDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    CommandFinder *cf = static_cast<CommandFinder *>(editor);
    model->setData(index, cf->text(), Qt::EditRole);
}

void CommandFinderDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void CommandFinderDelegate::setEditorData(QWidget *editor,
        const QModelIndex &index) const
{
    CommandFinder *commandFinder = static_cast<CommandFinder *>(editor);
    commandFinder->setText(index.model()->data(index, Qt::EditRole).toString());
}

QSize CommandFinderDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sizeHint = QItemDelegate::sizeHint(option , index);
    sizeHint.setHeight(sizeHint.height() + 15);
    return sizeHint;
    //    return QSize (30 , 30);
}
///

///
QWidget *ShortCutDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &/* option */,
                                        const QModelIndex &/* index */) const
{
    RazorShortcutButton *button = new RazorShortcutButton(parent);
    /// auto trigger click event
    button->click();
    return button;
}

void ShortCutDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    RazorShortcutButton *button = static_cast<RazorShortcutButton *>(editor);
    model->setData(index, button->text(), Qt::EditRole);
}

void ShortCutDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ShortCutDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    RazorShortcutButton *button = static_cast<RazorShortcutButton *>(editor);
    button->setText(index.model()->data(index, Qt::EditRole).toString());
}

QSize ShortCutDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sizeHint = QItemDelegate::sizeHint(option , index);
    sizeHint.setHeight(sizeHint.height() + 15);
    return sizeHint;
    //    return QSize ( 30 , 20 );
}
///

ShortcutEditor::ShortcutEditor(QWidget *parent)
    : QWidget(parent),
      mMenu(new QMenu(this))
{
    setupUi(this);
    noneString = tr("None");

    addNewButton->setIcon(XdgIcon::fromTheme("list-add"));
    removeSelectedButton->setIcon(XdgIcon::fromTheme("edit-delete"));

    /// actions
    QAction *addNewAct = new QAction(addNewButton->icon() , tr("Add Shortcut") , this);
    connect(addNewAct , SIGNAL(triggered()) , SLOT(addEmpty()));
    connect(addNewButton , SIGNAL(clicked()) , SLOT(addEmpty()));

    QAction *removeCurrentAct = new QAction(removeSelectedButton->icon() , tr("Remove") , this);
    connect(removeCurrentAct , SIGNAL(triggered()) , SLOT(removeCurrent()));
    connect(removeSelectedButton , SIGNAL(clicked()) , SLOT(removeCurrent()));

    QAction *addGroupAct = new QAction(tr("New Group") , this);
    connect(addGroupAct , SIGNAL(triggered()) , SLOT(addGroup()));
    connect(addGroupButton , SIGNAL(clicked()) , SLOT(addGroup()));

    QAction *resetChangesAct = new QAction(tr("Reset Changes") , this);
    connect(resetChangesAct , SIGNAL(triggered()) , SLOT(resetChanges()));
    connect(resetButton , SIGNAL(clicked()) , SLOT(resetChanges()));

    connect(closeButton , SIGNAL(clicked()) , SLOT(close()));

    mMenu->addAction(addGroupAct);
    mMenu->addSeparator();
    mMenu->addAction(addNewAct);
    mMenu->addAction(removeCurrentAct);
    mMenu->addSeparator();
    mMenu->addAction(resetChangesAct);
    ///


    /// setup tree
    mTreeWidget->setItemDelegateForColumn(1 , new ShortCutDelegate(this));
    mTreeWidget->setItemDelegateForColumn(2 , new CommandFinderDelegate(this));

    connect(mTreeWidget , SIGNAL(customContextMenuRequested(QPoint)) , SLOT(showMenu()));
    connect(mTreeWidget , SIGNAL(itemChanged(QTreeWidgetItem *, int))
            , SLOT(shortcutChanged(QTreeWidgetItem *, int)));

    connect(mTreeWidget , SIGNAL(itemClicked(QTreeWidgetItem * , int)) , SLOT(editItem(QTreeWidgetItem * , int)));

    loadSettings();
}

void ShortcutEditor::editItem(QTreeWidgetItem *item , int column)
{
    /// don't edit shortcut / command for top level items
    if (NULL != item->parent())
    {
        mTreeWidget->editItem(item , column);
    }
}

void ShortcutEditor::loadSettings()
{
    RazorSettings *mSettings = new RazorSettings("globalkeyshortcuts", this);
    QSet<QString> existingShortcuts;
    QMap<QString, QTreeWidgetItem *> groupItems;

    foreach(const QString & key , mSettings->childGroups())
    {
        mSettings->beginGroup(key);

        const QString &groupName = mSettings->value("Group" , "Programs").toString();
        const QString &cmd = mSettings->value("Exec").toString();
        const QString &descr = mSettings->value("Comment").toString();
        bool enabled = mSettings->value("Enabled").toBool();

        /// ignore empty ones , or conflicting ones
        if (! existingShortcuts.contains(key) &&
                !(descr.isEmpty() && key.isEmpty() && cmd.isEmpty()))
        {
            QTreeWidgetItem *parentItem;
            if (groupItems.contains(groupName))
            {
                parentItem = groupItems.value(groupName);
            }
            else
            {
                parentItem = new QTreeWidgetItem(QStringList() << groupName);
                mTreeWidget->insertTopLevelItem(mTreeWidget->topLevelItemCount() , parentItem);
                mTreeWidget->setFirstItemColumnSpanned(parentItem, true);

                groupItems.insert(groupName , parentItem);
            }

            QTreeWidgetItem *childItem = new QTreeWidgetItem(QStringList() << descr << key << cmd);
            childItem->setFlags(childItem->flags() | Qt::ItemIsEditable);
            childItem->setCheckState(0 , enabled ? Qt::Checked : Qt::Unchecked);

            /// insert into database
            mapping.insert(key , childItem);

            parentItem->addChild(childItem);

            existingShortcuts.insert(key);
        }

        mSettings->endGroup();
    }

    /// add default group
    if (mTreeWidget->topLevelItemCount() == 0)
    {
        addGroup();
    }
    else
    {
        mTreeWidget->expandAll();
    }
    mTreeWidget->resizeColumnToContents(0);
    mTreeWidget->resizeColumnToContents(1);

    delete mSettings;
}

void ShortcutEditor::showMenu()
{
    mMenu->exec(QCursor::pos());
}

void ShortcutEditor::shortcutChanged(QTreeWidgetItem *item , int col)
{
    /// only available for razor button
    if (col != 1)
    {
        return;
    }

    QString sc = item->text(1);

    if (! sc.isEmpty() && sc != noneString)
    {
        if (mapping.contains(sc))
        {
            QTreeWidgetItem *old_item = mapping.value(sc);
            if (item == old_item)
            {
                mapping.insert(sc , item);
                return;
            }

            /// conflict with existing , remove old one
            if (replaceExisting(sc))
            {
                mapping.value(sc)->setText(1 , noneString);
                mapping.insert(sc , item);
            }
            /// cancel
            else
            {
                item->setText(1 , noneString);
            }
        }
        else
        {
            /// insert into database
            mapping.insert(sc , item);
        }
    }

}

void ShortcutEditor::resetChanges()
{
    mTreeWidget->clear();
    loadSettings();
}

void ShortcutEditor::addGroup()
{
    QTreeWidgetItem *groupItem = new QTreeWidgetItem(QStringList() << tr("New Group"));
    groupItem->setFlags(groupItem->flags());
    mTreeWidget->insertTopLevelItem(0 , groupItem);
}

void ShortcutEditor::addEmpty()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << "" << noneString << "");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setCheckState(0 , Qt::Checked);

    QTreeWidgetItem *currentItem = mTreeWidget->currentItem();
    if (! currentItem && mTreeWidget->topLevelItemCount() > 0)
    {
        currentItem = mTreeWidget->topLevelItem(0);
    }

    if (currentItem)
    {
        if (! currentItem->parent())
        {
            currentItem->addChild(item);
        }
        else
        {
            if (mTreeWidget->topLevelItemCount() > 0)
            {
                mTreeWidget->topLevelItem(mTreeWidget->topLevelItemCount() - 1)->addChild(item);
            }
        }
    }

    if (item->parent())
    {
        item->parent()->setExpanded(true);
    }
}

void ShortcutEditor::removeCurrent()
{

    QTreeWidgetItem *item = mTreeWidget->currentItem();

    if (item)
    {
        const QString &text = item->text(0);

        /// removing a group , careful
        if (! item->parent())
        {
            if (item->childCount() > 0)
            {
                if (cancelDeleteGroup(text))
                {
                    return;
                }
            }

            /// kill whole group
            const QModelIndex &idx = mTreeWidget->currentIndex();
            mTreeWidget->model()->removeRow(idx.row());

            return;

        }
        else if (cancelDeleteSingle(text))
        {
            return;
        }

        /// remove shortcut mapping
        mapping.remove(text);
        /// remove single child
        item->parent()->removeChild(item);
    }
}

bool ShortcutEditor::cancelDeleteGroup(const QString& group)
{
    return QMessageBox::No == QMessageBox::question(this,
                    tr("Question"),
                    tr("Delete group: %1? (everything inside will be removed altogether)").arg(group),
                    QMessageBox::Yes , QMessageBox::No);
}

bool ShortcutEditor::cancelDeleteSingle(const QString& item)
{
    return QMessageBox::No == QMessageBox::question(this,
                    tr("Question") ,
                    tr("Delete %1?").arg(item),
                    QMessageBox::Yes, QMessageBox::No);
}

bool ShortcutEditor::replaceExisting(const QString& item)
{
    return QMessageBox::Yes == QMessageBox::question(NULL,
                    tr("Question"),
                    tr("Binding for %1 already exists. Replace old one?").arg(item),
                    QMessageBox::Yes , QMessageBox::No);
}

ShortcutEditor::~ShortcutEditor()
{
    RazorSettings *mSettings = new RazorSettings("globalkeyshortcuts", this);
    mSettings->clear();

    for (int i = 0 ; i < mTreeWidget->topLevelItemCount() ; ++ i)
    {
        QTreeWidgetItem *topItem = mTreeWidget->topLevelItem(i);
        for (int j = 0 ; j < topItem->childCount() ; ++ j)
        {
            // assert ( col < topItem->child(j)->columnCount() == 3 );
            const QString &groupName = topItem->text(0);

            const QString &descr = topItem->child(j)->text(0);
            const QString &shortcut = topItem->child(j)->text(1);
            const QString &cmd = topItem->child(j)->text(2).trimmed();
            bool enabled = topItem->child(j)->checkState(0);

            // ignore empty ones
            if (shortcut == "None" && cmd.isEmpty())
            {
                continue;
            }

            mSettings->beginGroup(shortcut);

            mSettings->setValue("Exec" , cmd);
            mSettings->setValue("Comment" , descr);
            mSettings->setValue("Group" , groupName);
            mSettings->setValue("Enabled" , enabled);

            mSettings->endGroup();
        }
    }

    delete mSettings;

}
