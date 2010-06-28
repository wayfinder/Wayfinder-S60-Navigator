/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef NAV2INTERFACE_H
#define NAV2INTERFACE_H

#include "machine.h"
#include "GuiProt/ServerEnums.h"
#include <list>
#include <vector>
#include "WFID.h"


class TRequestStatus;

namespace isab{
   //Forward declarations
   class Buffer;
   class QueueSerial;
   class Nav2Release;
   class Nav2Develop;
   class Thread;
   namespace Nav2Error {
      class Nav2ErrorTable;
   }
   class AudioCtrlLanguage;
   class Nav2;
   class Module;
   class CtrlHub;
   class Log;
   class GlobalData;
   class TlsGlobalData;
   class ArchGlobalData;
}

#ifdef __SYMBIAN32__
void Panic(::isab::Thread* thread);
#endif

namespace isab{

   /** This class is used for passing parameters to Nav2's
    * constructor. It has no other purpose.*/
   class Nav2StartupData {
      public:
      /** This enum is used to specify which GPS protocol to use. This
       * affects which kind GPS module will be instantiated in Nav2.*/
      enum GpsProtocol {
         /** Use the NMEA 0183 interpreter.*/
         NMEA,
         /** Use the Trimble interpreter.*/
         Trimble,
      };

      /** Constructor.
       * The constructor takes three path arguments;
       * * Language resource base path. (LRBP)
       * * Common data base path. (CDBP)
       * * Data base path. (DBP)
       *
       * The LRBP is the base point of the sound clips used to
       * construct the voice instructions. 
       * This directory will contain subdirectories for different
       * encodings, i.e. Vorbis.
       * The encodeing directories will then have subdirectories for
       * different languages, i.e. EN, SV, DE, RU, IS, TU, PO.
       *
       * The CDBP is the base directory for all data files that are
       * the same no matter the architecture. This is useful for
       * making preinstalled media that can be moved between different
       * devices. This directory will contain the map cache, the
       * parameter files, and saved routes.
       *
       * The DBP is the base directory for platform dependent data
       * files. This directory should have a name that is associated
       * with the platform (wayfinder_s80 for Symbian Series 80). 
       *
       * @param wft  Wayfinder type. One of the values from 
       *             isab::GuiProtEnums::WayfinderType.
       * @param rbp  Language resource base path. 
       * @param cdbp Common data base path.
       * @param dbp  Data base path.
       * @param etbl Pointer to an error message table of the correct
       *             language. Nav2 will take ownership of this object.
       * @param asyn Pointer to an AudioCtrlLanguage object of the 
       *             correct language. Nav2 will take ownership 
       *             of this object.
       */
      Nav2StartupData(GuiProtEnums::WayfinderType wft,
                      const char* rbp,
                      const char* cdbp,
                      const char* dbp,
                      Nav2Error::Nav2ErrorTable * etbl,
                      AudioCtrlLanguage * asyn) :
         wfType            (wft),
         resourceBasePath  (rbp),
         commondataBasePath (cdbp),
         dataBasePath        (dbp),
         errorTable          (etbl), // Ownership is taken of this object
         audioSyntax         (asyn), // Ownership is taken of this object
         simulatorHost       (NULL),
         simulatorPort       (0),
         simulateFromFile    (NULL),         
         debugOutput         (NULL),
         superLogLevels      (0xff), //LOG_ALL
         testshellHost       (NULL),
         testshellPort       (0),
         guiHost             (NULL),
         guiPort             (0),
         extraServerHost     (NULL),
         extraServerPort     (0),
         fakeNavServerCom    (false),
         testshellRegTest    (false),
         majorVer            (0),
         minorVer            (0),
         buildVer            (0),
         majorRes            (0),
         minorRes            (0),
         buildRes            (0),
         clientType          ("no-arch-set"),
         clientTypeOptions   ("none"),
         useExceptionHandler (true),
         language            (-1),
         gpsProtocol         (NMEA),
         serialNumber        (NULL),
         httpProto           (false),
         httpUserAgent       (NULL),
         httpRequest         (NULL),
         httpHostOverride    (NULL),
         useIAPProxy         (false),
         hardIapProxyHost    (NULL),
         hardIapProxyPort    (8080),
         userClipString      (NULL),
         tcpsymbianserial    (false),
         useTracking         (true),
         using_fav_ver_2     (true),
         allowWLAN           (false),
         maxnbroutstanding   (MAX_UINT32),
         licenceKeyType      (NULL),
         wayfinderIDStartUP  (false)
         //hardwareIDs       Doesn't have a default value.
      { }

