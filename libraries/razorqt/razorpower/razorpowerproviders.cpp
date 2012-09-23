/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Petr Vanek <petr@scribus.info>
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


#include "razorpowerproviders.h"
#include <QtDBus/QDBusInterface>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include "razorqt/razornotification.h"

#define UPOWER_SERVICE          "org.freedesktop.UPower"
#define UPOWER_PATH             "/org/freedesktop/UPower"
#define UPOWER_INTERFACE        UPOWER_SERVICE

#define CONSOLEKIT_SERVICE      "org.freedesktop.ConsoleKit"
#define CONSOLEKIT_PATH         "/org/freedesktop/ConsoleKit/Manager"
#define CONSOLEKIT_INTERFACE    "org.freedesktop.ConsoleKit.Manager"

#define RAZOR_SERVICE      "org.razorqt.session"
#define RAZOR_PATH         "/RazorSession"
#define RAZOR_INTERFACE    "org.razorqt.session"


#define PROPERTIES_INTERFACE    "org.freedesktop.DBus.Properties"


/************************************************
 Helper func
 ************************************************/
void printDBusMsg(const QDBusMessage &msg)
{
    qWarning() << "** Dbus error **************************";
    qWarning() << "Error name " << msg.errorName();
    qWarning() << "Error msg  " << msg.errorMessage();
    qWarning() << "****************************************";
}


/************************************************
 Helper func
 ************************************************/
bool dbusCall(const QString &service,
              const QString &path,
              const QString &interface,
              const QDBusConnection &connection,
              const QString & method,
              RazorPowerProvider::DbusErrorCheck errorCheck = RazorPowerProvider::CheckDBUS
              )
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid())
    {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        if (errorCheck == RazorPowerProvider::CheckDBUS)
        {
            RazorNotification::notify(
                                    QObject::tr("Power Manager Error"),
                                    QObject::tr("QDBusInterface is invalid")+ "\n\n" + service + " " + path + " " + interface + " " + method,
                                    "razor-logo.png");
        }
        return false;
    }

    QDBusMessage msg = dbus.call(method);

    if (!msg.errorName().isEmpty())
    {
        printDBusMsg(msg);
        if (errorCheck == RazorPowerProvider::CheckDBUS)
        {
            RazorNotification::notify(
                                    QObject::tr("Power Manager Error (D-BUS call)"),
                                    msg.errorName() + "\n\n" + msg.errorMessage(),
                                    "razor-logo.png");
        }
    }

    // If the method no returns value, we believe that it was successful.
    return msg.arguments().isEmpty() ||
           msg.arguments().first().toBool();
}


/************************************************
 Helper func
 ************************************************/
bool dbusGetProperty(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString & property
                    )
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid())
    {
        qWarning() << "dbusGetProperty: QDBusInterface is invalid" << service << path << interface << property;
//        RazorNotification::notify(QObject::tr("Razor Power Manager"),
//                                  "razor-logo.png",
//                                  QObject::tr("Power Manager Error"),
//                                  QObject::tr("QDBusInterface is invalid")+ "\n\n" + service +" " + path +" " + interface +" " + property);

        return false;
    }

    QDBusMessage msg = dbus.call("Get", dbus.interface(), property);

    if (!msg.errorName().isEmpty())
    {
        printDBusMsg(msg);
//        RazorNotification::notify(QObject::tr("Razor Power Manager"),
//                                  "razor-logo.png",
//                                  QObject::tr("Power Manager Error (Get Property)"),
//                                  msg.errorName() + "\n\n" + msg.errorMessage());
    }

    return !msg.arguments().isEmpty() &&
            msg.arguments().first().value<QDBusVariant>().variant().toBool();
}




/************************************************
 RazorPowerProvider
 ************************************************/
RazorPowerProvider::RazorPowerProvider(QObject *parent):
    QObject(parent)
{
}


RazorPowerProvider::~RazorPowerProvider()
{
}



/************************************************
 UPowerProvider
 ************************************************/
UPowerProvider::UPowerProvider(QObject *parent):
    RazorPowerProvider(parent)
{
}


UPowerProvider::~UPowerProvider()
{
}


bool UPowerProvider::canAction(RazorPower::Action action) const
{
    QString command;
    QString property;
    switch (action)
    {
    case RazorPower::PowerHibernate:
        property = "CanHibernate";
        command  = "HibernateAllowed";
        break;

    case RazorPower::PowerSuspend:
        property = "CanSuspend";
        command  = "SuspendAllowed";
        break;

    default:
        return false;
    }

    return  dbusGetProperty(  // Whether the system is able to hibernate.
                UPOWER_SERVICE,
                UPOWER_PATH,
                PROPERTIES_INTERFACE,
                QDBusConnection::systemBus(),
                property
            )
            &&
            dbusCall( // Check if the caller has (or can get) the PolicyKit privilege to call command.
                UPOWER_SERVICE,
                UPOWER_PATH,
                UPOWER_INTERFACE,
                QDBusConnection::systemBus(),
                command,
                // canAction should be always silent because it can freeze
                // g_main_context_iteration Qt event loop in QMessageBox
                // on panel startup if there is no DBUS running.
                RazorPowerProvider::DontCheckDBUS
            );
}


