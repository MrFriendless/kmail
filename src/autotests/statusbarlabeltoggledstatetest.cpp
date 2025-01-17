/*
  Copyright (c) 2014-2019 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "statusbarlabeltoggledstatetest.h"
#include "../widgets/statusbarlabeltoggledstate.h"
#include <QTest>
#include <qtestmouse.h>
#include <QSignalSpy>

StatusBarLabelToggledStateTest::StatusBarLabelToggledStateTest(QObject *parent)
    : QObject(parent)
{
}

StatusBarLabelToggledStateTest::~StatusBarLabelToggledStateTest()
{
}

void StatusBarLabelToggledStateTest::shouldHasDefaultValue()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    QVERIFY(!widget.toggleMode());
}

void StatusBarLabelToggledStateTest::shouldChangeState()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    widget.setToggleMode(true);
    QVERIFY(widget.toggleMode());
    QVERIFY(!widget.text().isEmpty());

    widget.setToggleMode(true);
    QVERIFY(widget.toggleMode());

    widget.setToggleMode(false);
    QVERIFY(!widget.toggleMode());
}

void StatusBarLabelToggledStateTest::shouldEmitSignalWhenChangeState()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    QSignalSpy spy(&widget, &StatusBarLabelToggledState::toggleModeChanged);
    widget.setToggleMode(true);
    QCOMPARE(spy.count(), 1);

    widget.setToggleMode(false);
    QCOMPARE(spy.count(), 2);
}

void StatusBarLabelToggledStateTest::shouldNotEmitSignalWhenWeDontChangeState()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    QSignalSpy spy(&widget, &StatusBarLabelToggledState::toggleModeChanged);
    widget.setToggleMode(false);
    QCOMPARE(spy.count(), 0);

    widget.setToggleMode(true);
    QCOMPARE(spy.count(), 1);

    widget.setToggleMode(true);
    QCOMPARE(spy.count(), 1);
}

void StatusBarLabelToggledStateTest::shouldEmitSignalWhenClickOnLabel()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    QSignalSpy spy(&widget, &StatusBarLabelToggledState::toggleModeChanged);
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));
    QTest::mouseClick(&widget, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);

    QTest::mouseClick(&widget, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
}

void StatusBarLabelToggledStateTest::shouldChangeTestWhenStateChanged()
{
    StatusBarLabelToggledState widget;
    widget.setStateString(QStringLiteral("toggle"), QStringLiteral("untoggle"));
    const QString initialText = widget.text();
    widget.setToggleMode(true);
    const QString newText = widget.text();
    QVERIFY(initialText != newText);

    widget.setToggleMode(false);
    QCOMPARE(widget.text(), initialText);

    widget.setToggleMode(true);
    QCOMPARE(widget.text(), newText);
}

QTEST_MAIN(StatusBarLabelToggledStateTest)
