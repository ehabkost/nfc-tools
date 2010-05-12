/*  Copyright 2007 by Alexis Ménard <darktears31@gmail.com>

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
#ifndef NOTIFIERVIEW_H
#define NOTIFIERVIEW_H

// Qt
#include <QTreeView>

class QModelIndex;

namespace Notifier
{
  /**
  * @short The view used to display information in a device popup
  *
  */
  class NotifierView : public QTreeView
  {
  Q_OBJECT

  public:
      /**
      * Constructor of view
      * @param parent the parent of this object
      **/
      NotifierView(QWidget *parent = 0);

      /**
      * Default destructor
      **/
      virtual ~NotifierView();

      /**
       * Creates rects in widget coordinates for the model
       */
      void calculateRects();

      /**
       * Reimplemented from QTreeView
       */
      QRect visualRect(const QModelIndex &index) const;

  protected:
      /**
      * Call when the view is resized
      * @param event the resize event
      **/
      void resizeEvent(QResizeEvent * event);

      /**
      * Call when a mouse move event occurs
      * @param event the mouse event
      **/
      void mouseMoveEvent(QMouseEvent *event);

      /**
       * Call when a mouse press event occurs
       * @param event the mouse event
       **/
      void mousePressEvent(QMouseEvent *event);

      /**
       * Call when a mouse release event occurs
       * @param event the mouse event
       **/
      void mouseReleaseEvent(QMouseEvent *event);

      /**
      * Call when cursor leave the widget
      * @param event the leave event
      **/
      void leaveEvent(QEvent *event);

      /**
      * Move the cursor in the way describe by cursorAction
      * @param cursorAction the cursor action
      **/
      QModelIndex moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers );

      /**
      * Call when the view is paint
      * @param event the paint event
      **/
      void paintEvent(QPaintEvent *event);

      /**
      * Paint a header item
      * @param painter the painter used to paint
      * @param itemRect the rect where the item will be paint
      * @param index the QModelIndex that represent the item to paint
      **/
      void paintHeaderItem(QPainter &painter,const QRect &itemRect,const QModelIndex &index);

      /**
      * Paint an item in the view by using the delegate
      * @param painter the painter used to paint
      * @param itemRect the rect where the item will be paint
      * @param index the QModelIndex that represent the item to paint
      **/
      void paintItem(QPainter &painter,const QRect &itemRect,const QModelIndex &index);

      /**
      * Return an index at the position "point" if exist otherwise return a default
      * constructed value of QModelIndex
      * @param point the point where we will looking for an item
      **/
      QModelIndex indexAt(const QPoint& point) const;

      static const int HEADER_LEFT_MARGIN = 5;
      static const int HEADER_TOP_MARGIN = 5;
      static const int HEADER_HEIGHT = 35;
      static const int COLUMN_EJECT_MARGIN = 5;
      static const int COLUMN_EJECT_SIZE = 50;
      static const int TOP_OFFSET = 5;

  private:
      ///The hovered index
      QPersistentModelIndex m_hoveredIndex;
      QHash<QModelIndex,QRect> itemRects;
  };

}
#endif // NOTIFIERVIEW_H
