Information about Wayfinder Access source code.
-----------------------------------------------

The supplied source code is the Wayfinder Access related parts of the Wayfinder Navigator.
Supported platforms are S60 v2 and v3. 
To build a Wayfinder Access application, it is necessary to combine the supplied source code
here with the source code in the CPP folder directly in the repository.

The supplied source code consists of three different parts:

Blindmode/CPP/Targets/Wayfinder/symbian-r6:
--------------------------------
This is the code that handles the application UI, including views, containers etc.
The following basic functionality is present:
* Functionality to present necessary information in labels that can be read by text-to-speech programs (such as Nuance Talks).
* Where am I functionality, displaying landmark information in relation to user's position.
* Locked navigation mode, which is a special navigation mode when walking with extended information about the user's surroundings.

Blindmode/CPP/Modules:
------------
This is the code that is used for persistant storage of some access specific settings.

Blindmode/CPP/Targets/MapLib:
------------------
This is the code that draws the map and calculates where crossings, poi:s and user defined features (favorites) are located in relation to the user's current position.
This is standalone code (however, it needs the server backend). It is complete in the sense that all files are there in order to compile it (so no need to add additional files from the original MapLib folder in the repository). It is possible to compile a linux gtk based test client for MapLib. See MapLib test client section.

The map data is downloaded from the server backend and then parsed and displayed by the client.


MapLib test client:
--------------------
To compile and run on a linux computer (tested on CentOS 5, 64 bit version):
1. Go to Blindmode/CPP/Targets/MapLib folder 'cd Blindmode/CPP/Targets/MapLib'
2. Run 'make'
3. Run test application 'Client/MapClient/bin-linux-x86_64/MapClient oss-xml.services.wayfinder.com 80'

In case the compilation fails due to missing standard libraries, such as zlib, gtk and similar, these must be installed first.
The test client has the following functionality:
* Detect user defined features (favorites) in the visible map area
* Detect POI:s in the visible map area.
* Detect crossings in the visible map area.
* Click on the map to display information about that position.0
* Pan the map area using arrow keys.


