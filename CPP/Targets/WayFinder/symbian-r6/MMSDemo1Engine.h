/*
* ============================================================================
*  Name     : CMMSEngine from CMMSDemo1Engine.h
*  Part of  : MMSDemo1
*  Created  : aaumala
*  Description:
*     Engine header file for MMSDemo1 application
*  Version  : 1.0
*  Copyright: Nokia Corp. 2003
* ============================================================================
*/


#ifndef __MMSDEMO1ENGINE_H__
#define __MMSDEMO1ENGINE_H__

#if !defined(__MMSCONST_H__)
#include <mmsconst.h>                      // for MMS constants
#endif

#if !defined(__MSVAPI_H__)
#include <msvapi.h>                        // for MMsvSessionObserver 
#endif

#if !defined(__MSVIDS_H__)
#include <msvids.h>                         // for Message type IDs
#endif

// Forward declarations
class CMmsClientMtm;
class CClientMtmRegistry;



// local enumeration for addressee values in MMS MTM API
enum TAddresseeType
    {
    ETypeTo = EMmsTo,
    ETypeCc = EMmsCc,
    ETypeBcc = EMmsBcc
    };

/**
* CMMSDemo1Engine MMS client engine class.
* This class handles interaction with the MMS Server through a client interface, CMmsClientMtm.
* 
*/
class CMMSEngine : public MMsvSessionObserver
    {
public:
    /**
     * Symbian OS 2-phase, static construction.
     * @return A pointer to the created CMMSEngine object.
     */
    IMPORT_C static CMMSEngine* NewL();
    IMPORT_C static CMMSEngine* NewLC(); 
    virtual ~CMMSEngine();

//
// methods offered to the Demo Application
//

    // message creation methods
    void CreateMessageL();          
/*    TBool CreateReplyL( TMsvId aMessage );
    TBool CreateForwardL( TMsvId aMessage );

    // adding objects i.e. attachments
    TMsvId AddTextObjectL(const TDesC& aText, const TDesC& aFilename);
    TMsvId AddTextObjectL(const TDesC& aText);

    void AddObjectL(const TDesC& aPath, TDesC8& aAttMimeType);
    void AddObjectL(const TDesC& aPath, TMsvId& aNewAttId);                     
*/
    // sending
    void SendMessageL();
/*
    // not implemented in current version
    void GetServiceXxxParamL(); 
    
    // addressee manipulation
    void AddAddresseeL(const TDesC& aAddress, TAddresseeType aType = ETypeTo);        

    virtual void SetMessageRootL( const TMsvId aId );
*/
    // Saving changes to message entry
    void SaveMessageL();
/*
    // Loading an existing message from a folder
    TBool LoadMessageL(TMsvId aMessageEntry);

    void SetSubjectL(const TDesC& aSubject);

    // Deleting messages from a folder
    void DeleteMessageL(TMsvId aParent, TMsvId aMessage);

    // reading attachments or addressees from current message
    TBool GetObjectsL(CDesC16ArrayFlat& aArray);
    const CDesCArray& GetAddresseesL( TAddresseeType aType = ETypeTo );

    // To read all MMS messages in a given folder
    CMsvEntrySelection* ReadMessagesL(TMsvId aFolder, CDesC16Array& aDescriptorList);
*/
    // To query if engine has a proper message loaded or not
    TBool MessageReady();
/*
    // To query if there are new messages in Inbox
    TInt NewMessages();

    // Setting a MIME type for attachment
    void SetObjectTypeL(TMsvId aAttId, TDesC8& aMimeType);
*/
public: // from MMsvSessionObserver
    void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);

    /**
     * Checks if the construction is complete and the object is ready for use.
     * @return EFalse if not.
     */
    TBool ConstructionComplete();

    /**
     * Sleeps until the construction is complete.
     */
    void SleepUntilConstructionComplete();

    /**
     * Gets a reference to the mmsclientmtm.
     * @return A reference to the mmsclientmtm.
     */
    CMmsClientMtm& getMmsClientMtm() const {
       return *iMmsMtm;
    }

protected:
//    TInt32 MessageSize();
//    void GetObjectSizeL();
//    void ValidateMessageL();
//    void GetServiceL();
//    void SetService(); // not supported
//    void RemoveAddresseeL();
//    TBool MoveMessageL(TMsvId aTarget);

private:
    void ConstructL();
    void CompleteConstructL();
    CMMSEngine();

    // messaging initialisation
    void InitServerSessionL();
    void InitClientMTML();
    
    
private:

    CMmsClientMtm* iMmsMtm;             // Client MTM that handles message manipulation
    CClientMtmRegistry* iMtmReg;        // This MTM registry gives us handles to client MTMs
    CMsvSession* iSession;              // Session to the messaging server
    TInt iNewMessages;                  // ETrue if there are unread messages in inbox for this app
    CMsvEntrySelection* iNewMessageIds; // New message ids are kept here until they are read
    TBool iMessageReady;                // Checking if message is ready for sending
                                        //  (in current implementation message is ready if it has at least
                                        //   one addressee).
    };


#endif // __MMSDEMO1ENGINE_H__
