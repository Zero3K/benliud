// CategoryTreeItemData.h: interface for the CCategoryTreeItemData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CATEGORYTREEITEMDATA_H
#define _CATEGORYTREEITEMDATA_H

#include <wx/wx.h>
#include <wx/treectrl.h>


class CCategoryTreeItemData : public wxTreeItemData 
{
public:
	CCategoryTreeItemData(int nodeid,int parent);
	virtual ~CCategoryTreeItemData();
	int GetParent();
	int GetNodeId();
protected:
	int m_NodeId;
	int m_ParentId;
};


#endif 