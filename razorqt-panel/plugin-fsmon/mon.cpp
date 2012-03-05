#include "mon.h"
#include <sys/vfs.h>
#include <iostream>
		
const char *unit_labels[] = { "B" , "KiB" , "MiB" , "GiB" , "TB" , "PB" , "EB" };

EXPORT_RAZOR_PANEL_PLUGIN_CPP (FSMon)

FSMon::FSMon(const RazorPanelPluginStartInfo* startInfo, QWidget* parent):
	RazorPanelPlugin (startInfo , parent),
	mSettings (new RazorSettings ("razor-panel/fsmon" , this ))
{
	directoryPath = mSettings->value ("monitor/path" , "/").toString();

	label = new QLabel (this);
	label->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	layout()->addWidget (label);

	rebuild ( directoryPath );

	watcher.addPath (directoryPath);
	connect (&watcher , SIGNAL (directoryChanged (QString) ) , SLOT ( rebuild (QString) ) );
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
			per = 100 - 100 * (double)avail_size / (double)total_size;

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
