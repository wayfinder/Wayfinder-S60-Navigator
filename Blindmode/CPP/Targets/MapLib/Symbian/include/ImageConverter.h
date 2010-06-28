/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMGCONVERTER_H
#define IMGCONVERTER_H

#include <coecntrl.h>  // for coe control
#include <bitmaptransforms.h>

#include "MapPlotter.h"

using namespace isab;

// class CImage;
// class CImageRotator;

/** This class handles asynchronous image loads of gifs, pngs, and
 * other formats.  The class is meant to be used from within the
 * CImage class.*/
class CImageConverter : public CActive
{
   /** The CImage object that shall receive loaded images.*/
   class CImage& iPicture;
   /** The asynchronous service provider.*/
   class CImageDecoder* iDecoder;
   /** Bitmap to store the image in.*/
   class CFbsBitmap* iBitmap;
   /** If the loaded image supports transparency or alpha channels,
    * this variable will hold the necessary mask.*/
   class CFbsBitmap* iMask;
   /** This object is used to rotate images.*/
   class CBitmapRotator* iBmpRotator;
protected:
   /** Constructor. Protected since it's not meant for direct
    * use. Create new CImageConverter object through the static NewLC
    * and NewL functions.
    * @param aPicture the CImage object that shall receive loaded images.
    * @param aPriority the priority assigned to the asynchronous 
    *                  conversion request.
    *                  defaults to <code>EPriorityStandard</code>
    */
   CImageConverter(class CImage& aPicture, 
                   enum TPriority aPriority = EPriorityStandard);
   /** Second part of the two-phase construction. 
    * @param aFs the filesession to use. It's very important that no other 
    *            part of the program closes this session while the 
    *            CImageConverter is using it.
    * @param aFile the fully qualified filename of the image to load.
    */
   void ConstructL(RFs aFs, const TDesC& aFile);
public:
   /** Starts an asynchronous image load and pushes the
    * CimageConverter object representing it onto the CleanupStack.
    * @param aPicture the CImage object that shall receive the loaded image.
    * @param aFs the filesession to use for file operations. 
    * @param aFile the fully qualified filename of the image to load.
    * @param aPriority the priority of this asynchronous request. 
    *                  Defaults to <code>EPriorityStandard</code>.
    * @return the object that represents this asynchronous request.
    */
   static class CImageConverter* NewLC(class CImage& aPicture, 
                                       RFs aFs, const TDesC& aFile,
                                       enum TPriority aPriority = EPriorityStandard);
   /** Starts an asynchronous image load.
    * @param aPicture the CImage object that shall receive the loaded image.
    * @param aFs the filesession to use for file operations. 
    * @param aFile the fully qualified filename of the image to load.
    * @param aPriority the priority of this asynchronous request. 
    *                  Defaults to <code>EPriorityStandard</code>.
    * @return the object that represents this asynchronous request.
    */
   static class CImageConverter* NewL(class CImage& aPicture, 
                                      RFs aFs, const TDesC& aFile,
                                      enum TPriority aPriority = EPriorityStandard);
   /** Destructor.*/
   virtual ~CImageConverter();
   /** Handles the Active objects request completion event. 
    * Calls the <code>DecodeComplete</code> member function of the 
    * <code>iPicture</code> object.*/
   virtual void RunL();
   /** Implements cancellation of an outstanding request, in this case
    * the loading and converting of an image file.*/
   virtual void DoCancel();
   
   
   // My Rotate Message
   void RotateBitmap(CBitmapRotator::TRotationAngle);   
      
};


/** Objects of this class represents a single image loaded from
 *  file. The image can be drawn to the screen.  */
class CImage : public CCoeControl, public isab::BitMap
{
public:
   
   enum TImageRotateAngle{
      E0Degrees      = 0,
      E360Degrees    = 0,
      ENoRotation    = 0,
      E90Degrees     = 1,
      E180Degrees    = 2,
      E270Degrees    = 3
   };      
   
   enum TRotateStatus{
      ERotationNone      = 5,
      ERotationPending   = 6,
      ERotationDone      = 7
   };      
   
   
   
public: // Constructors and destructor
   /** Constructor. */
   CImage();
   
   /** Destructor. */
   ~CImage();
   /** Second phase of the two phase construction.*/ 
   void ConstructL( );

public: // New functions

   
		/* flags for loading status */
		enum
		{
			ENoneLoaded=10,
			ELoading,
			EConverting,
			ELoaded
		};
		/* flags for type of file to load */
		enum
		{
			EImageNull=100,
			EImageBMP,
			EImageGIF,
			EImagePNG,
			EImageJPG,
			EImageMBM,
			EImageBMPWithMask,
			EImageGIFWithMask,
			EImagePNGWithMask,
			EImageJPGWithMask,
			EImageMBMWithMask,
			EImageMemBMP,
			EImageMemGIF,
			EImageMemPNG,
			EImageMemJPG,
			EImageMemBMPWithMask,
			EImageMemGIFWithMask,
			EImageMemPNGWithMask,
			EImageMemJPGWithMask
		};



