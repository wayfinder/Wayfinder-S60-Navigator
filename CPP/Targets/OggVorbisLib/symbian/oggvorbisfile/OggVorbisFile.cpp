// OggVorbisFile.cpp
//
// Copyright © Symbian Software Ltd 2004.  All rights reserved.
//

 
#ifdef SEP_HEAP
#  define VORBIS_TRAP(err, c) { User::SwitchHeap(iVorbisHeap); TRAP(err, c); User::SwitchHeap(iDefaultHeap); }
#  define VORBIS_TRAPD(err, c) TInt err; VORBIS_TRAP(err, c);
#else
#  define VORBIS_TRAP(err, c) c;
#  define VORBIS_TRAPD(err, c) TInt err = KErrNone; c;
#endif

#include <e32base.h>
#include <errno.h>
#include <mmf/server/mmfclip.h>
#include "OggVorbisFile.h"
#include "OggVorbisFileBody.h"


//--------------------------------------------------------------------------
//--------------------------- COggVorbisFile -------------------------------
//--------------------------------------------------------------------------
EXPORT_C COggVorbisFile* COggVorbisFile::NewL(TUid aPlayUid)
    {
    COggVorbisFile* self = new(ELeave)COggVorbisFile(aPlayUid);
    CleanupStack::PushL(self);
    self->iBody = CBody::NewL();
    CleanupStack::Pop(self);
    return self;
    }
    
COggVorbisFile::COggVorbisFile(TUid aPlayUid)
    : MDataSource(aPlayUid)
    {
    }
    
EXPORT_C COggVorbisFile::~COggVorbisFile()
    {
    delete iBody;
    }
    
EXPORT_C void COggVorbisFile::OpenL(CMMFClip& aClip)
    {
    iBody->OpenL(aClip);
    }

EXPORT_C TBool COggVorbisFile::IsOpen()
    {
    return iBody->IsOpen();;
    }    

EXPORT_C void COggVorbisFile::PCMOutL(CMMFDataBuffer& aBuffer)
    {
    iBody->PCMOutL(aBuffer);
    }
    
EXPORT_C TInt COggVorbisFile::GetDecoderPositionL()
    {
    return iBody->GetDecoderPositionL();
    }    
    
EXPORT_C void COggVorbisFile::SetPositionL(TTimeIntervalMicroSeconds aPos)
    {
    // using tremor: so milliseconds not seconds
    TInt64 milliseconds = aPos.Int64()/TInt64(1000);
    //RDebug::Printf("SetPositionL: %f\n", milliseconds.GetTInt());
    iBody->SetPositionL(milliseconds.GetTInt());
    }
    
EXPORT_C TTimeIntervalMicroSeconds COggVorbisFile::Duration()    
    {
    TInt64 time(iBody->Duration());
    // using tremor: this returns milliseconds, not seconds as libvorbisfile
    return TTimeIntervalMicroSeconds(time*1000);
    }
    
EXPORT_C TInt COggVorbisFile::GetNumberOfMetaDataEntries()
    {
    return 0;
    }


EXPORT_C CMMFMetaDataEntry* COggVorbisFile::GetMetaDataEntryL(TInt /*aIndex*/)
    {
    return NULL;
    }
    
EXPORT_C void COggVorbisFile::ResetL()
    {
    iBody->ResetL();
    }
    
EXPORT_C void COggVorbisFile::SetSink(MDataSink* aSink)
    {
    iBody->SetSink(aSink);
    }
    


// from MDataSource:

TFourCC COggVorbisFile::SourceDataTypeCode(TMediaId aMediaId)
    {
    if (aMediaId.iMediaType==KUidMediaTypeAudio)
        {
        return 1; // only support 1st stream for now
        }
    else return 0;
    }

void COggVorbisFile::FillBufferL(CMMFBuffer* aBuffer, MDataSink* aConsumer,TMediaId aMediaId)
    {
    if ((aMediaId.iMediaType==KUidMediaTypeAudio)&&(aBuffer->Type()==KUidMmfDescriptorBuffer))
        {
        //BufferEmptiedL(aBuffer);
        CMMFDataBuffer* db = static_cast<CMMFDataBuffer*>(aBuffer);
        iBody->PCMOutL(*db);
        SetSink(aConsumer);
        aConsumer->BufferFilledL(db);
        }
    else User::Leave(KErrNotSupported);
    }

 void COggVorbisFile::BufferEmptiedL(CMMFBuffer* aBuffer)
    {
    if (aBuffer->Type()==KUidMmfDescriptorBuffer)
        {
        CMMFDataBuffer* db = static_cast<CMMFDataBuffer*>(aBuffer);
        iBody->PCMOutL(*db);
        iBody->iSink->EmptyBufferL(db, this, TMediaId(KUidMediaTypeAudio));
        }
    else User::Leave(KErrNotSupported);
    }

