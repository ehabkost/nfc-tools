/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

//own
#include "DesknfcPlasmoid.h"
#include "NotifierView.h"
#include "NotifierDialog.h"

//Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QTimer>

//KDE
#include <KIcon>
#include <KConfigDialog>
#include <KStandardDirs>
#include <KDesktopFile>
#include <kdesktopfileactions.h>
#include <KIconLoader>

//plasma
#include <Plasma/Dialog>
//use for desktop view
#include <Plasma/IconWidget>
#include <Plasma/Theme>

//solid
#include <solid/device.h>
#include <solid/storagedrive.h>


using namespace Plasma;
using namespace Notifier;

K_EXPORT_PLASMA_APPLET(desknfc, DesknfcPlasmoid)

DesknfcPlasmoid::DesknfcPlasmoid(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_devManager(0),
      m_icon(0),
      m_iconName(""),
      m_dialog(0),
      m_numberItems(0),
      m_itemsValidity(0)
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);

    // let's initialize the widget
    resize(widget()->sizeHint());
}

DesknfcPlasmoid::~DesknfcPlasmoid()
{
    delete m_icon;
    delete m_dialog;
}

void DesknfcPlasmoid::init()
{
    KConfigGroup cg = config();
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);

    m_devManager = new NfcDeviceManager();
    
    m_icon = new Plasma::IconWidget(KIcon("desknfc",NULL), QString());
    m_iconName = QString("desknfc");

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(m_icon->icon());

    //feed the list with what is already reported by the engine

    //connect to engine when a device is plug
    connect(m_devManager, SIGNAL(devicePlugged(uchar, QString)),
            this, SLOT(onDeviceAdded(uchar, const QString&)));
    connect(m_devManager, SIGNAL(deviceUnplugged(uchar,QString)),
            this, SLOT(onDeviceRemoved(uchar, const QString&)));

    if(! m_devManager->haveNfcdConnection() ) {
        m_dialog->nfcdOffline();
    }

    fillPreviousDevices();
}

QWidget *DesknfcPlasmoid::widget()
{
    if (!m_dialog) {
        m_dialog = new NotifierDialog(this);
        connect(m_dialog, SIGNAL(itemSelected()), this, SLOT(hidePopup()));
    }

    return m_dialog->dialog();
}

void DesknfcPlasmoid::fillPreviousDevices()
{
    m_fillingPreviousDevices = true;
    foreach (const QString & nDev, m_devManager->getDeviceList() ) {
        NfcDevice* dev = m_devManager->getDeviceByName(nDev);
        disconnect(dev,SIGNAL(targetFieldEntered(QString,QString)),
            this, SLOT(onTargetAdded(QString,QString)));
        connect(dev,SIGNAL(targetFieldEntered(QString,QString)),
            this, SLOT(onTargetAdded(QString,QString)));
        disconnect(dev,SIGNAL(targetFieldLeft(QString,QString)),
            this, SLOT(onTargetRemoved(QString,QString)));
        connect(dev,SIGNAL(targetFieldLeft(QString,QString)),
            this, SLOT(onTargetRemoved(QString,QString)));
        foreach(const QString & uidTarget, dev->getTargetList()) {
            NfcTarget* target = dev->getTargetByUid(uidTarget);
            m_dialog->insertTarget(target,dev->getName());
            foreach(Content* content, target->getTargetContent()) {
                //onDeviceAdded(0, content->getType());
                m_dialog->insertContent(content,dev->getName(), target->getUid());
            }
        }
    }
    m_fillingPreviousDevices = false;
}

void DesknfcPlasmoid::changeNotifierIcon(const QString& name)
{
    if (m_icon && name.isNull()) {
        m_icon->setIcon(m_iconName);
    } else if (m_icon) {
        m_icon->setIcon(name);
    }

    setPopupIcon(m_icon->icon());
}

void DesknfcPlasmoid::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
    }
}

