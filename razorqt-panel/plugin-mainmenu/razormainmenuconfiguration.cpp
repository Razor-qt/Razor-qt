/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2011 Razor team
 * Authors:
 *   Maciej Płaza <plaza.maciej@gmail.com>
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


#include "razormainmenuconfiguration.h"
#include "razormainmenuspinbox.h"
#include "ui_razormainmenuconfiguration.h"
#include <qtxdg/xdgmenu.h>
#include <QDebug>

#include <QtGui/QFileDialog>

RazorMainMenuConfiguration::RazorMainMenuConfiguration(QSettings &settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RazorMainMenuConfiguration),
    mSettings(settings),
    mOldSettings(settings)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("MainMenuConfigurationWindow");
    ui->setupUi(this);

	loadSettings();

	connect(ui->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->buttons->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(onResetClick()));
    connect(ui->showTextCB, SIGNAL(toggled(bool)), ui->textL, SLOT(setEnabled(bool)));
    connect(ui->showTextCB, SIGNAL(toggled(bool)), ui->textLE, SLOT(setEnabled(bool)));
	connect(ui->chooseMenuFilePB, SIGNAL(clicked()), this, SLOT(chooseMenuFile()));
	connect(ui->sizeSB, SIGNAL(focusOutSignal()), this, SLOT(saveSettings()));
}

RazorMainMenuConfiguration::~RazorMainMenuConfiguration()
{
	delete ui;
}

void RazorMainMenuConfiguration::closeEvent(QCloseEvent *e)
{
	saveSettings();
	QDialog::closeEvent(e);
}

void RazorMainMenuConfiguration::saveSettings()
{
	qDebug() << "Saving main menu plugin settings...";
	mSettings.setValue("text", ui->textLE->text());
	mSettings.setValue("showText", ui->showTextCB->isChecked());
	mSettings.setValue("iconSize", ui->sizeSB->value());
	mSettings.setValue("menu_file", ui->menuFilePathLE->text());
	mSettings.setValue("shortcut", ui->shortcutEd->keySequence().toString());
}

void RazorMainMenuConfiguration::loadSettings()
{
    ui->showTextCB->setChecked(mSettings.value("showText", false).toBool());
	ui->textLE->setText(mSettings.value("text", tr("Start")).toString());
	ui->sizeSB->setValue(mSettings.value("iconSize", 16).toInt());

    QString menuFile = mSettings.value("menu_file", "").toString();
    if (menuFile.isEmpty())
    {
        menuFile = XdgMenu::getMenuFileName();
    }
    ui->menuFilePathLE->setText(menuFile);
	ui->shortcutEd->setKeySequence(mSettings.value("shortcut", "Alt+F1").toString());
}

void RazorMainMenuConfiguration::onResetClick()
{
	mOldSettings.loadToSettings();
	loadSettings();
}

void RazorMainMenuConfiguration::chooseMenuFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose menu file"), "~", tr("Menu files (*.menu)"));
    if (!path.isEmpty())
        ui->menuFilePathLE->setText(path);
}
