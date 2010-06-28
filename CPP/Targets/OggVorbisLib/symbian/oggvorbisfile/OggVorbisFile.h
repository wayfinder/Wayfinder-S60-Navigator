// OggVorbisFile.h
//
// Copyright © Symbian Software Ltd 2004.  All rights reserved.
//

#ifndef __OGGVORBISFILE_H
#define __OGGVORBISFILE_H

class COggVorbisFile : public CBase, public MDataSource
    {
    class CBody;
public:
    IMPORT_C static COggVorbisFile* NewL(TUid aPlayUid);
    
    IMPORT_C ~COggVorbisFile();
    
    IMPORT_C void OpenL(CMMFClip& aClip); // open and verify

    IMPORT_C TBool IsOpen();
    
    IMPORT_C TTimeIntervalMicroSeconds Duration();
    
    IMPORT_C void PCMOutL(CMMFDataBuffer& aBuffer);
    
    IMPORT_C TInt GetDecoderPositionL();
    
    IMPORT_C void SetPositionL(TTimeIntervalMicroSeconds aPos);
    
    IMPORT_C TInt GetNumberOfMetaDataEntries();
    
    IMPORT_C CMMFMetaDataEntry* GetMetaDataEntryL(TInt aIndex);
    
    IMPORT_C void ResetL();
    
    IMPORT_C void SetSink(MDataSink* aSink);
    
    // from MDataSource:
    virtual TFourCC SourceDataTypeCode(TMediaId aMediaId);
    virtual void FillBufferL(CMMFBuffer* aBuffer, MDataSink* aConsumer,TMediaId aMediaId);
    virtual void BufferEmptiedL(CMMFBuffer* aBuffer);
    virtual TBool CanCreateSourceBuffer();
    virtual CMMFBuffer* CreateSourceBufferL(TMediaId aMediaId, TBool &aReference);
    virtual void ConstructSourceL(  const TDesC8& aInitData );
    
    //informational:
    IMPORT_C TInt SampleRate();
    IMPORT_C TInt Channels();
    IMPORT_C TInt BytesPerWord();
    IMPORT_C TInt BitRate();
    
    IMPORT_C void DbgSetHeapFail(RHeap::TAllocFail aType, TInt aWhen);
    IMPORT_C void DbgHeapMark();
    IMPORT_C void DbgHeapMarkEnd();
    
private:
    COggVorbisFile(TUid aPlayUid);
    void ConstructL();
    
private:
    CBody* iBody;
    };
    
    
#endif
