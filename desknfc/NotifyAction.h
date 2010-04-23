#ifndef __NOTIFY_ACTION_H__
#define __NOTIFY_ACTION_H__

#include <QtCore/QObject>
#include <QAction>

class NotifyAction: public QObject
{
    Q_OBJECT
public:
    NotifyAction(QAction* action, QString key);
    ~NotifyAction();

    const QString key();
    const QString text();
    void setNotificationId(uint id);

public Q_SLOTS:
    void NotifyActionInvoked(uint id, const QString &action_key);
    void NotifyNotificationClosed(uint id, uint reason);

private:
    QAction* _action;
    QString _key;
    uint _notificationId;
};

#endif
