/*   -*- c++ -*-
 *   accountdialog.h
 *
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _ACCOUNT_DIALOG_H_
#define _ACCOUNT_DIALOG_H_

#include <kdialogbase.h>
#include "kmfoldercombobox.h"

class QRegExpValidator;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QRadioButton;
class KIntNumInput;
class KMAccount;
class KMFolder;
class KMServerTest;
class QButtonGroup;
namespace KMail {
  class SieveConfigEditor;
}

class AccountDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AccountDialog( const QString & caption, KMAccount *account,
		   QWidget *parent=0, const char *name=0, bool modal=true );
    virtual ~AccountDialog();
  private:
    struct LocalWidgets
    {
      QLabel       *titleLabel;
      QLineEdit    *nameEdit;
      QComboBox    *locationEdit;
      QRadioButton *lockMutt;
      QRadioButton *lockMuttPriv;
      QRadioButton *lockProcmail;
      QComboBox    *procmailLockFileName;
      QRadioButton *lockFcntl;
      QRadioButton *lockNone;
      QLineEdit    *precommand;
#if 0
      QCheckBox    *resourceCheck;
      QPushButton  *resourceClearButton;
      QPushButton  *resourceClearPastButton;
#endif
      QCheckBox    *excludeCheck;
      QCheckBox    *intervalCheck;
      QLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      QComboBox    *folderCombo;
      QComboBox    *identityCombo;
    };

    struct MaildirWidgets
    {
      QLabel       *titleLabel;
      QLineEdit    *nameEdit;
      QComboBox    *locationEdit;
      QLineEdit    *precommand;
#if 0
      QCheckBox    *resourceCheck;
      QPushButton  *resourceClearButton;
      QPushButton  *resourceClearPastButton;
#endif
      QCheckBox    *excludeCheck;
      QCheckBox    *intervalCheck;
      QLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      QComboBox    *folderCombo;
      QComboBox    *identityCombo;
    };

    struct PopWidgets
    {
      QLabel       *titleLabel;
      QLineEdit    *nameEdit;
      QLineEdit    *loginEdit;
      QLineEdit    *passwordEdit;
      QLineEdit    *hostEdit;
      QLineEdit    *portEdit;
      QLineEdit    *precommand;
      QButtonGroup *encryptionGroup;
      QRadioButton *encryptionNone;
      QRadioButton *encryptionSSL;
      QRadioButton *encryptionTLS;
      QButtonGroup *authGroup;
      QRadioButton *authUser;
      QRadioButton *authPlain;
      QRadioButton *authLogin;
      QRadioButton *authCRAM_MD5;
      QRadioButton *authDigestMd5;
      QRadioButton *authAPOP;
      QPushButton  *checkCapabilities;
      QCheckBox    *usePipeliningCheck;
      QCheckBox    *storePasswordCheck;
      QCheckBox    *leaveOnServerCheck;
#if 0
      QCheckBox    *resourceCheck;
      QPushButton  *resourceClearButton;
      QPushButton  *resourceClearPastButton;
#endif
      QCheckBox    *excludeCheck;
      QCheckBox    *intervalCheck;
      QCheckBox    *filterOnServerCheck;
      QLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      KIntNumInput *filterOnServerSizeSpin;
      QComboBox    *folderCombo;
      QComboBox    *identityCombo;
    };

    struct ImapWidgets
    {
      QLabel       *titleLabel;
      QLineEdit    *nameEdit;
      QLineEdit    *loginEdit;
      QLineEdit    *passwordEdit;
      QLineEdit    *hostEdit;
      QLineEdit    *portEdit;
      QLineEdit    *prefixEdit;
#if 0
      QCheckBox    *resourceCheck;
      QPushButton  *resourceClearButton;
      QPushButton  *resourceClearPastButton;
#endif
      QCheckBox    *autoExpungeCheck;     // only used by normal (online) IMAP
      QCheckBox    *hiddenFoldersCheck;
      QCheckBox    *subscribedFoldersCheck;
      QCheckBox    *loadOnDemandCheck;
      QCheckBox    *storePasswordCheck;
      QCheckBox    *progressDialogCheck;  // only used by Disconnected IMAP
      QCheckBox    *excludeCheck;
      QCheckBox    *intervalCheck;
      QLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      QButtonGroup *encryptionGroup;
      QRadioButton *encryptionNone;
      QRadioButton *encryptionSSL;
      QRadioButton *encryptionTLS;
      QButtonGroup *authGroup;
      QRadioButton *authUser;
      QRadioButton *authPlain;
      QRadioButton *authLogin;
      QRadioButton *authCramMd5;
      QRadioButton *authDigestMd5;
      QRadioButton *authAnonymous;
      QPushButton  *checkCapabilities;
			KMFolderComboBox  *trashCombo;
    };

  private slots:
    virtual void slotOk();
    void slotLocationChooser();
    void slotMaildirChooser();
    void slotEnablePopInterval( bool state );
    void slotEnableImapInterval( bool state );
    void slotEnableLocalInterval( bool state );
    void slotEnableMaildirInterval( bool state );
    void slotFontChanged();
    void slotLeaveOnServerClicked();
    void slotFilterOnServerClicked();
    void slotPipeliningClicked();
    void slotPopEncryptionChanged(int);
    void slotImapEncryptionChanged(int);
    void slotCheckPopCapabilities();
    void slotCheckImapCapabilities();
    void slotPopCapabilities( const QStringList &, const QStringList & );
    void slotImapCapabilities( const QStringList &, const QStringList & );
#if 0
    // Moc doesn't understand #if 0, so they are also commented out
    // void slotClearResourceAllocations();
    // void slotClearPastResourceAllocations();
#endif

  private:
    void makeLocalAccountPage();
    void makeMaildirAccountPage();
    void makePopAccountPage();
    void makeImapAccountPage( bool disconnected = false );
    void setupSettings();
    void saveSettings();
    void checkHighest( QButtonGroup * );
    static unsigned int popCapabilitiesFromStringList( const QStringList & );
    static unsigned int imapCapabilitiesFromStringList( const QStringList & );
    void enablePopFeatures( unsigned int );
    void enableImapAuthMethods( unsigned int );

  private:
    LocalWidgets mLocal;
    MaildirWidgets mMaildir;
    PopWidgets   mPop;
    ImapWidgets  mImap;
    KMAccount    *mAccount;
    QValueList<QGuardedPtr<KMFolder> > mFolderList;
    QStringList  mFolderNames;
    KMServerTest *mServerTest;
    enum EncryptionMethods {
      NoEncryption = 0,
      SSL = 1,
      TLS = 2
    };
    enum Capabilities {
      Plain      =   1,
      Login      =   2,
      CRAM_MD5   =   4,
      Digest_MD5 =   8,
      Anonymous  =  16,
      APOP       =  32,
      Pipelining =  64,
      TOP        = 128,
      UIDL       = 256,
      STLS       = 512, // TLS for POP
      STARTTLS   = 512, // TLS for IMAP
      AllCapa    = 0xffffffff
    };
    unsigned int mCurCapa;
    unsigned int mCapaNormal;
    unsigned int mCapaSSL;
    unsigned int mCapaTLS;
    KMail::SieveConfigEditor *mSieveConfigEditor;
    QRegExpValidator *mValidator;
};


#endif
