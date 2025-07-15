/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// CheckWnd.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "CheckWnd.h"
#include "memdc.h"

// CCheckWnd

CCheckWnd::CCheckWnd()
{
	m_nMaxStringWidth=16;
	m_nMaxStringHeight=16;
	m_bVertScroll=false;
	m_bHoriScroll=false;
	m_hWnd = NULL;
}

CCheckWnd::~CCheckWnd()
{
}


BEGIN_MESSAGE_MAP(CCheckWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
//	ON_WM_MOUSEHWHEEL()
// CCheckWnd message handlers
void CCheckWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	HDC hMemDC=::CreateCompatibleDC(dc.m_hDC);

	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	CRect rect;
	this->GetClientRect(&rect);

	//������屳��
	CRect rf;
	rf.left=rf.top=0;
	rf.right=m_nMaxStringWidth;
	rf.bottom=m_nMaxStringHeight*m_StringList.size();
	if(rf.right < rect.right) rf.right=rect.right;
	if(rf.bottom < rect.bottom) rf.bottom=rect.bottom;

	//�ڼ����ַ�������ʱ�����ܴ�����һ���������Խ�����ͼ�Ŀ�������50������
	rf.right+=50;
	rf.bottom+=50;

	HBITMAP hBitmap;
	hBitmap=::CreateCompatibleBitmap(hMemDC, rf.Width(), rf.Height());
	::SelectObject(hMemDC, hBitmap);
	::FillRect(hMemDC, rf, (HBRUSH) (COLOR_WINDOW+1));


	//ȫ������memdc�� ��Ȼ�󵹵�dc�ϵĺ��ʵ�λ�ú�����
	for(int i=0;i<m_StringList.size();i++)
	{
		//RECT er;
		//er.left=0+2;
		//er.top=i*m_nMaxStringHeight+2;
		//er.right=16-2;
		//er.bottom=(i+1)*m_nMaxStringHeight-2;
		//::DrawEdge(hMemDC, &er, EDGE_BUMP, BF_RECT); 

		::Rectangle(hMemDC, 0+2, i*m_nMaxStringHeight+2, 16-2, (i+1)*m_nMaxStringHeight-2);

		POINT old;
		if(m_SelectList[i])
		{
			::MoveToEx(hMemDC, 4, i*m_nMaxStringHeight+9, &old);
			::LineTo(hMemDC, 8, (i+1)*m_nMaxStringHeight-5);
			::LineTo(hMemDC, 12, (i)*m_nMaxStringHeight+5);
		}

		::DrawText(hMemDC, m_StringList[i], m_StringList[i].GetLength(),
			CRect(16, i*m_nMaxStringHeight, m_nMaxStringWidth, 
			(i+1)*m_nMaxStringHeight),DT_LEFT|DT_VCENTER);

		::MoveToEx(hMemDC, 0, (i+1)*m_nMaxStringHeight, &old);
		::LineTo(hMemDC, rf.right, (i+1)*m_nMaxStringHeight);

	}

	if(!m_bVertScroll && !m_bHoriScroll)
	{
		::BitBlt(dc.m_hDC, 0,0, rect.Width(), rect.Height(), hMemDC, 0, 0, SRCCOPY);
	}
	else if(m_bVertScroll && !m_bHoriScroll)
	{
		int pos=GetScrollPos(SB_VERT);
		::BitBlt(dc.m_hDC, 0,0, rect.Width(), rect.Height(), hMemDC, 0, pos*m_nMaxStringHeight, SRCCOPY);
	}
	else if(!m_bVertScroll && m_bHoriScroll)
	{
		int pos=GetScrollPos(SB_HORZ);
		::BitBlt(dc.m_hDC, 0,0, rect.Width(), rect.Height(), hMemDC, pos, 0, SRCCOPY);
	}
	else
	{
		int pos1=GetScrollPos(SB_VERT);
		int pos2=GetScrollPos(SB_HORZ);

		::BitBlt(dc.m_hDC, 0,0, rect.Width(), rect.Height(), hMemDC, pos2, pos1*m_nMaxStringHeight, SRCCOPY);
	}

	::DeleteObject(hBitmap);
	::DeleteObject(hMemDC);
}

