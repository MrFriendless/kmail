/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "messageactions.h"

#include "settings/kmailsettings.h"
#include "kmreaderwin.h"
#include "kmkernel.h"
#include "mailcommon/mailkernel.h"
#include "kmmainwidget.h"
#include "util.h"
#include "kmcommands.h"
#include <TemplateParser/CustomTemplatesMenu>

#include <PimCommonAkonadi/AnnotationDialog>
#include <MessageCore/MessageCoreSettings>
#include "MessageCore/MailingList"
#include "messageviewer/messageviewersettings.h"
#include "messageviewer/headerstyleplugin.h"

#include <AkonadiCore/itemfetchjob.h>
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/ChangeRecorder>
#include <QAction>
#include <AkonadiSearch/Debug/akonadisearchdebugdialog.h>
#include <KIO/KUriFilterSearchProviderActions>

#include "messagecomposer/followupreminderselectdatedialog.h"
#include "job/createfollowupreminderonexistingmessagejob.h"

#include <AkonadiCore/ItemFetchJob>
#include <KActionMenu>
#include <KActionCollection>
#include "kmail_debug.h"
#include <KLocalizedString>
#include <KXMLGUIClient>
#include <KRun>
#include <QMenu>
#include <KUriFilter>
#include <KStringHandler>
#include <QIcon>

#include <QVariant>
#include <QWidget>
#include <AkonadiCore/collection.h>
#include <AkonadiCore/entityannotationsattribute.h>
#include <mailcommon/mailutil.h>

using namespace KMail;