TBool COggVorbisFile::CanCreateSourceBuffer()
    {
    return EFalse;
    }

CMMFBuffer* COggVorbisFile::CreateSourceBufferL(TMediaId /*aMediaId*/, TBool &aReference)
    {
    aReference = EFalse;
    return NULL;
    }
    
void COggVorbisFile::ConstructSourceL(  const TDesC8& /*aInitData*/ )
    {
    }
    
EXPORT_C TInt COggVorbisFile::SampleRate()
    {
    return iBody->SampleRate();
    }
    
EXPORT_C TInt COggVorbisFile::Channels()
    {
    return iBody->Channels();
    }
    
EXPORT_C TInt COggVorbisFile::BytesPerWord()
    {
    return 2;
    }

EXPORT_C TInt COggVorbisFile::BitRate()
    {
    return iBody->BitRate();
    }
    
EXPORT_C void COggVorbisFile::DbgSetHeapFail(RHeap::TAllocFail aType, TInt aWhen)    
    {
    iBody->DbgSetHeapFail(aType, aWhen);
    }
    
EXPORT_C void COggVorbisFile::DbgHeapMark()
    {
    iBody->DbgHeapMark();
    }
    
EXPORT_C void COggVorbisFile::DbgHeapMarkEnd()
    {
    iBody->DbgHeapMarkEnd();
    }
    

//--------------------------------------------------------------------------
//--------------------------------- Body -----------------------------------
//--------------------------------------------------------------------------

