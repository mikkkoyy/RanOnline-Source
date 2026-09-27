#pragma once

#include <vector>

typedef std::vector<CString> LOADING_TIP_VEC;

namespace	NLOADINGTIP
{
	extern LOADING_TIP_VEC		n_vecTip;

	int GetTipSize();
	void InsertTip( CString strText );
	CString GetTip( int nIndex );
	void Clear();
};

namespace	NLOADINGTHREAD
{
	extern	LPDIRECT3DDEVICEQ*	n_ppd3dDevice;
	extern	HWND				n_hWnd;
	extern	BOOL				n_bWait;
	extern	BOOL				n_bRender;
	extern	char				n_szAppPath[MAX_PATH];
	extern	int					n_ExitState;
	extern	CString				n_strTextureName;
	extern	CString				n_strMapName;
	extern	BOOL				n_bTIP;

	enum
	{
		eNOSTART,
		eNORMAL,
		eERROR
	};

	HRESULT	StartThreadLOAD (	LPDIRECT3DDEVICEQ* ppd3dDevice, 
								HWND hWnd, 
								char* szAppPath, 
								const CString & strTextureName,
								const CString & strMapName,
								BOOL bTIP );
	void	WaitThread ();
	void	EndThread ();

	BOOL	GetWait ();
	void	DeWait ();
	void	EnWait ();

	void	SetStep ( int step );
	int		GetStep ();

	int		ErrorLoadingTexture();
	int		ErrorCreateVB();
};