#include "globalaccel.h"
#include <razorqt/razorsettings.h>

GlobalAccel::GlobalAccel ()
{
	RazorSettings *m_shortcutSettings = new RazorSettings("globalaccel", this);
	foreach (const QString & group , m_shortcutSettings->childGroups())
	{
		m_shortcutSettings->beginGroup (group);
		const QString & cmd = m_shortcutSettings->value ("Exec").toString();
		bool enabled = m_shortcutSettings->value ("Enabled").toBool();
		if ( enabled && ! cmd.isEmpty() )
		{
			mapping.insert ( QKeySequence (group) , cmd );
		}
		m_shortcutSettings->endGroup ();
	}

//	mapping.insert ( QKeySequence ("Meta+E") , "kde-open /" );
//	mapping.insert ( QKeySequence ("Meta+F") , "chromium-browser" );
//	mapping.insert ( QKeySequence ("Meta+K") , "xkill" );
//	mapping.insert ( QKeySequence ( Qt::Key_Print ) , "ksnapshot" );
//	mapping.insert ( QKeySequence ( "Meta+O" ) , "amixer sset Master,0 3-" );
//	mapping.insert ( QKeySequence ( "Meta+P" ) , "amixer sset Master,0 3+" );
}

void GlobalAccel::bindDefault ()
{
	foreach (const QKeySequence & keySequence , mapping.keys() )
	{
		qDebug() << "Binding: " << keySequence << mapping.value ( keySequence );

		QxtGlobalShortcut *sc = new QxtGlobalShortcut (this);
		sc->setShortcut ( keySequence );
		connect (sc , SIGNAL(activated()) , SLOT(launchApp()));
	}
}

void GlobalAccel::launchApp ()
{
	QxtGlobalShortcut *sc = qobject_cast<QxtGlobalShortcut*>(sender());
	QProcess::startDetached ( mapping.value (sc->shortcut()) );
}
