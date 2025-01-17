/*
 * This file is part of KMail.
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * Parts based on KMail code by:
 * Various authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "attachmentcontroller.h"

#include "attachmentview.h"
#include <MailCommon/FolderSettings>
#include "settings/kmailsettings.h"
#include "editor/kmcomposerwin.h"
#include "kmkernel.h"
#include "kmreadermainwin.h"
#include "mailcommon/mailutil.h"
#include <KIdentityManagement/Identity>
#include <AkonadiCore/itemfetchjob.h>
#include <kcontacts/addressee.h>
#include <AkonadiCore/ItemFetchScope>
#include "kmail_debug.h"

#include <QGpgME/Protocol>

#include <MessageComposer/AttachmentModel>
#include <MessageCore/AttachmentPart>

using namespace KMail;
using namespace KPIM;
using namespace MailCommon;
using namespace MessageCore;

AttachmentController::AttachmentController(MessageComposer::AttachmentModel *model, AttachmentView *view, KMComposerWin *composer)
    : AttachmentControllerBase(model, composer, composer->actionCollection())
    , mComposer(composer)
    , mView(view)
{
    connect(composer, &KMComposerWin::identityChanged,
            this, &AttachmentController::identityChanged);

    connect(view, &AttachmentView::contextMenuRequested, this, &AttachmentControllerBase::showContextMenu);
    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AttachmentController::selectionChanged);
    connect(view, &QAbstractItemView::doubleClicked,
            this, &AttachmentController::doubleClicked);

    connect(this, &AttachmentController::refreshSelection, this, &AttachmentController::selectionChanged);

    connect(this, &AttachmentController::showAttachment, this, &AttachmentController::onShowAttachment);
    connect(this, &AttachmentController::selectedAllAttachment, this, &AttachmentController::slotSelectAllAttachment);
    connect(model, &MessageComposer::AttachmentModel::attachItemsRequester, this, &AttachmentController::addAttachmentItems);
    connect(this, &AttachmentController::actionsCreated, this, &AttachmentController::slotActionsCreated);
}

AttachmentController::~AttachmentController()
{
}

void AttachmentController::slotSelectAllAttachment()
{
    mView->selectAll();
}

void AttachmentController::identityChanged()
{
    const KIdentityManagement::Identity &identity = mComposer->identity();

    // "Attach public key" is only possible if OpenPGP support is available:
    enableAttachPublicKey(QGpgME::openpgp());

    // "Attach my public key" is only possible if OpenPGP support is
    // available and the user specified his key for the current identity:
    enableAttachMyPublicKey(QGpgME::openpgp() && !identity.pgpEncryptionKey().isEmpty());
}

void AttachmentController::attachMyPublicKey()
{
    const KIdentityManagement::Identity &identity = mComposer->identity();
    qCDebug(KMAIL_LOG) << identity.identityName();
    exportPublicKey(QString::fromLatin1(identity.pgpEncryptionKey()));
}

void AttachmentController::slotActionsCreated()
{
    // Disable public key actions if appropriate.
    identityChanged();

    // Disable actions like 'Remove', since nothing is currently selected.
    selectionChanged();
}

void AttachmentController::addAttachmentItems(const Akonadi::Item::List &items)
{
    Akonadi::ItemFetchJob *itemFetchJob = new Akonadi::ItemFetchJob(items, this);
    itemFetchJob->fetchScope().fetchFullPayload(true);
    itemFetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    connect(itemFetchJob, &Akonadi::ItemFetchJob::result, mComposer, &KMComposerWin::slotFetchJob);
}

void AttachmentController::selectionChanged()
{
    const QModelIndexList selectedRows = mView->selectionModel()->selectedRows();
    AttachmentPart::List selectedParts;
    selectedParts.reserve(selectedRows.count());
    for (const QModelIndex &index : selectedRows) {
        AttachmentPart::Ptr part = mView->model()->data(
            index, MessageComposer::AttachmentModel::AttachmentPartRole).value<AttachmentPart::Ptr>();
        selectedParts.append(part);
    }
    setSelectedParts(selectedParts);
}

void AttachmentController::onShowAttachment(KMime::Content *content, const QByteArray &charset)
{
    const QString charsetStr = QString::fromLatin1(charset);
    if (content->bodyAsMessage()) {
        KMime::Message::Ptr m(new KMime::Message);
        m->setContent(content->bodyAsMessage()->encodedContent());
        m->parse();
        KMReaderMainWin *win = new KMReaderMainWin();
        win->showMessage(charsetStr, m);
        win->show();
    } else {
        KMReaderMainWin *win
            = new KMReaderMainWin(content, MessageViewer::Viewer::Text, charsetStr);
        win->show();
    }
}

void AttachmentController::doubleClicked(const QModelIndex &itemClicked)
{
    if (!itemClicked.isValid()) {
        qCDebug(KMAIL_LOG) << "Received an invalid item clicked index";
        return;
    }
    // The itemClicked index will contain the column information. But we want to retrieve
    // the AttachmentPart, so we must recreate the QModelIndex without the column information
    const QModelIndex &properItemClickedIndex = mView->model()->index(itemClicked.row(), 0);
    AttachmentPart::Ptr part = mView->model()->data(
        properItemClickedIndex,
        MessageComposer::AttachmentModel::AttachmentPartRole).value<AttachmentPart::Ptr>();

    // We can't edit encapsulated messages, but we can view them.
    if (part->isMessageOrMessageCollection()) {
        viewAttachment(part);
    } else {
        editAttachment(part);
    }
}