      ~Nav2StartupData()
      {
         for (std::vector<isab::HWID*>::iterator it = hardwareIDs.begin();
              it != hardwareIDs.end(); it++) {
            delete *it;
         }
         hardwareIDs.clear();
      }


      /** Sets options for receiving simulated GPS data from a tcp socket.
       * @param host a fully qualified domain name, 
       *             or NULL for a listening socket.
       * @param port the port number for the remote host or listening socket.
       */
      void setSimulation(const char *host, uint16 port) 
                      { simulatorHost = host; simulatorPort=port; }

      /** Sets options for receiving simulated GPS data from a tcp socket.
       * @param host a fully qualified domain name, 
       *             or NULL for a listening socket.
       * @param port the port number for the remote host or listening socket.
       * @param prot the GPS protocol to use.
       */
      void setSimulation(const char *host, uint16 port, enum GpsProtocol prot) 
      { simulatorHost = host; simulatorPort=port; gpsProtocol = prot;}
      
      /** Sets options for a TestShell connection.
       * @param host a fully qualified domain name, 
       *             or NULL for a listening socket.
       * @param port the port number for the remote host or listening socket.
       */
      void setTestShell(const char *host, uint16 port) 
                      { testshellHost = host; testshellPort=port; }

      /** Sets options for a internet connection to GUI. If this
       * option is set it will be impossible to connect to Nav2 through
       * the Channel interface.
       * @param host a fully qualified domain name, 
       *             or NULL for a listening socket.
       * @param port the port number for the remote host or listening socket.
       */
      void setGui(const char *host, uint16 port) 
                      { guiHost = host; guiPort=port; }

      /** Adds a server to the server list. The added server will have
       * higher priority than any server in the parameter file, but
       * will not be stored in the parameter file.
       * @param host a fully qualified domain name.
       * @param port the port number for the remote host.
       */
      void setExtraServer(const char *host, uint16 port) 
                      { extraServerHost = host; extraServerPort=port; }
      /** Sets the program version numbers reported to the server.
       * @param major program major version.
       * @param minor program minor version.
       * @param build program build sequence number.*/
      void setProgramVersion(uint32 major, uint32 minor, uint32 build)
                      { majorVer=major; minorVer=minor; buildVer=build; }

      /** Sets the resource version numbers reported to the server.
       * @param major resource major version.
       * @param minor resource minor version.
       * @param build resource build sequence number.*/
      void setResourceVersion(uint32 major, uint32 minor, uint32 build)
                      { majorRes=major; minorRes=minor; buildRes=build; }
      /** If this option is set Nav2 will try to load a file of
       * simulated GPS data instead of accepting GPS data through a
       * channel or over the Internet. This option overrides the
       * options set through setSimulate.
       * @param file the file name. The file is assumed to be found 
       *             in the directory specified in Wayfinder base path 
       */
      void setSimulateFromFile(const char* file){ simulateFromFile = file; }
      /** If this option is set, this serial number will be used to
       * identify the client in all communication with the server. If
       * it is not set, Nav2 will try to retrieve the serial number by
       * using the GetIMEI function. This function does not work on
       * all platforms, so be careful.*/
      void setSerialNumber(const char* sNo){ serialNumber = sNo; }

      /** 
       * If this option is set, all communication with the server will be 
       * http encapsulated.
       */
      void setHttpProto( bool val ) { httpProto = val; }

