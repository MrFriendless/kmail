/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2013-2019 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMREADERWIN_H
#define KMREADERWIN_H

#include "kmail_export.h"

#include <QWidget>
#include <QUrl>
#include <MessageViewer/Viewer>
#include <MimeTreeParser/BodyPart>
#include <AkonadiCore/Item>
#include <KContacts/Addressee>
class KActionCollection;
class QAction;
class KToggleAction;
class QMenu;
namespace MessageViewer {
class CSSHelper;
}

namespace MimeTreeParser {
class AttachmentStrategy;
}

class KJob;

/**
   This class implements a "reader window", that is a window
   used for reading or viewing messages.
*/

class KMAIL_EXPORT KMReaderWin : public QWidget
{
    Q_OBJECT

public:
    explicit KMReaderWin(QWidget *parent, QWidget *mainWindow, KActionCollection *actionCollection);
    ~KMReaderWin() override;

    /** Read settings from app's config file. */
    void readConfig();

    /** Get/set the message attachment strategy. */
    const MessageViewer::AttachmentStrategy *attachmentStrategy() const;

    void setAttachmentStrategy(const MessageViewer::AttachmentStrategy *strategy);

    /** Get selected override character encoding.
      @return The encoding selected by the user or an empty string if auto-detection
      is selected. */
    QString overrideEncoding() const;
    /** Set the override character encoding. */
    void setOverrideEncoding(const QString &encoding);
    void setPrinting(bool enable);

    void setMessage(const Akonadi::Item &item, MimeTreeParser::UpdateMode updateMode = MimeTreeParser::Delayed);

    void setMessage(const KMime::Message::Ptr &message);

    /** Instead of settings a message to be shown sets a message part
      to be shown */
    void setMsgPart(KMime::Content *aMsgPart);

    /** Clear the reader and discard the current message. */
    void clear(bool force = false);

    void update(bool force = false);

    /** Return selected text */
    QString copyText() const;

    /** Override default html mail setting */
    bool htmlOverride() const;
    void setHtmlOverride(bool override);
    MessageViewer::Viewer::DisplayFormatMessage displayFormatMessageOverwrite() const;
    void setDisplayFormatMessageOverwrite(MessageViewer::Viewer::DisplayFormatMessage format);

    /** Override default load external references setting */
    bool htmlLoadExtOverride() const;
    void setHtmlLoadExtDefault(bool loadExtDefault);
    void setHtmlLoadExtOverride(bool loadExtOverride);

    /** Is html mail to be supported? Takes into account override */
    bool htmlMail() const;

    /** Is loading ext. references to be supported? Takes into account override */
    bool htmlLoadExternal();

    /** Returns the MD5 hash for the list of new features */
    static QString newFeaturesMD5();

    /** Display a generic HTML splash page instead of a message */
    void displaySplashPage(const QString &templateName, const QVariantHash &data);

    /** Display the about page instead of a message */
    void displayAboutPage();

    /** Display the 'please wait' page instead of a message */
    void displayBusyPage();
    /** Display the 'we are currently in offline mode' page instead of a message */
    void displayOfflinePage();

    void displayResourceOfflinePage();

    bool isFixedFont() const;
    void setUseFixedFont(bool useFixedFont);
    MessageViewer::Viewer *viewer() const;
    KToggleAction *toggleFixFontAction() const;
    QAction *mailToComposeAction() const;
    QAction *mailToReplyAction() const;
    QAction *mailToForwardAction() const;
    QAction *addAddrBookAction() const;
    QAction *openAddrBookAction() const;
    QAction *copyAction() const;
    QAction *selectAllAction() const;
    QAction *copyURLAction() const;
    QAction *copyImageLocation() const;
    QAction *urlOpenAction() const;
    QAction *urlSaveAsAction() const;
    QAction *addBookmarksAction() const;
    QAction *toggleMimePartTreeAction() const;
    QAction *speakTextAction() const;
    QAction *downloadImageToDiskAction() const;
    QAction *viewSourceAction() const;
    QAction *findInMessageAction() const;
    QAction *saveAsAction() const;
    QAction *saveMessageDisplayFormatAction() const;
    QAction *resetMessageDisplayFormatAction() const;
    QAction *editContactAction() const;