bool UPowerProvider::doAction(RazorPower::Action action)
{
    QString command;

    switch (action)
    {
    case RazorPower::PowerHibernate:
        command = "Hibernate";
        break;

    case RazorPower::PowerSuspend:
        command = "Suspend";
        break;

    default:
        return false;
    }


    return dbusCall(UPOWER_SERVICE,
             UPOWER_PATH,
             UPOWER_INTERFACE,
             QDBusConnection::systemBus(),
             command );
}



/************************************************
 ConsoleKitProvider
 ************************************************/
ConsoleKitProvider::ConsoleKitProvider(QObject *parent):
    RazorPowerProvider(parent)
{
}


ConsoleKitProvider::~ConsoleKitProvider()
{
}


bool ConsoleKitProvider::canAction(RazorPower::Action action) const
{
    QString command;

    switch (action)
    {
    case RazorPower::PowerReboot:
        command = "CanRestart";
        break;

    case RazorPower::PowerShutdown:
        command = "CanStop";
        break;

    default:
        return false;
    }

    return dbusCall(CONSOLEKIT_SERVICE,
                    CONSOLEKIT_PATH,
                    CONSOLEKIT_INTERFACE,
                    QDBusConnection::systemBus(),
                    command,
                    // canAction should be always silent because it can freeze
                    // g_main_context_iteration Qt event loop in QMessageBox
                    // on panel startup if there is no DBUS running.
                    RazorPowerProvider::DontCheckDBUS
                   );
}


bool ConsoleKitProvider::doAction(RazorPower::Action action)
{
    QString command;

    switch (action)
    {
    case RazorPower::PowerReboot:
        command = "Restart";
        break;

    case RazorPower::PowerShutdown:
        command = "Stop";
        break;

    default:
        return false;
    }

    return dbusCall(CONSOLEKIT_SERVICE,
             CONSOLEKIT_PATH,
             CONSOLEKIT_INTERFACE,
             QDBusConnection::systemBus(),
             command
            );
}



/************************************************
  RazorProvider
 ************************************************/
RazorProvider::RazorProvider(QObject *parent):
    RazorPowerProvider(parent)
{
}


RazorProvider::~RazorProvider()
{
}


bool RazorProvider::canAction(RazorPower::Action action) const
{
    switch (action)
    {
        case RazorPower::PowerLogout:
            // there can be case when razo-session does not run
            return dbusCall(RAZOR_SERVICE, RAZOR_PATH, RAZOR_SERVICE,
                            QDBusConnection::sessionBus(), "canLogout",
                            RazorPowerProvider::DontCheckDBUS);
        default:
            return false;
    }
}


bool RazorProvider::doAction(RazorPower::Action action)
{
    QString command;

    switch (action)
    {
    case RazorPower::PowerLogout:
        command = "logout";
        break;

    default:
        return false;
    }

    return dbusCall(RAZOR_SERVICE,
             RAZOR_PATH,
             RAZOR_INTERFACE,
             QDBusConnection::sessionBus(),
             command
            );
}



/************************************************
  HalProvider
 ************************************************/
HalProvider::HalProvider(QObject *parent):
    RazorPowerProvider(parent)
{
}


HalProvider::~HalProvider()
{
}


bool HalProvider::canAction(RazorPower::Action action) const
{
    return false;
}


bool HalProvider::doAction(RazorPower::Action action)
{
    return false;
}


/************************************************
  CustomProvider
 ************************************************/
CustomProvider::CustomProvider(QObject *parent):
    RazorPowerProvider(parent),
    mSettings("power")
{
}

CustomProvider::~CustomProvider()
{
}

bool CustomProvider::canAction(RazorPower::Action action) const
{
    switch (action)
    {
    case RazorPower::PowerShutdown:
        return mSettings.contains("shutdownCommand");

    case RazorPower::PowerReboot:
        return mSettings.contains("rebootCommand");

    case RazorPower::PowerHibernate:
        return mSettings.contains("hibernateCommand");

    case RazorPower::PowerSuspend:
        return mSettings.contains("suspendCommand");

    case RazorPower::PowerLogout:
        return mSettings.contains("logoutCommand");

    default:
        return false;
    }
}

bool CustomProvider::doAction(RazorPower::Action action)
{
    QString command;

    switch(action)
    {
    case RazorPower::PowerShutdown:
        command = mSettings.value("shutdownCommand").toString();
        break;

    case RazorPower::PowerReboot:
        command = mSettings.value("rebootCommand").toString();
        break;

    case RazorPower::PowerHibernate:
        command = mSettings.value("hibernateCommand").toString();
        break;

    case RazorPower::PowerSuspend:
        command = mSettings.value("suspendCommand").toString();
        break;

    case RazorPower::PowerLogout:
        command = mSettings.value("logoutCommand").toString();
        break;

    default:
        return false;
    }

    return QProcess::startDetached(command);
}
