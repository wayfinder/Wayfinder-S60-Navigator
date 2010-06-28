/*
* ============================================================================
*  Name     : MMSDemo1Engine from MMSDemo1Engine.cpp
*  Part of  : MMSDemo1
*  Created  : 28.10.2002 by Forum Nokia
*  Implementation notes:
*     MMS engine source file.
*  Version  : 1.0 aaumala
*  Copyright: Nokia Corp. 2003
* ============================================================================
*/
#include <mtclreg.h>                        // for CClientMtmRegistry 
#include <msvstd.h>                         // for TMsvId
#include <mmsclient.h>                      // for CMmsClientMtm
#include <mtmdef.h>                         // for KMsvMessagePartDescription etc.
#include <unistd.h> // sleep xxx

#include "MMSDemo1Engine.h"                 // own definitions


// Own constants

// define own short names for the standard messaging folders
const TMsvId KInbox = KMsvGlobalInBoxIndexEntryId;
const TMsvId KOutbox = KMsvGlobalOutBoxIndexEntryId;
const TMsvId KDrafts = KMsvDraftEntryId;
const TMsvId KSent = KMsvSentEntryId;

//
// CMMSEngine class
//

// public methods
/*
-------------------------------------------------------------------------------

    CMMSEngine::NewL()

    Description: Static 2-phase construction.

    Return value: CMMSEngine pointer

-------------------------------------------------------------------------------
*/
EXPORT_C CMMSEngine* CMMSEngine::NewL()
    {
    CMMSEngine* self = NewLC();
    CleanupStack::Pop(); // self
    return self;
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::NewLC()

    Description: Static 2-phase construction. Reserves memory for a new object,
                 the caller must take care of releasing the memory.

    Return value: CMMSEngine pointer (is left into CleanupStack)

-------------------------------------------------------------------------------
*/
EXPORT_C CMMSEngine* CMMSEngine::NewLC()
    {
    CMMSEngine* self = new(ELeave) CMMSEngine();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::~CMMSEngine()

    Description: Destructor.

    Return value: N/A

-------------------------------------------------------------------------------
*/ 
CMMSEngine::~CMMSEngine()
    {
    // the pointers must be released in the correct order
    // (MTM before registry and the session must be the last to delete)
        
    if(iMmsMtm)
        delete iMmsMtm;

    if(iMtmReg)
        delete iMtmReg;
    
    if(iSession)
        delete iSession;

    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::CreateMessageL()

    Description: Create a new multimedia message. This method takes no
                 parameters, default values are used for the parent folder and 
                 service settings.

                 Member data is set to new values by this method.

    Return value: N/A 

-------------------------------------------------------------------------------
*/
void CMMSEngine::CreateMessageL() 
    {
    
    // By default create messages in the Drafts folder
    CMsvEntry* entry = NULL;
    entry = iSession->GetEntryL( KMsvDraftEntryId );
    CleanupStack::PushL(entry);
    iMmsMtm->SwitchCurrentEntryL( entry->EntryId() );
    // Create new, using Default MM Settings
#ifndef NAV2_CLIENT_SERIES60_V3
    iMmsMtm->CreateMessageL( iMmsMtm->DefaultSettingsL() );
#endif

// message is not ready until a recipient has been set ->   iMessageReady = ETrue;
    CleanupStack::PopAndDestroy(); // entry
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::CreateReplyL()

    Description: Creating a reply message entry from a message that is passed 
                 in as a parameter. The forward message
                 entry is left as the current entry of the MTM.
                 The context is being switched here so all unsaved changes to 
                 an old entry are lost!

                 This implementation creates a reply to the sender only, use
                 KMsvMessagePartRecipient in PartList specification to create 
                 a reply-to-all. The sender of the original message is set as 
                 the recipient of this message.

    Return value: ETrue/EFalse if the operation was successfull or not

-------------------------------------------------------------------------------
*/
/*TBool CMMSEngine::CreateReplyL(TMsvId aMessageEntry)
    {
    iMessageReady = EFalse;
    // Set the given message as the current entry
    iMmsMtm->SwitchCurrentEntryL( aMessageEntry );

    // then create a forward message from the current msg.
    TMsvId parentId = KDrafts;                              // create fwd. msg. into Drafts folder

    TMsvPartList PartList(KMsvMessagePartDescription);      // including subject field
    CMsvOperationWait* wait = CMsvOperationWait::NewLC();   // left in CS
    CMsvOperation* oper = iMmsMtm->ReplyL(parentId, PartList, wait->iStatus);
    CleanupStack::PushL(oper);
    wait->Start();
    CActiveScheduler::Start();

    // see if any errors occured
    if(oper->iStatus.Int() != KErrNone)
        {
        // failed
        return EFalse;
        }

    // we can get the entry id from the progress info of the operation
    TPckgBuf<TMsvId> pkg;
    pkg.Copy(oper->ProgressL());
    TMsvId progress = 0;
    progress = pkg();

	// Load created message to iMmsMtm
    LoadMessageL(progress);

    CleanupStack::PopAndDestroy(2);  // wait, oper
    iMessageReady = ETrue;
    return ETrue;
    }
*/

/*
-------------------------------------------------------------------------------

    CMMSEngine::CreateForwardL()

    Description: Creating a forward message entry from a message that is passed 
                 in as a parameter. If the operation is successful, the forward 
                 message entry is left as the current entry of the MTM.

                 The context is being switched here so all unsaved changes to 
                 an old entry are lost!

    Return value: ETrue/EFalse if the operation was successfull or not

-------------------------------------------------------------------------------
*/
/*TBool CMMSEngine::CreateForwardL( TMsvId aMessageEntry )
    {
    iMessageReady = EFalse;
    // Set the given message as the current entry
    iMmsMtm->SwitchCurrentEntryL( aMessageEntry );

    // then create a forward message from the current msg.
    TMsvId parentId = KDrafts;                              // create fwd. msg. into Drafts folder
    
    TMsvPartList PartList(KMsvMessagePartDescription);      // including subject field
    CMsvOperationWait* wait = CMsvOperationWait::NewLC();   // left in CS
    CMsvOperation* oper = iMmsMtm->ForwardL(parentId, PartList, wait->iStatus);
    CleanupStack::PushL(oper);
    wait->Start();
    CActiveScheduler::Start();

    // see if any errors occured
    if(oper->iStatus.Int() != KErrNone)
        {
        // failed
        return EFalse;
        }

    // we can get the entry id from the progress info of the operation
    TPckgBuf<TMsvId> pkg;
    pkg.Copy(oper->ProgressL());
    TMsvId progress = 0;
    progress = pkg();

	// Load created message to iMmsMtm
    LoadMessageL(progress);

    CleanupStack::PopAndDestroy(2);  // wait, oper
    iMessageReady = ETrue;
    return ETrue;
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::AddTextObjectL()

    Description: Creating a new text attachment. 
                 This method creates a new text/plain type of an attachment
                 from the given TDesC parameter and filename.

    Return value: ID of the created attachment

-------------------------------------------------------------------------------
*/
/*TMsvId CMMSEngine::AddTextObjectL(const TDesC& aText, const TDesC& aFilename)
    {
    // blank id for the new attachment
    TMsvId AttId = KMsvNullIndexEntryId;

    // create the attachment
    iMmsMtm->CreateTextAttachmentL(AttId, aText, aFilename);

    iMmsMtm->SetAttachmentCharsetL(AttId, KMmsUtf8); // send text in Utf8 format

    return AttId;
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::AddTextObjectL()

    Description: Creating a new text attachment. 
                 This method creates a new text/plain type of an attachment
                 from the given TDesC parameter.

    Return value: ID of the created attachment

-------------------------------------------------------------------------------
*/
/*TMsvId CMMSEngine::AddTextObjectL(const TDesC& aText)
    {
    // blank id for the new attachment
    TMsvId AttId = KMsvNullIndexEntryId;

    // create without a file name
    iMmsMtm->CreateTextAttachmentL(AttId, aText);

    iMmsMtm->SetAttachmentCharsetL(AttId, KMmsUtf8); // send text in Utf8 format

    return AttId;
    }
*/

/*
-------------------------------------------------------------------------------

    CMMSEngine::AddObjectL()

    Description: Creating a new attachment. 
                 This method calls another overload to create an attachment 
                 and then sets the desired mime type for the attachment.

    Return value: N/A

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::AddObjectL(const TDesC& aPath, TDesC8& aAttMimeType)
    {
    // New id for the attachment, value is given from the MTM when att entry is
    // created.
    TMsvId AttId = KMsvNullIndexEntryId;
    // First create the attachment (calling to an overload of this method, see below)
    AddObjectL(aPath, AttId);
    // then add the mime type information
    iMmsMtm->SetAttachmentTypeL(AttId, aAttMimeType);
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::AddObjectL()

    Description: Creating a new attachment. 

    Return value: TMsvId of the created attachment entry in aNewAttId

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::AddObjectL(const TDesC& aPath, TMsvId& aNewAttId)
    {
    // Create a new attachment under our message entry
    // we'll get a new id from the client MTM
    iMmsMtm->CreateAttachment2L(aNewAttId, aPath);
 
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::SendMessageL()

    Description: Send the current message through the MMS server MTM.
                 The message should be validated before calling SendMessageL().

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::SendMessageL()
    {

    // Start sending the message via the Server MTM to the MMS server
    CMsvOperationWait* wait = CMsvOperationWait::NewLC(); // left in CS
    wait->iStatus = KRequestPending;
    CMsvOperation* op = NULL;
    op = iMmsMtm->SendL( wait->iStatus );
    CleanupStack::PushL( op );
    wait->Start();
    CActiveScheduler::Start();

    // The following is to ignore the completion of other active objects. It is not
    // needed if the app has a command absorbing control.
    while( wait->iStatus.Int() == KRequestPending )
        {
        CActiveScheduler::Start();
        }

    CleanupStack::PopAndDestroy(2); // op, wait

    iMessageReady = EFalse;
    }


//
// This method is to be implemented in a future MMS demo
//
/*void CMMSEngine::GetServiceXxxParamL()    
    {
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::AddAddresseeL()

    Description: Add an addressee. If no type is given, addressee is typed
                 as To-addressee by default (see class definition).

    Return value: N/A

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::AddAddresseeL(const TDesC& aAddress, TAddresseeType aType)
    {
    // add a new addressee and define a type
    iMmsMtm->AddTypedAddresseeL(aAddress, (TMmsRecipients)aType);
    iMessageReady = ETrue;
    }
*/
/*void 
CMMSEngine::SetMessageRootL( const TMsvId aId )
{
   iMmsMtm->SetMessageRootL(aId);
}
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::SaveMessageL()

    Description: Save the current message. If you do not call this method, 
                 all changes made will be lost when the context is changed.
                 Also a new message is invisible as default (even after sending)
                 see below how to change the visibility flags.

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::SaveMessageL()
    {
    // IMPORTANT: Set visibility flags
    // after this the message will be visible to the user
    TMsvEntry ent = iMmsMtm->Entry().Entry();
    ent.SetInPreparation(EFalse);
    ent.SetVisible(ETrue);            
    iMmsMtm->Entry().ChangeL(ent);

    // Save message
    iMmsMtm->SaveMessageL();
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::LoadMessageL()

    Description: Load a message to the MTM. Message is loaded and set as current.

    Return value: TBool

-------------------------------------------------------------------------------
*/
/*TBool CMMSEngine::LoadMessageL(TMsvId aMessageEntry)
    {
    iMmsMtm->SwitchCurrentEntryL( aMessageEntry );
    
    iMmsMtm->LoadMessageL();

    // check unread value & change when necessary
    TMsvEntry tEntry = iMmsMtm->Entry().Entry();
    if(tEntry.Unread())
        {
        tEntry.SetUnread(EFalse);
        iMmsMtm->Entry().ChangeL(tEntry);
        // decrease new messages count
        if(iNewMessages > 0)
            iNewMessages--;
        }

    iMessageReady = ETrue;
    return ETrue;
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::SetSubjectL()

    Description: Set the subject of the current message.

    Return value: N/A

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::SetSubjectL(const TDesC& aSubject)
    {
    iMmsMtm->SetSubjectL(aSubject);
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::DeleteMessageL()

    Description: Delete a message. The caller will provide IDs for the parent
                 and the message entry to be deleted.

    Return value: N/A

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::DeleteMessageL(TMsvId aParent, TMsvId aMessage)
    {
    // Point our entry to the parent of the message to be deleted    
    CMsvEntry* cEntry = iSession->GetEntryL(aParent); // change context
    CleanupStack::PushL(cEntry);
    
    // Delete selected message
    cEntry->DeleteL(aMessage);

    CleanupStack::PopAndDestroy(); // cEntry
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::GetObjectsL()

    Description: Read the attachment file paths and return them in a descriptor
                 array.

    Return value: N/A, attachment file paths in an array

-------------------------------------------------------------------------------
*/
/*TBool CMMSEngine::GetObjectsL(CDesC16ArrayFlat& aArray)
    {
    TBool returnValue = EFalse;
    // Read attachments into parameters
    CMsvEntrySelection* att = iMmsMtm->GetAttachmentsL();
    CleanupStack::PushL(att);

    if(att->Count() > 0)
        {
        for(TInt i = 0; i < att->Count(); i++)
            {
            TFileName fileName;
            // this is given a TMsvId of the att entry and the descriptor for path name
            iMmsMtm->GetAttachmentPathL(att->At(i), fileName );

            aArray.InsertL(i, fileName);
            }
        returnValue = ETrue;
        }
    CleanupStack::PopAndDestroy(); // att
    return returnValue;
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::ReadMessagesL()

    Description: Read all MMS messages in a folder. Read message IDs are returned
                 in a CMsvEntrySelection list. Message index entry descriptions 
                 and details are written into parameter list.
                 Note: This method changes the context!

    Return value: CMsvEntrySelection, caller has to free memory after use! 

                  A list of message descriptions is returned in aDescriptorList

-------------------------------------------------------------------------------
*/
/*CMsvEntrySelection* CMMSEngine::ReadMessagesL(TMsvId aFolder, CDesC16Array& aDescriptorList)
    {
    // first take a handle to folder
    TMsvSelectionOrdering sort;
	sort.SetShowInvisibleEntries(ETrue);    // we want to handle also the invisible entries
    // Take a handle to the folder entry
	CMsvEntry* parentEntry = CMsvEntry::NewL(*iSession, aFolder, sort);
	CleanupStack::PushL(parentEntry);
	
    // A selection of all MMS entries
    CMsvEntrySelection* entries = parentEntry->ChildrenWithMtmL(KUidMsgTypeMultimedia);
    CleanupStack::PushL(entries);

    // go through the selection and read descriptions into a list
    for(TInt i = 0; i < entries->Count(); i++)
        {
        // we can use the mtm to go through the selection.
        iMmsMtm->SwitchCurrentEntryL( (*entries)[i] );
        aDescriptorList.AppendL(iMmsMtm->Entry().Entry().iDetails);
        aDescriptorList.AppendL(iMmsMtm->Entry().Entry().iDescription);
        }

	CleanupStack::Pop(2);   // parentEntry, entries
    delete parentEntry;
    
    return entries; // Caller must free this memory after use!
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::MessageReady()

    Description: This returns true if the engine has a message loaded, ready
                 for sending.

    Return value: ETrue / EFalse

-------------------------------------------------------------------------------
*/
TBool CMMSEngine::MessageReady()
    {
    return iMessageReady;
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::NewMessages()

    Description: This returns true if there are new MMS messages in the Inbox.

    Return value: ETrue / EFalse

-------------------------------------------------------------------------------
*/
/*TInt CMMSEngine::NewMessages()
    {
    return iNewMessages;
    }
*/
/*
-----------------------------------------------------------------------------

    CMMSEngine::HandleSessionEventL()

    Receives session events from observer and calls event handling functions. 

    The type of event is indicated by the value of aEvent. The 
    interpretation of the TAny arguments depends on this type. For most 
    event types, the action that is taken, for example updating the 
    display, is client-specific. 

-----------------------------------------------------------------------------
*/
void CMMSEngine::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* /*aArg3*/)
    {
    switch (aEvent)
        {

        // Message events handling, e.g. receiving
        case EMsvEntriesCreated:
            // We are interested in messages that are created in Inbox
            TMsvId* parentId;
            parentId = static_cast<TMsvId*>(aArg2);          // parent entry id from the session event
            
            if ( *parentId == KMsvGlobalInBoxIndexEntryId )  // new entry has been created in Inbox folder
                {
                // We take the created entries into a selection
                CMsvEntrySelection* entries = static_cast<CMsvEntrySelection*>(aArg1);

                // entry pointer for making changes in the actual message contexts
                CMsvEntry* entry;

                //Process each created entry, one at a time.
                for(TInt i = 0; i < entries->Count(); i++)
                    {
                    entry = iSession->GetEntryL( entries->At(i) );  // this reserves memory for a new CMsvEntry
                    CleanupStack::PushL(entry);
                    TMsvEntry msvEntry(entry->Entry());
                    if(msvEntry.iMtm == KUidMsgTypeMultimedia) // message is a MMS
                        {
                        // We could also check that message is of correct content
                        // but as this is the mms engine and message handling is designed to
                        // be only general, we don't do it. One possibility to id a message content
                        // would be to check the subject field if it has some application specific value etc.
                        iNewMessages++;
                        }
                    CleanupStack::PopAndDestroy(entry);
                    }
                }
            break;

        // This event tells us that the session has been opened
        case EMsvServerReady:
            CompleteConstructL();       // Construct the mtm registry & MMS mtm
            break;
        
        default:
            // All other events are ignored
            break;
        }

    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::GetAddresseesL()

    Description: Get message addressees of a given type (To or Cc).

    Return value: CDesCArray&

-------------------------------------------------------------------------------
*/
/*const CDesCArray& CMMSEngine::GetAddresseesL(TAddresseeType aType)
    {
    // Read addressees
    return iMmsMtm->TypedAddresseeList((TMmsRecipients)aType);
    }
*/
/*
-------------------------------------------------------------------------------

    CMMSEngine::SetObjectTypeL()

    Description: Set the MIME type for a given attachment.

    Return value: N/A

-------------------------------------------------------------------------------
*/
/*void CMMSEngine::SetObjectTypeL(TMsvId aAttId, TDesC8& aMimeType)
    {
    iMmsMtm->SetAttachmentTypeL(aAttId, aMimeType);
    }
*/
//
// protected methods
//

/*
-------------------------------------------------------------------------------

    CMMSEngine::MessageSize()

    Description: Returns the size of the message. Returned value is valid for 
                 the last message state saved by SaveMessageL(). Any changes 
                 after that are not counted in the returned size value.

    Return value: TInt32

-------------------------------------------------------------------------------
*/
/*TInt32 CMMSEngine::MessageSize()
    {
    return iMmsMtm->MessageSize();
    }
*/

//
// Additional methods to be implemented in a future MMS demo
//
/*void CMMSEngine::GetObjectSizeL()
    {
    }

void CMMSEngine::ValidateMessageL()
    {
    }

void CMMSEngine::GetServiceL()
    {
    }

void CMMSEngine::SetService() // not supported
    {
    }

void CMMSEngine::RemoveAddresseeL()
    {
    }
*/

/*
-------------------------------------------------------------------------------

    CMMSEngine::MoveMessageL()

    Description: Move current message to another folder.

    Return value: TBool

-------------------------------------------------------------------------------
*/
/*TBool CMMSEngine::MoveMessageL(TMsvId aTarget)
    {
    TMsvId parentId = iMmsMtm->Entry().Entry().Parent();
    TMsvId currentId = iMmsMtm->Entry().Entry().Id();
    if(aTarget == parentId)
        {
        // no need to move
        return EFalse;
        }

    // Set up our entry to the parent folder so that we can move
    // the message. 

    CMsvEntry* cEntry = iSession->GetEntryL( parentId );
    CleanupStack::PushL(cEntry);
    // view all entries (also invisible ones)
    cEntry->SetSortTypeL( TMsvSelectionOrdering( KMsvNoGrouping, EMsvSortByNone, ETrue ) );
    
    // wait object for the operation (move)
    CMsvOperationWait* wait = CMsvOperationWait::NewLC(); // left in CS
    wait->Start();
    
    // new move operation
    CMsvOperation* oper = cEntry->MoveL( currentId, aTarget, wait->iStatus );
    CleanupStack::PushL(oper);

    // Start a new wait loop
    CActiveScheduler::Start();
    
    if( wait->iStatus.Int() != KErrNone )
        {
        CleanupStack::PopAndDestroy(3); // oper, wait, cEntry
        return EFalse;
        }
    
    CleanupStack::PopAndDestroy(3); // oper, wait, cEntry
    return ETrue;
    }
*/
//
// private methods
//

/*
-------------------------------------------------------------------------------

    CMMSEngine::ConstructL()

    Description: 2nd phase Constructor.

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::ConstructL()
    {
    // Open a connection to messaging server
    InitServerSessionL();
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::CompleteConstructL()

    Description: 2nd phase Constructor. This method completes construction
                 after the server session has been opened.

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::CompleteConstructL()
    {
    // create client MTM
    InitClientMTML();
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::CMMSEngine()

    Description: Constructor.

    Return value: N/A

-------------------------------------------------------------------------------
*/
CMMSEngine::CMMSEngine()
    {
    iNewMessages = 0;  // New messages count tells only the amount of messages 
                       // received while running. (not the actual state of Inbox)
    iMessageReady = EFalse;
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::InitServerSessionL()

    Description: Initialising a session to the messaging server. This is done
                 in async and the execution is returned to the ConstructL() 
                 immediately.

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::InitServerSessionL()
    {
    // Create CMsvSession
    iSession = CMsvSession::OpenAsyncL(*this); 
    // New session is opened asynchronously
    // CompleteConstructL() is called when async call is finished.
    }

/*
-------------------------------------------------------------------------------

    CMMSEngine::InitClientMTML()

    Description: Initialising a client MTM registry and the actual 
                 MMS client MTM.

    Return value: N/A

-------------------------------------------------------------------------------
*/
void CMMSEngine::InitClientMTML()
    {
    // We call our session for a MtmClientRegistry
    iMtmReg = CClientMtmRegistry::NewL(*iSession);
    // This registry is used to instantiate a new mtm.
    iMmsMtm = (CMmsClientMtm*) iMtmReg->NewMtmL( KUidMsgTypeMultimedia );
    }

TBool 
CMMSEngine::ConstructionComplete()
{
   return (iMmsMtm != NULL);
}

void 
CMMSEngine::SleepUntilConstructionComplete()
{
   while (!ConstructionComplete()) {
      sleep(1); // one second.
   }
}

// End of File
