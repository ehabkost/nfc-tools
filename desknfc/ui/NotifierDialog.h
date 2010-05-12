/***************************************************************************
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

#ifndef NOTIFIERDIALOG_H
#define NOTIFIERDIALOG_H

//Qt
#include <QWidget>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QHash>

//solid
#include <solid/solidnamespace.h>


//to remove
#include <QTreeView>

//own
#include "../NfcDevice.h"
#include "../NfcTarget.h"
#include "../Content.h"

class QModelIndex;
class QStandardItemModel;
class QStandardItem;
class QLabel;

//own
class DesknfcPlasmoid;

//desktop view
namespace Plasma
{
    class Icon;
    class Dialog;
}
namespace Solid
{
    class Device;
}
namespace Notifier
{
  class NotifierView;


  /**
  * @short The panel used to display devices in a popup
  *
  */
  class NotifierDialog : public QObject
  {
  Q_OBJECT
  
      public:

          
          ///Specific role for the data-model
          enum SpecificRoles {
              SolidUdiRole = Qt::UserRole + 1,
              PredicateFilesRole = Qt::UserRole + 2,
              ActionRole = Qt::UserRole + 3,
              IconNameRole = Qt::UserRole + 4,
              ScopeRole = Qt::UserRole + 5,
              SubTitleMandatoryRole = Qt::UserRole + 6
          };
  
          /**
          * Constructor of the dialog
          * @param dp the DesknfcPlasmoid attached to this dialog
          * @param area where the dialog is displayed
          * @param parent the parent of this object
          **/
          NotifierDialog(DesknfcPlasmoid * applet,QObject *parent = 0);
 
          /**
          * Default destructor
          **/
          virtual ~NotifierDialog();

          /**
          * Returns the related QWidget.
          **/
          QWidget * dialog();
         
          /**
          * Hide the dialog
          **/
          void hide();

          /**
          * Clear the dialog
          **/
          void clear();

          /**
          * Show the dialog
          **/
          void show();

          /**
          * insert a device in the data-model of the dialog
          * @param name the name of the device
          **/
          void insertDevice(const QString &name);

          /**
          *  insert a content in the data-model of the dialog
          *  @param content a pointer to the content
          *  @param name the name of the device
          **/
          void insertContent(Content* content, const QString &name, const QString &uid);

          void setUnMount(bool unmount,const QString &name);
  
          /**
          * Allow to set data which will be displayed by the view
          * @param name the name of the device 
          * @param data the data
          * @param role the role in the data-model
          **/
          void setDeviceData(const QString &name, QVariant data, int role);
          
          /**
          * Allow to get a data display by the view
          * @param name the name of the device 
          * @param role the role where is the data
          **/
          QVariant getDeviceData(const QString &name, int role);

          /**
          * Remove an element in the dialog
          * @param name the name of the element 
          **/
          void removeElement(const QString &name);

                     
          /**
          * Remove a device in the view (provided by convenience)
          * @param index the index where the data will be delete 
          **/
          void removeDevice(int index);

          /**
          * Return the number of items displayed
          * 
          **/
          int countDevices();

          /**
          * get the udi of a device displayed in the dialog
          * @param index the index of the device 
          **/
          QString getDeviceUdi(int index);

      signals :

          void itemSelected();

      private slots:
          /**
           * @internal called when a teardown error occurs
           */
          void showTeardownError();

          /**
           * @internal called when an eject error occurs
           */
          void showStorageEjectDoneError();

          /**
          * @internal slot called when user has click on a item in the dialog
          * @param index the model index which is clicked
          **/
          void itemClicked(const QModelIndex & index);

          /**
          * @internal slot called when an eject is finished
          * @param errorData the error if problem
          * @param error type of error given by solid
          **/
          void storageEjectDone(Solid::ErrorType error, QVariant errorData);

          /**
          * @internal slot called when a storage tear is finished
          * @param errorData the error if problem
          * @param error type of error given by solid
          **/
          void storageTeardownDone(Solid::ErrorType error, QVariant errorData);
 
          /**
          * @internal slot called to restore to the notifier his icon
          **/
          void resetNotifierIcon();

          /**
          * @internal update the color of the label to follow plasma theme 
          *
          **/
          void updateColors();


      private :
          /**
          * @internal build the dialog depending where it is 
          **/
          void buildDialog();

          /**
          * @internal get the model index in the data-model by using the udi in parameter 
          * @param udi the udi used to find the model index
          **/
          QModelIndex indexForUdi(const QString &udi) const;

          ///The data-model used to store devices
          QStandardItemModel *m_hotplugModel;

          ///The widget which display the panel
          QWidget *m_widget;
          ///The tree view used to display the content
          NotifierView *m_notifierView;

          ///QLabel which represent the title
          QLabel * m_label;

          ///The applet attached to this item
          DesknfcPlasmoid * m_applet;

          ///Root item in treeview
          QStandardItem *m_rootItem;

          ///Contents category item in treeview
          QStandardItem *m_contentsCategoryItem;

          QList< QPair<QModelIndex,Content*> > m_contents_index;

          /**
          * @internal Search a category with same name. If not find, create a new category in top of treeview
          * @param categoryName the name of the category for device
          **/
          QStandardItem* searchOrCreateDeviceCategory(const QString &categoryName);

          /**
          * @internal get The category name of a device plugged
          * @param device the solid device plugged in hardware
          **/
          QString getCategoryNameOfDevice(const Solid::Device& device);
  };

}

#endif