COggVorbisFile::CBody* COggVorbisFile::CBody::NewL()
    {
    CBody* self = new(ELeave) CBody;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void COggVorbisFile::CBody::ConstructL()
    {
#ifdef SEP_HEAP    
    User::LeaveIfError(iVorbisChunk.CreateLocal(0x4000, 0x20000, EOwnerThread));
    iVorbisHeap = User::ChunkHeap(iVorbisChunk, 0x2000);
    
    iDefaultHeap = &User::Heap();
#endif
    
    ResetL();
    }
    
COggVorbisFile::CBody::~CBody()
    {
    VORBIS_TRAPD(err, { /*ov_clear(&iFile);*/ CloseSTDLIB(); }); // free up global memory alloc'd by stdlib.
    // can't do anything with err here!
    err=err;

#ifdef SEP_HEAP
//    iVorbisHeap->Close();
    iVorbisChunk.Close();
//    delete iVorbisHeap;
//    delete iDefaultHeap; // do we need to do this??? safe???
#endif
    }
    
void COggVorbisFile::CBody::OpenL(CMMFClip& aClip)
    {
    iClip = &aClip;
    iFilePos=0;
    iClip->SourcePrimeL();
    TInt err;
    VORBIS_TRAPD(err2, err = ov_open_callbacks(this, &iFile, NULL, 0, KOggVorbisCallbacks); );
    User::LeaveIfError(err2);
    switch (err)
        {
    case OV_ENOTVORBIS:
    case OV_EVERSION:
    case OV_EBADHEADER:
        User::Leave(KErrCorrupt);
    case OV_EFAULT:
        VORBIS_TRAP(err, ov_clear(&iFile));
        User::LeaveIfError(err);
        User::Leave(KErrGeneral);
        }
    iOggSection = iFile.current_serialno;
    }
    
void COggVorbisFile::CBody::PCMOutL(CMMFDataBuffer& aBuffer)
    {
    TDes8& buf = aBuffer.Data();
    /*buf.FillZ(buf.MaxLength());
    aBuffer.Data().SetLength(buf.MaxLength());*/
    TInt written = 0;
    TBool eof = EFalse;
    while ((written<buf.MaxLength())&&(!eof))
        {
    
        TUint8* ptr = const_cast<TUint8*>(buf.Ptr())+written;
        TInt res;
        VORBIS_TRAPD(err, res=ov_read(&iFile, reinterpret_cast<char*>(ptr), 
                           buf.MaxLength()-written, &iOggSection));
        User::LeaveIfError(err);
        if (res<0)
            { // some sort of error with the stream - ignore and carry on
            }
        else if (res==0) 
            { // eof
            eof=ETrue;
            }
        else
            { // read res bytes of data
            written+=res;
            }
        }
    buf.SetLength(written);
    if (eof) aBuffer.SetLastBuffer(ETrue);
    }

void COggVorbisFile::CBody::ResetL()
    {
    iClip=NULL;
    VORBIS_TRAPD(err, ov_clear(&iFile));
    User::LeaveIfError(err);
    iFilePos=0;
    }

void COggVorbisFile::CBody::SetSink(MDataSink* aSink)
    {
    iSink=aSink;
    }


TInt COggVorbisFile::CBody::BitRate()
    {
    return ov_bitrate(&iFile, iOggSection);
    }
    
TBool COggVorbisFile::CBody::IsOpen()
    {
    return (iClip!=NULL);
    }
    
TInt COggVorbisFile::CBody::GetDecoderPositionL()
    {
    TInt pos;
    VORBIS_TRAPD(err, pos=ov_pcm_tell(&iFile));
    User::LeaveIfError(err);
    return pos;
    }
   
void COggVorbisFile::CBody::SetPositionL(TInt aPos)
    {
    VORBIS_TRAPD(err, ov_time_seek(&iFile, aPos));
    User::LeaveIfError(err);
    }
   
TInt COggVorbisFile::CBody::Duration()
    {
    return (TInt)ov_time_total(&iFile, 0); // this does nothing with the heap
    }
   
TInt COggVorbisFile::CBody::SampleRate()
    {
    return iFile.vi->rate;
    }

TInt COggVorbisFile::CBody::Channels()
    {
    return iFile.vi->channels;
    }
    
void COggVorbisFile::CBody::DbgSetHeapFail(RHeap::TAllocFail aType, TInt aWhen)    
    {
#   ifdef SEP_HEAP
    User::SwitchHeap(iVorbisHeap);
    errno=errno; // make sure stdlib is initialised before doing setfail
#   endif
    __UHEAP_SETFAIL(aType, aWhen);
#   ifdef SEP_HEAP
    User::SwitchHeap(iDefaultHeap);
#   endif
    }
    
void COggVorbisFile::CBody::DbgHeapMark()
    {
#   ifdef SEP_HEAP
    User::SwitchHeap(iVorbisHeap);
#   endif
    __UHEAP_MARK;
#   ifdef SEP_HEAP
    User::SwitchHeap(iDefaultHeap);
#   endif
    }
    
void COggVorbisFile::CBody::DbgHeapMarkEnd()
    {
#   ifdef SEP_HEAP
    User::SwitchHeap(iVorbisHeap);
#   endif
    __UHEAP_MARKEND;
#   ifdef SEP_HEAP
    User::SwitchHeap(iDefaultHeap);
#   endif
    }
    
size_t COggVorbisFile::CBody::read_func(void* ptr, size_t size, size_t nitems) 
    {
#ifdef SEP_HEAP    // in case iClip->Size does something with the heap
    User::SwitchHeap(iDefaultHeap);
#endif
    if ((size==0)||(nitems==0)) return 0;
    
    CMMFDescriptorBuffer* buf = CMMFDescriptorBuffer::NewL(size*nitems);
    CleanupStack::PushL(buf);
    
    iClip->ReadBufferL(buf, iFilePos);
    TInt read = buf->Data().Length();
    read-=read%size; // round down to mutliple of size
    iFilePos+=read;
    
    // copy data from buf into pointer given
    TPtr8 tptr(static_cast<TUint8*>(ptr), size*nitems);
    tptr = buf->Data();
    
    CleanupStack::PopAndDestroy(buf);
#ifdef SEP_HEAP    
    User::SwitchHeap(iVorbisHeap);
#endif
    return read/size;    
    }

int COggVorbisFile::CBody::seek_func(ogg_int64_t offset, int whence)
    {
#ifdef SEP_HEAP    // in case iClip->Size does something with the heap
    User::SwitchHeap(iDefaultHeap);
#endif
    switch (whence)
        {
    case SEEK_SET:
        iFilePos = offset;
        break;
    case SEEK_CUR:
        iFilePos+=offset;
        break;
    case SEEK_END:
        iFilePos = iClip->Size()+offset;
        break;
        }
    // check new position is valid?
#ifdef SEP_HEAP    
    User::SwitchHeap(iVorbisHeap);
#endif
    return 0;
    }

int COggVorbisFile::CBody::close_func()
    {
    return 0;
    }

long COggVorbisFile::CBody::tell_func()
    {
    return iFilePos;
    }
    
    
//--------------------------------------------------------------------------
//------------------------------ Callbacks ---------------------------------
//--------------------------------------------------------------------------


size_t ovf_read_func(void* ptr, size_t size, size_t nitems, void* file)
    {
    return static_cast<COggVorbisFile::CBody*>(file)->read_func(ptr, size, nitems);
    }

int ovf_seek_func(void* datasource, ogg_int64_t offset, int whence)
    {
    return static_cast<COggVorbisFile::CBody*>(datasource)->seek_func(offset, whence);
    }

int ovf_close_func(void* datasource)
    {
    return static_cast<COggVorbisFile::CBody*>(datasource)->close_func();
    }

long ovf_tell_func(void* datasource)
    {
    return static_cast<COggVorbisFile::CBody*>(datasource)->tell_func();
    }


GLDEF_C TInt E32Dll(TDllReason)
        {
        return KErrNone;
        }
