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
#include "folderarchiveagentcheckcollection.h"
#include "kmail_debug.h"
#include "folderarchiveaccountinfo.h"

#include <KLocalizedString>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionCreateJob>
#include <QDate>

FolderArchiveAgentCheckCollection::FolderArchiveAgentCheckCollection(FolderArchiveAccountInfo *info, QObject *parent)
    : QObject(parent)
    , mCurrentDate(QDate::currentDate())
    , mInfo(info)
{
}

FolderArchiveAgentCheckCollection::~FolderArchiveAgentCheckCollection()
{
}

void FolderArchiveAgentCheckCollection::start()
{
    Akonadi::Collection col(mInfo->archiveTopLevel());
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(col, Akonadi::CollectionFetchJob::FirstLevel);
    connect(job, &Akonadi::CollectionFetchJob::result, this, &FolderArchiveAgentCheckCollection::slotInitialCollectionFetchingFirstLevelDone);
}

void FolderArchiveAgentCheckCollection::slotInitialCollectionFetchingFirstLevelDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KMAIL_LOG) << job->errorString();
        Q_EMIT checkFailed(i18n("Cannot fetch collection. %1", job->errorString()));
        return;
    }

    QString folderName;
    switch (mInfo->folderArchiveType()) {
    case FolderArchiveAccountInfo::UniqueFolder:
        //Nothing
        break;
    case FolderArchiveAccountInfo::FolderByMonths:
        //TODO translate ?
        folderName = QStringLiteral("%1-%2").arg(mCurrentDate.month()).arg(mCurrentDate.year());
        break;
    case FolderArchiveAccountInfo::FolderByYears:
        folderName = QStringLiteral("%1").arg(mCurrentDate.year());
        break;
    }

    if (folderName.isEmpty()) {
        Q_EMIT checkFailed(i18n("Folder name not defined."));
        return;
    }

    Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);

    const Akonadi::Collection::List cols = fetchJob->collections();
    for (const Akonadi::Collection &collection : cols) {
        if (collection.name() == folderName) {
            Q_EMIT collectionIdFound(collection);
            return;
        }
    }
    createNewFolder(folderName);
}

void FolderArchiveAgentCheckCollection::createNewFolder(const QString &name)
{
    Akonadi::Collection parentCollection(mInfo->archiveTopLevel());
    Akonadi::Collection collection;
    collection.setParentCollection(parentCollection);
    collection.setName(name);
    collection.setContentMimeTypes(QStringList() << QStringLiteral("message/rfc822"));

    Akonadi::CollectionCreateJob *job = new Akonadi::CollectionCreateJob(collection);
    connect(job, &Akonadi::CollectionCreateJob::result, this, &FolderArchiveAgentCheckCollection::slotCreateNewFolder);
}

void FolderArchiveAgentCheckCollection::slotCreateNewFolder(KJob *job)
{
    if (job->error()) {
        qCWarning(KMAIL_LOG) << job->errorString();
        Q_EMIT checkFailed(i18n("Unable to create folder. %1", job->errorString()));
        return;
    }
    Akonadi::CollectionCreateJob *createJob = qobject_cast<Akonadi::CollectionCreateJob *>(job);
    Q_EMIT collectionIdFound(createJob->collection());
}
