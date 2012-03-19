/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Aaron Lewis <the.warl0ck.1989@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */
#include "mon.h"
#include <sys/vfs.h>
#include <iostream>

const char *unit_labels[] = { "B" , "KiB" , "MiB" , "GiB" , "TB" , "PB" , "EB" };

EXPORT_RAZOR_PANEL_PLUGIN_CPP (FSMon)

FSMon::FSMon(const RazorPanelPluginStartInfo* startInfo, QWidget* parent):
	RazorPanelPlugin (startInfo , parent)
{
	setObjectName ("fsmon");
	directoryPath = settings().value ("path" , "/").toString();

	label = new QLabel (this);
	label->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	layout()->addWidget (label);

	rebuild ( directoryPath );

	watcher.addPath (directoryPath);
	connect (&watcher , SIGNAL (directoryChanged (QString) ) , SLOT ( rebuild (QString) ) );
}

FSMon::~FSMon()
{
}

void FSMon::showConfigureDialog()
{
	const QString & dir = QFileDialog::getExistingDirectory(this, tr("Select path to spy on"),
                                                 directoryPath,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
	if ( ! dir.isEmpty () && directoryPath != dir )
	{
		watcher.removePath (directoryPath);
		watcher.addPath (dir);

		directoryPath = dir;
		rebuild (directoryPath);

		settings().setValue ("path" , directoryPath);
	}
}

void FSMon::rebuild( const QString & path )
{
	struct statfs buf;

	if ( ! statfs(path.toStdString().c_str() , &buf) )
	{
		int i = 0 , maxlen = sizeof (unit_labels) / sizeof (char*);
		double avail_size = buf.f_bavail * buf.f_bsize , total_size = buf.f_blocks * buf.f_bsize;

		int per = 100;
		if ( total_size != 0 )
			per = 100 * (double)avail_size / (double)total_size;

		for ( ; i < maxlen ; ++ i )
		{
			if ( avail_size > 1024 )
				avail_size /= 1024;
			else
				break;
		}

		char str[21];
		snprintf ( str , 20 , "%.2f %s" , avail_size , unit_labels[i]);
		label->setText (str);
		if ( per <= 10 )
			label->setStyleSheet ("color: red");
		else
			label->setStyleSheet ("");
	}

}
