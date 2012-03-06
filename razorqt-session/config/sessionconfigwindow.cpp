/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
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
 
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtGui/QStringListModel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>

#include <razorqt/razorsettings.h>
#include <qtxdg/xdgdesktopfile.h>
#include <qtxdg/xdgdirs.h>
#include <qtxdg/xdgicon.h>

#include "sessionconfigwindow.h"
#include "autostartedit.h"
#include "../src/windowmanager.h"


SessionConfigWindow::SessionConfigWindow()
    : QMainWindow(),
      m_restart(false)
{
    setupUi(this);

    // pages
    new QListWidgetItem(XdgIcon::fromTheme("preferences-desktop-display-color"), tr("Basic Settings"), listWidget);
    new QListWidgetItem(XdgIcon::fromTheme("preferences-desktop-filetype-association"), tr("Default Applications"), listWidget);
    new QListWidgetItem(XdgIcon::fromTheme("preferences-desktop-launch-feedback"), tr("Autostart"), listWidget);
    new QListWidgetItem(XdgIcon::fromTheme("preferences-system-session-services"), tr("Environment (Advanced)"), listWidget);
	new QListWidgetItem(XdgIcon::fromTheme("preferences-desktop-keyboard.png"), tr("Keyboard Shortcuts"), listWidget);
    listWidget->setCurrentRow(0);

    m_settings = new RazorSettings("session", this);
    m_shortcutSettings = new RazorSettings("globalaccel", this);
    m_cache = new RazorSettingsCache(m_settings);
    restoreSettings();

    // UI stuff
    connect(findWmButton, SIGNAL(clicked()), this, SLOT(findWmButton_clicked()));
	//
	connect ( shortcutTableWidget , SIGNAL (customContextMenuRequested(QPoint)) , SLOT(popupShortcutEditorMenu(QPoint)));

	shortcutEditorMenu = new QMenu (this);
    QAction *removeCurrentShortcut = new QAction ( tr("Remove") , this );
    connect (removeCurrentShortcut , SIGNAL(triggered()) , SLOT(removeCurrentShortcut()));

	QAction *resetShortcut = new QAction ( tr("Reset") , this );
	connect (resetShortcut , SIGNAL(triggered()) , SLOT(resetShortcuts()));

	QAction *addNewShortcut = new QAction ( tr("Add") , this );
	connect (addNewShortcut , SIGNAL(triggered()) , SLOT(addNewShortcut()));

	shortcutEditorMenu->addAction(addNewShortcut);
    shortcutEditorMenu->addAction(removeCurrentShortcut);
	shortcutEditorMenu->addSeparator();
	shortcutEditorMenu->addAction(resetShortcut);
    //
    connect(terminalButton, SIGNAL(clicked()), this, SLOT(terminalButton_clicked()));
    connect(browserButton, SIGNAL(clicked()), this, SLOT(browserButton_clicked()));
    //
    connect(envAddButton, SIGNAL(clicked()), this, SLOT(envAddButton_clicked()));
    connect(envDeleteButton, SIGNAL(clicked()), this, SLOT(envDeleteButton_clicked()));
    //
    connect(autoStartAddButton, SIGNAL(clicked()), SLOT(autoStartAddButton_clicked()));
    connect(autoStartEditButton, SIGNAL(clicked()), SLOT(autoStartEditButton_clicked()));
    connect(autoStartDeleteButton, SIGNAL(clicked()), SLOT(autoStartDeleteButton_clicked()));
    //
    connect(wmComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setRestart()));
    connect(wmComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(setRestart()));
    connect(panelCheckBox, SIGNAL(clicked()), this, SLOT(setRestart()));
    connect(desktopCheckBox, SIGNAL(clicked()), this, SLOT(setRestart()));
    connect(runnerCheckBox, SIGNAL(clicked()), this, SLOT(setRestart()));
    connect(appswitcherCheckBox, SIGNAL(clicked()), this, SLOT(setRestart()));
    connect(policyKitCheckBox, SIGNAL(clicked()), this, SLOT(setRestart()));
    connect(terminalComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setRestart()));
    connect(terminalComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(setRestart()));
    connect(browserComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setRestart()));
    connect(browserComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(setRestart()));

    connect(buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonsAction(QAbstractButton*)));
}

