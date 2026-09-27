#ifndef _GMTOOL_ODBC_HANDLE_ENVIRONMENT_H_
#define _GMTOOL_ODBC_HANDLE_ENVIRONMENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment (lib,"odbc32.lib")

#include <sqlext.h> 
#include <odbcinst.h>
#include "s_DbBase.h"

class CCGMToolOdbcHenv
{
public:
	static CCGMToolOdbcHenv* GetInstance();
	static void ReleaseInstance();	
	
	SQLHENV m_hEnv;

	int	 CreateHandle();
	void CloseHandle();
	SQLHENV GetHandle();

private:
	CCGMToolOdbcHenv(void);
	~CCGMToolOdbcHenv(void);

	static CCGMToolOdbcHenv* SelfInstance;
};

#endif // _GMTOOL_ODBC_HANDLE_ENVIRONMENT_H_