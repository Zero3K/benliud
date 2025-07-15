/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

This code is published under GPL v2 license.

****************************************************************/

// CheckWnd.cpp : implementation file

#include "stdafx.h"
#include "benliud.h"
#include "CheckWnd.h"

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

	HDC hMemDC = CreateCompatibleDC(hDC);

	// Get client rectangle
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	// Calculate drawing rectangle      
	RECT rf;
	rf.left = rf.top = 0;
	rf.right = m_nMaxStringWidth;
	rf.bottom = m_nMaxStringHeight * m_StringList.size();
	if(rf.right < rect.right) rf.right = rect.right;
	if(rf.bottom < rect.bottom) rf.bottom = rect.bottom;

	// Add padding                                                     
	rf.right += 50;
	rf.bottom += 50;

	HBITMAP hBitmap;
	hBitmap = CreateCompatibleBitmap(hDC, rf.right - rf.left, rf.bottom - rf.top);
	SelectObject(hMemDC, hBitmap);
	FillRect(hMemDC, &rf, (HBRUSH)(COLOR_WINDOW+1));

	// Draw items
	for(int i = 0; i < (int)m_StringList.size(); i++)
	{
		// Draw checkbox rectangle
		Rectangle(hMemDC, 0+2, i*m_nMaxStringHeight+2, 16-2, (i+1)*m_nMaxStringHeight-2);

		POINT old;
		if(m_SelectList[i])
		{
			// Draw checkmark
			MoveToEx(hMemDC, 4, i*m_nMaxStringHeight+9, &old);
			LineTo(hMemDC, 8, (i+1)*m_nMaxStringHeight-5);
			LineTo(hMemDC, 12, (i)*m_nMaxStringHeight+5);
		}

		// Draw text
		RECT textRect = {16, i*m_nMaxStringHeight, m_nMaxStringWidth, (i+1)*m_nMaxStringHeight};
		DrawText(hMemDC, m_StringList[i].c_str(), (int)m_StringList[i].length(),
			&textRect, DT_LEFT|DT_VCENTER);

		// Draw separator line
		MoveToEx(hMemDC, 0, (i+1)*m_nMaxStringHeight, &old);
		LineTo(hMemDC, rf.right, (i+1)*m_nMaxStringHeight);
	}

	// Copy from memory DC to screen DC
	if(!m_bVertScroll && !m_bHoriScroll)
	{
		BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, 0, SRCCOPY);
	}
	else if(m_bVertScroll && !m_bHoriScroll)
	{
		int pos = GetScrollPos(m_hWnd, SB_VERT);
		BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, pos*m_nMaxStringHeight, SRCCOPY);
	}
	else if(!m_bVertScroll && m_bHoriScroll)
	{
		int pos = GetScrollPos(m_hWnd, SB_HORZ);
		BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, pos, 0, SRCCOPY);
	}
	else
	{
		int pos1 = GetScrollPos(m_hWnd, SB_VERT);
		int pos2 = GetScrollPos(m_hWnd, SB_HORZ);
		BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, pos2, pos1*m_nMaxStringHeight, SRCCOPY);
	}

	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	
	EndPaint(m_hWnd, &ps);
}

BOOL CCheckWnd::OnEraseBkgnd(HDC hDC)
{
	return TRUE;
}

bool CCheckWnd::IsSelected(int itemid)
{
	return m_SelectList[itemid];
}

bool CCheckWnd::IsAnySelected()
{
	for(int i = 0; i < (int)m_SelectList.size(); i++)
	{
		if(m_SelectList[i]) return true;
	}
	return false;
}

void CCheckWnd::AddItem(const std::wstring& item, BOOL sel)
{
	HDC hDC = GetDC(m_hWnd);
	SIZE size;
	GetTextExtentPoint32(hDC, item.c_str(), (int)item.length(), &size);

	if(size.cx + 16 > m_nMaxStringWidth) 
	{
		m_nMaxStringWidth = size.cx + 16;
	}

	if(size.cy > m_nMaxStringHeight)
	{
		m_nMaxStringHeight = size.cy >= 18 ? size.cy : 18;
	}

	m_StringList.push_back(item);
	m_SelectList.push_back(sel);

	ReleaseDC(m_hWnd, hDC);
}