      /**
       * If set to a non NULL value this string is used as Http User-
       * Agent header field.
       */
      void setHttpUserAgent( const char* val ) { httpUserAgent = val; }

      /** 
       * If this option is set, the IAP will be checked for proxy setting
       * and used if set in the IAP.
       */
      void setUseIAPProxy( bool val ) { useIAPProxy = val; }

      /** 
       * If this option is set, the proxy host of the IAP hardcoded.
       * See also setHardIapProxyPort.
       */
      void setHardIapProxyHost( const char* val ) { hardIapProxyHost = val; }

      /** 
       * If this option is set, the proxy port of the IAP hardcoded.
       */
      void setHardIapProxyPort( uint16 val ) { hardIapProxyPort = val; }

      /** 
       * Pass a clipping string to UiCtrl, which will affect the username
       * as seen by the user.
       */
      void setUsernameClipString( const char* val ) { userClipString = val; }

      /** 
       * If this option is set, the tracking will be used depending on
       * tracking parameter. If not set no tracking will be done.
       */
      void setUseTracking( bool val ) { useTracking = val; }

      /**
       * Set the maximum number of outstanding server requests.
       */
      void setmaxnbroutstanding( uint32 m ) { maxnbroutstanding = m; }

      /**
       * Set the licence key type. "imei" for imei or "imsi" for imsi.
       * Defaults to IMEI if not explicitly set to a vlaue.
       */
      void setLicenceKeyType( const char* l ) { licenceKeyType = l; } 

      /** The Wayfinder type, should be one of the values in
          isab::GuiProtEnums::WayfinderType. This parameter affects
          certain behaviour, mostly in the GUI, and is reported to the
          server at each connection.*/
      enum GuiProtEnums::WayfinderType wfType;
      /** The path to the image resources. 
       * Typically "\system\apps\wayfinderimg".*/
      const char*                 resourceBasePath;
      /** The Wayfinder common data path.
       * Typically "\system\data\wayfinder".*/
      const char*                 commondataBasePath;
      /** The Wayfinder branded data path.
       * Typically "\system\data\brandname".*/
      const char*                 dataBasePath;
      /** Pointer to an error message table for the chosen language.*/
      Nav2Error::Nav2ErrorTable * errorTable;
      /** Pointer to an AudioCtrlLanguage object for the chose language.*/
      AudioCtrlLanguage *         audioSyntax;
      /** FQDN of the internet host that provides simulated GPS data.
       * If NULL and simulatorPort != 0, Nav2 will open a listening
       * socket instead.*/
      const char *                simulatorHost;
      /** The port on the remote host or of the listening socket used
       * for simulated GPS data. If 0, no socket will be opened.*/
      uint16                      simulatorPort;
      /** The file to load simulated GPS data from. If NULL no file
       * will be opened. If set has higher priority than
       * simulatorPort.*/
      const char*                 simulateFromFile;
      /** A string describing where to send debug output. See
       * isab::Log::addOutput for legal output specifer string.*/
      const char*                 debugOutput;
      /** TODO */
      uint8                       superLogLevels;
      /** FQDN of the internet host that provides TestShell
       * connection.  If NULL and testshellPort != 0, Nav2 will open a
       * listening socket instead.*/
      const char*                 testshellHost;
      /** The port on the remote host or of the listening socket used
       * for TestShell. If 0, no socket will be opened and no
       * TestShell object created.*/
      uint16                      testshellPort;
      /** FQDN of the internet host that provide the user interface.
       * If NULL and guiPort != 0, Nav2 will open a
       * listening socket instead.*/
      const char*                 guiHost;
      /** The port on the remote host or of the listening socket used
       * for user interface connection. If 0, no socket will be opened.
       * If set to anything but 0, the Channel interface will be inactive.
       */
      uint16                      guiPort;
      /** The FQDN of an extra non-persistent server. This server will
       * have a higher priority than the persistent servers.*/
      const char*                 extraServerHost;
      /** The port of the extra server.*/
      uint16                      extraServerPort;
      /** Use FakeNavServerCom instead of NavServerCom. This means *
       * that most server requests will receive no reply. Search and
       * route requests will have a set answer independent of request
       * parameters. */
      bool                        fakeNavServerCom;
      /** Sets the regression test flag in TestShell. Affects
       * TestShell output.*/
      bool                        testshellRegTest;
      /** The program versions as reported to the server.*/
      uint32                      majorVer, minorVer, buildVer;
      /** The resource versions as reported to the server.*/
      uint32                      majorRes, minorRes, buildRes;
      /** The client type as reported to the server. Only a few
       * strings are accepted by the server, see the client_type
       * documentation. The clienttype string specifies certain
       * behaviour in the server.*/
      const char*                 clientType;
      /** This string can be used in combination with the clientType
       * string in the server to select different service levels.*/
      const char*                 clientTypeOptions;
      /** This turns on the exception handler in Nav2. The exception
       * handler should always be used in release build. */
      bool                        useExceptionHandler;
      /** Specifies the language used. See
       * isab::NavServerComEnums::languageCode for legal values.*/
      /* NOTE!!! This parameter is currently unused! */
      int                         language;
      /** Specifies the gps protocol to use. */
      enum GpsProtocol            gpsProtocol;
      /** Device serial number. Typically IEMI or NES. If NULL,
       * NavServerCom will attempt to retrieve the serial number by
       * using the GetIMEI function. This function doesn't work on all
       * platforms, so beware.*/
      const char*                 serialNumber;

