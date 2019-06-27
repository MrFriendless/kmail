/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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


#include <controlgui.h>

#include <kaboutdata.h>
#include <QApplication>

#include <KDBusService>
#include <KLocalizedString>

#include <KCrash/KCrash>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QIcon>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KLocalizedString::setApplicationDomain("kmail-refresh-settings");
    KCrash::initialize();
    KAboutData aboutData(QStringLiteral("kmail-refresh-settings"),
                         i18n("KMail Assistant for refreshing settings"),
                         QStringLiteral("0.1"),
                         i18n("KMail Assistant for refreshing settings"),
                         KAboutLicense::LGPL,
                         i18n("(c) 2019 Laurent Montel <montel@kde.org>"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Author"), QStringLiteral("montel@kde.org"));

    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kontact")));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    Akonadi::ControlGui::start(nullptr);

//    Dialog dlg(nullptr);
//    dlg.show();
//    // Unregister once the UI is closed, even if the app will continue running
//    // and generating keys in the background.
//    QObject::connect(&dlg, &Dialog::accepted, &service, &KDBusService::unregister);

    return app.exec();
}
