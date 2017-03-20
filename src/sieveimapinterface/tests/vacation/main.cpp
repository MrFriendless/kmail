/*
  Copyright (c) 2013-2017 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>
#include <qapplication.h>
#include <QCommandLineParser>
#include <kaboutdata.h>
#include <QStandardPaths>

#include <KSieveUi/MultiImapVacationDialog>
#include <KSieveUi/MultiImapVacationManager>
#include <KSieveUi/SieveImapInstanceInterfaceManager>
#include "../../../sieveimapinterface/kmailsieveimapinstanceinterface.h"
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("vacationmultiscripttest"),
                         i18n("VacationMultiScriptTest_Gui"),
                         QStringLiteral("1.0"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.setQuitOnLastWindowClosed(true);
    KSieveUi::SieveImapInstanceInterfaceManager::self()->setSieveImapInstanceInterface(new KMailSieveImapInstanceInterface);
    KSieveUi::MultiImapVacationManager manager;
    KSieveUi::MultiImapVacationDialog dlg(&manager);
    QObject::connect(&dlg, &KSieveUi::MultiImapVacationDialog::okClicked, &app, &QApplication::quit);
    QObject::connect(&dlg, &KSieveUi::MultiImapVacationDialog::cancelClicked, &app, &QApplication::quit);

    dlg.show();

    return app.exec();
}