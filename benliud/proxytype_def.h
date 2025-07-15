/***************************************************************************
 *            proxytype_def.h
 *
 *  Sun Dec 24 15:16:53 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _PROXYTYPE_H
#define _PROXYTYPE_H

enum _PROXY_TYPE
{
	_PROXY_HTTP=0,
	_PROXY_FTP=1,
	_PROXY_MMS=2,
	_PROXY_RTSP=3,
	_PROXY_SOCKSAUTO=4,
	_PROXY_SOCKSV4=5,
	_PROXY_SOCKSV4A=6,	
	_PROXY_SOCKSV5=7,

};

#endif
