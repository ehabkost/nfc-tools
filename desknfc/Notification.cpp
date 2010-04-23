#include "Notification.h"

#include "NotifyAction.h"
/*


class Notification: pulbic QObject;
{
    Q_OBJECT

    Notification(NotifyInterface notify, QString summary, QString body);
    ~Notification();

    void addAction(NotifyAction* action);

    void send();

protected:
    QList<QAction*> actions;
};

*/

Notification::Notification(OrgFreedesktopNotificationsInterface* notify, const QString summary, const QString body)
{
    _summary = summary;
    _body = body;
    _notify = notify;
}

Notification::~Notification()
{
}

void
Notification::addAction(QAction* action)
{
    actions << new NotifyAction(action, QString::number(actions.size()));
}

void
Notification::send()
{
/* uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon, 
			const QString &summary, const QString &body, 
			const QStringList &actions, 
			const QVariantMap &hints, int timeout)
*/
    /* Check if notify D-Bus interface is up */
    if (_notify->isValid() ) {
        /* Build Notify's action list */
        QStringList actionList;
        for(int i = 0; i < actions.size(); i++) {
            NotifyAction* action = actions.at(i);
            actionList << action->key();
            actionList << action->text();
        }

        /* Send notification and get its id. */
        /* TODO Handle app_name without using this const... */
        uint notify_id = _notify->Notify("DeskNfc", 0, "", _summary, _body, actionList, QVariantMap(), 3000);

        /* Set Nofication ID to action. It able action to handle notification callback itself */
        for(int i = 0; i < actions.size(); i++) {
            NotifyAction* action = actions.at(i);
            action->setNotificationId(notify_id);
            connect( _notify, SIGNAL(ActionInvoked(uint, const QString)), action, SLOT(NotifyActionInvoked(uint, const QString)) );
            connect( _notify, SIGNAL(NotificationClosed(uint, uint)), action, SLOT(NotifyNotificationClosed(uint, uint)) );
        }
    }
    
}