BOOL CCheckWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;

}

bool CCheckWnd::IsSelected(int itemid)
{
	return m_SelectList[itemid];
}

bool CCheckWnd::IsAnySelected()
{
	for(int i=0;i<m_SelectList.size(); i++)
	{
		if(m_SelectList[i]) return true;
	}

	return false;
}

void CCheckWnd::AddItem(const std::wstring& item, BOOL sel)
{
	CDC* pDC= this->GetDC();
	CSize size=pDC->GetTextExtent(item);

	if(size.cx +16> m_nMaxStringWidth) 
	{
		m_nMaxStringWidth = size.cx +16;
	}

	if(size.cy > m_nMaxStringHeight)
	{
		m_nMaxStringHeight= size.cy >=18? size.cy : 18;
	}

	m_StringList.push_back(item);
	m_SelectList.push_back(sel);

	return ;
}
void CCheckWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	ReCalScroll(cx, cy);
}

void CCheckWnd::ReCalScroll(int cx, int cy)
{
	//�����Ƿ���Ҫ������
	//CRect rect;
	//this->GetWindowRect(rect);
	//int cx=rect.right;
	//int cy=rect.bottom;

	int nsw=::GetSystemMetrics(SM_CXVSCROLL); //��ֱ����������
	int nvh=::GetSystemMetrics(SM_CYHSCROLL); //ˮƽ�������߶�

	// �ȼ���ɷ���������û�й�����
	if(m_SelectList.size()*m_nMaxStringHeight < cy &&
		m_nMaxStringWidth < cx)
	{
		//������Ҫ
		ShowScrollBar(SB_BOTH, false);
		m_bVertScroll=false;
		m_bHoriScroll=false;

		//CString s;
		//s.Format(L"no scroll, size=%d, height=%d, cy=%d", m_SelectList.size(), m_nMaxStringHeight, cy);
		//MessageBox(s);
	}

	//����ֻҪˮƽ�������Ƿ���
	else if(m_SelectList.size()*m_nMaxStringHeight < cy - nvh )
	{//ֻҪˮƽ����
		ShowScrollBar(SB_HORZ, true);
		ShowScrollBar(SB_VERT, false);
		SetScrollRange(SB_HORZ, 0, m_nMaxStringWidth - cx, false);
		SetScrollPos(SB_HORZ, 0, false);
		m_bVertScroll=false;
		m_bHoriScroll=true;

		//MessageBox(L"need hscroll");
	}

	//����ֻҪ��ֱ�������Ƿ���
	else if(m_nMaxStringWidth < cx-nsw )
	{//ֻҪ��ֱ�����͹�
		ShowScrollBar(SB_VERT, true);
		ShowScrollBar(SB_HORZ, false);
		SetScrollRange(SB_VERT, 0, m_SelectList.size() - cy/m_nMaxStringHeight, false);
		SetScrollPos(SB_VERT, 0, false);
		m_bVertScroll=true;
		m_bHoriScroll=false;

		//if(m_SelectList.size()*m_nMaxStringHeight < cy)
		//{
		//	MessageBox(L" no vscroll need");
		//}
		//else
		//{
		//	CString s;
		//	s.Format(L"size=%d, height=%d, rect height=%d, need vscroll",
		//		m_SelectList.size(), m_nMaxStringHeight,cy);
		//	MessageBox(s);
		//}

		//if(m_nMaxStringWidth < cx-nsw)
		//{
		//	MessageBox(L" no hscroll need");
		//}

	}
	//ʣ��ľ�������ȫҪ��
	else
	{
		ShowScrollBar(SB_BOTH, true);
		SetScrollRange(SB_HORZ, 0, m_nMaxStringWidth - (cx-nsw), false);
		SetScrollRange(SB_VERT, 0, m_SelectList.size() - (cy-nvh)/m_nMaxStringHeight, false);
		SetScrollPos(SB_HORZ, 0, false);
		SetScrollPos(SB_VERT, 0, false);
		m_bVertScroll=true;
		m_bHoriScroll=true;

	}
}
void CCheckWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	switch(nSBCode)
	{
	case SB_LEFT:
	case SB_ENDSCROLL:
		break;
	case SB_LINELEFT:
		{
			SetScrollPos(SB_HORZ, --nPos);
			Invalidate();
		}
		break;
	case SB_LINERIGHT:
		{
			SetScrollPos(SB_HORZ, ++nPos);
			Invalidate();
		}
		break;
	case SB_PAGELEFT:
		{
			int pos=GetScrollPos(SB_HORZ);
			pos-=100;
			if(pos<0) pos=0;
			SetScrollPos(SB_HORZ, pos);
			Invalidate();
		}
		break;
	case SB_PAGERIGHT:
		{
			int pos=GetScrollPos(SB_HORZ);
			pos+=100;
			int MinPos, MaxPos;
			GetScrollRange(SB_HORZ, &MinPos, &MaxPos);
			if(pos>MaxPos) pos=MaxPos;
			SetScrollPos(SB_HORZ, pos);
			Invalidate();
			
		}
		break;
	case SB_RIGHT :
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos(SB_HORZ, nPos);
			Invalidate();
			
		}
		break;

	}//Invalidate();

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCheckWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	switch(nSBCode)
	{
	case SB_BOTTOM:
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
		{
			SetScrollPos(SB_VERT, ++nPos);
			Invalidate();
		}
		break;
	case SB_LINEUP:
		{
			SetScrollPos(SB_VERT, --nPos);
			Invalidate();
		}
		break;
	case SB_PAGEUP:
		{
			int pos=GetScrollPos(SB_VERT);
			CRect rect;
			this->GetClientRect(rect);
			pos -= rect.Height()/m_nMaxStringHeight;

			if(pos<0) pos=0;
			SetScrollPos(SB_VERT, pos);
			Invalidate();
		}
		break;
	case SB_PAGEDOWN:
		{
			int pos=GetScrollPos(SB_VERT);
			
			CRect rect;
			this->GetClientRect(rect);
			pos += rect.Height()/m_nMaxStringHeight;

			int MinPos, MaxPos;
			GetScrollRange(SB_VERT, &MinPos, &MaxPos);
			if(pos>MaxPos) pos=MaxPos;
			SetScrollPos(SB_VERT, pos);
			Invalidate();
			
		}
		break;
	case SB_TOP:
		{
			SetScrollPos(SB_VERT, 0);
			Invalidate();
		}
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos(SB_VERT, nPos);
			Invalidate();
			
		}
		break;

	}
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CCheckWnd::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnMButtonDown(nFlags, point);
}

void CCheckWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	int line=point.y / m_nMaxStringHeight;

	if(m_bVertScroll)
	{
		line+=GetScrollPos(SB_VERT);
		
	}

	if(m_SelectList.size() > line)
	{
		m_SelectList[line]=!m_SelectList[line];
	}

	Invalidate();
	CWnd::OnLButtonDown(nFlags, point);
}

BOOL CCheckWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bVertScroll)
	{
		int pos=GetScrollPos(SB_VERT);
		int MinPos, MaxPos;
		GetScrollRange(SB_VERT, &MinPos, &MaxPos);

		if(zDelta>0)
		{//up
			if(pos < MaxPos)
			{
				int line=zDelta/WHEEL_DELTA;
				pos+=line;
				if(pos>MaxPos) pos=MaxPos;
				SetScrollPos(SB_VERT, pos);
				Invalidate();
			}
		}
		else
		{//down
			int line=zDelta/WHEEL_DELTA;
			if(pos>0)
			{
				pos -= line;
				if(pos<0) pos=0;
				SetScrollPos(SB_VERT, pos);
				Invalidate();
			}
		}
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

//BOOL CCheckWnd::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	// This feature requires Windows Vista or greater.
//	// The symbol _WIN32_WINNT must be >= 0x0600.
//	// TODO: Add your message handler code here and/or call default
//
//	return TRUE;
//	//return CWnd::OnMouseHWheel(nFlags, zDelta, pt);
//}
