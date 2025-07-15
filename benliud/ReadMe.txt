================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : benliud Project Overview
===============================================================================

The application wizard has created this benliud application for 
you.  This application not only demonstrates the basics of using the Microsoft 
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your benliud application.

benliud.vcproj
    This is the main project file for VC++ projects generated using an application wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    application wizard.

benliud.h
    This is the main header file for the application.  It includes other
    project specific headers and declares the CbenliudApp application class.

benliud.cpp
    This is the main application source file that contains the application
    class CbenliudApp.



benliudsp.rc
    This is the project's main resource file listing of all of the Microsoft Windows
    resources that the project uses when compiling for the Smartphone platform, or a
    platform that supports the same user interface model.  It includes the icons,
    bitmaps, and cursors that are stored in the RES subdirectory.  This file can be 
    directly edited in Microsoft Visual C++. Your project resources are in 
    1033. When the .rc file is persisted, the defines in the data section
    are persisted as the hexadecimal version of the numeric value they are defined to
    rather than the friendly name of the define.

res\benliudsp.rc2
    This file contains resources that are not edited by Microsoft 
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

res\benliud.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file.


/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.


/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

benliudDoc.h, benliudDoc.cpp - the document
    These files contain your CbenliudDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CbenliudDoc::Serialize).

benliudView.h, benliudView.cpp - the view of the document
    These files contain your CbenliudView class.
    CbenliudView objects are used to view CbenliudDoc objects.




/////////////////////////////////////////////////////////////////////////////

Other Features:

   Command bar in main frame with menu and adornments

Windows Sockets
    The application has support for establishing communications over TCP/IP networks.

/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named benliud.pch and a precompiled types file named StdAfx.obj.

Resourcesp.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is in a 
language other than the operating system's current language, you will need 
to copy the corresponding localized resources MFC90XXX.DLL to your application
directory ("XXX" stands for the language abbreviation.  For example,
MFC90DEU.DLL contains resources translated to German.)  If you don't do this,
some of the UI elements of your application will remain in the language of the
operating system.

/////////////////////////////////////////////////////////////////////////////