void DesknfcPlasmoid::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    if (data.size() > 0) {
        //data from hotplug engine
        if (!data["predicateFiles"].isNull()) {
            int nb_actions = 0;
            QString lastActionLabel;
            foreach (const QString &desktop, data["predicateFiles"].toStringList()) {
                QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
                QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
                nb_actions += services.size();
                if (services.size() > 0) {
                    lastActionLabel = QString(services[0].text());
                }
            }
            m_dialog->setDeviceData(source,data["predicateFiles"],NotifierDialog::PredicateFilesRole);
            m_dialog->setDeviceData(source,data["text"], Qt::DisplayRole);

            //icon name
            m_dialog->setDeviceData(source,data["icon"], NotifierDialog::IconNameRole);
            //icon data
            m_dialog->setDeviceData(source,KIcon(data["icon"].toString()), Qt::DecorationRole);

            if (nb_actions > 1) {
                QString s = i18np("1 action for this device",
                                  "%1 actions for this device",
                                  nb_actions);
                m_dialog->setDeviceData(source, s, NotifierDialog::ActionRole);
            } else {
                m_dialog->setDeviceData(source, lastActionLabel, NotifierDialog::ActionRole);
            }

        //data from soliddevice engine
        } else {
            kDebug() << "DesknfcPlasmoid::solidDeviceEngine updated" << source;
            if (data["Device Types"].toStringList().contains("Storage Access")) {
                if (data["Accessible"].toBool() == true) {
                    m_dialog->setUnMount(true,source);

                    //set icon to mounted device
                    QStringList overlays;
                    overlays << "emblem-mounted";
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString(), NULL, overlays), Qt::DecorationRole);
                } else if (data["Device Types"].toStringList().contains("OpticalDisc")) {
                    //Unmounted optical drive

                    //set icon to unmounted device
                    m_dialog->setUnMount(true,source);
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString()), Qt::DecorationRole);
                } else {
                    m_dialog->setUnMount(false,source);

                    //set icon to unmounted device
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString()), Qt::DecorationRole);
                }
            }
            // actions specific for other types of devices will go here
            if (data["Device Types"].toStringList().contains("Storage Volume")) {
                if (data["Device Types"].toStringList().contains("OpticalDisc")) {
                    m_dialog->setUnMount(true, source);
                }
            }
        }
    }
}

void DesknfcPlasmoid::notifyDevice(const QString &name)
{
    m_lastPlugged<<name;

    if (!m_fillingPreviousDevices) {
        showPopup();
    }
}

void DesknfcPlasmoid::toolTipAboutToShow()
{
    /*Plasma::ToolTipContent toolTip;
    if (!m_lastPlugged.isEmpty()) {
        Solid::Device device(m_lastPlugged.last());

        toolTip.setSubText(i18n("Last plugged in device: %1", device.product()));
        toolTip.setImage(KIcon(device.icon()));
    } else {
        toolTip.setSubText(i18n("No devices plugged in"));
        toolTip.setImage(KIcon("device-notifier"));
    }

    Plasma::ToolTipManager::self()->setContent(this, toolTip);*/
}

void DesknfcPlasmoid::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void DesknfcPlasmoid::removeLastDeviceNotification(const QString &name)
{
    m_lastPlugged.removeAll(name);
}

void DesknfcPlasmoid::onDeviceAdded(uchar useless, const QString &name)
{
    m_dialog->clear();
    fillPreviousDevices();
    NfcDevice* dev = m_devManager->getDeviceByName(name);
    disconnect(dev,SIGNAL(targetFieldEntered(QString,QString)),
        this, SLOT(onTargetAdded(QString,QString)));
    connect(dev,SIGNAL(targetFieldEntered(QString,QString)),
        this, SLOT(onTargetAdded(QString,QString)));
    disconnect(dev,SIGNAL(targetFieldLeft(QString,QString)),
        this, SLOT(onTargetRemoved(QString,QString)));
    connect(dev,SIGNAL(targetFieldEntered(QString,QString)),
        this, SLOT(onTargetRemoved(QString,QString)));
}

void DesknfcPlasmoid::onDeviceRemoved(uchar useless, const QString &name) {
    (void)useless;
    //NfcDevice* dev = m_devManager->getDeviceByName(name);
    m_dialog->clear();
    fillPreviousDevices();
}

void DesknfcPlasmoid::onTargetRemoved(const QString &name, const QString &uid) {
    (void)name;
    m_dialog->clear();
    fillPreviousDevices();
}

void DesknfcPlasmoid::onTargetAdded(const QString &uid, const QString &name) {
    m_dialog->clear();
    fillPreviousDevices();
}

void DesknfcPlasmoid::onSourceRemoved(uchar useless, const QString &name)
{
    (void)useless;
    //m_solidEngine->disconnectSource(name, this);
    //m_solidDeviceEngine->disconnectSource(name, this);

    
    removeLastDeviceNotification(name);
}

#include "DesknfcPlasmoid.moc"
