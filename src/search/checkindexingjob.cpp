/*
   Copyright (C) 2016-2019 Montel Laurent <montel@kde.org>

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

#include "checkindexingjob.h"
#include "kmail_debug.h"
#include <AkonadiSearch/PIM/indexeditems.h>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/CollectionStatistics>

#include <PimCommon/PimUtil>

CheckIndexingJob::CheckIndexingJob(Akonadi::Search::PIM::IndexedItems *indexedItems, QObject *parent)
    : QObject(parent)
    , mIndexedItems(indexedItems)
{
}

CheckIndexingJob::~CheckIndexingJob()
{
}

void CheckIndexingJob::askForNextCheck(quint64 id, bool needToReindex)
{
    Q_EMIT finished(id, needToReindex);
    deleteLater();
}

void CheckIndexingJob::setCollection(const Akonadi::Collection &col)
{
    mCollection = col;
}

void CheckIndexingJob::start()
{
    if (mCollection.isValid()) {
        Akonadi::CollectionFetchJob *fetch = new Akonadi::CollectionFetchJob(mCollection,
                                                                             Akonadi::CollectionFetchJob::Base);
        fetch->fetchScope().setIncludeStatistics(true);
        connect(fetch, &KJob::result, this, &CheckIndexingJob::slotCollectionPropertiesFinished);
    } else {
        qCWarning(KMAIL_LOG) << "Collection was not valid";
        askForNextCheck(-1);
    }
}

void CheckIndexingJob::slotCollectionPropertiesFinished(KJob *job)
{
    Akonadi::CollectionFetchJob *fetch = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    Q_ASSERT(fetch);
    if (fetch->collections().isEmpty()) {
        qCWarning(KMAIL_LOG) << "No collection fetched";
        askForNextCheck(-1);
        return;
    }

    mCollection = fetch->collections().constFirst();
    const qlonglong result = mIndexedItems->indexedItems(mCollection.id());
    bool needToReindex = false;
    qCDebug(KMAIL_LOG) << "name :" << mCollection.name() << " mCollection.statistics().count() " << mCollection.statistics().count() << "stats.value(mCollection.id())" << result;
    if (mCollection.statistics().count() != result) {
        needToReindex = true;
        qCDebug(KMAIL_LOG) << "Reindex collection :" << "name :" << mCollection.name();
    }
    askForNextCheck(mCollection.id(), needToReindex);
}
