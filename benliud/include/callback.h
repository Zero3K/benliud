/***************************************************************************
 *            callback.h
 *
 *  Wed Nov 29 22:45:56 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/


#ifndef _CALLBACK_H
#define _CALLBACK_H



#include "../include/module_def.h"
#include "../include/datatype_def.h"
#include "../include/msgtype_def.h"
#include "../include/protocol_def.h"
#include "../include/callback_def.h"
#include "../include/proxytype_def.h"
#include <vector>
#include <string>
#include <wx/wx.h>
//for bittorrent
extern "C" void gBTTaskEvent_CALLBACK(int,BTEVENT,llong,llong,void*);
extern "C" void gBTTaskPiece_CALLBACK(int,BTEVENT,int,int*,float);
extern "C" void gBTTaskName_CALLBACK(int,wchar_t*);

extern "C" bool gCreatedir_CALLBACK(wchar_t*);
extern "C" bool gMB2WC_CALLBACK(const char*, wchar_t*,int,wchar_t*);


//BT DHT call back
extern "C" int gBTKAD_SERVICE(BTKADCALLTYPE ,char* ,char* , int , int* );

//UPNP service callback
extern "C" int gUPNP_SERVICE(UPNPCALLTYPE, char*, unsigned short, bool);

//common service event notice
extern "C" void gSERVICE_EVENT(_SERVICE_TYPE type,_SERVICE_EVENT evt, int intval);

extern "C" bool gDB_FORTSHARE(DBOPTTYPE type,char* hash, int* len, char** content);
extern "C" bool gDB_FORDHT(bool save, int count, unsigned int* ip, unsigned int* port);

extern "C" void gMSG_CALLBACK(const char* msg);

//global common
bool gCONV_TRY(std::vector<std::string>& samples, wxCSConv& conv);
bool gCONV_DETECT(std::vector<std::string>& samples, wxString& encode);
bool gInsert_Torrent_To_DataBase(std::string& content);

#endif
