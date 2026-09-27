/*!
 * \file HLibColorValue.h
 *
 * \author Juver
 * \date April 2017
 *
 * 
 */

#pragma once

namespace _HLIB
{	
	DWORD color_dword_word( WORD _wColor );
	WORD color_word_dword( DWORD _dwColor );
	DWORD color_argb_int( int _nColorA, int _nColorR, int _nColorG, int _nColorB );
	DWORD color_argb_float( float _fColorA, float _fColorR, float _fColorG, float _fColorB );
	DWORD color_argb_rgb( COLORREF _color );
	int color_get_int_a( DWORD _dwColor );
	int color_get_int_r( DWORD _dwColor );
	int color_get_int_g( DWORD _dwColor );
	int color_get_int_b( DWORD _dwColor );
	float color_get_float_a( DWORD _dwColor );
	float color_get_float_r( DWORD _dwColor );
	float color_get_float_g( DWORD _dwColor );
	float color_get_float_b( DWORD _dwColor );
	
};