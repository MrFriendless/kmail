/*
   Copyright (C) 2012-2019 Montel Laurent <montel@kde.org>

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
#ifndef ARCHIVEMAILINFO_H
#define ARCHIVEMAILINFO_H

#include "MailCommon/BackupJob"
#include <KConfigGroup>
#include <Collection>
#include <QUrl>
#include <QDate>

class ArchiveMailInfo
{
public:
    explicit ArchiveMailInfo();
    explicit ArchiveMailInfo(const KConfigGroup &config);
    ArchiveMailInfo(const ArchiveMailInfo &info);
    ~ArchiveMailInfo();

    ArchiveMailInfo &operator=(const ArchiveMailInfo &old);

    enum ArchiveUnit {
        ArchiveDays = 0,
        ArchiveWeeks,
        ArchiveMonths,
        ArchiveYears
    };

    QUrl realUrl(const QString &folderName, bool &dirExist) const;

    bool isValid() const;

    Akonadi::Collection::Id saveCollectionId() const;
    void setSaveCollectionId(Akonadi::Collection::Id collectionId);

    void setSaveSubCollection(bool b);
    bool saveSubCollection() const;

    void setUrl(const QUrl &url);
    QUrl url() const;

    void readConfig(const KConfigGroup &config);
    void writeConfig(KConfigGroup &config);

    void setArchiveType(MailCommon::BackupJob::ArchiveType type);
    MailCommon::BackupJob::ArchiveType archiveType() const;

    void setArchiveUnit(ArchiveMailInfo::ArchiveUnit unit);
    ArchiveMailInfo::ArchiveUnit archiveUnit() const;

    void setArchiveAge(int age);
    int archiveAge() const;

    void setLastDateSaved(const QDate &date);
    QDate lastDateSaved() const;

    int maximumArchiveCount() const;
    void setMaximumArchiveCount(int max);

    QStringList listOfArchive(const QString &foldername, bool &dirExist) const;

    bool isEnabled() const;
    void setEnabled(bool b);

    bool operator ==(const ArchiveMailInfo &other) const;

private:
    QString dirArchive(bool &dirExit) const;
    QDate mLastDateSaved;
    int mArchiveAge = 1;
    MailCommon::BackupJob::ArchiveType mArchiveType = MailCommon::BackupJob::Zip;
    ArchiveUnit mArchiveUnit = ArchiveMailInfo::ArchiveDays;
    Akonadi::Collection::Id mSaveCollectionId = -1;
    QUrl mPath;
    int mMaximumArchiveCount = 0;
    bool mSaveSubCollection = false;
    bool mIsEnabled = true;
};

#endif // ARCHIVEMAILINFO_H
