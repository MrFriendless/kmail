/*
  Copyright (c) 2015-2019 Montel Laurent <montel@kde.org>

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

#include "potentialphishingdetaildialogtest.h"
#include "../potentialphishingdetaildialog.h"
#include "../potentialphishingdetailwidget.h"
#include <QTest>
#include <QStandardPaths>
PotentialPhishingDetailDialogTest::PotentialPhishingDetailDialogTest(QObject *parent)
    : QObject(parent)
{
}

PotentialPhishingDetailDialogTest::~PotentialPhishingDetailDialogTest()
{
}

void PotentialPhishingDetailDialogTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void PotentialPhishingDetailDialogTest::shouldHaveDefaultValue()
{
    PotentialPhishingDetailDialog dlg;
    PotentialPhishingDetailWidget *w = dlg.findChild<PotentialPhishingDetailWidget *>(QStringLiteral("potentialphising_widget"));
    QVERIFY(w);
}

QTEST_MAIN(PotentialPhishingDetailDialogTest)
