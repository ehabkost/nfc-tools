#ifndef __NOTIFICATION_H__
#define __NOTIFICATION_H__

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QAction>

#include "NotifyInterface.h"

class QString;
class NotifyAction;

class Notification: public QObject
{
    Q_OBJECT

public:
    Notification(OrgFreedesktopNotificationsInterface* notify, const QString summary, const QString body);
    ~Notification();

    void addAction(QAction* action);
    void send();

protected:
    QList<NotifyAction*> actions;

private:
    QString _body;
    QString _summary;
    OrgFreedesktopNotificationsInterface * _notify;
};

#endif