void SessionConfigWindow::restoreSettings()
{
    // window managers
    QStringList knownWMs;
    foreach (WindowManager i, getWindowManagerList(true))
    {
        knownWMs << i.command;
    }

    QString wm = m_settings->value("windowmanager", "openbox").toString();
    handleCfgComboBox(wmComboBox, knownWMs, wm);

    // modules
    QHash<QString,QCheckBox*> modules;
    modules["razor-panel"] = panelCheckBox;
    modules["razor-desktop"] = desktopCheckBox;
    modules["razor-runner"] = runnerCheckBox;
    modules["razor-appswitcher"] = appswitcherCheckBox;
    modules["razor-policykit-agent"] = policyKitCheckBox;
    modules["razor-globalaccel"] = globalaccelCheckBox;
    
    m_settings->beginGroup("modules");
    foreach(QString i, modules.keys())
    {
        bool enable = m_settings->value(i, true).toBool();
        modules[i]->setChecked(enable);
    }
    m_settings->endGroup();

    // default applications **********************************************
    m_settings->beginGroup("environment");
    QString terminal = m_settings->value("TERM").toString();
    QString browser = m_settings->value("BROWSER").toString();
    m_settings->endGroup();
    
    if (terminal.isEmpty()) terminal = qgetenv("TERM");
    if (browser.isEmpty()) browser = qgetenv("BROWSER");
        
    QStringList knownTerms;
    knownTerms << "qterminal" << "xterm" << "konsole" << "uterm";
    handleCfgComboBox(terminalComboBox, knownTerms, terminal);
    QStringList knownBrowsers;
    knownBrowsers << "chromium-browser" << "firefox" << "arora" << "konqueror" << "opera";
    handleCfgComboBox(browserComboBox, knownBrowsers, browser);

    // XDG autostart *****************************************************
    QAbstractItemModel* oldModel = xdgAutoStartView->model();
    mXdgAutoStartModel = new AutoStartItemModel(xdgAutoStartView);
    xdgAutoStartView->setModel(mXdgAutoStartModel);
    delete oldModel;
    xdgAutoStartView->setExpanded(mXdgAutoStartModel->index(0, 0), true);
    xdgAutoStartView->setExpanded(mXdgAutoStartModel->index(1, 0), true);

    // environment variables (advanced) **********************************
    m_settings->beginGroup("environment");
    QString value;
    envTreeWidget->clear();
    foreach (QString i, m_settings->childKeys())
    {
        value = m_settings->value(i).toString();
        QTreeWidgetItem *item = new QTreeWidgetItem(envTreeWidget, QStringList() << i << value);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        envTreeWidget->addTopLevelItem(item);
    }
    m_settings->endGroup();

	///
	foreach (const QString & shortcutGroup , m_shortcutSettings->childGroups())
	{
		m_shortcutSettings->beginGroup ( shortcutGroup );
		const QString & command = m_shortcutSettings->value ("Exec").toString();
		const QString & comment = m_shortcutSettings->value ("Comment").toString();
		bool enabled = m_shortcutSettings->value ("Enabled").toBool();

		QCheckBox *checkBox = new QCheckBox (this);
		checkBox->setChecked (enabled);

		RazorShortcutButton *pushButton = new RazorShortcutButton (this);
		pushButton->setText ( shortcutGroup );

		existingShortcuts.insert ( shortcutGroup , pushButton );
		connect (pushButton , SIGNAL( keySequenceChanged (QString) ) , this , SLOT (shortcutChanged ( QString ) ) );

		int row = shortcutTableWidget->rowCount ();
		shortcutTableWidget->setRowCount (row + 1);

		shortcutTableWidget->setCellWidget ( row , 0 , checkBox );
		shortcutTableWidget->setItem ( row , 1 , new QTableWidgetItem ( comment ) );
		shortcutTableWidget->setCellWidget ( row , 2 , pushButton );
		shortcutTableWidget->setItem ( row , 3 , new QTableWidgetItem ( command ) );

		m_shortcutSettings->endGroup ();
	}

	shortcutTableWidget->resizeColumnToContents (0);
	shortcutTableWidget->resizeColumnToContents (1);
}

