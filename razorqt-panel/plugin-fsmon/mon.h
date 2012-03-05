#ifndef RAZOR_FSMon_H
#define RAZOR_FSMon_H

#include "../panel/razorpanelplugin.h"

#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include <QFileSystemWatcher>
#include <QProcess>
#include <razorqt/razorsettings.h>

class FSMon : public RazorPanelPlugin
{
    Q_OBJECT
public:
    FSMon(const RazorPanelPluginStartInfo* startInfo, QWidget* parent = 0);
    ~FSMon(){};

	virtual RazorPanelPlugin::Flags flags() const { return HaveConfigDialog ; }

private:
	RazorSettings *mSettings;

private slots:
	void rebuild ( const QString & path );

protected slots:
	void showConfigureDialog ();

private:
	QLabel *label;
	QFileSystemWatcher watcher;
	QString directoryPath;
};

EXPORT_RAZOR_PANEL_PLUGIN_H

#endif
