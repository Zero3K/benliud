/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

// TorrentFile.h: interface for the CTorrentFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TORRENT_FILE_H
#define _TORRENT_FILE_H

#include "BenNode.h"
#include "TFileInfo.h"
#include "SHA1.h"
#include <deque>

namespace BencodeLib
{
	class CTorrentFile  
	{
		class _nl
		{
		public:
			std::string ip;
			int port;
		};	
	public:
		bool IsSingleFile();
		CBenNode& GetRootNode();
		TFileInfo GetFileInfo(int seq,bool withvirtual=false);
		llong GetTotalSize(bool withvirtual=false);
		std::string GetPieceHash(unsigned int i);
		unsigned int GetPieceCount();
		bool IsUtf8Valid();
		std::string GetInfoHash() const;
		bool MakeInfoHash();
		//char* EncodeToStream(int& buflen,CBenNode* pnode=NULL);
		bool GetNode(int seq,std::string& ip, int& port);
		std::string GetPublisher();
		unsigned int GetPieceLength();
		std::string GetName();
		llong GetFileLength(int seq, bool withvirtual=false);
		std::string GetFileName(int seq, bool withvirtual=false);
		int GetFileNumber(bool withvirtual=false);
		int GetNodeNumber();
		int GetAnnounceNumber();
		std::string GetAnnounce(int seq);
		std::string GetComment();
		std::string GetCreateBy();
		long GetCreationDate();

		int ExtractKeys();
		int ReadBuf(char *filebuf,int len);
		int ReadFile(const wchar_t* pathfile);
		CTorrentFile();
		virtual ~CTorrentFile();


		//
	protected:
		//void Encode(CBenNode* pnode,char* buf,int& pos);
		void ExtractOther();
		bool ExtractMultiFile();
		bool ExtractSingleFile();
		bool ExtractFileInfo();
		bool ExtractPieceLength();
		bool ExtractPieces();
		bool ExtractNodes();
		bool ExtractName();
		bool ExtractAnnounceList();
		bool ExtractAnnounce();
		//void ConvertAscii(const unsigned char* buf,int len,char* out);
		void Clean();

	protected:
		bool m_bUtf8Name;
		bool m_bUtf8Path;
		bool m_bSingleFile;
		long					m_CreateDate;
		unsigned int			m_PieceLength;

		//char*					m_Stream;//encode stream
		llong					m_nTotalSize; //total size including virtual file
		llong					m_nTotalSizeWithoutVirtual;	//no virtual file totalsize
		std::string				m_InfoHash;// the info dict hash
		std::string				m_Announce; //also put in m_AnnounceList
		std::string				m_Name;  //if it's a group of file ,it is dir name.
		std::string				m_Publisher;
		std::string				m_CreateBy;
		std::string				m_Comment;	

		std::vector<std::string> m_AnnounceList;

		//�����������ϵĴ洢��ʽ�����ܴ��������ļ���׼���滻
		//	std::vector<std::string>	m_FileList;  //utf8 name if have
		//	std::vector<llong>			m_FileLength;//the length of file

		//������µĴ洢��ʽ�������ʽ�ſ��Դ��������ļ�
		std::vector<TFileInfo>		m_FileInfoList; //

		std::vector<_nl>		 m_NodeList;  //initial node
		std::vector<std::string> m_FileHash;

	private:
		CBenNode				 m_Root;

	};
}
#endif // !defined(AFX_TORRENTFILE_H__8A17C2EA_B297_4EAF_ABA8_FDF3324E5826__INCLUDED_)
