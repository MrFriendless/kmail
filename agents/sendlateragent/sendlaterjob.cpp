/*
   Copyright (C) 2013-2019 Montel Laurent <montel@kde.org>

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

#include "sendlaterjob.h"
#include "sendlaterinfo.h"

#include "MessageComposer/AkonadiSender"
#include <MessageComposer/Util>
#include <MessageCore/StringUtil>

#include <mailtransportakonadi/transportattribute.h>
#include <mailtransportakonadi/sentbehaviourattribute.h>
#include <mailtransport/transport.h>
#include <mailtransport/transportmanager.h>

#include <ItemFetchJob>
#include <ItemDeleteJob>

#include <KNotification>
#include <KLocalizedString>
#include "sendlateragent_debug.h"

SendLaterJob::SendLaterJob(SendLaterManager *manager, SendLater::SendLaterInfo *info, QObject *parent)
    : QObject(parent)
    , mManager(manager)
    , mInfo(info)
{
    qCDebug(SENDLATERAGENT_LOG) << " SendLaterJob::SendLaterJob" << this;
}

SendLaterJob::~SendLaterJob()
{
    qCDebug(SENDLATERAGENT_LOG) << " SendLaterJob::~SendLaterJob()" << this;
}

void SendLaterJob::start()
{
    if (mInfo) {
        if (mInfo->itemId() > -1) {
            const Akonadi::Item item = Akonadi::Item(mInfo->itemId());
            Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob(item, this);
            mFetchScope.fetchAttribute<MailTransport::TransportAttribute>();
            mFetchScope.fetchAttribute<MailTransport::SentBehaviourAttribute>();
            mFetchScope.setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
            mFetchScope.fetchFullPayload(true);
            fetch->setFetchScope(mFetchScope);
            connect(fetch, &Akonadi::ItemFetchJob::itemsReceived, this, &SendLaterJob::slotMessageTransfered);
            connect(fetch, &Akonadi::ItemFetchJob::result, this, &SendLaterJob::slotJobFinished);
            fetch->start();
        } else {
            qCDebug(SENDLATERAGENT_LOG) << " message Id is invalid";
            sendError(i18n("Not message found."), SendLaterManager::ItemNotFound);
        }
    } else {
        sendError(i18n("Not message found."), SendLaterManager::UnknownError);
        qCDebug(SENDLATERAGENT_LOG) << " Item is null. It's a bug!";
    }
}

void SendLaterJob::slotMessageTransfered(const Akonadi::Item::List &items)
{
    if (items.isEmpty()) {
        sendError(i18n("No message found."), SendLaterManager::ItemNotFound);
        qCDebug(SENDLATERAGENT_LOG) << " slotMessageTransfered failed !";
        return;
    } else if (items.count() == 1) {
        //Success
        mItem = items.first();
        return;
    }
    qCDebug(SENDLATERAGENT_LOG) << "Error during fetching message.";
    sendError(i18n("Error during fetching message."), SendLaterManager::TooManyItemFound);
}

void SendLaterJob::slotJobFinished(KJob *job)
{
    if (job->error()) {
        sendError(i18n("Cannot fetch message. %1", job->errorString()), SendLaterManager::CanNotFetchItem);
        return;
    }
    if (!MailTransport::TransportManager::self()->showTransportCreationDialog(nullptr, MailTransport::TransportManager::IfNoTransportExists)) {
        qCDebug(SENDLATERAGENT_LOG) << " we can't create transport ";
        sendError(i18n("We can't create transport"), SendLaterManager::CanNotCreateTransport);
        return;
    }

    if (mItem.isValid()) {
        const KMime::Message::Ptr msg = MessageComposer::Util::message(mItem);
        if (!msg) {
            sendError(i18n("Message is not a real message"), SendLaterManager::CanNotFetchItem);
            return;
        }
        //TODO verify encryption + signed
        updateAndCleanMessageBeforeSending(msg);

        if (!mManager->sender()->send(msg, MessageComposer::MessageSender::SendImmediate)) {
            sendError(i18n("Cannot send message."), SendLaterManager::MailDispatchDoesntWork);
        } else {
            if (!mInfo->isRecurrence()) {
                Akonadi::ItemDeleteJob *fetch = new Akonadi::ItemDeleteJob(mItem, this);
                connect(fetch, &Akonadi::ItemDeleteJob::result, this, &SendLaterJob::slotDeleteItem);
            } else {
                sendDone();
            }
        }
    }
}

void SendLaterJob::updateAndCleanMessageBeforeSending(const KMime::Message::Ptr &msg)
{
    msg->date()->setDateTime(QDateTime::currentDateTime());
    MessageComposer::Util::removeNotNecessaryHeaders(msg);
    msg->assemble();
}

void SendLaterJob::slotDeleteItem(KJob *job)
{
    if (job->error()) {
        qCDebug(SENDLATERAGENT_LOG) << " void SendLaterJob::slotDeleteItem( KJob *job ) :" << job->errorString();
    }
    sendDone();
}

void SendLaterJob::sendDone()
{
    KNotification::event(QStringLiteral("mailsend"),
                         QString(),
                         i18n("Message sent"),
                         QStringLiteral("kmail"),
                         nullptr,
                         KNotification::CloseOnTimeout,
                         QStringLiteral("akonadi_sendlater_agent"));
    mManager->sendDone(mInfo);
    deleteLater();
}

void SendLaterJob::sendError(const QString &error, SendLaterManager::ErrorType type)
{
    KNotification::event(QStringLiteral("mailsendfailed"),
                         QString(),
                         error,
                         QStringLiteral("kmail"),
                         nullptr,
                         KNotification::CloseOnTimeout,
                         QStringLiteral("akonadi_sendlater_agent"));
    mManager->sendError(mInfo, type);
    deleteLater();
}
