/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

// TorrentFile.cpp: implementation of the CTorrentFile class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/TorrentFile.h"
//#include "../../sha1lib/include/sha1.h"
#include "../include/TFileInfo.h"
#include "../include/Tools.h"
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace BencodeLib
{
	CTorrentFile::CTorrentFile()
	{
		//m_Stream=NULL;
		m_bSingleFile=true;
		m_nTotalSize=0;
		m_nTotalSizeWithoutVirtual=0;
	}

	CTorrentFile::~CTorrentFile()
	{
		//if(m_Stream) delete[] m_Stream;
	}


	//read torrent file and parse it,if fail return <0;
	int CTorrentFile::ReadFile(const wchar_t *pathfile)
	{

#if defined (WIN32)||defined(WINCE)
		HANDLE hFile=::CreateFile(pathfile, GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if(hFile==INVALID_HANDLE_VALUE) return -1;

		DWORD nFileSizeLow, nFileSizeHigh;
		nFileSizeLow=::GetFileSize(hFile, &nFileSizeHigh);
		if(nFileSizeLow==0xFFFFFFFF) return -2;
		if(nFileSizeHigh!=0) return -3; //too big file.
		if(nFileSizeLow>=10*1024*1024) return -4; //too big file.

		DWORD nPos=::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if(nPos==0xFFFFFFFF) return -5;

		PBYTE pBuf=new BYTE[nFileSizeLow];

		BOOL bRet=::ReadFile(hFile, pBuf, nFileSizeLow, &nFileSizeHigh, NULL);
		if(!bRet) return -6;

		::CloseHandle(hFile);

		int nret=ReadBuf((char*)pBuf,nFileSizeLow);

		delete[] pBuf;

		return nret;

#else //linux
		FILE *fp=NULL;
		char utf8path[512];
		Tools::UCS2UTF(pathfile,utf8path,512);
		fp=fopen(utf8path,"rb");
		if(fp==NULL) return -1;


		fseek(fp,0,SEEK_END);
		long flen=ftell(fp);

		if(flen>2*1024*1024) {
			fclose(fp);
			return -2;
		}

		char *fbuf=new char[flen];
		fseek(fp,0,SEEK_SET);


		if(flen!=long(fread(fbuf,1,flen,fp)))
		{
			fclose(fp);
			delete[] fbuf;
			return -3;
		}

		fclose(fp);

		//ok all file data is in memory
		//read it

		int nret=ReadBuf(fbuf,flen);

		delete[] fbuf;

		return nret;
#endif



	}



	int CTorrentFile::ReadBuf(char *fbuf, int flen)
	{
		Clean();

		std::deque<int> m_OpQueue;

		//ok all file data is in memory
		//read it

		//pieces data is binary data,we convert to ascii string if find the key word of "pieces"
		//here is the mark
		//bool piecesMark=false; 


		char buf[512];
		char* pmove=fbuf;

		while(pmove-fbuf<flen)
		{
			switch(*pmove)
			{
			case 'd':
				m_Root.OpenDictionary();
				m_OpQueue.push_back(0); //dict=0
				pmove++;
				break;
			case 'l':
				m_Root.OpenList();
				m_OpQueue.push_back(1); //list=1
				pmove++;
				break;
			case 'i':
				{//find the 'e' for number end
					pmove++;
					char *begin=pmove;
					while(*pmove!='e' && pmove-fbuf<flen) pmove++;
					if(*pmove=='e') {
						memcpy(buf,begin,pmove-begin);
						buf[pmove-begin]='\0';
#ifdef WIN32
						llong iv=_atoi64(buf);
#else
						llong iv=atoll(buf);
#endif
						m_Root.AddValue(iv);
						pmove++;
					}
					else
					{
						return -4;
					}
				}
				break;
			case 'e':
				if(m_OpQueue.empty()) {
					return -5;
				}

				pmove++;

				if(m_OpQueue.back()==1)
				{
					m_OpQueue.pop_back();
					m_Root.CloseList();
				}
				else 
				{
					m_OpQueue.pop_back();
					m_Root.CloseDictionary();

					//because this CloseDictionary() may close the root dictionary,
					//and some torrent still have data after the main dictionary closed,
					//so , if the root dict is closed we end reading the data
					if(m_OpQueue.empty()) goto errquit;
				}

				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					//find the number
					char* begin=pmove;
					while(*pmove>='0' && *pmove<='9') pmove++;
					if(*pmove!=':') {
						return -6;
					}
					memcpy(buf,begin,pmove-begin);
					buf[pmove-begin]='\0';
					int slen=atoi(buf);

					pmove++;//skip ":"

					m_Root.AddValue(pmove,slen);
					pmove+=slen;
				}
				break;
			default:
				//some torrent may extern the key that we don't recgnize
				//so if we don't know the key, we just skip and quit.
				//or it will put a text end char '0x0A' at the tail of file
				//printf("key=%02X don't know",*pmove);
				//printf("pmove-fbuf=%d, flen=%d\n", pmove-fbuf, flen);
				goto errquit;
			}
		}

errquit:
		//some torrent not close all opend list or dictionary at end.
		int left=m_OpQueue.size();
		if(left==0) return 0;
		else return -6;

	}

	//extract keys we need, 
	//{announce,announce-list}
	//{name,piece length,pieces,[length,files],path,path.utf-8,name.utf-8
	//多文件中name是目录名
	//扩展nodes,publisher,publisher-url,publisher-url.utf-8
	//node list{12:69.160.77.87,55559} 前为IP后端口
	/*
	info：分两种模式，单文件和多文件。字典型。

	单文件的情况下需要的字段：

	length：文件长度，单位字节。整型
	md5sum：文件内容的md5校验（可选）不过一般都有，为了安全嘛，也可防一下攻击和病毒。
	name：名称，实际上可以是任意的，但是建议用文件名。字符串
	piece length：每个piece的字节数，也就是按多少字节来分片的。整型
	pieces：把每一片的所有20个字节的sha1哈希值连在一起。字符串

	多文件下需要的字段：
	files：字典列表。每个字典的字段。
	length：文件长度，单位字节。整型
	md5sum：文件内容的md5校验（可选）不过一般都有。
	path：文件路径，例："dir1/dir2/file.ext"写成："l4:dir14:dir28:file.exte"
	name：名称，实际上可以是任意的，但是建议用目录名。字符串
	piece length：每个piece的字节数，也就是按多少字节来分片的。整型
	pieces：把每一片的所有20个字节的sha1哈希值连在一起。字符串
	announce:通知服务器的URL，即Tracker的地址，字符串。
	announce-list:通知服务器URL的列表。（可选）一般都会有。列表型。
	creation date:创建时间。标准Unix格式，秒数从 1970年1月1日 00:00:00 
	格林威治标准时间开始算，整型。
	comment:注释。（可选）。
	created by: 创建者，创建程序的名称和版本。
	*/
	int CTorrentFile::ExtractKeys()
	{
		if(!ExtractAnnounce()) return -1;
		if(!ExtractAnnounceList()) return -2;
		if(!ExtractName()) return -3;
		if(!ExtractFileInfo()) return -4;
		if(!ExtractPieceLength()) return -5;
		if(!ExtractPieces()) return -6;
		if(!MakeInfoHash()) return -7;
		if(!ExtractNodes()) return -8;
		ExtractOther();
		return 0;
	}



	bool CTorrentFile::ExtractAnnounce()
	{
		//announce must be a string.
		CBenNode* ann=m_Root.GetKeyValue("announce");

		if(ann!=NULL)
		{
			if(ann->GetType()==beString)
			{
				ann->GetStringValue(m_Announce);
				m_AnnounceList.push_back(m_Announce);
				return true;
			}
			else
			{
				return false;
			}

		}

		return true; //the announce now can be empty
	}

	bool CTorrentFile::ExtractAnnounceList()
	{//maybe some torrent not have annlist
		CBenNode* annl=m_Root.GetKeyValue("announce-list");
		if(annl!=NULL)
		{
			if(annl->GetType()==beList)
			{
				int count=annl->GetNumberOfList();
				for(int i=0;i<count;i++)
				{
					CBenNode* lis=annl->GetListMember(i);
					if(lis->GetType()==beString)
					{

						std::string tt;
						lis->GetStringValue(tt);
						//duplicate check
						bool have=false;
						std::vector<std::string>::const_iterator it;
						for(it=m_AnnounceList.begin();it!=m_AnnounceList.end();it++)
						{
							if(*it==tt)
							{
								have=true;
								break;
							}
						}
						if(!have)
							m_AnnounceList.push_back(tt);
					}
					else if(lis->GetType()==beList)
					{
						int count2=lis->GetNumberOfList();
						for(int j=0;j<count2;j++)
						{
							CBenNode* l2=lis->GetListMember(j);
							if(l2->GetType()==beString)
							{
								std::string tt;
								l2->GetStringValue(tt);
								//duplicate check
								bool have=false;
								std::vector<std::string>::const_iterator it;
								for(it=m_AnnounceList.begin();it!=m_AnnounceList.end();it++)
								{
									if(*it==tt)
									{
										have=true;
										break;
									}
								}
								if(!have)
									m_AnnounceList.push_back(tt);
							}
							else
							{
								return false;
							}
						}
					}
					else 
					{
						return false;
					}
				}
			}
			else
			{
				return false;//wrong type
			}

		}

		return true; //the announce-list can be empty
	}

	bool CTorrentFile::ExtractName()
	{
		//try to get name in info node
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;
		//try name.utf-8 first
		CBenNode* name=info->GetKeyValue("name.utf-8");
		if(name!=NULL)
		{
			if(name->GetType()==beString)
			{
				name->GetStringValue(m_Name);
				m_bUtf8Name=true;
				return true;
			}
			else
			{
				return false;
			}
		}

		name=info->GetKeyValue("name");
		if(name!=NULL)
		{
			if(name->GetType()==beString)
			{

				name->GetStringValue(m_Name);
				m_bUtf8Name=false;
				return true;
			}
			else
			{
				return false;
			}
		}	

		return false;
	}

	bool CTorrentFile::ExtractNodes()
	{


		CBenNode* node=m_Root.GetKeyValue("nodes");//m_Root.GetKeyValue("info");
		if(node==NULL) return true; //some torrent don't have nodes,it's ok.


		if(node->GetType()==beList)
		{
			int count=node->GetNumberOfList();
			for(int i=0;i<count;i++)
			{
				CBenNode* lm=node->GetListMember(i);
				if(lm->GetType()==beList) //one list for ip and port
				{
					int n=lm->GetNumberOfList();
					//n==2??
					if(n!=2) return false;
					CBenNode* ip=lm->GetListMember(0);  //string
					CBenNode* po=lm->GetListMember(1);  //int

					if(ip->GetType()!=beString||po->GetType()!=beInt) return false;

					_nl newnode;

					ip->GetStringValue(newnode.ip);

					newnode.port=(int)po->GetIntValue();
					m_NodeList.push_back(newnode);
				}
			}
		}

		return true;
	}

	//pieces maps to a string whose length is a multiple of 20
	//because we convert binary data to ascii data, so is multiple of 40
	bool CTorrentFile::ExtractPieces()
	{
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;
		CBenNode* piece=info->GetKeyValue("pieces");
		if(piece==NULL) return false;
		if(piece->GetType()!=beString) return false;

		std::string tt;
		piece->GetStringValue(tt);
		if(tt.length()%20!=0) return false;


		char* temp=new char[tt.length()];
		memcpy(temp,tt.data(),tt.length());

		int count=tt.length()/20;
		for(int i=0;i<count;i++)
		{
			std::string pie;
			pie.append((char*)(temp+i*20),20);
			pie.resize(20);

			m_FileHash.push_back(pie);
		}
		delete[] temp;

		return true;
	}
	//to get the piece length,piece length is almost always a power of two,
	bool CTorrentFile::ExtractPieceLength()
	{
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;
		CBenNode* plen=info->GetKeyValue("piece length");
		if(plen==NULL) return false;

		if(plen->GetType()==beInt)
		{
			m_PieceLength=(unsigned int)(plen->GetIntValue());
			return true;
		}
		return false;
	}

	//get single file or files info
	bool CTorrentFile::ExtractFileInfo()
	{
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;

		//qurey if many files

		CBenNode* files=info->GetKeyValue("files");
		CBenNode* length=info->GetKeyValue("length");

		if(files==NULL && length==NULL) return false;

		if(files!=NULL && length!=NULL) return false;

		if(files!=NULL) return ExtractMultiFile();
		else return ExtractSingleFile();

	}

	//in the case only one file in torrent ,call this
	bool CTorrentFile::ExtractSingleFile()
	{
		//for single file ,name got from ExtractName() is the file name
		//so just got the file length and put it to the file list
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;

		CBenNode* length=info->GetKeyValue("length");
		if(length==NULL) return false;

		m_bSingleFile=true;

		//单文件应该不存在虚拟文件的情况，就不检查了
		//这里是新的处理方式

		TFileInfo nfile;
		nfile.index=0;
		nfile.name=m_Name;
		nfile.offset=0;
		nfile.size=length->GetIntValue();
		nfile.vfile=false;//不是虚拟文件
		m_FileInfoList.push_back(nfile);
		m_nTotalSize=nfile.size; //record totalsize
		m_nTotalSizeWithoutVirtual=m_nTotalSize;
		return true;
	}

	//in the case many file in torrent ,call this
	bool CTorrentFile::ExtractMultiFile()
	{
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;

		CBenNode* files=info->GetKeyValue("files");
		if(files==NULL) return false;

		if(files->GetType()!=beList) return false;

		int count=files->GetNumberOfList();

		//新的文件处理方式，可以处理虚拟文件

		m_nTotalSize=0;
		m_nTotalSizeWithoutVirtual=0;

		for(int i=0;i<count;i++)
		{

			CBenNode* file=files->GetListMember(i);

			if(file->GetType()!=beDict) return false;

			//how many dict items?

			//try to find path.utf-8,this is the real pathfile ,the name is root dir
			CBenNode* path=file->GetKeyValue("path.utf-8");
			if(path==NULL) {
				m_bUtf8Path=false;
				path=file->GetKeyValue("path");
			}else{
				m_bUtf8Path=true;
			}

			if(path==NULL) return false;

			std::string fpath;
			//path should be a list.
			if(path->GetType()==beList)
			{
				int layer=path->GetNumberOfList();
				for(int j=0;j<layer;j++)
				{

					CBenNode* dirs=path->GetListMember(j);
					if(dirs->GetType()!=beString) return false;
					std::string tmp;
					dirs->GetStringValue(tmp);
#ifdef WIN32
					if(!fpath.empty())	fpath+=std::string("\\");
					fpath+=tmp; 
#else
					if(!fpath.empty())	fpath+=std::string("/");
					fpath+=tmp;
#endif
				}
			}

			//ok here get the file path
			TFileInfo nfile;

			CBenNode* flen=file->GetKeyValue("length");
			if(flen==NULL) return false;
			if(flen->GetType()!=beInt) return false;

			if( fpath.length() >= strlen("_____padding_file") &&
				fpath.substr(0,strlen("_____padding_file"))== "_____padding_file" )
			{
				//bitcomet virtual file name
				nfile.vfile=true;
			}
			else if(fpath.length() >= strlen("_benliud_empty_file") &&
				fpath.substr(0,strlen("_benliud_empty_file")) == "_benliud_empty_file" )
			{
				//benliud virtual file name
				nfile.vfile=true;
			}
			else
			{
				nfile.vfile=false;
			}

			nfile.index=i;
			nfile.name=fpath;
			nfile.offset=m_nTotalSize;
			nfile.size=flen->GetIntValue();
			m_nTotalSize+=nfile.size;

			if(!nfile.vfile)
			{
				m_nTotalSizeWithoutVirtual+=nfile.size;
			}

			m_FileInfoList.push_back(nfile);

		}


		m_bSingleFile=false;

		return true;
	}

	//no important keys
	void CTorrentFile::ExtractOther()
	{
		//create by, createdata,comment,publisher
		CBenNode* cb=m_Root.GetKeyValue("created by.utf-8");
		if(cb==NULL) cb=m_Root.GetKeyValue("created by");
		if(cb!=NULL && cb->GetType()==beString)
		{
			cb->GetStringValue(m_CreateBy);
			if(m_CreateBy.size() > 512) m_CreateBy=m_CreateBy.substr(0,512);
		}
		else
		{
			m_CreateBy.resize(0);
		}

		CBenNode* pb=m_Root.GetKeyValue("publisher.utf-8");
		if(pb==NULL) pb=m_Root.GetKeyValue("publisher");

		if(pb!=NULL && pb->GetType()==beString)
		{
			pb->GetStringValue(m_Publisher);
			if(m_Publisher.size() > 512) m_Publisher=m_Publisher.substr(0,512);
		}
		else
		{
			m_Publisher.resize(0);
		}

		CBenNode* cd=m_Root.GetKeyValue("creation date");
		if(cd!=NULL && cd->GetType()==beInt)
		{
			m_CreateDate=(long)cd->GetIntValue();
		}
		else
		{
			m_CreateDate=0;
		}

		CBenNode* cm=m_Root.GetKeyValue("comment.utf-8");
		if(cm==NULL) cm=m_Root.GetKeyValue("comment");
		if(cm!=NULL && cm->GetType()==beString)
		{
			cm->GetStringValue(m_Comment);
			if(m_Comment.size() > 512) m_Comment=m_Comment.substr(0,512);
		}
		else
		{
			m_Comment.resize(0);
		}
	}

	void CTorrentFile::Clean()
	{
		m_Root.Clean();
		m_CreateDate=0;

		m_AnnounceList.clear();

		m_NodeList.clear();

		//why vc6's string haven't clear()???

		m_Announce.resize(0);

		m_Name.resize(0);

		m_CreateBy.resize(0);

		m_Comment.resize(0);

		m_Publisher.resize(0);

		m_FileHash.clear();

		m_CreateDate=0;
		m_PieceLength=0;
		m_PieceLength=0;

		m_bUtf8Name=false;
		m_bUtf8Path=false;

		m_nTotalSize=0;
		m_nTotalSizeWithoutVirtual=0;
		m_FileInfoList.clear();
	}

	long CTorrentFile::GetCreationDate()
	{
		return m_CreateDate;
	}

	std::string CTorrentFile::GetCreateBy()
	{
		return m_CreateBy;
	}

	std::string CTorrentFile::GetComment()
	{
		return m_Comment;
	}

	std::string CTorrentFile::GetAnnounce(int seq)
	{
		return m_AnnounceList[seq];
	}

	int CTorrentFile::GetAnnounceNumber()
	{
		return m_AnnounceList.size();
	}

	int CTorrentFile::GetNodeNumber()
	{
		return m_NodeList.size();
	}

	int CTorrentFile::GetFileNumber(bool withvirtual)
	{
		if(withvirtual)
		{
			return m_FileInfoList.size();
		}
		else
		{
			int count=0;

			std::vector<TFileInfo>::const_iterator it;
			for(it=m_FileInfoList.begin();it!=m_FileInfoList.end();it++)
			{
				if(it->vfile) continue;
				else count++;
			}

			return count;
		}

	}

	std::string CTorrentFile::GetFileName(int seq, bool withvirtual)
	{
		if(withvirtual)
		{
			//assert(seq>=0 && seq<m_FileInfoList.size());

			return m_FileInfoList[seq].name;
		}
		else
		{
			int count=0;

			std::vector<TFileInfo>::const_iterator it;
			for(it=m_FileInfoList.begin();it!=m_FileInfoList.end();it++)
			{
				if(it->vfile) 
				{
					continue;
				}
				else 
				{
					if( seq==count )
					{
						return it->name;
					}
					else 
					{
						count++;
					}
				}
			}

			assert(false);
			return "error_to_get_filename";
		}

	}

	llong CTorrentFile::GetFileLength(int seq, bool withvirtual)
	{
		if(withvirtual)
		{
			//assert(seq>=0 && seq<m_FileInfoList.size());
			return m_FileInfoList[seq].size;
		}
		else
		{
			int count=0;

			std::vector<TFileInfo>::const_iterator it;
			for(it=m_FileInfoList.begin();it!=m_FileInfoList.end();it++)
			{
				if(it->vfile) continue;
				else 
				{
					if(count==seq)
						return it->size;
					else
						count++;
				}
			}

			assert(false);
			return 0; //error!!!
		}

	}

	//if a single file ,this is the file name, if multi file this is dir name
	//it's just a suggest name.
	std::string CTorrentFile::GetName()
	{
		return m_Name;
	}

	unsigned int CTorrentFile::GetPieceLength()
	{
		return m_PieceLength;
	}

	std::string CTorrentFile::GetPublisher()
	{
		return m_Publisher;
	}

	//the node maybe for DHT bootup
	bool CTorrentFile::GetNode(int seq,std::string &ip, int &port)
	{
		ip=m_NodeList[seq].ip;
		port=m_NodeList[seq].port;
		return true;
	}
	//
	////to encode a bennode to stream, if pnode==NULL ,encode root
	//char* CTorrentFile::EncodeToStream(int& buflen,CBenNode *pnode)
	//{
	//	if(m_Stream) delete[] m_Stream;
	//	m_Stream=new char[1024*1024]; //make a 1M buffer to accept data
	//
	//	int pos=0;
	//	if(pnode==NULL)
	//	{//encode root
	//		Encode(&m_Root,m_Stream,pos);
	//	}
	//	else
	//	{
	//		Encode(pnode,m_Stream,pos);
	//	}
	//
	//	//the length is pos
	//	buflen=pos;
	//	return m_Stream;
	//}
	//
	////do encode 
	//void CTorrentFile::Encode(CBenNode *pnode, char *buf, int &pos)
	//{
	//	if(pnode->GetType()==beList)
	//	{
	//		buf[pos]='l';
	//		pos++;
	//
	//		int count=pnode->GetNumberOfList();
	//		for(int i=0;i<count;i++)
	//		{
	//			Encode(pnode->GetListMember(i),buf,pos);
	//		}
	//		buf[pos]='e';
	//		pos++;
	//	}
	//	else if(pnode->GetType()==beDict)
	//	{
	//		buf[pos]='d';
	//		pos++;
	//
	//		int count=pnode->GetNumberOfDict();
	//		for(int i=0;i<count;i++)
	//		{
	//			Encode(pnode->GetListMember(i),buf,pos);
	//		}
	//		buf[pos]='e';
	//		pos++;
	//	}
	//	else if(pnode->GetType()==beInt)
	//	{
	//		//i34e
	//		llong i=pnode->GetIntValue();
	//		char tbuf[28];
	//#ifdef WIN32
	//		sprintf_s(tbuf,28,"i%I64de",i); 
	//#else
	//		sprintf(tbuf,"i%llde",i);
	//#endif
	//		memcpy(buf+pos,tbuf,strlen(tbuf));
	//		pos+=strlen(tbuf);
	//
	//	}
	//	else if(pnode->GetType()==beString)
	//	{
	//		//5:abcde
	//		std::string tt;
	//		pnode->GetStringValue(tt);
	//		int len=tt.length();
	//
	//		char tbuf[25];
	//#ifdef WIN32
	//		sprintf_s(tbuf,25,"%d:",len);
	//#else
	//		sprintf(tbuf,"%d:",len);
	//#endif
	//		memcpy(buf+pos,tbuf,strlen(tbuf));
	//		pos+=strlen(tbuf);
	//		
	//		memcpy(buf+pos,tt.data(),len);
	//
	//		pos+=len;
	//	}
	//	else if(pnode->GetType()==beNull)
	//	{
	//		return;
	//	}
	//	else if(pnode->GetType()==beDictPair)
	//	{
	//		//output key
	//		std::string key=pnode->GetKey();
	//		char tbuf[25];
	//#ifdef WIN32
	//		sprintf_s(tbuf,25,"%d:",key.length());
	//#else
	//		sprintf(tbuf,"%d:",key.length());
	//#endif
	//		memcpy(buf+pos,tbuf,strlen(tbuf));
	//		pos+=strlen(tbuf);
	//		memcpy(buf+pos,key.c_str(),key.length());
	//		pos+=key.length();
	//		//output keyvalue
	//		Encode(pnode->GetKeyValue(),buf,pos);
	//	}
	//	else
	//	{
	//		return ;
	//	}
	//}

	//when extracekeys finish ,make info hash
	bool CTorrentFile::MakeInfoHash()
	{
		CBenNode* info=m_Root.GetKeyValue("info");
		if(info==NULL) return false;

		std::string infostream;
		info->ToStream(infostream);

		HashLib::CSHA1 tmp;
		tmp.Hash(infostream.data(), infostream.size());
		m_InfoHash.clear();
		m_InfoHash.append((const char*)tmp.GetHash(), tmp.GetHashLen());
		//m_InfoHash=Tools::SHA1String(infostream);

		return true;
	}

	std::string CTorrentFile::GetInfoHash() const
	{
		return m_InfoHash;
	}

	bool CTorrentFile::IsUtf8Valid()
	{
		return m_bSingleFile ? m_bUtf8Name : (m_bUtf8Name && m_bUtf8Path);
	}

	unsigned int CTorrentFile::GetPieceCount()
	{
		return m_FileHash.size();
	}

	std::string CTorrentFile::GetPieceHash(unsigned int i)
	{
		return m_FileHash[i];
	}

	llong CTorrentFile::GetTotalSize(bool withvirtual)
	{

		if(withvirtual)
			return m_nTotalSize;
		else
			return m_nTotalSizeWithoutVirtual;
	}

	TFileInfo CTorrentFile::GetFileInfo(int seq,bool withvirtual)
	{

		if(withvirtual)
		{
			//assert(seq >=0 && seq< m_FileInfoList.size());

			return m_FileInfoList[seq];
		}
		else
		{
			int count=0;

			TFileInfo fi;

			std::vector<TFileInfo>::const_iterator it;
			for(it=m_FileInfoList.begin();it!=m_FileInfoList.end();it++)
			{
				if(it->vfile) continue;
				else 
				{
					if(count==seq)
					{
						fi=(*it);
						return fi;
					}
					else
					{
						count++;
					}
				}
			}

			assert(false);
			return fi;//error!
		}
	}

	CBenNode& CTorrentFile::GetRootNode()
	{
		return m_Root;
	}


	//多文件的主目录需要外部补
	bool CTorrentFile::IsSingleFile()
	{
		return m_bSingleFile;
	}

}