      /**
       * If using Http encapsulation, default false.
       */
      bool httpProto;

      /**
       * The Http User-Agent, default NULL.
       */
      const char* httpUserAgent;

      /**
       * The HTTP request, default NULL.
       */
      const char* httpRequest;

      /**
       * If not NULL will override the content of the HTTP Host header.
       * Default NULL.
       */
      const char* httpHostOverride;
      /**
       * If to use IAP proxy setting, default false.
       */
      bool useIAPProxy;

      /**
       * The hardcoded IAP proxy host, if any.
       */
      const char* hardIapProxyHost;

      /**
       * The hardcoded IAP proxy port, see also hardIapProxyHost.
       */
      uint16 hardIapProxyPort;

      /**
       * Username clip string, if any.
       */
      const char* userClipString;

      ///use the special tcpsymbianserial module.
      bool tcpsymbianserial;

      /**
       * If true the tracking will be used depending on
       * tracking parameter. If not set no tracking will be done.
       */
      bool useTracking;

      /**
       * If gui uses favorites version 2.
       */
      bool using_fav_ver_2;

      /**
       * Set allowWLAN to true to make the TCP layer consider WLAN
       * connections.
       * Only works on UIQ3. 
       */
      bool allowWLAN;

      /**
       * The maximum number of outstanding server requests.
       */
      uint32 maxnbroutstanding;

      /**
       * The licence key type. "imei" for imei or "imsi" for imsi.
       */
      const char* licenceKeyType;

      /**
       * If using WF ID startup
       */
      bool wayfinderIDStartUP;

      /**
       * A vector that contains a struct with all hardware id:s.
       */
      std::vector<HWID*> hardwareIDs; 

   };
   
   /// This is a RAII class that starts and stops the Nav2 processes
   /// on creation resp. destruction.
   class Nav2{
   public:
      ///Constructs a Nav2 system and starts it.
      ///@param sdata an instance of Nav2StartupData containing
      ///all startup options for Nav2.
      Nav2(Nav2StartupData &sdata);
      
      /// This function causes Nav2 to start shutting down, but
      /// doesn't block execution. No cleanup is made.
      void Halt();

#ifdef __SYMBIAN32__
      ///Called in roundabout ways from the Nav2 ExceptionHandler.
      ///@param thread pointer to the thread that caused the exception.
      void Panic(Thread* thread);
#endif

