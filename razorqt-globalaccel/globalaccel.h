#include <QProcess>
#include <QSettings>
#include <QMap>
#include <QKeySequence>
#include <QObject>
#include <QDebug>
#include <iostream>

#include <razorqxt/qxtglobalshortcut.h>

class GlobalAccel: public QObject
{
    Q_OBJECT

public:
	~GlobalAccel() {};
    GlobalAccel ();

	void bindDefault ();

private:
    QMap<QKeySequence,QString> mapping;

private slots:
    void launchApp ();
};
