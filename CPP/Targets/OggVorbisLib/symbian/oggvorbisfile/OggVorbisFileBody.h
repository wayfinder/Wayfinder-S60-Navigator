// OggVorbisFileBody.h
//
// Copyright © Symbian Software Ltd 2004.  All rights reserved.
//

#ifndef __OGGVORBISFILEBODY_H
#define __OGGVORBISFILEBODY_H

#include <stdlib.h>
#include "ivorbisfile.h"

class COggVorbisFile::CBody : public CBase
    {
public:
    static CBody* NewL();
    
    CBody::~CBody();
    
    void OpenL(CMMFClip& aClip);
    void PCMOutL(CMMFDataBuffer& aBuffer);
    void ResetL();
    void SetSink(MDataSink* aSink);
    
    TInt BitRate();
    TBool IsOpen();
    TInt GetDecoderPositionL();
    void SetPositionL(TInt aPos);
    TInt Duration();
    TInt SampleRate();
    TInt Channels();

    void DbgSetHeapFail(RHeap::TAllocFail aType, TInt aWhen);
    void DbgHeapMark();
    void DbgHeapMarkEnd();

    // callbacks from libvorbisfile:
    size_t read_func(void* ptr, size_t size, size_t nitems);
    int seek_func(ogg_int64_t offset, int whence);
    int close_func();
    long tell_func();
    
private:
    void ConstructL();
    
public:    
    MDataSink* iSink;
    
private:
    OggVorbis_File iFile;
    CMMFClip* iClip;
    TInt iFilePos;
    TInt iOggSection;
    
#ifdef SEP_HEAP    
    RHeap* iDefaultHeap;
    RHeap* iVorbisHeap;
    RChunk iVorbisChunk;
#endif
    };

size_t ovf_read_func(void* ptr, size_t size, size_t nitems, void* datasource);

int ovf_seek_func(void* datasource, ogg_int64_t offset, int whence);

int ovf_close_func(void* datasource);

long ovf_tell_func(void* datasource);

static const ov_callbacks KOggVorbisCallbacks = 
    { &ovf_read_func, &ovf_seek_func, &ovf_close_func, &ovf_tell_func };




#endif
