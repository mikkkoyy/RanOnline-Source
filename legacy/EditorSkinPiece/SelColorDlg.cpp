// SelColorDlg.cpp : implementation file
//

// Bug fix by gl.tter (CodeProject)
// http://www.codeproject.com/miscctrl/LeaColourSelector.asp?select=1750422&df=100&forumid=34632&msg=1750422
//
// Many thanks again.
//
// Original version displayed a slightly unusual colour spectrum.
// For more details on this please refer to the message posting.
//

#include "stdafx.h"
#include "SelColorDlg.h"
#include ".\selcolordlg.h"

// create an instance of the custom colours class
CColorSet SetColorDlg_Custom;

// CSelColorDlg dialog

IMPLEMENT_DYNAMIC(CSelColorDlg, CDialog)
CSelColorDlg::CSelColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelColorDlg::IDD, pParent), nDragMode(-1), nDragSrc(-1),
	m_nHue(0), m_nSat(240), m_nLum(120), m_nRed(255), m_nGreen(0),
	m_nBlue(0), m_nNewRed(255), m_nNewGreen(0), m_nNewBlue(0),
	m_nInsertPos(0)
{
	// initialise the default position
	m_ptPos.x = m_ptPos.y = 0;
	// initialise the colour reference
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// initialise the positions of the custom colors
	InitCustomColors();

	// initialise the color spectrum
	InitColorSpectrum();
	// regenerate the color bar
	GenColorBar();
}

CSelColorDlg::CSelColorDlg(BYTE nRedVal, BYTE nGreenVal,
						   BYTE nBlueVal, CWnd *pParent /*=NULL*/)
	: CDialog(CSelColorDlg::IDD, pParent), nDragMode(-1), nDragSrc(-1),
	m_nHue(0), m_nSat(0), m_nLum(0), m_nRed(nRedVal), m_nGreen(nGreenVal),
	m_nBlue(nBlueVal), m_nNewRed(nRedVal), m_nNewGreen(nGreenVal), m_nNewBlue(nBlueVal),
	m_nInsertPos(0)
{
	// select this initial colour
	//SelectColor(nRedVal, nGreenVal, nBlueVal);

	// restore the old color
	RGBtoHLS(RGB(nRedVal, nGreenVal, nBlueVal), &m_nHue, &m_nLum, &m_nSat);
	m_ptPos.x = (long)(((float)m_nHue / 239.0f) * 190.0f);
	m_ptPos.y = (long)(190.0f - (((float)m_nSat / 240.0f) * 190.0f));
	if(m_ptPos.x < 0) m_ptPos.x = 0;
	if(m_ptPos.x > 190) m_ptPos.x = 190;
	if(m_ptPos.y < 0) m_ptPos.y = 0;
	if(m_ptPos.y > 190) m_ptPos.y = 190;
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// initialise the positions of the custom colors
	InitCustomColors();

	// initialise the color spectrum
	InitColorSpectrum();
	// regenerate the color bar
	GenColorBar();
}

CSelColorDlg::~CSelColorDlg()
{
	// clean up the dc objects
	m_bmpSpectrum.DeleteObject();
	m_bmpBar.DeleteObject();
	m_tempPen.DeleteObject();
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::DDX_TextEx(CDataExchange* pDX, UINT nID, BYTE nMin, BYTE nMax, BYTE &nValue)
{
	CEdit* pCurEdit = (CEdit*)GetDlgItem(nID);
	CString strTemp;
	if(pDX->m_bSaveAndValidate)
	{
		pCurEdit->GetWindowText(strTemp);
		int nOffset = 0, nLength = strTemp.GetLength();
		for(int nPos = 0; nPos < nLength; nPos++)
		{
			char nCurChar = strTemp[nPos - nOffset];
			if(isalpha(nCurChar) || !isalnum(nCurChar))
			{
				strTemp.Delete(nPos);
				nOffset++;
			}
		}
		if(!strTemp.IsEmpty())
		{
			int nVal = atoi(strTemp);
			if(nVal < nMin) nVal = nMin;
			if(nVal > nMax) nVal = nMax;
			nValue = nVal;
			char szTemp[65];
			itoa(nValue, szTemp, 10);
			strTemp = szTemp;
		}
		int nStart, nEnd;
		pCurEdit->GetSel(nStart, nEnd);
		pCurEdit->SetWindowText(strTemp);
		pCurEdit->SetSel(nStart, nEnd);
	}
	else
	{
		if(nValue < nMin) nValue = nMin;
		if(nValue > nMax) nValue = nMax;
		int nStart, nEnd;
		pCurEdit->GetSel(nStart, nEnd);
		nEnd = nStart;
		strTemp.Format("%d", nValue);
		pCurEdit->SetWindowText(strTemp);
		pCurEdit->SetSel(nStart, nEnd);
	}
}

void CSelColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_REF, m_strRef);

	DDX_TextEx(pDX, IDC_HUE, 0, 239, m_nHue);
	DDX_TextEx(pDX, IDC_SAT, 0, 240, m_nSat);
	DDX_TextEx(pDX, IDC_LUM, 0, 240, m_nLum);
	DDX_TextEx(pDX, IDC_RED, 0, 255, m_nNewRed);
	DDX_TextEx(pDX, IDC_GREEN, 0, 255, m_nNewGreen);
	DDX_TextEx(pDX, IDC_BLUE, 0, 255, m_nNewBlue);
}