   /** This function is called by CImageDecoder objects when they are done.
    * @param aBitmap a pointer to a bitmap object. This CImage object receives 
    *                ownership of the object that aBitmap points at. 
    * @param aMask a pointer to a bitmap object containing the mask of 
    *              the loaded image. May be NULL. If non-NULL, this 
    *              CImage object receives ownership of the bitmap 
    *              pointed to by aMask.*/
   void DecodeComplete(class CFbsBitmap* aBitmap, 
                       class CFbsBitmap* aMask);
   /** Causes the current image to be discarded and a new image to be
    * loaded from file.
    * @param aFs a filesession object.
    * @param aFile the fully qualified filename of the image file to load.*/
   void OpenL( RFs& aFs, const TDesC &aFile );
   /** Called when an asynchronous image load operation failed or was
    * canceled.*/
   void DecodeCanceled();
   /** Rotate the imageclockwise the specified angle.
    * @param aAngle the amount to rotate.*/
   void RotateImageClockwiseL(TImageRotateAngle aAngle);
   /** Callback for when a rotation is complete. */
   void RotationComplete();
   //   void SizeChanged();

   TSize GetSize() const;

   TBool IsReady() const;

private: 
   /** Called when the bitmap, but not mask, has been rotated.*/
   void RotateImage();
   /** This function is called by the window server when this
    * component needs to be redrawn.
    * @param aRect the region of the control to be
    *              redrawn. Co-ordinates are relative to this 
    *              controls origin (top left corner).
    */
   void Draw(const TRect& aRect) const;
   
private: //data
   /** Pointer to active imageconverter. Supposed to be null whenever
    * a convertion is in progress.*/
   mutable class CImageConverter* iActive;
   /** Local RFs. Should be removed.*/
   RFs iFs;
   /** Rotator object should be NULL whenever rotation is not in progress.*/
   class CImageRotator* iBmpRot;
   TImageRotateAngle iAngle;
   TRotateStatus iRotateStatus;

   TBool iIsReady;
public:
   class CFbsBitmap* iBitmap;
   class CFbsBitmap* iMask;
};





class CImageRotator : public CActive
{
public:
   /** The CImage object that shall receive loaded images.*/
   TInt RotationNum ;
   class CImage* iPicture;
   CBitmapRotator::TRotationAngle iAngle;    
   /** The asynchronous service provider.*/
   /** Bitmap to store the image in.*/
   class CFbsBitmap* iBitmap;
   class CFbsBitmap* iMask;
   class CBitmapRotator* iBmpRotator;
protected:
   /** Constructor. Protected since it's not meant for direct
    * use. Create new CImageConverter object through the static NewLC
    * and NewL functions.
    * @param aPicture the CImage object that shall receive loaded images.
    * @param aPriority the priority assigned to the saynchronous conversion request.
    *                  defaults to <code>EPriorityStandard</code>
    */
   CImageRotator(class CImage* aPicture, CBitmapRotator::TRotationAngle aAngle,
                   enum TPriority aPriority = EPriorityStandard);
   /** Second part of the two-phase construction. 
    * 
    */
   void ConstructL();
public:
   /** Starts an asynchronous image load and pushes the
    * CimageConverter object representing it onto the CleanupStack.
    * @param aPicture the CImage object that shall receive the loaded image.
    * 
    * @param aPriority the priority of this asynchronous request. 
    *                  Defaults to <code>EPriorityStandard</code>.
    * @return the object that represents this asynchronous request.
    */
   static class CImageRotator* NewLC(class CImage* aPicture, 
                                       CBitmapRotator::TRotationAngle aAngle,
                                       enum TPriority aPriority = EPriorityStandard);
   /** Starts an asynchronous image load.
    * @param aPicture the CImage object that shall receive the loaded image.
    * 
    * @param aPriority the priority of this asynchronous request. 
    *                  Defaults to <code>EPriorityStandard</code>.
    * @return the object that represents this asynchronous request.
    */
   static class CImageRotator* NewL(class CImage* aPicture, 
                                      CBitmapRotator::TRotationAngle aAngle,
                                      enum TPriority aPriority = EPriorityStandard);
   /** Destructor.*/
   virtual ~CImageRotator();
   /** Handles the Active objects request completion event. 
    * Calls the <code>DecodeComplete</code> member function of the 
    * <code>iPicture</code> object.*/
   virtual void RunL();
   /** Implements cancellation of an outstanding request, in this case
    * the loading and converting of an image file.*/
   virtual void DoCancel();
   
   void RotateBitmap();  
      
};

#endif

// End of File