MessageActions::MessageActions(KActionCollection *ac, QWidget *parent)
    : QObject(parent)
    , mParent(parent)
{
    mWebShortcutMenuManager = new KIO::KUriFilterSearchProviderActions(this);
    mReplyActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18nc("Message->", "&Reply"), this);
    ac->addAction(QStringLiteral("message_reply_menu"), mReplyActionMenu);
    connect(mReplyActionMenu, &KActionMenu::triggered, this, &MessageActions::slotReplyToMsg);

    mReplyAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18n("&Reply..."), this);
    ac->addAction(QStringLiteral("reply"), mReplyAction);
    ac->setDefaultShortcut(mReplyAction, Qt::Key_R);
    connect(mReplyAction, &QAction::triggered, this, &MessageActions::slotReplyToMsg);
    mReplyActionMenu->addAction(mReplyAction);

    mReplyAuthorAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18n("Reply to A&uthor..."), this);
    ac->addAction(QStringLiteral("reply_author"), mReplyAuthorAction);
    ac->setDefaultShortcut(mReplyAuthorAction, Qt::SHIFT + Qt::Key_A);
    connect(mReplyAuthorAction, &QAction::triggered, this, &MessageActions::slotReplyAuthorToMsg);
    mReplyActionMenu->addAction(mReplyAuthorAction);

    mReplyAllAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-all")), i18n("Reply to &All..."), this);
    ac->addAction(QStringLiteral("reply_all"), mReplyAllAction);
    ac->setDefaultShortcut(mReplyAllAction, Qt::Key_A);
    connect(mReplyAllAction, &QAction::triggered, this, &MessageActions::slotReplyAllToMsg);
    mReplyActionMenu->addAction(mReplyAllAction);

    mReplyListAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-list")), i18n("Reply to Mailing-&List..."), this);
    ac->addAction(QStringLiteral("reply_list"), mReplyListAction);

    ac->setDefaultShortcut(mReplyListAction, Qt::Key_L);
    connect(mReplyListAction, &QAction::triggered, this, &MessageActions::slotReplyListToMsg);
    mReplyActionMenu->addAction(mReplyListAction);

    mNoQuoteReplyAction = new QAction(i18n("Reply Without &Quote..."), this);
    ac->addAction(QStringLiteral("noquotereply"), mNoQuoteReplyAction);
    ac->setDefaultShortcut(mNoQuoteReplyAction, Qt::SHIFT + Qt::Key_R);
    connect(mNoQuoteReplyAction, &QAction::triggered, this, &MessageActions::slotNoQuoteReplyToMsg);

    mListFilterAction = new QAction(i18n("Filter on Mailing-&List..."), this);
    ac->addAction(QStringLiteral("mlist_filter"), mListFilterAction);
    connect(mListFilterAction, &QAction::triggered, this, &MessageActions::slotMailingListFilter);

    mStatusMenu = new KActionMenu(i18n("Mar&k Message"), this);
    ac->addAction(QStringLiteral("set_status"), mStatusMenu);

    KMMainWidget *mainwin = kmkernel->getKMMainWidget();
    if (mainwin) {
        QAction *action = mainwin->akonadiStandardAction(Akonadi::StandardMailActionManager::MarkMailAsRead);
        mStatusMenu->addAction(action);

        action = mainwin->akonadiStandardAction(Akonadi::StandardMailActionManager::MarkMailAsUnread);
        mStatusMenu->addAction(action);

        mStatusMenu->addSeparator();
        action = mainwin->akonadiStandardAction(Akonadi::StandardMailActionManager::MarkMailAsImportant);
        mStatusMenu->addAction(action);

        action = mainwin->akonadiStandardAction(Akonadi::StandardMailActionManager::MarkMailAsActionItem);
        mStatusMenu->addAction(action);
    }

    mAnnotateAction = new QAction(QIcon::fromTheme(QStringLiteral("view-pim-notes")), i18n("Add Note..."), this);
    ac->addAction(QStringLiteral("annotate"), mAnnotateAction);
    connect(mAnnotateAction, &QAction::triggered, this, &MessageActions::annotateMessage);

    mEditAsNewAction = new QAction(QIcon::fromTheme(QStringLiteral("accessories-text-editor")), i18n("&Edit As New"), this);
    ac->addAction(QStringLiteral("editasnew"), mEditAsNewAction);
    connect(mEditAsNewAction, &QAction::triggered, this, &MessageActions::editCurrentMessage);
    ac->setDefaultShortcut(mEditAsNewAction, Qt::Key_T);

    mPrintAction = KStandardAction::print(this, &MessageActions::slotPrintMessage, ac);
    mPrintPreviewAction = KStandardAction::printPreview(this, &MessageActions::slotPrintPreviewMsg, ac);

    mForwardActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-forward")), i18nc("Message->", "&Forward"), this);
    ac->addAction(QStringLiteral("message_forward"), mForwardActionMenu);

    mForwardAttachedAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-forward")),
                                         i18nc("@action:inmenu Message->Forward->",
                                               "As &Attachment..."),
                                         this);
    connect(mForwardAttachedAction, SIGNAL(triggered(bool)), parent, SLOT(slotForwardAttachedMessage()));

    ac->addAction(QStringLiteral("message_forward_as_attachment"), mForwardAttachedAction);

    mForwardInlineAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-forward")),
                                       i18nc("@action:inmenu Message->Forward->",
                                             "&Inline..."),
                                       this);
    connect(mForwardInlineAction, SIGNAL(triggered(bool)), parent, SLOT(slotForwardInlineMsg()));

    ac->addAction(QStringLiteral("message_forward_inline"), mForwardInlineAction);

    setupForwardActions(ac);

    mRedirectAction = new QAction(i18nc("Message->Forward->", "&Redirect..."), this);
    ac->addAction(QStringLiteral("message_forward_redirect"), mRedirectAction);
    connect(mRedirectAction, SIGNAL(triggered(bool)), parent, SLOT(slotRedirectMessage()));

    ac->setDefaultShortcut(mRedirectAction, QKeySequence(Qt::Key_E));
    mForwardActionMenu->addAction(mRedirectAction);

    mMailingListActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail-message-new-list")), i18nc("Message->", "Mailing-&List"), this);
    connect(mMailingListActionMenu->menu(), &QMenu::triggered, this, &MessageActions::slotRunUrl);
    ac->addAction(QStringLiteral("mailing_list"), mMailingListActionMenu);
    mMailingListActionMenu->setEnabled(false);

    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::itemChanged, this, &MessageActions::slotItemModified);
    connect(kmkernel->folderCollectionMonitor(), &Akonadi::Monitor::itemRemoved, this, &MessageActions::slotItemRemoved);

    mCustomTemplatesMenu = new TemplateParser::CustomTemplatesMenu(parent, ac);

    connect(mCustomTemplatesMenu, SIGNAL(replyTemplateSelected(QString)), parent, SLOT(slotCustomReplyToMsg(QString)));
    connect(mCustomTemplatesMenu, SIGNAL(replyAllTemplateSelected(QString)), parent, SLOT(slotCustomReplyAllToMsg(QString)));
    connect(mCustomTemplatesMenu, SIGNAL(forwardTemplateSelected(QString)), parent, SLOT(slotCustomForwardMsg(QString)));
    connect(KMKernel::self(), &KMKernel::customTemplatesChanged, mCustomTemplatesMenu, &TemplateParser::CustomTemplatesMenu::update);

    forwardMenu()->addSeparator();
    forwardMenu()->addAction(mCustomTemplatesMenu->forwardActionMenu());
    replyMenu()->addSeparator();
    replyMenu()->addAction(mCustomTemplatesMenu->replyActionMenu());
    replyMenu()->addAction(mCustomTemplatesMenu->replyAllActionMenu());

    //Don't translate it. Shown only when we set env variable AKONADI_SEARCH_DEBUG
    mDebugAkonadiSearchAction = new QAction(QStringLiteral("Debug Akonadi Search..."), this);
    connect(mDebugAkonadiSearchAction, &QAction::triggered, this, &MessageActions::slotDebugAkonadiSearch);

    mAddFollowupReminderAction = new QAction(i18n("Add Followup Reminder..."), this);
    ac->addAction(QStringLiteral("message_followup_reminder"), mAddFollowupReminderAction);
    connect(mAddFollowupReminderAction, &QAction::triggered, this, &MessageActions::slotAddFollowupReminder);

    mSendAgainAction = new QAction(i18n("Send A&gain..."), this);
    ac->addAction(QStringLiteral("send_again"), mSendAgainAction);
    connect(mSendAgainAction, &QAction::triggered, this, &MessageActions::slotResendMessage);

    mNewMessageFromTemplateAction = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("New Message From &Template"), this);
    ac->addAction(QStringLiteral("use_template"), mNewMessageFromTemplateAction);
    connect(mNewMessageFromTemplateAction, &QAction::triggered, this, &MessageActions::slotUseTemplate);
    ac->setDefaultShortcut(mNewMessageFromTemplateAction, QKeySequence(Qt::SHIFT + Qt::Key_N));

    updateActions();
}