BEGIN_MESSAGE_MAP(CSelColorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_EN_CHANGE(IDC_HUE, OnEnChangeHue)
	ON_EN_CHANGE(IDC_SAT, OnEnChangeSat)
	ON_EN_CHANGE(IDC_LUM, OnEnChangeLum)
	ON_EN_CHANGE(IDC_RED, OnEnChangeColor)
	ON_EN_CHANGE(IDC_GREEN, OnEnChangeColor)
	ON_EN_CHANGE(IDC_BLUE, OnEnChangeColor)
	ON_EN_CHANGE(IDC_REF, OnEnChangeRef)
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(ID_ADDCOLOUR, OnBnClickedAddcolour)
END_MESSAGE_MAP()


// CSelColorDlg message handlers

#define HLSMAX		240		// H,L, and S vary over 0-HLSMAX
#define RGBMAX		255		// R,G, and B vary over 0-RGBMAX
							// HLSMAX BEST IF DIVISIBLE BY 6
							// RGBMAX, HLSMAX must each fit in a byte.

// Hue is undefined if Saturation is 0 (grey-scale)
// This value determines where the Hue scrollbar is
// initially set for achromatic colors
#define UNDEFINED (HLSMAX * 2 / 3)

void CSelColorDlg::RGBtoHLS(COLORREF lRGBColor, BYTE* H, BYTE* L, BYTE* S)
{
	WORD R, G, B;					// input RGB values
	BYTE cMax, cMin;				// max and min RGB values
	WORD Rdelta, Gdelta, Bdelta;	// intermediate value: % of spread from max

	// get R, G, and B out of DWORD
	R = GetRValue(lRGBColor);
	G = GetGValue(lRGBColor);
	B = GetBValue(lRGBColor);

	// calculate lightness 
	cMax = (BYTE)max(max(R, G), B);
	cMin = (BYTE)min(min(R, G), B);
	*L = (((cMax + cMin) * HLSMAX) + RGBMAX) / (2 * RGBMAX);

	if(cMax == cMin)				// r = g = b --> achromatic case 
	{
		*S = 0;						// saturation
		*H = UNDEFINED;				// hue
	}
	else
	{								// chromatic case
		// saturation
		if(*L <= (HLSMAX / 2))
		*S = (((cMax - cMin) * HLSMAX) + ((cMax + cMin) / 2)) / (cMax + cMin);
		else
		*S = (((cMax - cMin) * HLSMAX) + ((2 * RGBMAX - cMax - cMin) / 2))
			/ (2 * RGBMAX - cMax - cMin);

		// hue 
		Rdelta = (((cMax - R) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
		Gdelta = (((cMax - G) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
		Bdelta = (((cMax - B) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);

		if(R == cMax)
		*H = Bdelta - Gdelta;
		else if(G == cMax)
		*H = (HLSMAX / 3) + Rdelta - Bdelta;
		else /* B == cMax */ 
		*H = ((2 * HLSMAX) / 3) + Gdelta - Rdelta;

		if(*H < 0)
		*H += HLSMAX;
		if(*H > HLSMAX)
		*H -= HLSMAX;
	}
}

WORD CSelColorDlg::HueToRGB(WORD n1, WORD n2, WORD hue)
{
	// range check: note values passed add/subtract thirds of range
	if(hue < 0)
		hue += HLSMAX;

	if(hue > HLSMAX)
		hue -= HLSMAX;

	// return r,g, or b value from this tridrant
	if(hue < (HLSMAX / 6))
		return (n1 + (((n2 - n1) * hue + (HLSMAX / 12)) / (HLSMAX / 6)));
	if(hue < (HLSMAX / 2))
		return (n2);
	if(hue < ((HLSMAX * 2) / 3))
		return (n1 + (((n2 - n1) * (((HLSMAX * 2) / 3) - hue) + (HLSMAX / 12)) / (HLSMAX / 6)));
	else
		return n1;
}

DWORD CSelColorDlg::HLStoRGB(WORD hue, WORD lum, WORD sat)
{
    WORD R, G, B;				// RGB component values
    WORD Magic1, Magic2;		// calculated magic numbers (really!)

    if(sat == 0)				// achromatic case
	{
        R = G = B = (lum * RGBMAX) / HLSMAX;
        if(hue != UNDEFINED) {
        /* ERROR */ 
        }
    }
    else						// chromatic case
	{
		// set up magic numbers
		if (lum <= (HLSMAX / 2))
		Magic2 = (lum * (HLSMAX + sat) + (HLSMAX / 2)) / HLSMAX;
		else
		Magic2 = lum + sat - ((lum * sat) + (HLSMAX / 2)) / HLSMAX;
		Magic1 = 2 * lum - Magic2;

		// get RGB, change units from HLSMAX to RGBMAX 
		R = (HueToRGB(Magic1, Magic2, hue + (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
		G = (HueToRGB(Magic1, Magic2, hue) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
		B = (HueToRGB(Magic1, Magic2, hue - (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
    }
    return(RGB(R, G, B));
}

void CSelColorDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// draw the graduated color bar
	DrawGradBar(&dc);
	// draw the spectrum
	DrawColorSpectrum(&dc);
	// draw the color bar
	DrawColorBar(&dc);
	// draw the current position
	DrawCross(&dc, &m_ptPos);
	// draw the bars arrow
	DrawBarArrow(&dc, m_nLum);
	// draw the color boxes
	DrawCurColorBox(&dc);
	DrawNewColorBox(&dc);
	// draw the custom color boxes
	DrawCustomColors(&dc);

	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CSelColorDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// if an item is being dragged then...
	if(nDragMode != -1)
	{
		// use the drag item cursor
		SetCursor(LoadCursor(AfxGetResourceHandle(),
			MAKEINTRESOURCE(IDC_DRAGITEM)));
		// return TRUE for success
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CSelColorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// if the mouse is over the color spectrum
	if(point.x >= 14 && point.x <= 204 &&
		point.y >= 13 && point.y <= 203)
	{
		// create an instance of the drawing context
		CClientDC dc(this);

		// capture this window
		SetCapture();

		// say that the mouse button is down
		nDragMode = 0;

		// draw the current position
		DrawColorSpectrum(&dc);

		// drag the current position around
		m_ptPos.x = point.x - 14;
		m_ptPos.y = point.y - 13;
		if(m_ptPos.x < 0) m_ptPos.x = 0;
		if(m_ptPos.x > 190) m_ptPos.x = 190;
		if(m_ptPos.y < 0) m_ptPos.y = 0;
		if(m_ptPos.y > 190) m_ptPos.y = 190;

		// draw the current position
		DrawCross(&dc, &m_ptPos);

		// get the colour from the new position
		m_nHue = (BYTE)(((float)m_ptPos.x / 190.0f) * 239.0f);
		m_nSat = (BYTE)(240.0f - (((float)m_ptPos.y / 190.0f) * 240.0f));
		COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
		m_nNewRed = GetRValue(rgbColor);
		m_nNewGreen = GetGValue(rgbColor);
		m_nNewBlue = GetBValue(rgbColor);
		m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

		// regenerate the color bar
		GenColorBar();
		// redraw the colour bar
		DrawColorBar(&dc);
		// redraw the new color box
		DrawNewColorBox(&dc);

		// update the dialogs data
		UpdateData(FALSE);
	}
	// otherwise if the mouse is over the colour adjust bar then...
	else if(point.x >= 218 && point.x <= 238 &&
		point.y >= 13 && point.y <= 203)
	{
		// create an instance of the drawing context
		CClientDC dc(this);

		// capture this window
		SetCapture();

		// say that the mouse button is down
		nDragMode = 1;

		// recalculate the luminance value
		m_nLum = (BYTE)(((point.y - 13.0f) / 190.0f) * 240.0f);
		if(point.y < 13) m_nLum = 0;
		if(point.y > 203) m_nLum = 240;
		m_nLum = 240 - m_nLum;

		// redraw the new arrow
		DrawBarArrow(&dc, m_nLum);

		// get the colour from the new position
		m_nHue = (BYTE)((m_ptPos.x / 190.0f) * 239.0f);
		m_nSat = (BYTE)(240.0f - ((m_ptPos.y / 190.0f) * 240.0f));
		COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
		m_nNewRed = GetRValue(rgbColor);
		m_nNewGreen = GetGValue(rgbColor);
		m_nNewBlue = GetBValue(rgbColor);
		m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

		// redraw the new color box
		DrawNewColorBox(&dc);

		// update the dialogs data
		UpdateData(FALSE);
	}
	// otherwise if the mouse is over the old color box then...
	else if(point.x >= 272 && point.x <= 316 &&
		point.y >= 30 && point.y <= 49)
	{
		// say that we are dragging a color
		nDragMode = 2;
		// say what we are dragging
		nDragSrc = -3;

		// say that the mouse hasn't yet moved
		m_bMouseMove = false;

		// store the color of this color box
		m_nDragRed = m_nRed;
		m_nDragGreen = m_nGreen;
		m_nDragBlue = m_nBlue;

		// send a message to update the mouse pointer
		SendMessage(WM_SETCURSOR);
	}
	// otherwise if the mouse is over the new color box then...
	else if(point.x >= 359 && point.x <= 403 &&
		point.y >= 30 && point.y <= 49)
	{
		// say that we are dragging a color
		nDragMode = 2;
		// say what we are dragging
		nDragSrc = -2;

		// say that the mouse hasn't yet moved
		m_bMouseMove = false;

		// store the color of this color box
		m_nDragRed = m_nNewRed;
		m_nDragGreen = m_nNewGreen;
		m_nDragBlue = m_nNewBlue;
	}
	else
	{
		// find out which custom color box the mouse is over (if any!)
		char nCustomBox = GetCustomBox(&point);
		if(nCustomBox != -1)
		{
			// say that we are dragging a color
			nDragMode = 2;
			// say what we are dragging
			nDragSrc = nCustomBox;

			// say that the mouse hasn't yet moved
			m_bMouseMove = false;

			// get the color within this custom color box
			COLORREF rgbColor = SetColorDlg_Custom.GetColor(nCustomBox);
			// store this color as the drag color
			m_nDragRed = GetRValue(rgbColor);
			m_nDragGreen = GetGValue(rgbColor);
			m_nDragBlue = GetBValue(rgbColor);
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CSelColorDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// if the left mouse button is down then...
	if(nDragMode == 0)
	{
		// create an instance of the drawing context
		CClientDC dc(this);

		// draw the current position
		DrawColorSpectrum(&dc);

		// drag the current position around
		m_ptPos.x = point.x - 14;
		m_ptPos.y = point.y - 13;
		if(m_ptPos.x < 0) m_ptPos.x = 0;
		if(m_ptPos.x > 190) m_ptPos.x = 190;
		if(m_ptPos.y < 0) m_ptPos.y = 0;
		if(m_ptPos.y > 190) m_ptPos.y = 190;

		// draw the current position
		DrawCross(&dc, &m_ptPos);

		// get the colour from the new position
		m_nHue = (BYTE)((m_ptPos.x / 190.0f) * 239.0f);
		m_nSat = (BYTE)(240.0f - ((m_ptPos.y / 190.0f) * 240.0f));
		COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
		m_nNewRed = GetRValue(rgbColor);
		m_nNewGreen = GetGValue(rgbColor);
		m_nNewBlue = GetBValue(rgbColor);
		m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

		// regenerate the color bar
		GenColorBar();
		// redraw the colour bar
		DrawColorBar(&dc);
		// redraw the new color box
		DrawNewColorBox(&dc);

		// update the dialogs data
		UpdateData(FALSE);
	}
	else if(nDragMode == 1)
	{
		// create an instance of the drawing context
		CClientDC dc(this);

		// recalculate the luminance value
		m_nLum = (BYTE)(((point.y - 13.0f) / 190.0f) * 240.0f);
		if(point.y < 13) m_nLum = 0;
		if(point.y > 203) m_nLum = 240;
		m_nLum = 240 - m_nLum;

		// redraw the new arrow
		DrawBarArrow(&dc, m_nLum);

		// get the colour from the new position
		m_nHue = (BYTE)((m_ptPos.x / 190.0f) * 239.0f);
		m_nSat = (BYTE)(240.0f - ((m_ptPos.y / 190.0f) * 240.0f));
		COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
		m_nNewRed = GetRValue(rgbColor);
		m_nNewGreen = GetGValue(rgbColor);
		m_nNewBlue = GetBValue(rgbColor);
		m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

		// redraw the new color box
		DrawNewColorBox(&dc);

		// update the dialogs' data
		UpdateData(FALSE);
	}
	else if(nDragMode == 2 && !m_bMouseMove)
	{
		// send a message to update the mouse pointer
		SendMessage(WM_SETCURSOR);
	}

	// say that the mouse has been moved
	m_bMouseMove = true;

	CDialog::OnMouseMove(nFlags, point);
}

void CSelColorDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// if a color was being dragged then...
	if(nDragMode == 2)
	{
		// otherwise if the mouse over any of the custom colours?
		if(point.x >= 11 && point.x <= 242 &&
			point.y >= 216 && point.y <= 260)
		{
			// the custom color is yet unknown
			int nColorBox = GetCustomBox(&point);
			// if the mouse wasn't moved or the color being dragged
			// was dropped onto itself then...
			if(!m_bMouseMove || nColorBox == nDragSrc)
			{
				// select the color being dragged
				SelectColor(m_nDragRed, m_nDragGreen, m_nDragBlue);
			}
			// if the mouse was moved then drag the item
			else if(m_bMouseMove)
			{
				// create an instance of the drawing context
				CClientDC dc(this);

				// add this color to the custom color box
				SetColorDlg_Custom.SetColor(nColorBox,
					RGB(m_nDragRed, m_nDragGreen, m_nDragBlue));
				// redraw the custom color boxes
				DrawCustomColors(&dc);

				// if necessary change the colour box value
				if(nColorBox > m_nInsertPos)
				{
					// set the colour box position as required
					m_nInsertPos = nColorBox + 1;
					// if required correct this colour box value
					if(m_nInsertPos >= SetColorDlg_Custom.GetCount())
						m_nInsertPos = 0;
				}
			}
		}
		// otherwise if the mouse is over the new color box then...
		else if((nDragSrc == -3 && !m_bMouseMove) ||
			(point.x >= 359 && point.x <= 403 &&
			point.y >= 30 && point.y <= 49))
		{
			// select the color being dragged
			SelectColor(m_nDragRed, m_nDragGreen, m_nDragBlue);
		}
	}

	// say that the mouse button has been released
	m_bMouseMove = false;
	nDragMode = -1;

	// send a message to update the mouse pointer
	SendMessage(WM_SETCURSOR);

	// release the capture of this window
	ReleaseCapture();

	CDialog::OnLButtonUp(nFlags, point);
}

void CSelColorDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// cancel all drag functions
	nDragMode = -1;
	nDragSrc = -1;

	// send a message to update the mouse pointer
	SendMessage(WM_SETCURSOR);

	CDialog::OnRButtonDown(nFlags, point);
}

void CSelColorDlg::InitCustomColors(bool bMonochrome /*=false*/)
{
	// initialise the positions of the custom colors
	int nHalfWay = SetColorDlg_Custom.GetCount() / 2;
	int nCurColor, nNewLeft, nNewTop;
	for(int nColor = 0; nColor < nHalfWay; nColor++)
	{
		nCurColor = nColor;
		nNewLeft = (nCurColor * 13) + ((nCurColor + 1) * 11);
		nNewTop = 216;
		for(int nCycle = 0; nCycle < 2; nCycle++)
		{
			SetColorDlg_Custom.SetPos(nCurColor, nNewLeft, nNewTop);
			if(bMonochrome)
			{
				char nColorVal = 255 - (nCurColor * 13);
				SetColorDlg_Custom.SetColor(nCurColor,
					RGB(nColorVal, nColorVal, nColorVal));
			}
			nNewTop = 243;
			nCurColor += nHalfWay;
		}
	}
}

void CSelColorDlg::InitColorSpectrum()
{
	// Bug fix by gl.tter (CodeProject)
	// http://www.codeproject.com/miscctrl/LeaColourSelector.asp?select=1750422&df=100&forumid=34632&msg=1750422
	//
	// Many thanks again.
	//
	// Original version displayed a slightly unusual colour spectrum.
	// For more details on this please refer to the message posting.
	//

	// declare some storage space for the spectrum
	//each line of bitmap must be divisible with 4, so we pad it with 0
	//(width*3+pad)*height
	BYTE bits[(190*3+2)*190];
	BYTE *temp=bits;

	// generate the colour spectrum
	for(int y = 0; y < 190; y++)
	{
		for(int x = 0; x < 190; x++)
		{
			COLORREF rgb;
			int r, g, b;

			rgb = HLStoRGB(239 * x / 189, 120, 240 * y / 189);
			r = GetRValue(rgb);
			g = GetGValue(rgb);
			b = GetBValue(rgb);

			*temp++ = b;
			*temp++ = g;
			*temp++ = r;
		}
		//each line of bitmap must be divisible with 4, so we pad it with 0
		temp+=2;
	}

	// create a bitmap from this bit data
	m_bmpSpectrum.DeleteObject();
	//m_bmpSpectrum.CreateBitmap(190, 190, 1, 32, bits);

	CClientDC pDC(NULL);
	CDC memDC;
	memDC.CreateCompatibleDC(&pDC);

	BITMAPINFO bi;
	BITMAPINFOHEADER &bih = bi.bmiHeader;
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biClrUsed = 0;
	bih.biCompression = 0;
	bih.biHeight = 190;
	bih.biPlanes = 1;
	bih.biSize = 40; //size of this struct
	bih.biSizeImage = (190 * 3 + 2) * 190;
	bih.biWidth = 190;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;

	HBITMAP bmp = CreateDIBitmap(pDC, &bih, CBM_INIT, bits, &bi, DIB_RGB_COLORS);
	m_bmpSpectrum.Attach(bmp);
}

void CSelColorDlg::DrawGradBar(CDC* pDC)
{
	// get the dialog boxes client rectangle
	CRect rectClient;
	GetClientRect(&rectClient);
	for(int i = 0; i < 31; i++)
	{
		// draw the white panel at the bottom of the dialog box
		m_tempPen.DeleteObject();
		m_tempPen.CreatePen(PS_SOLID, 1, RGB(255 - ((91 / 32) * i), 255 - ((91 / 32) * i), 255 - ((91 / 32) * i)));
		pDC->SelectObject(&m_tempPen);
		pDC->MoveTo(0, rectClient.bottom - i);
		pDC->LineTo(rectClient.right, rectClient.bottom - i);
	}
	m_tempPen.DeleteObject();
	m_tempPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	pDC->SelectObject(&m_tempPen);
	pDC->MoveTo(3, rectClient.bottom - 34);
	pDC->LineTo(rectClient.right - 3, rectClient.bottom - 34);
	m_tempPen.DeleteObject();
	m_tempPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
	pDC->SelectObject(&m_tempPen);
	pDC->MoveTo(3, rectClient.bottom - 33);
	pDC->LineTo(rectClient.right - 3, rectClient.bottom - 33);
	// clean up the dc objects as required
	m_tempPen.DeleteObject();
}

void CSelColorDlg::DrawFrameBorder(CDC* pDC, LPRECT rect)
{
	// get the required colors
	COLORREF rgbBtnHigh = RGB(255, 255, 255);
	COLORREF rgbBtnShdw = RGB(160, 160, 160);
	COLORREF rgbDark = RGB(120, 120, 120);

	// draw the border
	pDC->Draw3dRect(rect, rgbDark, rgbDark);
	rect->left--;
	rect->right++;
	rect->top--;
	rect->bottom++;
	pDC->Draw3dRect(rect, rgbBtnShdw, rgbBtnHigh);
}

void CSelColorDlg::GenColorBar()
{
	BYTE bits[(1*3+2)*190];
	BYTE *temp=bits;

	// generate the color bar
	float lum;
	BYTE hue = (BYTE)((m_ptPos.x / 190.0f) * 239.0f);
	BYTE sat = (BYTE)(240.0f - ((m_ptPos.y / 190.0f) * 240.0f));
	for(int l = 0; l < 190; l++)
	{
		lum = (l / 190.0f) * 240.0f;
		COLORREF rgb = HLStoRGB(hue, (WORD)lum, sat);
		*temp = GetBValue(rgb);
		temp++;
		*temp = GetGValue(rgb);
		temp++;
		*temp = GetRValue(rgb);
		temp++;
		//each line of bitmap must be divisible with 4, so we pad it with 0
		temp+=1;
	}

	// create a bitmap from this bit data
	m_bmpBar.DeleteObject();

	CClientDC pDC(NULL);
	CDC memDC;
	memDC.CreateCompatibleDC(&pDC);

	BITMAPINFO bi;
	BITMAPINFOHEADER &bih = bi.bmiHeader;
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biClrUsed = 0;
	bih.biCompression = 0;
	bih.biHeight = 190;
	bih.biPlanes = 1;
	bih.biSize = 40; //size of this struct
	bih.biSizeImage = (1 * 3 + 2) * 190;
	bih.biWidth = 1;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;

	HBITMAP bmp = CreateDIBitmap(pDC, &bih, CBM_INIT, bits, &bi, DIB_RGB_COLORS);
	m_bmpBar.Attach(bmp);
}

void CSelColorDlg::DrawColorSpectrum(CDC* pDC)
{
	// draw the spectrum
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&m_bmpSpectrum);
	pDC->BitBlt(14, 13, 250, 250, &memDC, 0, 0, SRCCOPY);
	// draw the spectrums' border
	RECT rectBorder = { 13, 12, 205, 204 };
	DrawFrameBorder(pDC, &rectBorder);
	// clean up the dc objects as required
	memDC.DeleteDC();
}

void CSelColorDlg::DrawColorBar(CDC* pDC)
{
	// draw the color bar
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&m_bmpBar);
	pDC->StretchBlt(218, 13, 20, 190, &memDC, 0, 0, 1, 190, SRCCOPY);
	// draw the color bars' border
	RECT rectBorder = { 217, 12, 239, 204 };
	DrawFrameBorder(pDC, &rectBorder);
	// clean up the dc objects as required
	memDC.DeleteDC();
}

void CSelColorDlg::DrawCurColorBox(CDC* pDC)
{
	// draw the current color box
	m_tempBrush.CreateSolidBrush(RGB(m_nRed, m_nGreen, m_nBlue));
	RECT rectBox = { 272, 30, 316, 49 };
	pDC->FillRect(&rectBox, &m_tempBrush);
	// draw the current color box's border
	RECT rectBorder = { 271, 29, 317, 50 };
	DrawFrameBorder(pDC, &rectBorder);
	// clean up the dc objects as required
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::DrawNewColorBox(CDC* pDC)
{
	// draw the current color box
	m_tempBrush.CreateSolidBrush(RGB(m_nNewRed, m_nNewGreen, m_nNewBlue));
	RECT rectBox = { 359, 30, 403, 49 };
	pDC->FillRect(&rectBox, &m_tempBrush);
	// draw the new color box's border
	RECT rectBorder = { 358, 29, 404, 50 };
	DrawFrameBorder(pDC, &rectBorder);
	// clean up the dc objects as required
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::DrawCustomColors(CDC* pDC)
{
	// draw the custom colors' boxes
	int nXPos = 11;
	int nYPos = 216;
	int nTotalColors = SetColorDlg_Custom.GetCount();
	int nHalfWay = nTotalColors / 2;
	for(int nColor = 0; nColor < nTotalColors; nColor++)
	{
		// calculate the position to draw this custom color box
		if(nColor >= nHalfWay)
		{
			nHalfWay = nTotalColors;
			nXPos = 11;
			nYPos += 25;
		}
		m_tempBrush.DeleteObject();
		m_tempBrush.CreateSolidBrush(SetColorDlg_Custom.GetColor(nColor));
		// shade the square in
		RECT rectSquare = { nXPos, nYPos, nXPos + 14, nYPos + 13 };
		pDC->FillRect(&rectSquare, &m_tempBrush);
		// draw the spectrums' border
		RECT rectBorder = { nXPos - 1, nYPos - 1, nXPos + 15, nYPos + 14 };
		DrawFrameBorder(pDC, &rectBorder);
		// mvove to the next x position
		nXPos += 24;
	}
	// clean up the dc objects as required
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::DrawCross(CDC* pDC, LPPOINT ptPos)
{
	// prepare the dc for the following drawing events
	m_tempPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	pDC->SelectObject(&m_tempPen);

	// clean up the window
	m_tempBrush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	RECT rectBar1 = { 4, 3, 205, 11 };
	pDC->FillRect(&rectBar1, &m_tempBrush);
	RECT rectBar2 = { 4, 11, 12, 204 };
	pDC->FillRect(&rectBar2, &m_tempBrush);
	RECT rectBar3 = { 4, 205, 205, 214 };
	pDC->FillRect(&rectBar3, &m_tempBrush);
	RECT rectBar4 = { 206, 3, 215, 213 };
	pDC->FillRect(&rectBar4, &m_tempBrush);

	// display the selected position
	int nX = m_ptPos.x + 14;
	int nY = m_ptPos.y + 13;
	pDC->MoveTo(nX, nY - 10);
	pDC->LineTo(nX, nY - 2);
	pDC->MoveTo(nX, nY + 3);
	pDC->LineTo(nX, nY + 11);
	pDC->MoveTo(nX - 10, nY);
	pDC->LineTo(nX - 2, nY);
	pDC->MoveTo(nX + 3, nY);
	pDC->LineTo(nX + 11, nY);

	// clean up the dc again
	m_tempPen.DeleteObject();
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::DrawBarArrow(CDC* pDC, int nLum)
{
	// prepare the dc for the following drawing events
	m_tempPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	pDC->SelectObject(&m_tempPen);

	// clean up the window
	m_tempBrush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	RECT CleanUp = { 240, 5, 248, 209 };
	pDC->FillRect(&CleanUp, &m_tempBrush);

	nLum = (int)(204 - ((nLum / 240.0f) * 190.0f));

	// display the selected position
	for(int nLine = 0; nLine < 6; nLine++)
	{
		pDC->MoveTo(240 + nLine, nLum - nLine);
		pDC->LineTo(240 + nLine, nLum + nLine);
	}

	// clean up the dc again
	m_tempPen.DeleteObject();
	m_tempBrush.DeleteObject();
}

void CSelColorDlg::OnEnChangeHue()
{
	// create an instance of the drawing context
	CClientDC dc(this);

	// update the dialog boxes' data
	UpdateData();

	// draw the current position
	DrawColorSpectrum(&dc);

	// drag the current position around
	float fTemp = (m_nHue / 239.0f) * 190.0f;
	m_ptPos.x = (long)fTemp;
	if(m_ptPos.x < 0) m_ptPos.x = 0;
	if(m_ptPos.x > 190) m_ptPos.x = 190;

	COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);//RGB(m_nNewRed, m_nNewGreen, m_nNewBlue);
	m_nNewRed = GetRValue(rgbColor);
	m_nNewGreen = GetGValue(rgbColor);
	m_nNewBlue = GetBValue(rgbColor);
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// regenerate the color bar
	GenColorBar();
	// draw the current position
	DrawCross(&dc, &m_ptPos);
	// redraw the colour bar
	DrawColorBar(&dc);
	// redraw the new color box
	DrawNewColorBox(&dc);
	// redraw the new arrow
	DrawBarArrow(&dc, m_nLum);

	// update the dialog boxes' data
	UpdateData(FALSE);
}

void CSelColorDlg::OnEnChangeSat()
{
	// create an instance of the drawing context
	CClientDC dc(this);

	// update the dialog boxes' data
	UpdateData();

	// draw the current position
	DrawColorSpectrum(&dc);

	// drag the current position around
	m_ptPos.y = (long)(190.0f - ((m_nSat / 240.0f) * 190.0f));
	if(m_ptPos.y < 0) m_ptPos.y = 0;
	if(m_ptPos.y > 190) m_ptPos.y = 190;

	COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
	m_nNewRed = GetRValue(rgbColor);
	m_nNewGreen = GetGValue(rgbColor);
	m_nNewBlue = GetBValue(rgbColor);
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// regenerate the color bar
	GenColorBar();
	// draw the current position
	DrawCross(&dc, &m_ptPos);
	// redraw the colour bar
	DrawColorBar(&dc);
	// redraw the new color box
	DrawNewColorBox(&dc);
	// redraw the new arrow
	DrawBarArrow(&dc, m_nLum);

	// update the dialog boxes' data
	UpdateData(FALSE);
}

void CSelColorDlg::OnEnChangeLum()
{
	// create an instance of the drawing context
	CClientDC dc(this);

	// update the dialog boxes' data
	UpdateData();

	COLORREF rgbColor = HLStoRGB(m_nHue, m_nLum, m_nSat);
	m_nNewRed = GetRValue(rgbColor);
	m_nNewGreen = GetGValue(rgbColor);
	m_nNewBlue = GetBValue(rgbColor);
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// redraw the new color box
	DrawNewColorBox(&dc);
	// redraw the new arrow
	DrawBarArrow(&dc, m_nLum);

	// update the dialog boxes' data
	UpdateData(FALSE);
}

void CSelColorDlg::OnEnChangeColor()
{
	// create an instance of the drawing context
	CClientDC dc(this);

	// update the dialog boxes' data
	UpdateData();

	// draw the current position
	DrawColorSpectrum(&dc);

	RGBtoHLS(RGB(m_nNewRed, m_nNewGreen, m_nNewBlue),
		&m_nHue, &m_nLum, &m_nSat);
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	m_ptPos.x = (long)((m_nHue / 239.0f) * 190.0f);
	m_ptPos.y = (long)((m_nSat / 240.0f) * 190.0f);

	// drag the current position around
	m_ptPos.y = (long)(190.0f - ((m_nSat / 240.0f) * 190.0f));
	if(m_ptPos.y < 0) m_ptPos.y = 0;
	if(m_ptPos.y > 190) m_ptPos.y = 190;

	// regenerate the color bar
	GenColorBar();
	// draw the current position
	DrawCross(&dc, &m_ptPos);
	// redraw the colour bar
	DrawColorBar(&dc);
	// redraw the new color box
	DrawNewColorBox(&dc);
	// redraw the new arrow
	DrawBarArrow(&dc, m_nLum);

	// update the dialog boxes' data
	UpdateData(FALSE);
}

void CSelColorDlg::OnEnChangeRef()
{
	// update the dialog boxes data
	UpdateData();
	// update the reference code
	sscanf((LPCTSTR)m_strRef, "%02X%02X%02X", &m_nNewRed, &m_nNewGreen, &m_nNewBlue);
	// select this color
	SelectColor(m_nNewRed, m_nNewGreen, m_nNewBlue);
}

void CSelColorDlg::SelectColor(BYTE nRed, BYTE nGreen, BYTE nBlue)
{
	// create an instance of the drawing context
	CClientDC dc(this);

	// draw the current position
	DrawColorSpectrum(&dc);

	// restore the old color
	m_nNewRed = nRed;
	m_nNewGreen = nGreen;
	m_nNewBlue = nBlue;
	RGBtoHLS(RGB(nRed, nGreen, nBlue), &m_nHue, &m_nLum, &m_nSat);
	m_ptPos.x = (long)((m_nHue / 239.0f) * 190.0f);
	m_ptPos.y = (long)(190.0f - ((m_nSat / 240.0f) * 190.0f));
	if(m_ptPos.x < 0) m_ptPos.x = 0;
	if(m_ptPos.x > 190) m_ptPos.x = 190;
	if(m_ptPos.y < 0) m_ptPos.y = 0;
	if(m_ptPos.y > 190) m_ptPos.y = 190;
	m_strRef.Format("%02X%02X%02X", m_nNewRed, m_nNewGreen, m_nNewBlue);

	// regenerate the color bar
	GenColorBar();
	// redraw the color bars' arrow
	DrawBarArrow(&dc, m_nLum);
	// draw the current position
	DrawCross(&dc, &m_ptPos);
	// redraw the colour bar
	DrawColorBar(&dc);
	// redraw the new color box
	DrawNewColorBox(&dc);

	// update the dialogs data
	UpdateData(FALSE);
}

char CSelColorDlg::GetCustomBox(LPPOINT ptPos)
{
	// find out which custom color the mouse is over
	for(int nColor = 0; nColor < SetColorDlg_Custom.GetCount(); nColor++)
	{
		// if the mouse is over this item then...
		int nLeft = SetColorDlg_Custom.GetLeft(nColor);
		int nTop = SetColorDlg_Custom.GetTop(nColor);
		// if the point is over this image box then return this
		// color box index number
		if(ptPos->x >= nLeft && ptPos->x <= nLeft + 15 &&
			ptPos->y >= nTop && ptPos->y <= nTop + 17)
			return nColor;
	}
	return -1;
}

COLORREF CSelColorDlg::GetCustomColor(char nPos)
{
	// return the custom color specified
	return SetColorDlg_Custom.GetColor(nPos);
}

void CSelColorDlg::SetCustomColor(char nPos, COLORREF rgbColor)
{
	// modify the custom color specified
	SetColorDlg_Custom.SetColor(nPos, rgbColor);
}

void CSelColorDlg::OnBnClickedAddcolour()
{
	// create an instance of the client dc object
	CClientDC dc(this);

	// previous color reference
	int nPrevColor = 0;

	// search through all colour boxes backwards
	for(int nCustomColor = SetColorDlg_Custom.GetCount() - 1;
		nCustomColor >= 0; nCustomColor--)
	{
		// if this colour is non-white then store this colour box index
		if(SetColorDlg_Custom.GetColor(nCustomColor) == 0xFFFFFF)
			nPrevColor = nCustomColor;
		else	// otherwise skip the rest of this for loop
			break;
	}

	// if required fix the colour value
	if(nPrevColor >= SetColorDlg_Custom.GetCount()) nPrevColor = 0;
	// switch to use this number if required
	if(nPrevColor) m_nInsertPos = nPrevColor;

	// store the current colour at the next location
	SetColorDlg_Custom.SetColor(m_nInsertPos,
		RGB(m_nNewRed, m_nNewGreen, m_nNewBlue));

	// increment the insert at position value
	m_nInsertPos++;
	// if required fix the colour position value
	if(m_nInsertPos >= SetColorDlg_Custom.GetCount()) m_nInsertPos = 0;

	// redraw this colour box
	DrawCustomColors(&dc);
}
