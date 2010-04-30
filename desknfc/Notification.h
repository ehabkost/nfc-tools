#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <KNotification>
#include <KFileItem>
#include <QString>
#include <QDesktopServices>
#include <QUrl>
#include "Content.h"

class Notification : public KNotification {

  Q_OBJECT

  public:
    Notification(QString,Content*,QString);
    /*~Notification();*/

  public slots:
    void open();

  protected:
    Content* _content;
    QString _path;

  private:
    

};

#endif