      /// Since the Nav2 object exposes two datastreams, Bluetooth and
      /// GuiData, each stream is handled through a Channel object.
      class Channel{
         friend class Nav2;
         /// Pointer to the QueueSerial object used for this data stream.
         QueueSerial* m_qs;
         ///Private constructor, used only by Nav2.
         Channel(QueueSerial* qs);
      public:
         /// Write data into the channel.
         ///@param data pointer to the data.
         ///@param length number of bytes to write.
         ///@return true if everything is well with Nav2, false if a
         ///        panic or similar has occured.
         bool writeData(const uint8* data, int length);
         
         /// Reads data from Channel.  
         ///@param buf a pointer to the Buffer where the read data 
         ///           shall be copied. This pointer must not be NULL. 
         ///@return true if everything is well with Nav2, false if a
         ///        panic or similar has occured.
         bool readData(Buffer* buf);

#ifdef __SYMBIAN32__
         void armReader(TRequestStatus *reqStat);
         void cancelArm();
#endif

         ///Signals to Nav2 that there is someone listening and/or
         ///writing to this channel.
         void connect();

         ///Signals to Nav2 that there is no longer anyone listening
         ///and/or writing on this channel. Must be called before Halt.
         void disconnect();

         ///Returns the recommended poll interval of this channel.
         ///@return the recommended poll interval in microseconds.
         int getPollInterval();

         ///Tests if there is any data available to read.  
         ///@return true if there is any data to read from this
         ///channel, false otherwise.
         bool empty();
      };
      
      ///Factory method that returns a new Channel object connected to
      ///the GPS data channel. It is the callers responsibility to
      ///delete the returned object.
      ///NOTE: The Channel will not accept the same kind of GPS data as the 
      ///      TCP simulator data port. GPS data that are input through the 
      ///      Channel object are assumed to be in Sirf? format.
      ///@return a Channel object.
      class Channel* getBluetoothChannel();
      ///Factory method that returns a new Channel object connected to
      ///the Gui message channel. It is the callers responsibility to
      ///delete the returned object.
      ///@return a Channel object.
      class Channel* getGuiChannel();

      /// The destructor. It's not necessarry to call disconnect on
      /// any Channel objects before deleteing the Nav2 object, but it
      /// is recommended.
      virtual ~Nav2();

      /** A blocking function that waits until all threads in Nav2 has
       * shut down, and then returns all resources. This should be
       * called instead of just destroying the object if a blocking
       * behaviour is wanted. */
      void WaitNavigate2();

   protected:
      /** Shuts down the Nav2 system, wait for all thread to die and then
       * returns all resources to the system. this call may block for a
       * while, and it may be better to first call StartShutdownNavigate2
       * and later call WaitNavigate2.*/
      void ShutdownNavigate2();

      /** Starts the shudown procedure of the Nav2 system built by
       * StartNavigate2.  Don't forget to call WaitNavigate2 later so that
       * all resources are deallocated properly.
       */
      void StartShutdownNavigate2();

      QueueSerial* m_gui;
      QueueSerial* m_bt;

      /* Stuff that used to be in StartNavigate2Data */
      typedef std::list<Module *> ModuleList;
      typedef std::list<Module *>::iterator ModuleIterator;
      ModuleList allModules;
      CtrlHub* hub;

      int initialized;
      int shutdownstarted;
      Log* mainlog;

      Nav2Error::Nav2ErrorTable * m_errorTable;
      AudioCtrlLanguage         * m_audioSyntax;

# ifdef __SYMBIAN32__
      TlsGlobalData *m_tlsGlobalData;
      ArchGlobalData *m_archGlobalData;
# endif
      GlobalData *m_globalData;

   protected:
      void CleanupNavigate2();
      void deleteModule(Module* module);
      void joinModule(Module* module, const char* name, long millis, int nanos);
   };

   class Nav2Release : public Nav2 {
   public:
      Nav2Release(Nav2StartupData &sdata);
   };

   class Nav2Develop : public Nav2 {
   public:
      Nav2Develop(Nav2StartupData &sdata);
   };

   class Nav2Linux : public Nav2 {
   public:
      Nav2Linux(Nav2StartupData &sdata);
   };

}
#endif