SessionConfigWindow::~SessionConfigWindow()
{
    delete m_cache;
}

void SessionConfigWindow::closeEvent(QCloseEvent * event)
{
    m_settings->clear();

    // basic settings ****************************************************
    
    m_settings->setValue("windowmanager", wmComboBox->currentText());

    m_settings->beginGroup("modules");
    m_settings->setValue("razor-panel", panelCheckBox->isChecked());
    m_settings->setValue("razor-desktop", desktopCheckBox->isChecked());
    m_settings->setValue("razor-runner", runnerCheckBox->isChecked());
    m_settings->setValue("razor-appswitcher", appswitcherCheckBox->isChecked());
    m_settings->setValue("razor-policykit-agent", policyKitCheckBox->isChecked());
    m_settings->setValue("razor-globalaccel", globalaccelCheckBox->isChecked());
    m_settings->endGroup();

    
    // default applications **********************************************
    // see environment section

    // XDG Autostart *****************************************************
    mXdgAutoStartModel->writeChanges();

    // environment variables (advanced) **********************************
    m_settings->beginGroup("environment");
    m_settings->remove("");
    for(int i = 0; i < envTreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = envTreeWidget->topLevelItem(i);
        m_settings->setValue(item->text(0), item->text(1));
    }
    // handle TERM/BROWSER from defautl apps section
    if (!terminalComboBox->currentText().isEmpty())
        m_settings->setValue("TERM", terminalComboBox->currentText());
    if (!browserComboBox->currentText().isEmpty())
        m_settings->setValue("BROWSER", browserComboBox->currentText());
    m_settings->endGroup();

	///
	m_shortcutSettings->clear ();
	for ( int i = 0 ; i < shortcutTableWidget->rowCount() ; ++ i )
	{
		QCheckBox *checkBox = qobject_cast<QCheckBox*> (shortcutTableWidget->cellWidget(i , 0));
		const QString & comment = shortcutTableWidget->item (i , 1)->text ();
		RazorShortcutButton *button = qobject_cast<RazorShortcutButton*> (shortcutTableWidget->cellWidget(i , 2));
		const QString & command = shortcutTableWidget->item (i , 3)->text ();

		if ( comment.isEmpty() && command.isEmpty() )
			continue;

		m_shortcutSettings->beginGroup ( button->text() );
		m_shortcutSettings->setValue ("Exec" , command);
		m_shortcutSettings->setValue ("Enabled" , checkBox->isChecked ());
		m_shortcutSettings->setValue ("Comment" , comment);
		m_shortcutSettings->endGroup ();
	}
    
    if (m_restart) {
        QMessageBox::information(this, tr("Session Restart Required"),
                                tr("You need to restart desktop session (razor-session) "
                                    "to reload settings. Use logout from the main menu."
                                    )
                                );
    }
}

void SessionConfigWindow::shortcutChanged (const QString & keySequence)
{
	if ( keySequence == "None" )
	{
		return;
	}

	// already exists
	if ( existingShortcuts.contains (keySequence) )
	{
		RazorShortcutButton *button = qobject_cast<RazorShortcutButton*> (sender());
		// clear current bindings
		if ( QMessageBox::information (this , tr ("Key combination already in use") , tr("Replace old one ?") , 
					QMessageBox::Yes , QMessageBox::No ) == QMessageBox::Yes )
		{
			existingShortcuts.value (keySequence)->setText ("None");
			existingShortcuts.remove (keySequence);
		}
		// remove old bindings
		else
			button->setText ("None");
	}
}

void SessionConfigWindow::removeCurrentShortcut ()
{
	int row = shortcutTableWidget->currentRow();
	if ( row > -1 )
	{

		RazorShortcutButton *button = qobject_cast<RazorShortcutButton*> (shortcutTableWidget->cellWidget(row , 2));
		const QString & keySequence = button->text ();

		if ( keySequence != "None" )
		{
			existingShortcuts.remove ( keySequence );
			disconnect (button , SIGNAL( keySequenceChanged (QString) ) , this , SLOT (shortcutChanged ( QString ) ) );
		}

		shortcutTableWidget->removeRow(row);

	}

}

void SessionConfigWindow::resetShortcuts ()
{

}