MessageActions::~MessageActions()
{
    delete mCustomTemplatesMenu;
}

TemplateParser::CustomTemplatesMenu *MessageActions::customTemplatesMenu() const
{
    return mCustomTemplatesMenu;
}

void MessageActions::slotUseTemplate()
{
    if (!mCurrentItem.isValid()) {
        return;
    }
    KMCommand *command = new KMUseTemplateCommand(mParent, mCurrentItem);
    command->start();
}

QAction *MessageActions::editAsNewAction() const
{
    return mEditAsNewAction;
}

void MessageActions::setCurrentMessage(const Akonadi::Item &msg, const Akonadi::Item::List &items)
{
    mCurrentItem = msg;

    if (!items.isEmpty()) {
        if (msg.isValid()) {
            mVisibleItems = items;
        } else {
            mVisibleItems.clear();
        }
    }

    if (!msg.isValid()) {
        mVisibleItems.clear();
        clearMailingListActions();
    }

    updateActions();
}

KActionMenu *MessageActions::replyMenu() const
{
    return mReplyActionMenu;
}

QAction *MessageActions::replyListAction() const
{
    return mReplyListAction;
}

QAction *MessageActions::forwardInlineAction() const
{
    return mForwardInlineAction;
}

QAction *MessageActions::forwardAttachedAction() const
{
    return mForwardAttachedAction;
}

QAction *MessageActions::redirectAction() const
{
    return mRedirectAction;
}

KActionMenu *MessageActions::messageStatusMenu() const
{
    return mStatusMenu;
}

KActionMenu *MessageActions::forwardMenu() const
{
    return mForwardActionMenu;
}

QAction *MessageActions::annotateAction() const
{
    return mAnnotateAction;
}

QAction *MessageActions::printAction() const
{
    return mPrintAction;
}

QAction *MessageActions::printPreviewAction() const
{
    return mPrintPreviewAction;
}

QAction *MessageActions::listFilterAction() const
{
    return mListFilterAction;
}

KActionMenu *MessageActions::mailingListActionMenu() const
{
    return mMailingListActionMenu;
}