void CCheckWnd::OnSize(UINT nType, int cx, int cy)
{
	ReCalScroll(cx, cy);
}

void CCheckWnd::ReCalScroll(int cx, int cy)
{
	if(m_StringList.size() == 0) return;
	
	// Calculate if scrollbars are needed
	int totalHeight = m_nMaxStringHeight * m_StringList.size();
	int totalWidth = m_nMaxStringWidth;
	
	m_bVertScroll = (totalHeight > cy);
	m_bHoriScroll = (totalWidth > cx);
	
	// Set up scrollbar info
	if(m_bVertScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = (int)m_StringList.size() - 1;
		si.nPage = cy / m_nMaxStringHeight;
		SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);
	}
	else
	{
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
	}
	
	if(m_bHoriScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = totalWidth;
		si.nPage = cx;
		SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
		ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
	}
	else
	{
		ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
	}
}

void CCheckWnd::OnHScroll(UINT nSBCode, UINT nPos)
{
	int pos = GetScrollPos(m_hWnd, SB_HORZ);
	int oldPos = pos;
	
	switch(nSBCode)
	{
		case SB_LINELEFT:
			pos -= 10;
			break;
		case SB_LINERIGHT:
			pos += 10;
			break;
		case SB_PAGELEFT:
			pos -= 50;
			break;
		case SB_PAGERIGHT:
			pos += 50;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			pos = nPos;
			break;
	}
	
	if(pos < 0) pos = 0;
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE;
	GetScrollInfo(m_hWnd, SB_HORZ, &si);
	if(pos > si.nMax) pos = si.nMax;
	
	if(pos != oldPos)
	{
		SetScrollPos(m_hWnd, SB_HORZ, pos, TRUE);
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

void CCheckWnd::OnVScroll(UINT nSBCode, UINT nPos)
{
	int pos = GetScrollPos(m_hWnd, SB_VERT);
	int oldPos = pos;
	
	switch(nSBCode)
	{
		case SB_LINEUP:
			pos--;
			break;
		case SB_LINEDOWN:
			pos++;
			break;
		case SB_PAGEUP:
			pos -= 5;
			break;
		case SB_PAGEDOWN:
			pos += 5;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			pos = nPos;
			break;
	}
	
	if(pos < 0) pos = 0;
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE;
	GetScrollInfo(m_hWnd, SB_VERT, &si);
	if(pos > si.nMax) pos = si.nMax;
	
	if(pos != oldPos)
	{
		SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

void CCheckWnd::OnMButtonDown(UINT nFlags, POINT point)
{
	// Handle middle button click if needed
}

void CCheckWnd::OnLButtonDown(UINT nFlags, POINT point)
{
	// Check if click is on a checkbox
	int itemIndex = point.y / m_nMaxStringHeight;
	if(itemIndex >= 0 && itemIndex < (int)m_StringList.size())
	{
		if(point.x >= 0 && point.x <= 16)
		{
			// Toggle selection
			m_SelectList[itemIndex] = !m_SelectList[itemIndex];
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
	}
}

BOOL CCheckWnd::OnMouseWheel(UINT nFlags, short zDelta, POINT pt)
{
	if(m_bVertScroll)
	{
		int pos = GetScrollPos(m_hWnd, SB_VERT);
		pos -= (zDelta / WHEEL_DELTA);
		if(pos < 0) pos = 0;
		
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, SB_VERT, &si);
		if(pos > si.nMax) pos = si.nMax;
		
		SetScrollPos(m_hWnd, SB_VERT, pos, TRUE);
		InvalidateRect(m_hWnd, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}