    QMenu *viewHtmlOption() const;
    QAction *shareImage() const;

    QAction *addToExistingContactAction() const;

    Akonadi::Item message() const;

    QWidget *mainWindow() const;

    /** Enforce message decryption. */
    void setDecryptMessageOverwrite(bool overwrite = true);

    MessageViewer::CSSHelper *cssHelper() const;

    bool printSelectedText(bool preview);

    void setContactItem(const Akonadi::Item &contact, const KContacts::Addressee &address);
    void clearContactItem();

    bool mimePartTreeIsEmpty() const;
    KActionMenu *shareServiceUrlMenu() const;

    QList<QAction *> viewerPluginActionList(MessageViewer::ViewerPluginInterface::SpecificFeatureTypes features);

    QList<QAction *> interceptorUrlActions(const WebEngineViewer::WebHitTestResult &result) const;

    void setPrintElementBackground(bool printElementBackground);
    /** Force update even if message is the same */
    void clearCache();

    void hasMultiMessages(bool multi);

    void updateShowMultiMessagesButton(bool enablePreviousButton, bool enableNextButton);
Q_SIGNALS:
    void showStatusBarMessage(const QString &message);
    void zoomChanged(qreal factor);
    void showPreviousMessage();
    void showNextMessage();

public Q_SLOTS:

    /** The user selected "Find" from the menu. */
    void slotFind();
    /** Copy the selected text to the clipboard */
    void slotCopySelectedText();
    /** Operations on mailto: URLs. */
    void slotMailtoReply();
    void slotMailtoCompose();
    void slotMailtoForward();
    void slotMailtoAddAddrBook();
    void slotMailtoOpenAddrBook();
    /** Save the page to a file */
    void slotUrlSave();
    void slotAddBookmarks();
    void slotUrlClicked(const Akonadi::Item &, const QUrl &);
    void slotShowReader(KMime::Content *, bool html, const QString &);
    void slotShowMessage(const KMime::Message::Ptr &message, const QString &encoding);
    void slotDeleteMessage(const Akonadi::Item &);
    void slotSaveImageOnDisk();

    void slotPrintComposeResult(KJob *job);
    void slotEditContact();
    void contactStored(const Akonadi::Item &item);
    void slotContactEditorError(const QString &error);

    void slotContactHtmlOptions();
    void slotShareImage();
    void slotMailToAddToExistingContact();
    void slotPrintingFinished();

protected:
    QUrl urlClicked() const;
    QUrl imageUrlClicked() const;

private:
    void createActions();
    void updateHtmlActions();

private:
    KContacts::Addressee mSearchedAddress;
    Akonadi::Item mSearchedContact;
    QWidget *mMainWindow = nullptr;
    KActionCollection *mActionCollection = nullptr;

    QAction *mMailToComposeAction = nullptr;
    QAction *mMailToReplyAction = nullptr;
    QAction *mMailToForwardAction = nullptr;
    QAction *mAddAddrBookAction = nullptr;
    QAction *mOpenAddrBookAction = nullptr;
    QAction *mUrlSaveAsAction = nullptr;
    QAction *mAddBookmarksAction = nullptr;
    QAction *mImageUrlSaveAsAction = nullptr;
    QAction *mEditContactAction = nullptr;
    QAction *mViewAsHtml = nullptr;
    QAction *mLoadExternalReference = nullptr;
    QAction *mShareImage = nullptr;
    QAction *mAddEmailToExistingContactAction = nullptr;

    QMenu *mViewHtmlOptions = nullptr;

    MessageViewer::Viewer *mViewer = nullptr;
};

#endif