void MessageActions::slotItemRemoved(const Akonadi::Item &item)
{
    if (item == mCurrentItem) {
        mCurrentItem = Akonadi::Item();
        updateActions();
    }
}

void MessageActions::slotItemModified(const Akonadi::Item &item, const QSet< QByteArray > &partIdentifiers)
{
    Q_UNUSED(partIdentifiers);
    if (item == mCurrentItem) {
        mCurrentItem = item;
        const int numberOfVisibleItems = mVisibleItems.count();
        for (int i = 0; i < numberOfVisibleItems; ++i) {
            Akonadi::Item it = mVisibleItems.at(i);
            if (item == it) {
                mVisibleItems[i] = item;
            }
        }
        updateActions();
    }
}

void MessageActions::updateActions()
{
    const bool hasPayload = mCurrentItem.hasPayload<KMime::Message::Ptr>();
    bool itemValid = mCurrentItem.isValid();
    Akonadi::Collection parent;
    if (itemValid) { //=> valid
        parent = mCurrentItem.parentCollection();
    }
    if (parent.isValid()) {
        if (CommonKernel->folderIsTemplates(parent)) {
            itemValid = false;
        }
    }

    const bool multiVisible = !mVisibleItems.isEmpty() || mCurrentItem.isValid();
    const bool uniqItem = (itemValid || hasPayload) && (mVisibleItems.count() <= 1);
    mReplyActionMenu->setEnabled(hasPayload);
    mReplyAction->setEnabled(hasPayload);
    mNoQuoteReplyAction->setEnabled(hasPayload);
    mReplyAuthorAction->setEnabled(hasPayload);
    mReplyAllAction->setEnabled(hasPayload);
    mReplyListAction->setEnabled(hasPayload);
    mNoQuoteReplyAction->setEnabled(hasPayload);

    mAnnotateAction->setEnabled(uniqItem);
    mAddFollowupReminderAction->setEnabled(uniqItem);
    if (!mCurrentItem.hasAttribute<Akonadi::EntityAnnotationsAttribute>()) {
        mAnnotateAction->setText(i18n("Add Note..."));
    } else {
        mAnnotateAction->setText(i18n("Edit Note..."));
    }

    mStatusMenu->setEnabled(multiVisible);

    mPrintAction->setEnabled(mMessageView != nullptr);
    mPrintPreviewAction->setEnabled(mMessageView != nullptr);

    if (mCurrentItem.hasPayload<KMime::Message::Ptr>()) {
        if (mCurrentItem.loadedPayloadParts().contains("RFC822")) {
            updateMailingListActions(mCurrentItem);
        } else {
            Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mCurrentItem);
            job->fetchScope().fetchAllAttributes();
            job->fetchScope().fetchFullPayload(true);
            job->fetchScope().fetchPayloadPart(Akonadi::MessagePart::Header);
            job->fetchScope().fetchAttribute<Akonadi::EntityAnnotationsAttribute>();
            connect(job, &Akonadi::ItemFetchJob::result, this, &MessageActions::slotUpdateActionsFetchDone);
        }
    }
    mEditAsNewAction->setEnabled(uniqItem);
}

void MessageActions::slotUpdateActionsFetchDone(KJob *job)
{
    if (job->error()) {
        return;
    }

    Akonadi::ItemFetchJob *fetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    if (fetchJob->items().isEmpty()) {
        return;
    }
    const Akonadi::Item messageItem = fetchJob->items().constFirst();
    if (messageItem == mCurrentItem) {
        mCurrentItem = messageItem;
        updateMailingListActions(messageItem);
    }
}

void MessageActions::clearMailingListActions()
{
    mMailingListActionMenu->setEnabled(false);
    mListFilterAction->setEnabled(false);
    mListFilterAction->setText(i18n("Filter on Mailing-List..."));
}

