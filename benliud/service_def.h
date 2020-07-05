//service type basic define

#ifndef _SERVICE_DEF_H
#define _SERVICE_DEF_H

enum _SERVICE_TYPE
{
	_SERVICE_BTKAD=0,
	_SERVICE_EDKAD=1,
	_SERVICE_UPNP=2,
//	_SERVICE_MULTIKAD=3,
//	_SERVICE_DATABASE=3,
	_SERVICE_TSHARE=3, //torrent share service
};

//just for show the status light
//when ready ,light is green
//when gotdata, light is flick between yellow and green

enum _SERVICE_EVENT
{
	_SERVICE_READY=0,
	_SERVICE_GOTDATA=1,
	_SERVICE_GOTTOR=2, //获得了一个种子
	_SERVICE_GOTANN=3, //获得了一个发布
	_SERVICE_UPDATE=4, //发现了新版本，提示升级
};

#endif
