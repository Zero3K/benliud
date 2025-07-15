// IconLibrary.h: interface for the CIconLibrary class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ICONLIBRARY_H
#define _ICONLIBRARY_H

#include <wx/wx.h>
#include <wx/bitmap.h>
//global icon library for all code
class CIconLibrary  
{
public:

	wxBitmap Get_Bitmap_Small_Files();
	wxBitmap Get_Bitmap_Small_Files_Gray();
	wxBitmap Get_Bitmap_Small_Conf();
	wxBitmap Get_Bitmap_Small_Conf_Gray();
	wxBitmap Get_Bitmap_Small_Info();
	wxBitmap Get_Bitmap_Small_Info_Gray();
	wxBitmap Get_Bitmap_Small_Run();
	wxBitmap Get_Bitmap_Small_Run_Gray();
	wxBitmap Get_Bitmap_Small_Stop();
	wxBitmap Get_Bitmap_Small_Stop_Gray();


	wxBitmap Get_Bitmap_Small_Thindown();
	wxBitmap Get_Bitmap_Small_Thinup();
	wxBitmap Get_Bitmap_Small_YellowDown();
	wxBitmap Get_Bitmap_Small_RedDown();
	wxBitmap Get_Bitmap_Small_GrayDown();
	wxBitmap Get_Bitmap_Small_Up();
	wxBitmap Get_Bitmap_Small_GreenDown();

	wxBitmap Get_Bitmap_Memory();
	wxBitmap Get_Bitmap_Middle_DBPlus();
	wxBitmap Get_Bitmap_Small_Main_Xpm();
	wxBitmap Get_Bitmap_Small_RedClose();
	wxBitmap Get_Bitmap_Small_Close();
	wxBitmap Get_Bitmap_Small_LeftArrow();
	wxBitmap Get_Bitmap_Middle_Make();
	wxBitmap Get_Bitmap_Large_Main_Png();
	wxBitmap Get_Bitmap_Small_Main_Png();

	wxBitmap Get_Bitmap_Small_RedLight();
	wxBitmap Get_Bitmap_Small_Browser();
	wxBitmap Get_Bitmap_Small_Torrent();
	wxBitmap Get_Bitmap_Small_FileManager();
	wxBitmap Get_Bitmap_Small_Log();
	//wxBitmap Get_Bitmap_Large_Torrent();
	wxBitmap Get_Bitmap_Small_YellowLight();
	wxBitmap Get_Bitmap_Small_GreenLight();
	wxBitmap Get_Bitmap_Small_GrayLight();

	//wxBitmap Get_Bitmap_Large_Active();
	//wxBitmap Get_Bitmap_Large_FileAdmin();
	//wxBitmap Get_Bitmap_Large_Search();
	//wxBitmap Get_Bitmap_Large_Plugin();
	//wxBitmap Get_Bitmap_Large_Loginfo();
	//wxBitmap Get_Bitmap_Large_Option();
	//wxBitmap Get_Bitmap_Large_Bulletin();
	wxBitmap Get_Bitmap_Small_Flag();
	int	Get_Bitmap_Small_Number();
	wxBitmap Get_Bitmap_Small(int i);
	wxBitmap Get_Bitmap_Small_Search();
	wxBitmap Get_Bitmap_Small_Editor();
	wxIcon Get_Icon_Small_Logo();
	wxBitmap Get_Bitmap_Small_Logo();
	wxBitmap Get_Bitmap_Middle_Delete();
	wxBitmap Get_Bitmap_Middle_Pause();
	wxBitmap Get_Bitmap_Middle_Run();
	wxBitmap Get_Bitmap_Middle_New();
	wxBitmap Get_Bitmap_Middle_Open();
	wxBitmap Get_Bitmap_Middle_Config();
	wxBitmap Get_Bitmap_Middle_Forum();
	wxBitmap Get_Bitmap_Small_Finish();
	wxBitmap Get_Bitmap_Small_Fail();
	wxBitmap Get_Bitmap_Small_Stoping();
	wxBitmap Get_Bitmap_Small_StopingDelete();
	wxBitmap Get_Bitmap_Small_Pause();
	wxBitmap Get_Bitmap_Small_Wait();
	//wxBitmap Get_Bitmap_Small_Run();

	//wxBitmap Get_Bitmap_Small_MsgInfo();
	//wxBitmap Get_Bitmap_Small_MsgError();
	//wxBitmap Get_Bitmap_Small_MsgIn();
	//wxBitmap Get_Bitmap_Small_MsgOut();
	//wxBitmap Get_Bitmap_Small_MsgWarn();
	//wxBitmap Get_Bitmap_Small_MsgSucc();
	//wxBitmap Get_Bitmap_Small_MsgSys();
	wxBitmap Get_Bitmap_Small_Stat();
	wxBitmap Get_Bitmap_Small_Server();
	wxBitmap Get_Bitmap_Small_Options();
//	wxBitmap Get_Bitmap_Small_Files();
	wxBitmap Get_Bitmap_Small_New();
	wxBitmap Get_Bitmap_Small_Question();
	wxBitmap Get_Bitmap_Small_Answer();
	wxBitmap Get_Bitmap_Small_Share();
	wxBitmap Get_Bitmap_Small_Dotdotdot();
	wxBitmap Get_Bitmap_Middle_DBMinus();
	wxBitmap Get_Bitmap_Middle_Main_Png();
	wxBitmap Get_Bitmap_Small_Users_Png();
	CIconLibrary();
	virtual ~CIconLibrary();

};

#endif
