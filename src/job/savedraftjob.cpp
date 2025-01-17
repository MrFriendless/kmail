/*
  Copyright (c) 2014-2019 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "savedraftjob.h"
#include <Akonadi/KMime/MessageStatus>
#include <AkonadiCore/Item>
#include <AkonadiCore/ItemCreateJob>
#include "kmail_debug.h"
#include <Akonadi/KMime/MessageFlags>

SaveDraftJob::SaveDraftJob(const KMime::Message::Ptr &msg, const Akonadi::Collection &col, QObject *parent)
    : KJob(parent)
    , mMsg(msg)
    , mCollection(col)
{
}

SaveDraftJob::~SaveDraftJob()
{
}

void SaveDraftJob::start()
{
    Akonadi::Item item;
    item.setPayload(mMsg);
    item.setMimeType(KMime::Message::mimeType());
    item.setFlag(Akonadi::MessageFlags::Seen);
    Akonadi::MessageFlags::copyMessageFlags(*mMsg, item);

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(item, mCollection);
    connect(createJob, &Akonadi::ItemCreateJob::result, this, &SaveDraftJob::slotStoreDone);
}

void SaveDraftJob::slotStoreDone(KJob *job)
{
    if (job->error()) {
        qCDebug(KMAIL_LOG) << " job->errorString() : " << job->errorString();
        setError(job->error());
        setErrorText(job->errorText());
    }
    emitResult();
}
