#include "NotifyAction.h"

NotifyAction::NotifyAction(QAction* action, QString key): _key(key)
{
     _action = action;
}

NotifyAction::~NotifyAction()
{
}

const QString 
NotifyAction::text()
{
    QString t = _action->text();
    t.remove(QChar('&'));
    return t;
}

const QString 
NotifyAction::key()
{
    return _key;
}

void 
NotifyAction::setNotificationId(const uint id)
{
    _notificationId = id;
}

void
NotifyAction::NotifyActionInvoked(uint id, const QString &action_key)
{
    if((id == _notificationId) && (action_key == key())) {
         emit _action->trigger();
         this->deleteLater();
    }
}

void
NotifyAction::NotifyNotificationClosed(uint id, uint reason)
{
	(void)reason; // no warning!
    if(id == _notificationId) {
         this->deleteLater();
    }
}
