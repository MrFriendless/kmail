/*
   Copyright (C) 2009-2019 Montel Laurent <montel@kde.org>

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

#ifndef COLLECTIONTEMPLATESPAGE_H
#define COLLECTIONTEMPLATESPAGE_H
#include <AkonadiWidgets/collectionpropertiespage.h>

class QCheckBox;
namespace TemplateParser {
class TemplatesConfiguration;
}

template<typename T> class QSharedPointer;

class CollectionTemplatesPage : public Akonadi::CollectionPropertiesPage
{
    Q_OBJECT
public:
    explicit CollectionTemplatesPage(QWidget *parent = nullptr);
    ~CollectionTemplatesPage() override;

    void load(const Akonadi::Collection &col) override;
    void save(Akonadi::Collection &col) override;
    bool canHandle(const Akonadi::Collection &collection) const override;

private:
    void slotCopyGlobal();
    void slotChanged();
    void init();
    QCheckBox *mCustom = nullptr;
    TemplateParser::TemplatesConfiguration *mWidget = nullptr;
    QString mCollectionId;
    uint mIdentity = 0;
    bool mChanged = false;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionTemplatesPageFactory, CollectionTemplatesPage)

#endif /* COLLECTIONTEMPLATESPAGE_H */