void SessionConfigWindow::addNewShortcut ()
{
	QCheckBox *checkBox = new QCheckBox (this);
	checkBox->setChecked (true);

    RazorShortcutButton *pushButton = new RazorShortcutButton (this);
	pushButton->setText ( "None" );
	connect (pushButton , SIGNAL( keySequenceChanged (QString) ) , this , SLOT (shortcutChanged ( QString ) ) );

	int row = shortcutTableWidget->rowCount ();
	shortcutTableWidget->setRowCount (row + 1);

	shortcutTableWidget->setCellWidget ( row , 0 , checkBox );
	shortcutTableWidget->setItem ( row , 1 , new QTableWidgetItem ("") );
	shortcutTableWidget->setCellWidget ( row , 2 , pushButton );
	shortcutTableWidget->setItem ( row , 3 , new QTableWidgetItem ("") );
}

void SessionConfigWindow::popupShortcutEditorMenu (const QPoint & pos)
{
	Q_UNUSED (pos);
	shortcutEditorMenu->exec (QCursor::pos());
}

void SessionConfigWindow::handleCfgComboBox(QComboBox * cb,
                                            const QStringList &availableValues,
                                            const QString &value
                                           )
{
    QStringList realValues;
    foreach (QString s, availableValues)
    {
        if (findProgram(s))
            realValues << s;
    }
    cb->clear();
    cb->addItems(realValues);

    int ix = cb->findText(value);
    if (ix == -1)
        cb->lineEdit()->setText(value);
    else
        cb->setCurrentIndex(ix);
}

void SessionConfigWindow::updateCfgComboBox(QComboBox * cb,
                                            const QString &prompt
                                           )
{
    QString fname = QFileDialog::getOpenFileName(this, prompt, "/usr/bin/");
    if (fname.isEmpty())
        return;
    
    QFileInfo fi(fname);
    if (!fi.exists() || !fi.isExecutable())
        return;
    
    cb->lineEdit()->setText(fname);
}

void SessionConfigWindow::findWmButton_clicked()
{
    updateCfgComboBox(wmComboBox, tr("Select a window manager"));
    m_restart = true;
}

void SessionConfigWindow::terminalButton_clicked()
{
    updateCfgComboBox(terminalComboBox, tr("Select a terminal emulator"));
    m_restart = true;
}

void SessionConfigWindow::browserButton_clicked()
{
    updateCfgComboBox(browserComboBox, tr("Select a web browser"));
    m_restart = true;
}

void SessionConfigWindow::autoStartAddButton_clicked()
{
    AutoStartEdit edit;
    XdgDesktopFile* file = edit.createXdgFile();
    if (file)
    {
        QModelIndex index = xdgAutoStartView->selectionModel()->currentIndex();
        mXdgAutoStartModel->addEntry(index, file);
    }
}

void SessionConfigWindow::autoStartEditButton_clicked()
{
    AutoStartEdit edit;
    QModelIndex index = xdgAutoStartView->selectionModel()->currentIndex();
    XdgDesktopFile* file = static_cast<XdgDesktopFile*>(index.internalPointer());
    if (!file)
        return;
    if (edit.editXdgFile(file))
        mXdgAutoStartModel->setData(index, 0, Qt::UserRole);
}

void SessionConfigWindow::autoStartDeleteButton_clicked()
{
    QModelIndex index = xdgAutoStartView->selectionModel()->currentIndex();
    mXdgAutoStartModel->removeRow(index.row(), index.parent());
}

void SessionConfigWindow::envAddButton_clicked()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(envTreeWidget, QStringList() << "" << "");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    envTreeWidget->addTopLevelItem(item);
    envTreeWidget->setCurrentItem(item);
    m_restart = true;
}

void SessionConfigWindow::envDeleteButton_clicked()
{
    foreach (QTreeWidgetItem *i, envTreeWidget->selectedItems())
    {
        delete i;
    }
    m_restart = true;
}

void SessionConfigWindow::setRestart()
{
    m_restart = true;
}

void SessionConfigWindow::dialogButtonsAction(QAbstractButton *btn)
{
    if (buttons->buttonRole(btn) == QDialogButtonBox::ResetRole)
    {
        m_cache->loadToSettings();
        restoreSettings();
    }
    else
    {
        close();
    }
}
