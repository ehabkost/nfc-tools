/*
    Copyright 2008 by Alexis Ménard <darktears31@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "NotifierDialog.h"

//Qt
#include <QStandardItemModel>
#include <QModelIndex>
#include <QLabel>
#include <QVBoxLayout>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QHeaderView>
#include <QTimer>
#include <QMetaEnum>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KGlobalSettings>
#include <KMessageBox>

//plasma
#include <Plasma/Dialog>
#include <Plasma/Delegate>
#include <Plasma/Theme>

//solid
#include <solid/device.h>
#include <solid/opticaldisc.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>
#include <solid/deviceinterface.h>

//own
#include "NotifierView.h"

using namespace Notifier;
using namespace Plasma;

NotifierDialog::NotifierDialog(DesknfcPlasmoid * applet,QObject *parent)
    : QObject(parent),
      m_hotplugModel(0),
      m_widget(0),
      m_notifierView(0),
      m_label(0),
      m_applet(applet),
      m_rootItem(0)
{
    m_hotplugModel = new QStandardItemModel(this);
    buildDialog();
    //make the invisible root for tree device
    m_rootItem = m_hotplugModel->invisibleRootItem();
    m_contentsCategoryItem = new QStandardItem(tr("Contents"));
    m_hotplugModel->setData(m_contentsCategoryItem->index(),tr("Contents"),Qt::DisplayRole);
    m_rootItem->insertRow(0,m_contentsCategoryItem);
	 m_targetsCategoryItem = new QStandardItem(tr("Targets"));
    m_hotplugModel->setData(m_targetsCategoryItem->index(),tr("Targets"),Qt::DisplayRole);
    m_rootItem->insertRow(1,m_targetsCategoryItem);
    m_hotplugModel->setItem(0, 1, NULL);
    m_hotplugModel->setHeaderData(0, Qt::Horizontal,QString(""),Qt::EditRole);
    m_hotplugModel->setHeaderData(1, Qt::Horizontal,QString(""),Qt::EditRole);
}

NotifierDialog::~NotifierDialog()
{

}

void NotifierDialog::clear() {
    m_contents_index.clear();
    m_targets_index.clear();
    m_hotplugModel->clear();
    m_contentsCategoryItem = new QStandardItem(QString(tr("Contents")));
    m_rootItem = m_hotplugModel->invisibleRootItem();
    m_hotplugModel->setData(m_contentsCategoryItem->index(),tr("Contents"),Qt::DisplayRole);
    m_rootItem->insertRow(0,m_contentsCategoryItem);
    m_targetsCategoryItem = new QStandardItem(tr("Targets"));
    m_hotplugModel->setData(m_targetsCategoryItem->index(),tr("Targets"),Qt::DisplayRole);
    m_rootItem->insertRow(1,m_targetsCategoryItem);
    m_hotplugModel->setItem(0, 1, NULL);
    m_hotplugModel->setHeaderData(0, Qt::Horizontal,QString(""),Qt::EditRole);
    m_hotplugModel->setHeaderData(1, Qt::Horizontal,QString(""),Qt::EditRole);
}

QWidget * NotifierDialog::dialog()
{
    return m_widget;
}

void NotifierDialog::hide()
{
    m_widget->hide();
}

void NotifierDialog::show()
{
    m_widget->show();
}

QStandardItem* NotifierDialog::searchOrCreateDeviceCategory(const QString &categoryName)
{
    int rowCount = m_hotplugModel->rowCount();
    if(rowCount > 0)
    {
        int i = 0;
        while (i<rowCount)
        {
            QModelIndex index = m_hotplugModel->index(i, 0);
            QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();
            QStandardItem *currentItem = m_hotplugModel->itemFromIndex(index);
            if(currentItem)
            {
                QString currentItemName = currentItem->text();
                if (currentItemName == categoryName)
                {
                    //the category is find... we have to return the pointer on this category
                    return m_hotplugModel->itemFromIndex(index);
                }
            }
            i++;
        }
    }
    //insert a new category for device if not find and return the pointer
    QStandardItem *newCategory = new QStandardItem(QString(categoryName));
    m_hotplugModel->setData(newCategory->index(),categoryName,Qt::DisplayRole);
    m_rootItem->insertRow(0,newCategory);
    m_hotplugModel->setItem(0, 1, NULL);
    m_hotplugModel->setHeaderData(0, Qt::Horizontal,QString(""),Qt::EditRole);
    m_hotplugModel->setHeaderData(1, Qt::Horizontal,QString(""),Qt::EditRole);
    return newCategory;
}

void NotifierDialog::nfcdOffline() {
    KColorScheme colorTheme = KColorScheme(QPalette::Active, KColorScheme::View,Plasma::Theme::defaultTheme()->colorScheme());
    m_label->setText(i18n("<font color=\"%1\">Please launch NFCd</font>",colorTheme.foreground(KColorScheme::NormalText).color().name()));
}

void NotifierDialog::nfcdOnline() {
    KColorScheme colorTheme = KColorScheme(QPalette::Active, KColorScheme::View,Plasma::Theme::defaultTheme()->colorScheme());
    m_label->setText(i18n("<font color=\"%1\">Near Field Communication</font>",colorTheme.foreground(KColorScheme::NormalText).color().name()));
}

void NotifierDialog::insertContent(Content* content, const QString &devName, const QString &tgUid)
{
    QString name = "" + qHash( content->getData() );
    QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    item->setData(Plasma::Delegate::MainColumn, ScopeRole);
    item->setData(false, SubTitleMandatoryRole);
    item->setText(content->getDesc());
    item->setIcon(QIcon(content->getIcon())); 

    QStandardItem *actionItem = new QStandardItem();
    actionItem->setData(name, SolidUdiRole);

    QString udi = item->data(SolidUdiRole).toString();

    int rowNum = m_contentsCategoryItem->rowCount();
    m_contentsCategoryItem->insertRow(rowNum, item);
    m_contentsCategoryItem->setChild(rowNum, 1, actionItem);
    m_hotplugModel->setData(item->index(), content->getAssociatedServiceName(), NotifierDialog::ActionRole);

    m_contents_index << QPair<QModelIndex,Content*>(item->index(),content);
     
    m_notifierView->calculateRects();

}

void NotifierDialog::insertTarget(NfcTarget* tg, const QString &devName) {
	QString name = tg->getUuid().toString();
	QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    item->setData(Plasma::Delegate::MainColumn, ScopeRole);
    item->setData(false, SubTitleMandatoryRole);
    item->setText(tg->getName());
    //item->setIcon(QIcon(content->getIcon())); 

    QStandardItem *actionItem = new QStandardItem();
    actionItem->setData(name, SolidUdiRole);
    actionItem->setData(Plasma::Delegate::SecondaryActionColumn, ScopeRole);
    //setDeviceData(name, content->getType(), NotifierDialog::ActionRole);
    //item->setData("test?!", NotifierDialog::ActionRole);

    QString udi = item->data(SolidUdiRole).toString();

    int rowNum = m_targetsCategoryItem->rowCount();
    m_targetsCategoryItem->insertRow(rowNum, item);
    m_targetsCategoryItem->setChild(rowNum, 1, actionItem);
    m_hotplugModel->setData(item->index(), tg->getUid(), NotifierDialog::ActionRole);

    m_targets_index << QPair<QModelIndex,NfcTarget*>(item->index(),tg);

}

void NotifierDialog::setUnMount(bool unmount, const QString &name)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return;
    }
    QStandardItem *currentItem = m_hotplugModel->itemFromIndex(index);
    QStandardItem *childAction = currentItem->parent()->child(currentItem->row(), 1);
    QVariant icon;
    if (unmount) {
        icon = KIcon("media-eject");
    }
    else {
        icon = KIcon();
    }
    m_hotplugModel->setData(childAction->index(),icon,Qt::DecorationRole);
}

void NotifierDialog::setDeviceData(const QString &name, QVariant data, int role)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return;
    }
    if (role == Qt::DecorationRole) {
        QStandardItem *device = m_hotplugModel->itemFromIndex(index);
        QStandardItem *category = device->parent();
        QModelIndex parentIndex = category->index();
        if (!parentIndex.data(Qt::DecorationRole).isValid()) {
           m_hotplugModel->setData(parentIndex,data,role);
        }
    }
    m_hotplugModel->setData(index,data,role);
}

QVariant NotifierDialog::getDeviceData(const QString &name, int role)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return QVariant();
    }
    else {
        return index.data(role);
    }
}

void NotifierDialog::removeDevice(int index)
{
    m_hotplugModel->removeRow(index);
    m_notifierView->calculateRects();
}

int NotifierDialog::countDevices()
{
    return m_hotplugModel->rowCount();
}

QString NotifierDialog::getDeviceUdi(int index)
{
    QModelIndex modelIndex = m_hotplugModel->index(index, 0);
    return m_hotplugModel->data(modelIndex, SolidUdiRole).toString();
}

void NotifierDialog::buildDialog()
{
    m_widget = new QWidget();
    m_widget->setAttribute(Qt::WA_TranslucentBackground);
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Qt::transparent);
    m_widget->setPalette(p);

    QVBoxLayout *l_layout = new QVBoxLayout(m_widget);
    l_layout->setSpacing(0);
    l_layout->setMargin(0);

    m_label = new QLabel(m_widget);
    updateColors();

    QLabel *icon = new QLabel(m_widget);
    icon->setPixmap(KIcon("emblem-mounted").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));

    QHBoxLayout *l_layout2 = new QHBoxLayout(m_widget);
    l_layout2->setSpacing(0);
    l_layout2->setMargin(0);

    l_layout2->addWidget(icon);
    l_layout2->addWidget(m_label);

    l_layout2->setAlignment(Qt::AlignCenter);


    m_notifierView = new NotifierView(m_widget);
    m_notifierView->setModel(m_hotplugModel);
    m_notifierView->setMinimumSize(150,300);
    m_notifierView->setFocusPolicy(Qt::NoFocus);

    Plasma::Delegate *delegate = new Delegate(this);
    //map the roles of m_hotplugModel into the standard Plasma::Delegate roles
    delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, ActionRole);
    delegate->setRoleMapping(Plasma::Delegate::ColumnTypeRole, ScopeRole);
    delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
    m_notifierView->setItemDelegate(delegate);

    l_layout->addLayout(l_layout2);
    l_layout->addWidget(m_notifierView);
    m_widget->setLayout(l_layout);

    connect(m_notifierView, SIGNAL(clicked(const QModelIndex&)),this,SLOT(itemClicked(const QModelIndex&)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));    // allows updating of colors automatically
}

void NotifierDialog::storageTeardownDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        QTimer::singleShot(0, this, SLOT(showTeardownError()));
    } else {
        //m_notifier->changeNotifierIcon("dialog-ok");
        //m_notifier->update();
        QTimer::singleShot(5000, this, SLOT(resetNotifierIcon()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
}

void NotifierDialog::showTeardownError()
{
    //FIXME: modal dialog are bad m'kay
    KMessageBox::error(0, i18n("Could not unmount the device.\nOne or more files on this device are open within an application."), QString());
}

void NotifierDialog::storageEjectDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        QTimer::singleShot(0, this, SLOT(showStorageEjectDoneError()));
    } else {
        //m_notifier->changeNotifierIcon("dialog-ok");
        //m_notifier->update();
        QTimer::singleShot(2000, this, SLOT(resetNotifierIcon()));
    }
    //show the message only one time
    disconnect(sender(), SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
}

void NotifierDialog::showStorageEjectDoneError()
{
    KMessageBox::error(0, i18n("Cannot eject the disc.\nOne or more files on this disc are open within an application."), QString());
}

QModelIndex NotifierDialog::indexForUdi(const QString &udi) const
{
    int rowCount = m_hotplugModel->rowCount();
    for (int i=0; i < rowCount; ++i) {
        QModelIndex index = m_rootItem->index();
        QStandardItem *currentItem = m_rootItem;
        for (int j=0; j < currentItem->rowCount(); ++j) {
          QStandardItem *childItem = currentItem->child(j, 0);
          QString itemUdi = m_hotplugModel->data(childItem->index(), SolidUdiRole).toString();
          if (itemUdi == udi) {
              return childItem->index();
          }
        }
    }
    //Is it possible to go here?no...
    kDebug() << "We should not be here!";
    return QModelIndex();
}

void NotifierDialog::itemClicked(const QModelIndex &index)
{
    QList< QPair<QModelIndex, Content*> >::iterator itContents;
    for(itContents = m_contents_index.begin(); itContents != m_contents_index.end(); ++itContents) {
        if(itContents->first == index) {
            itContents->second->open();
            emit itemSelected();
            return;
        }
    }
    QList< QPair<QModelIndex, NfcTarget*> >::iterator itTargets;
    for(itTargets = m_targets_index.begin(); itTargets != m_targets_index.end(); ++itTargets ) {
        if(itTargets->first == index) {
            KFileDialog* fileChooser = new KFileDialog(KUrl(""),"",NULL);
            if(fileChooser->exec()) {
                //qDebug() << fileChooser->selectedFiles();
                KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, fileChooser->selectedFile());
                QString mimeType = fileItem.mimetype();
                QByteArray qb;
                QDataStream ds(&qb,QIODevice::WriteOnly);
                //QFile f(fileItem.url().path());
                ds << fileItem;
                qDebug() << fileItem.mimetype();
                itTargets->second->writeAFile(qb,"application/octet-stream");
            }
            delete fileChooser;
            emit itemSelected();
            return;
        }
    }
}

QString NotifierDialog::getCategoryNameOfDevice(const Solid::Device& device)
{
    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);
    for (int i = typeEnum.keyCount() - 1 ; i > 0; i--)
    {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device.asDeviceInterface(type);
        if (interface)
        {
            return Solid::DeviceInterface::typeToString(type);
        }
    }
    return 0;
}

void NotifierDialog::resetNotifierIcon()
{
    //m_notifier->changeNotifierIcon();
    //m_notifier->update();
}

void NotifierDialog::updateColors()
{
    KColorScheme colorTheme = KColorScheme(QPalette::Active, KColorScheme::View,Plasma::Theme::defaultTheme()->colorScheme());
    m_label->setText(i18n("<font color=\"%1\">Near Field Communication</font>",colorTheme.foreground(KColorScheme::NormalText).color().name()));

}

#include "NotifierDialog.moc"
