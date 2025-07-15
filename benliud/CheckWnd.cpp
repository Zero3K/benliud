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
// #include "memdc.h"  // MFC-specific memory DC not used

// CCheckWnd

CCheckWnd::CCheckWnd()
{
	m_nMaxStringWidth=16;
	m_nMaxStringHeight=16;
	m_bVertScroll=false;
	m_bHoriScroll=false;
	m_hWnd = NULL;
	m_pfnOriginalWndProc = NULL;
}

CCheckWnd::~CCheckWnd()
{
}

BOOL CCheckWnd::SubclassWindow(HWND hWnd)
{
	if (hWnd == NULL || !IsWindow(hWnd))
		return FALSE;
		
	m_hWnd = hWnd;
	
	// Store the original window procedure
	m_pfnOriginalWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
	
	// Store this pointer so the static window procedure can find it
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	
	return TRUE;
}

// Static window procedure
LRESULT CALLBACK CCheckWnd::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCheckWnd* pThis = (CCheckWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	
	if (pThis)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Instance message handler
LRESULT CCheckWnd::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_PAINT:
			OnPaint();
			return 0;
			
		case WM_ERASEBKGND:
			return OnEraseBkgnd((HDC)wParam);
			
		case WM_SIZE:
			OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
			return 0;
			
		case WM_HSCROLL:
			OnHScroll(LOWORD(wParam), HIWORD(wParam));
			return 0;
			
		case WM_VSCROLL:
			OnVScroll(LOWORD(wParam), HIWORD(wParam));
			return 0;
			
		case WM_MBUTTONDOWN:
			{
				POINT pt = {LOWORD(lParam), HIWORD(lParam)};
				OnMButtonDown(wParam, pt);
			}
			return 0;
			
		case WM_LBUTTONDOWN:
			{
				POINT pt = {LOWORD(lParam), HIWORD(lParam)};
				OnLButtonDown(wParam, pt);
			}
			return 0;
			
		case WM_MOUSEWHEEL:
			{
				POINT pt = {LOWORD(lParam), HIWORD(lParam)};
				return OnMouseWheel(LOWORD(wParam), (short)HIWORD(wParam), pt);
			}
	}
	
	// Call the original window procedure
	if (m_pfnOriginalWndProc)
		return CallWindowProc(m_pfnOriginalWndProc, hWnd, uMsg, wParam, lParam);
		
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// CCheckWnd message handlers
void CCheckWnd::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(m_hWnd, &ps);

	HDC hMemDC=::CreateCompatibleDC(hDC);

	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	//界面区域背景
	RECT rf;
	rf.left=rf.top=0;
	rf.right=m_nMaxStringWidth;
	rf.bottom=m_nMaxStringHeight*m_StringList.size();
	if(rf.right < rect.right) rf.right=rect.right;
	if(rf.bottom < rect.bottom) rf.bottom=rect.bottom;

	//�ڼ����ַ�������ʱ�����ܴ�����һ���������Խ�����ͼ�Ŀ�������50������
	rf.right+=50;
	rf.bottom+=50;

	HBITMAP hBitmap;
	hBitmap=::CreateCompatibleBitmap(hMemDC, rf.right - rf.left, rf.bottom - rf.top);
	::SelectObject(hMemDC, hBitmap);
	::FillRect(hMemDC, &rf, (HBRUSH) (COLOR_WINDOW+1));


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

		RECT textRect = {16, i*m_nMaxStringHeight, m_nMaxStringWidth, (i+1)*m_nMaxStringHeight};
		::DrawText(hMemDC, m_StringList[i].c_str(), m_StringList[i].length(),
			&textRect, DT_LEFT|DT_VCENTER);

		::MoveToEx(hMemDC, 0, (i+1)*m_nMaxStringHeight, &old);
		::LineTo(hMemDC, rf.right, (i+1)*m_nMaxStringHeight);

	}

	if(!m_bVertScroll && !m_bHoriScroll)
	{
		::BitBlt(hDC, 0,0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, 0, SRCCOPY);
	}
	else if(m_bVertScroll && !m_bHoriScroll)
	{
		int pos=GetScrollPos(m_hWnd, SB_VERT);
		::BitBlt(hDC, 0,0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, pos*m_nMaxStringHeight, SRCCOPY);
	}
	else if(!m_bVertScroll && m_bHoriScroll)
	{
		int pos=GetScrollPos(m_hWnd, SB_HORZ);
		::BitBlt(hDC, 0,0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, pos, 0, SRCCOPY);
	}
	else
	{
		int pos1=GetScrollPos(m_hWnd, SB_VERT);
		int pos2=GetScrollPos(m_hWnd, SB_HORZ);

		::BitBlt(hDC, 0,0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, pos2, pos1*m_nMaxStringHeight, SRCCOPY);
	}

	::DeleteObject(hBitmap);
	::DeleteObject(hMemDC);
	
	EndPaint(m_hWnd, &ps);
}