void MessageActions::updateMailingListActions(const Akonadi::Item &messageItem)
{
    if (!messageItem.hasPayload<KMime::Message::Ptr>()) {
        return;
    }
    KMime::Message::Ptr message = messageItem.payload<KMime::Message::Ptr>();
    const MessageCore::MailingList mailList = MessageCore::MailingList::detect(message);

    if (mailList.features() == MessageCore::MailingList::None) {
        clearMailingListActions();
    } else {
        // A mailing list menu with only a title is pretty boring
        // so make sure theres at least some content
        QString listId;
        if (mailList.features() & MessageCore::MailingList::Id) {
            // From a list-id in the form, "Birds of France <bof.yahoo.com>",
            // take "Birds of France" if it exists otherwise "bof.yahoo.com".
            listId = mailList.id();
            const int start = listId.indexOf(QLatin1Char('<'));
            if (start > 0) {
                listId.truncate(start - 1);
            } else if (start == 0) {
                const int end = listId.lastIndexOf(QLatin1Char('>'));
                if (end < 1) {   // shouldn't happen but account for it anyway
                    listId.remove(0, 1);
                } else {
                    listId = listId.mid(1, end - 1);
                }
            }
        }
        mMailingListActionMenu->menu()->clear();
        qDeleteAll(mMailListActionList);
        mMailListActionList.clear();
        mMailingListActionMenu->menu()->setTitle(KStringHandler::rsqueeze(i18n("Mailing List Name: %1", (listId.isEmpty() ? i18n("<unknown>") : listId)), 40));
        if (mailList.features() & MessageCore::MailingList::ArchivedAt) {
            // IDEA: this may be something you want to copy - "Copy in submenu"?
            addMailingListActions(i18n("Open Message in List Archive"), mailList.archivedAtUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Post) {
            addMailingListActions(i18n("Post New Message"), mailList.postUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Archive) {
            addMailingListActions(i18n("Go to Archive"), mailList.archiveUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Help) {
            addMailingListActions(i18n("Request Help"), mailList.helpUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Owner) {
            addMailingListActions(i18nc("Contact the owner of the mailing list", "Contact Owner"), mailList.ownerUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Subscribe) {
            addMailingListActions(i18n("Subscribe to List"), mailList.subscribeUrls());
        }
        if (mailList.features() & MessageCore::MailingList::Unsubscribe) {
            addMailingListActions(i18n("Unsubscribe from List"), mailList.unsubscribeUrls());
        }
        mMailingListActionMenu->setEnabled(true);

        QByteArray name;
        QString value;
        const QString lname = MailingList::name(message, name, value);
        if (!lname.isEmpty()) {
            mListFilterAction->setEnabled(true);
            mListFilterAction->setText(i18n("Filter on Mailing-List %1...", lname));
        }
    }
}

void MessageActions::replyCommand(MessageComposer::ReplyStrategy strategy)
{
    if (!mCurrentItem.hasPayload<KMime::Message::Ptr>()) {
        return;
    }

    const QString text = mMessageView ? mMessageView->copyText() : QString();
    KMCommand *command = new KMReplyCommand(mParent, mCurrentItem, strategy, text);
    connect(command, &KMCommand::completed, this, &MessageActions::replyActionFinished);
    command->start();
}

void MessageActions::setMessageView(KMReaderWin *msgView)
{
    mMessageView = msgView;
}

void MessageActions::setupForwardActions(KActionCollection *ac)
{
    disconnect(mForwardActionMenu, SIGNAL(triggered(bool)), nullptr, nullptr);
    mForwardActionMenu->removeAction(mForwardInlineAction);
    mForwardActionMenu->removeAction(mForwardAttachedAction);

    if (KMailSettings::self()->forwardingInlineByDefault()) {
        mForwardActionMenu->insertAction(mRedirectAction, mForwardInlineAction);
        mForwardActionMenu->insertAction(mRedirectAction, mForwardAttachedAction);
        ac->setDefaultShortcut(mForwardInlineAction, QKeySequence(Qt::Key_F));
        ac->setDefaultShortcut(mForwardAttachedAction, QKeySequence(Qt::SHIFT + Qt::Key_F));
        QObject::connect(mForwardActionMenu, SIGNAL(triggered(bool)),
                         mParent, SLOT(slotForwardInlineMsg()));
    } else {
        mForwardActionMenu->insertAction(mRedirectAction, mForwardAttachedAction);
        mForwardActionMenu->insertAction(mRedirectAction, mForwardInlineAction);
        ac->setDefaultShortcut(mForwardInlineAction, QKeySequence(Qt::Key_F));
        ac->setDefaultShortcut(mForwardAttachedAction, QKeySequence(Qt::SHIFT + Qt::Key_F));
        QObject::connect(mForwardActionMenu, SIGNAL(triggered(bool)),
                         mParent, SLOT(slotForwardAttachedMessage()));
    }
}

void MessageActions::setupForwardingActionsList(KXMLGUIClient *guiClient)
{
    QList<QAction *> forwardActionList;
    guiClient->unplugActionList(QStringLiteral("forward_action_list"));
    if (KMailSettings::self()->forwardingInlineByDefault()) {
        forwardActionList.append(mForwardInlineAction);
        forwardActionList.append(mForwardAttachedAction);
    } else {
        forwardActionList.append(mForwardAttachedAction);
        forwardActionList.append(mForwardInlineAction);
    }
    forwardActionList.append(mRedirectAction);
    guiClient->plugActionList(QStringLiteral("forward_action_list"), forwardActionList);
}

void MessageActions::slotReplyToMsg()
{
    replyCommand(MessageComposer::ReplySmart);
}

void MessageActions::slotReplyAuthorToMsg()
{
    replyCommand(MessageComposer::ReplyAuthor);
}

void MessageActions::slotReplyListToMsg()
{
    replyCommand(MessageComposer::ReplyList);
}

void MessageActions::slotReplyAllToMsg()
{
    replyCommand(MessageComposer::ReplyAll);
}

void MessageActions::slotNoQuoteReplyToMsg()
{
    if (!mCurrentItem.hasPayload<KMime::Message::Ptr>()) {
        return;
    }
    KMCommand *command = new KMReplyCommand(mParent, mCurrentItem, MessageComposer::ReplySmart, QString(), true);
    command->start();
}

void MessageActions::slotRunUrl(QAction *urlAction)
{
    const QVariant q = urlAction->data();
    if (q.type() == QVariant::Url) {
        new KRun(q.toUrl(), mParent);
    }
}

void MessageActions::slotMailingListFilter()
{
    if (!mCurrentItem.hasPayload<KMime::Message::Ptr>()) {
        return;
    }

    KMCommand *command = new KMMailingListFilterCommand(mParent, mCurrentItem);
    command->start();
}

void MessageActions::printMessage(bool preview)
{
    if (mMessageView) {
        bool result = false;
        if (MessageViewer::MessageViewerSettings::self()->printSelectedText()) {
            result = mMessageView->printSelectedText(preview);
        }
        if (!result) {
            const bool useFixedFont = MessageViewer::MessageViewerSettings::self()->useFixedFont();
            const QString overrideEncoding = MessageCore::MessageCoreSettings::self()->overrideCharacterEncoding();

            const Akonadi::Item message = mCurrentItem;
            KMPrintCommandInfo commandInfo;
            commandInfo.mMsg = message;
            commandInfo.mHeaderStylePlugin = mMessageView->viewer()->headerStylePlugin();
            commandInfo.mFormat = mMessageView->viewer()->displayFormatMessageOverwrite();
            commandInfo.mHtmlLoadExtOverride = mMessageView->viewer()->htmlLoadExternal();
            commandInfo.mPrintPreview = preview;
            commandInfo.mUseFixedFont = useFixedFont;
            commandInfo.mOverrideFont = overrideEncoding;
            commandInfo.mShowSignatureDetails = mMessageView->viewer()->showSignatureDetails() || MessageViewer::MessageViewerSettings::self()->alwaysShowEncryptionSignatureDetails();
            commandInfo.mShowEncryptionDetails = mMessageView->viewer()->showEncryptionDetails() || MessageViewer::MessageViewerSettings::self()->alwaysShowEncryptionSignatureDetails();

            KMPrintCommand *command
                = new KMPrintCommand(mParent, commandInfo);
            command->start();
        }
    } else {
        qCWarning(KMAIL_LOG) << "MessageActions::printMessage impossible to do it if we don't have a viewer";
    }
}

void MessageActions::slotPrintPreviewMsg()
{
    printMessage(true);
}

void MessageActions::slotPrintMessage()
{
    printMessage(false);
}

/**
 * This adds a list of actions to mMailingListActionMenu mapping the identifier item to
 * the url.
 *
 * e.g.: item = "Contact Owner"
 * "Contact Owner (email)" -> KRun( "mailto:bob@arthouseflowers.example.com" )
 * "Contact Owner (web)" -> KRun( "http://arthouseflowers.example.com/contact-owner.php" )
 */
void MessageActions::addMailingListActions(const QString &item, const QList<QUrl> &list)
{
    for (const QUrl &url : list) {
        addMailingListAction(item, url);
    }
}

/**
 * This adds a action to mMailingListActionMenu mapping the identifier item to
 * the url. See addMailingListActions above.
 */
void MessageActions::addMailingListAction(const QString &item, const QUrl &url)
{
    QString protocol = url.scheme().toLower();
    QString prettyUrl = url.toDisplayString();
    if (protocol == QLatin1String("mailto")) {
        protocol = i18n("email");
        prettyUrl.remove(0, 7);   // length( "mailto:" )
    } else if (protocol.startsWith(QLatin1String("http"))) {
        protocol = i18n("web");
    }
    // item is a mailing list url description passed from the updateActions method above.
    QAction *act
        = new QAction(i18nc("%1 is a 'Contact Owner' or similar action. %2 is a protocol normally web or email though could be irc/ftp or other url variant", "%1 (%2)", item, protocol), this);
    mMailListActionList.append(act);
    const QVariant v(url);
    act->setData(v);
    KMail::Util::addQActionHelpText(act, prettyUrl);
    mMailingListActionMenu->addAction(act);
}

void MessageActions::editCurrentMessage()
{
    KMCommand *command = nullptr;
    if (mCurrentItem.isValid()) {
        Akonadi::Collection col = mCurrentItem.parentCollection();
        qCDebug(KMAIL_LOG) << " mCurrentItem.parentCollection()" << mCurrentItem.parentCollection();
        // edit, unlike send again, removes the message from the folder
        // we only want that for templates and drafts folders
        if (col.isValid()
            && (CommonKernel->folderIsDraftOrOutbox(col)
                || CommonKernel->folderIsTemplates(col))
            ) {
            command = new KMEditItemCommand(mParent, mCurrentItem, true);
        } else {
            command = new KMEditItemCommand(mParent, mCurrentItem, false);
        }
        command->start();
    } else if (mCurrentItem.hasPayload<KMime::Message::Ptr>()) {
        command = new KMEditMessageCommand(mParent, mCurrentItem.payload<KMime::Message::Ptr>());
        command->start();
    }
}

void MessageActions::annotateMessage()
{
    if (!mCurrentItem.isValid()) {
        return;
    }

    QPointer<PimCommon::AnnotationEditDialog> dialog = new PimCommon::AnnotationEditDialog(mCurrentItem, mParent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MessageActions::addWebShortcutsMenu(QMenu *menu, const QString &text)
{
    mWebShortcutMenuManager->setSelectedText(text);
    mWebShortcutMenuManager->addWebShortcutsToMenu(menu);
}

QAction *MessageActions::debugAkonadiSearchAction() const
{
    return mDebugAkonadiSearchAction;
}

QAction *MessageActions::addFollowupReminderAction() const
{
    return mAddFollowupReminderAction;
}

void MessageActions::slotDebugAkonadiSearch()
{
    if (!mCurrentItem.isValid()) {
        return;
    }
    QPointer<Akonadi::Search::AkonadiSearchDebugDialog> dlg = new Akonadi::Search::AkonadiSearchDebugDialog;
    dlg->setAkonadiId(mCurrentItem.id());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setSearchType(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Emails);
    dlg->doSearch();
    dlg->show();
}

void MessageActions::slotResendMessage()
{
    if (!mCurrentItem.isValid()) {
        return;
    }
    KMCommand *command = new KMResendMessageCommand(mParent, mCurrentItem);
    command->start();
}

QAction *MessageActions::newMessageFromTemplateAction() const
{
    return mNewMessageFromTemplateAction;
}

QAction *MessageActions::sendAgainAction() const
{
    return mSendAgainAction;
}

void MessageActions::slotAddFollowupReminder()
{
    if (!mCurrentItem.isValid()) {
        return;
    }

    QPointer<MessageComposer::FollowUpReminderSelectDateDialog> dlg = new MessageComposer::FollowUpReminderSelectDateDialog(mParent);
    if (dlg->exec()) {
        const QDate date = dlg->selectedDate();
        CreateFollowupReminderOnExistingMessageJob *job = new CreateFollowupReminderOnExistingMessageJob(this);
        job->setDate(date);
        job->setCollection(dlg->collection());
        job->setMessageItem(mCurrentItem);
        job->start();
    }
    delete dlg;
}
