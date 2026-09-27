#include "stdafx.h"
#include "HLibColorValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace _HLIB
{
	DWORD color_dword_word( WORD _wColor )
	{
		WORD wR, wG, wB;
		wR = _wColor >> 10;
		wG = _wColor >> 5;
		wG = wG&0x1f;
		wB = _wColor&0x1f;

		wR = (WORD)( wR * 8.225806f );
		wG = (WORD)( wG * 8.225806f );
		wB = (WORD)( wB * 8.225806f );

		DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;

		return dwColor;
	};

	WORD color_word_dword( DWORD _dwColor )
	{
		WORD wR, wG, wB;
		wR = (WORD)((_dwColor&0xff0000)>>16);
		wG = (WORD)((_dwColor&0xff00)>>8);
		wB = (WORD)(_dwColor&0xff);

		wR = wR >> 3;
		wG = wG >> 3;
		wB = wB >> 3;

		return (wR<<10) + (wG<<5) + wB;
	};

	DWORD color_argb_int( int _nColorA, int _nColorR, int _nColorG, int _nColorB )
	{
		DWORD dwColor = (_nColorA << 24) | (_nColorR << 16) | (_nColorG << 8) | (_nColorB); 
		return dwColor;
	};

	DWORD color_argb_float( float _fColorA, float _fColorR, float _fColorG, float _fColorB )
	{
		int nA = int(_fColorA*255.0f);
		int nR = int(_fColorR*255.0f);
		int nG = int(_fColorG*255.0f);
		int nB = int(_fColorB*255.0f);

		return color_argb_int( nA, nR, nG, nB );
	};

	DWORD color_argb_rgb( COLORREF _color )
	{
		DWORD dwColor = (255 << 24) | (GetRValue(_color) << 16) | (GetGValue(_color) << 8) | (GetBValue(_color)); 
		return dwColor;
	};

	int color_get_int_a( DWORD _dwColor )
	{
		int nColor =  ((_dwColor) >> 24);
		return nColor;
	};

	int color_get_int_r( DWORD _dwColor )
	{
		int nColor = (((_dwColor) >> 16) & 0xff);
		return nColor;
	};

	int color_get_int_g( DWORD _dwColor )
	{
		int nColor =   (((_dwColor) >> 8) & 0xff);
		return nColor;
	};

	int color_get_int_b( DWORD _dwColor )
	{
		int nColor =  ((_dwColor) & 0xff);
		return nColor;
	};

	float color_get_float_a( DWORD _dwColor )
	{
		return float( color_get_int_a(_dwColor) / 255.0f );
	};

	float color_get_float_r( DWORD _dwColor )
	{
		return float( color_get_int_r(_dwColor) / 255.0f );
	};

	float color_get_float_g( DWORD _dwColor )
	{
		return float( color_get_int_g(_dwColor) / 255.0f );
	};

	float color_get_float_b( DWORD _dwColor )
	{
		return float( color_get_int_b(_dwColor) / 255.0f );
	};
};