BOOL CCheckWnd::OnEraseBkgnd(HDC hDC)
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
	HDC hDC = GetDC(m_hWnd);
	SIZE size;
	GetTextExtentPoint32(hDC, item.c_str(), item.length(), &size);

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

	ReleaseDC(m_hWnd, hDC);
	return ;
}
void CCheckWnd::OnSize(UINT nType, int cx, int cy)
{
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
		ShowScrollBar(m_hWnd, SB_BOTH, FALSE);
		m_bVertScroll=false;
		m_bHoriScroll=false;

		//CString s;
		//s.Format(L"no scroll, size=%d, height=%d, cy=%d", m_SelectList.size(), m_nMaxStringHeight, cy);
		//MessageBox(s);
	}

	//����ֻҪˮƽ�������Ƿ���
	else if(m_SelectList.size()*m_nMaxStringHeight < cy - nvh )
	{//ֻҪˮƽ����
		ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
		SetScrollRange(m_hWnd, SB_HORZ, 0, m_nMaxStringWidth - cx, FALSE);
		SetScrollPos(m_hWnd, SB_HORZ, 0, FALSE);
		m_bVertScroll=false;
		m_bHoriScroll=true;

		//MessageBox(L"need hscroll");
	}

	//����ֻҪ��ֱ�������Ƿ���
	else if(m_nMaxStringWidth < cx-nsw )
	{//ֻҪ��ֱ�����͹�
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);
		ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
		SetScrollRange(m_hWnd, SB_VERT, 0, m_SelectList.size() - cy/m_nMaxStringHeight, FALSE);
		SetScrollPos(m_hWnd, SB_VERT, 0, FALSE);
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
		ShowScrollBar(m_hWnd, SB_BOTH, TRUE);
		SetScrollRange(m_hWnd, SB_HORZ, 0, m_nMaxStringWidth - (cx-nsw), FALSE);
		SetScrollRange(m_hWnd, SB_VERT, 0, m_SelectList.size() - (cy-nvh)/m_nMaxStringHeight, FALSE);
		SetScrollPos(m_hWnd, SB_HORZ, 0, FALSE);
		SetScrollPos(m_hWnd, SB_VERT, 0, FALSE);
		m_bVertScroll=true;
		m_bHoriScroll=true;

	}
}
void CCheckWnd::OnHScroll(UINT nSBCode, UINT nPos)
{
	// TODO: Add your message handler code here and/or call default
	switch(nSBCode)
	{
	case SB_LEFT:
	case SB_ENDSCROLL:
		break;
	case SB_LINELEFT:
		{
			int pos = GetScrollPos(m_hWnd, SB_HORZ);
			if (pos > 0) {
				SetScrollPos(m_hWnd, SB_HORZ, pos - 1, TRUE);
				InvalidateRect(m_hWnd, NULL, TRUE);
			}
		}
		break;
	case SB_LINERIGHT:
		{
			int pos = GetScrollPos(m_hWnd, SB_HORZ);
			SetScrollPos(m_hWnd, SB_HORZ, pos + 1, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
		break;
	case SB_PAGELEFT:
		{
			int pos=GetScrollPos(m_hWnd, SB_HORZ);
			pos-=100;
			if(pos<0) pos=0;
			SetScrollPos(m_hWnd, SB_HORZ, pos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
		break;
	case SB_PAGERIGHT:
		{
			int pos=GetScrollPos(m_hWnd, SB_HORZ);
			pos+=100;
			int MinPos, MaxPos;
			GetScrollRange(m_hWnd, SB_HORZ, &MinPos, &MaxPos);
			if(pos>MaxPos) pos=MaxPos;
			SetScrollPos(m_hWnd, SB_HORZ, pos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
			
		}
		break;
	case SB_RIGHT :
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos(m_hWnd, SB_HORZ, nPos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
			
		}
		break;

	}//InvalidateRect(m_hWnd, NULL, TRUE);
}

void CCheckWnd::OnVScroll(UINT nSBCode, UINT nPos)
{

	switch(nSBCode)
	{
	case SB_BOTTOM:
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
		{
			int pos = GetScrollPos(m_hWnd, SB_VERT);
			SetScrollPos(m_hWnd, SB_VERT, pos + 1, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
		break;
	case SB_LINEUP:
		{
			int pos = GetScrollPos(m_hWnd, SB_VERT);
			if (pos > 0) {
				SetScrollPos(m_hWnd, SB_VERT, pos - 1, TRUE);
				InvalidateRect(m_hWnd, NULL, TRUE);
			}
		}
		break;
	case SB_PAGEUP:
		{
			int pos=GetScrollPos(m_hWnd, SB_VERT);
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			pos -= (rect.bottom - rect.top)/m_nMaxStringHeight;

			if(pos<0) pos=0;
			SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
		break;
	case SB_PAGEDOWN:
		{
			int pos=GetScrollPos(m_hWnd, SB_VERT);
			
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			pos += (rect.bottom - rect.top)/m_nMaxStringHeight;

			int MinPos, MaxPos;
			GetScrollRange(m_hWnd, SB_VERT, &MinPos, &MaxPos);
			if(pos>MaxPos) pos=MaxPos;
			SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
			
		}
		break;
	case SB_TOP:
		{
			SetScrollPos(m_hWnd, SB_VERT, 0, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos(m_hWnd, SB_VERT, nPos, TRUE);
			InvalidateRect(m_hWnd, NULL, TRUE);
			
		}
		break;

	}
}

void CCheckWnd::OnMButtonDown(UINT nFlags, POINT point)
{
	// TODO: Add your message handler code here and/or call default
}

void CCheckWnd::OnLButtonDown(UINT nFlags, POINT point)
{
	// TODO: Add your message handler code here and/or call default
	int line=point.y / m_nMaxStringHeight;

	if(m_bVertScroll)
	{
		line+=GetScrollPos(m_hWnd, SB_VERT);
		
	}

	if(m_SelectList.size() > line)
	{
		m_SelectList[line]=!m_SelectList[line];
	}

	InvalidateRect(m_hWnd, NULL, TRUE);
}

BOOL CCheckWnd::OnMouseWheel(UINT nFlags, short zDelta, POINT pt)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bVertScroll)
	{
		int pos=GetScrollPos(m_hWnd, SB_VERT);
		int MinPos, MaxPos;
		GetScrollRange(m_hWnd, SB_VERT, &MinPos, &MaxPos);

		if(zDelta>0)
		{//up
			if(pos < MaxPos)
			{
				int line=zDelta/WHEEL_DELTA;
				pos+=line;
				if(pos>MaxPos) pos=MaxPos;
				SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
				InvalidateRect(m_hWnd, NULL, TRUE);
			}
		}
		else
		{//down
			int line=zDelta/WHEEL_DELTA;
			if(pos>0)
			{
				pos -= line;
				if(pos<0) pos=0;
				SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
				InvalidateRect(m_hWnd, NULL, TRUE);
			}
		}
	}

	return TRUE;
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
