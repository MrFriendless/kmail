/*
   Copyright (C) 2014-2019 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FOLLOWUPREMINDERMANAGER_H
#define FOLLOWUPREMINDERMANAGER_H

#include <QObject>
#include <KSharedConfig>
#include <AkonadiCore/Item>
#include <QPointer>
namespace FollowUpReminder {
class FollowUpReminderInfo;
}
class FollowUpReminderNoAnswerDialog;
class FollowUpReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit FollowUpReminderManager(QObject *parent = nullptr);
    ~FollowUpReminderManager();

    void load(bool forceReloadConfig = false);
    void checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col);

    Q_REQUIRED_RESULT QString printDebugInfo() const;
private:
    Q_DISABLE_COPY(FollowUpReminderManager)
    void slotCheckFollowUpFinished(const QString &messageId, Akonadi::Item::Id id);

    void slotFinishTaskDone();
    void slotFinishTaskFailed();
    void slotReparseConfiguration();
    void answerReceived(const QString &from);
    Q_REQUIRED_RESULT QString infoToStr(FollowUpReminder::FollowUpReminderInfo *info) const;

    KSharedConfig::Ptr mConfig;
    QList<FollowUpReminder::FollowUpReminderInfo *> mFollowUpReminderInfoList;
    QPointer<FollowUpReminderNoAnswerDialog> mNoAnswerDialog;
    bool mInitialize = false;
};

#endif // FOLLOWUPREMINDERMANAGER_H
