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

#ifndef CHECKINDEXINGMANAGER_H
#define CHECKINDEXINGMANAGER_H

#include <QObject>
#include <AkonadiCore/Collection>
#include <QAbstractItemModel>
namespace Akonadi {
namespace Search {
namespace PIM {
class IndexedItems;
}
}
}
class QTimer;
class CheckIndexingManager : public QObject
{
    Q_OBJECT
public:
    explicit CheckIndexingManager(Akonadi::Search::PIM::IndexedItems *indexer, QObject *parent = nullptr);
    ~CheckIndexingManager();

    void start(QAbstractItemModel *collectionModel);

private:
    Q_DISABLE_COPY(CheckIndexingManager)
    void checkNextCollection();

    void indexingFinished(qint64 index, bool reindexCollection);

    void initializeCollectionList(QAbstractItemModel *model, const QModelIndex &parentIndex = QModelIndex());
    void createJob();
    void callToReindexCollection();

    Akonadi::Search::PIM::IndexedItems *mIndexedItems = nullptr;
    Akonadi::Collection::List mListCollection;
    QTimer *mTimer = nullptr;
    QList<qint64> mCollectionsIndexed;
    QList<qint64> mCollectionsNeedToBeReIndexed;
    int mIndex = 0;
    bool mIsReady = true;
};

#endif // CHECKINDEXINGMANAGER_H
