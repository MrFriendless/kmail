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

#include "followupremindermanager.h"
#include "followupreminderagent_debug.h"
#include "FollowupReminder/FollowUpReminderInfo"
#include "FollowupReminder/FollowUpReminderUtil"
#include "followupremindernoanswerdialog.h"
#include "jobs/followupreminderjob.h"
#include "jobs/followupreminderfinishtaskjob.h"
#include <Akonadi/KMime/SpecialMailCollections>

#include <KConfigGroup>
#include <KConfig>
#include <KSharedConfig>
#include <knotification.h>
#include <KLocalizedString>
#include <QRegularExpression>
using namespace FollowUpReminder;

FollowUpReminderManager::FollowUpReminderManager(QObject *parent)
    : QObject(parent)
{
    mConfig = KSharedConfig::openConfig();
}

FollowUpReminderManager::~FollowUpReminderManager()
{
    qDeleteAll(mFollowUpReminderInfoList);
    mFollowUpReminderInfoList.clear();
}

void FollowUpReminderManager::load(bool forceReloadConfig)
{
    if (forceReloadConfig) {
        mConfig->reparseConfiguration();
    }
    const QStringList itemList = mConfig->groupList().filter(QRegularExpression(QStringLiteral("FollowupReminderItem \\d+")));
    const int numberOfItems = itemList.count();
    QList<FollowUpReminder::FollowUpReminderInfo *> noAnswerList;
    for (int i = 0; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));

        FollowUpReminderInfo *info = new FollowUpReminderInfo(group);
        if (info->isValid()) {
            if (!info->answerWasReceived()) {
                mFollowUpReminderInfoList.append(info);
                if (!mInitialize) {
                    FollowUpReminderInfo *noAnswerInfo = new FollowUpReminderInfo(*info);
                    noAnswerList.append(noAnswerInfo);
                } else {
                    delete info;
                }
            } else {
                delete info;
            }
        } else {
            delete info;
        }
    }
    if (!noAnswerList.isEmpty()) {
        mInitialize = true;
        if (!mNoAnswerDialog.data()) {
            mNoAnswerDialog = new FollowUpReminderNoAnswerDialog;
            connect(mNoAnswerDialog.data(), &FollowUpReminderNoAnswerDialog::needToReparseConfiguration, this, &FollowUpReminderManager::slotReparseConfiguration);
        }
        mNoAnswerDialog->setInfo(noAnswerList);
        mNoAnswerDialog->show();
    }
}

void FollowUpReminderManager::slotReparseConfiguration()
{
    load(true);
}

void FollowUpReminderManager::checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col)
{
    if (mFollowUpReminderInfoList.isEmpty()) {
        return;
    }

    const Akonadi::SpecialMailCollections::Type type = Akonadi::SpecialMailCollections::self()->specialCollectionType(col);
    switch (type) {
    case Akonadi::SpecialMailCollections::Trash:
    case Akonadi::SpecialMailCollections::Outbox:
    case Akonadi::SpecialMailCollections::Drafts:
    case Akonadi::SpecialMailCollections::Templates:
    case Akonadi::SpecialMailCollections::SentMail:
        return;
    default:
        break;
    }

    FollowUpReminderJob *job = new FollowUpReminderJob(this);
    connect(job, &FollowUpReminderJob::finished, this, &FollowUpReminderManager::slotCheckFollowUpFinished);
    job->setItem(item);
    job->start();
}

void FollowUpReminderManager::slotCheckFollowUpFinished(const QString &messageId, Akonadi::Item::Id id)
{
    for (FollowUpReminderInfo *info : qAsConst(mFollowUpReminderInfoList)) {
        qCDebug(FOLLOWUPREMINDERAGENT_LOG) << "FollowUpReminderManager::slotCheckFollowUpFinished info:" << info;
        if (!info) {
            continue;
        }
        if (info->messageId() == messageId) {
            info->setAnswerMessageItemId(id);
            info->setAnswerWasReceived(true);
            answerReceived(info->to());
            if (info->todoId() != -1) {
                FollowUpReminderFinishTaskJob *job = new FollowUpReminderFinishTaskJob(info->todoId(), this);
                connect(job, &FollowUpReminderFinishTaskJob::finishTaskDone, this, &FollowUpReminderManager::slotFinishTaskDone);
                connect(job, &FollowUpReminderFinishTaskJob::finishTaskFailed, this, &FollowUpReminderManager::slotFinishTaskFailed);
                job->start();
            }
            //Save item
            FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(FollowUpReminder::FollowUpReminderUtil::defaultConfig(), info, true);
            break;
        }
    }
}

void FollowUpReminderManager::slotFinishTaskDone()
{
    qCDebug(FOLLOWUPREMINDERAGENT_LOG) << " Task Done";
    //TODO
}

void FollowUpReminderManager::slotFinishTaskFailed()
{
    qCDebug(FOLLOWUPREMINDERAGENT_LOG) << " Task Failed";
    //TODO
}

void FollowUpReminderManager::answerReceived(const QString &from)
{
    KNotification::event(QStringLiteral("mailreceived"),
                         QString(),
                         i18n("Answer from %1 received", from),
                         QStringLiteral("kmail"),
                         nullptr,
                         KNotification::CloseOnTimeout,
                         QStringLiteral("akonadi_followupreminder_agent"));
}

QString FollowUpReminderManager::printDebugInfo() const
{
    QString infoStr;
    if (mFollowUpReminderInfoList.isEmpty()) {
        infoStr = QStringLiteral("No mail");
    } else {
        for (FollowUpReminder::FollowUpReminderInfo *info : qAsConst(mFollowUpReminderInfoList)) {
            if (!infoStr.isEmpty()) {
                infoStr += QLatin1Char('\n');
            }
            infoStr += infoToStr(info);
        }
    }
    return infoStr;
}

QString FollowUpReminderManager::infoToStr(FollowUpReminder::FollowUpReminderInfo *info) const
{
    QString infoStr = QStringLiteral("****************************************");
    infoStr += QStringLiteral("Akonadi Item id :%1\n").arg(info->originalMessageItemId());
    infoStr += QStringLiteral("MessageId :%1\n").arg(info->messageId());
    infoStr += QStringLiteral("Subject :%1\n").arg(info->subject());
    infoStr += QStringLiteral("To :%1\n").arg(info->to());
    infoStr += QStringLiteral("Dead Line :%1\n").arg(info->followUpReminderDate().toString());
    infoStr += QStringLiteral("Answer received :%1\n").arg(info->answerWasReceived() ? QStringLiteral("true") : QStringLiteral("false"));
    infoStr += QStringLiteral("****************************************\n");
    return infoStr;
}
