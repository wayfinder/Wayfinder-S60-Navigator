# Microsoft Developer Studio Project File - Name="MC2Shared" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MC2Shared - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MC2Shared.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MC2Shared.mak" CFG="MC2Shared - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MC2Shared - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MC2Shared - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MC2Shared - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ox /Ot /Og /Op /Ob2 /I "..\..\include" /I "..\..\include\Windows" /I "..\..\..\Windows\include" /D "NDEBUG" /D "WIN32" /D "_LIB" /D "_UNICODE" /D "_WIN32" /D "ARCH_OS_WINDOWS_ACTIVEX" /Fr /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MC2Shared - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Zi /Od /I "..\..\include" /I "..\..\include\Windows" /I "..\..\..\Windows\include" /D "_DEBUG" /D "WIN32" /D "_LIB" /D "_UNICODE" /D "_WIN32" /D "ARCH_OS_WINDOWS_ACTIVEX" /Fr /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "MC2Shared - Win32 Release"
# Name "MC2Shared - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\BitBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\BitmapMovementHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\ClipUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\Cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\DBufRequester.cpp
# End Source File
# Begin Source File

SOURCE=..\DirectedPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\GfxConstants.cpp
# End Source File
# Begin Source File

SOURCE=..\GunzipUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpClientConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\LangTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\MapPlotter.cpp
# End Source File
# Begin Source File

SOURCE=..\MapProjection.cpp
# End Source File
# Begin Source File

SOURCE=..\MC2BoundingBox.cpp
# End Source File
# Begin Source File

SOURCE=..\MC2Coordinate.cpp
# End Source File
# Begin Source File

SOURCE=..\MC2Logging.cpp
# End Source File
# Begin Source File

SOURCE=..\MC2SimpleString.cpp
# End Source File
# Begin Source File

SOURCE=..\MemoryDBufRequester.cpp
# End Source File
# Begin Source File

SOURCE=..\MFileDBufReqIdxHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\MultiFileDBufRequester.cpp
# End Source File
# Begin Source File

SOURCE=..\RandomFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\RouteID.cpp
# End Source File
# Begin Source File

SOURCE=..\ScreenOrWorldCoordinate.cpp
# End Source File
# Begin Source File

SOURCE=..\SharedBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\SharedDBufRequester.cpp
# End Source File
# Begin Source File

SOURCE=..\SharedHttpDBufRequester.cpp
# End Source File
# Begin Source File

SOURCE=..\SharedHttpHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\TileFeature.cpp
# End Source File
# Begin Source File

SOURCE=..\TileFeatureArg.cpp
# End Source File
# Begin Source File

SOURCE=..\TileImportanceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMap.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapFormatDesc.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapGarbage.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapKeyHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapLayerInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapParams.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapTextHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapTextSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\TileMapUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\TransformMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\UserDefinedBitMapFeature.cpp
# End Source File
# Begin Source File

SOURCE=..\UserDefinedFeature.cpp
# End Source File
# Begin Source File

SOURCE=..\UserDefinedScaleFeature.cpp
# End Source File
# Begin Source File

SOURCE=..\WFDRMUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\src\WinBitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\src\WinMapControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\src\WinMapPlotter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\src\WinTCPConnectionHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\src\WinTileMapToolkit.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\Windows\ArchConfigMisc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Windows\ArchDataTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BitBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BitmapMovementHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ClipUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DBufConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DBufRequester.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DirectedPolygon.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\GfxConstants.h
# End Source File
# Begin Source File

SOURCE=..\..\include\GfxUtility.h
# End Source File
# Begin Source File

SOURCE=..\..\include\GunzipUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\GzipUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\HttpClientConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\InsideUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\LangTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\LogFile.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MapPlotter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MapPlotterConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MapPlotterTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MapProjection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MathUtility.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MC2BoundingBox.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MC2Coordinate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MC2Logging.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MC2Point.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MC2SimpleString.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Windows\mcostream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MemoryDBufRequester.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MFileDBufReqIdxHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MultiFileDBufRequester.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OverlapDetector.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ParamsNotice.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PixelBox.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RandomFuncs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RouteID.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ScreenOrWorldCoordinate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SharedBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SharedDBufRequester.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SharedHttpDBufRequester.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SharedHttpHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\TextConv.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileArgNames.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileCategory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileFeature.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileFeatureArg.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileImportanceTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMap.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapContainer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapCoord.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapFormatDesc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapGarbage.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapHandlerTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapKeyHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapLayerInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapParams.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapTextHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapTextSettings.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapToolkit.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TileMapUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TransformMatrix.h
# End Source File
# Begin Source File

SOURCE=..\..\include\UserDefinedBitMapFeature.h
# End Source File
# Begin Source File

SOURCE=..\..\include\UserDefinedFeature.h
# End Source File
# Begin Source File

SOURCE=..\..\include\UserDefinedScaleFeature.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WFDRMUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\WinBitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\WinMapControl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\WinMapPlotter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\WinTCPConnectionHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\include\WinTileMapToolkit.h
# End Source File
# End Group
# End Target
# End Project
