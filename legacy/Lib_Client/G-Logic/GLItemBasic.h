#ifndef GLITEM_BASIC_H_
#define GLITEM_BASIC_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "./GLItemDef.h"
#include "./GLCharDefine.h"
#include "./GLActivityDefine.h"

namespace ITEM
{
	struct SITEMBASIC_100
	{
		SNATIVEID	sNativeID;					//	고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )

		char		szName[ITEM_SZNAME];		//	아이템의 이름.	
		EMITEMLEVEL	emLevel;					//	아이템 가치 등급.

		DWORD		dwFlags;					//	Trade 속성.
		DWORD		dwPrice;					//	아이템 가격.

		EMEXITEM_TYPE	emItemType;					//	아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		char		szFieldFile[MAX_PATH];				//	바닥 형상 파일.
		char		szInventoryFile[MAX_PATH];			//	인벤토리 형상 파일.
		char		szWearingFIle[GLCI_NUM_6CLASS][MAX_PATH];	//	형상 파일.
		char		szComment[ITEM_SZCOMMENT];			//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_101
	{
		SNATIVEID	sNativeID;					//	고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )

		char		szName[ITEM_SZNAME];		//	아이템의 이름.	
		EMITEMLEVEL	emLevel;					//	아이템 가치 등급.

		DWORD		dwFlags;					//	Trade 속성.
		DWORD		dwPrice;					//	아이템 가격.

		EMEXITEM_TYPE	emItemType;					//	아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		char		szSelfBodyEffect[MAX_PATH];	//	자기 자신의 몸에 붓는 이팩트.
		char		szTargBodyEffect[MAX_PATH];	//	목표 유닛의 몸에 붓는 이팩트.
		char		szTargetEffect[MAX_PATH];	//	목표 지향 이팩트.

		char		szFieldFile[MAX_PATH];				//	바닥 형상 파일.
		char		szInventoryFile[MAX_PATH];			//	인벤토리 형상 파일.
		char		szWearingFIle[GLCI_NUM_6CLASS][MAX_PATH];	//	형상 파일.
		char		szComment[ITEM_SZCOMMENT];			//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_102
	{
		SNATIVEID	sNativeID;					//	고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )

		char		szName[ITEM_SZNAME];		//	아이템의 이름.	
		EMITEMLEVEL	emLevel;					//	아이템 가치 등급.

		DWORD		dwFlags;					//	Trade 속성.
		DWORD		dwPrice;					//	아이템 가격.

		EMEXITEM_TYPE	emItemType;					//	아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		char		szSelfBodyEffect[MAX_PATH];	//	자기 자신의 몸에 붓는 이팩트.
		char		szTargBodyEffect[MAX_PATH];	//	목표 유닛의 몸에 붓는 이팩트.
		char		szTargetEffect[MAX_PATH];	//	목표 지향 이팩트.

		char		szFieldFile[MAX_PATH];				//	바닥 형상 파일.
		char		szInventoryFile[MAX_PATH];			//	인벤토리 형상 파일.
		char		szWearingFIle[GLCI_NUM_6CLASS][MAX_PATH];	//	형상 파일.
		char		szComment[ITEM_SZCOMMENT];			//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_104
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_105
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_106
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.
	};

	struct SITEMBASIC_107
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 
	};

	struct SITEMBASIC_109
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 
	};

	struct SITEMBASIC_110
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevel;					//	요구 Level.

		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 
	};

	struct SITEMBASIC_111
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwPrice;					// 아이템 가격.

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevelDW;				//	요구 Level 하한선
		WORD		wReqLevelUP;				//  요구 Level 상한선
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 
	};

	struct SITEMBASIC_112
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwBuyPrice;					// 아이템 구입 가격.
		DWORD		dwSellPrice;				// 아이템 판매 가격

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevelDW;				//	요구 Level 하한선
		WORD		wReqLevelUP;				//  요구 Level 상한선
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 

		SNATIVEID		sSubID;						// Map mid/sid
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
	};


	struct SITEMBASIC_113
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwBuyPrice;					// 아이템 구입 가격.
		DWORD		dwSellPrice;				// 아이템 판매 가격

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevelDW;				//	요구 Level 하한선
		WORD		wReqLevelUP;				//  요구 Level 상한선
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 

		SNATIVEID		sSubID;						// Map mid/sid
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
	};

	struct SITEMBASIC_114
	{
		SNATIVEID	sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID	sGroupID;					// ???

		std::string	strName;					// 아이템의 이름.	
		EMITEMLEVEL	emLevel;					// 아이템 가치 등급.

		WORD		wGradeAttack;				// 공격 개조 등급
		WORD		wGradeDefense;				// 방어 개조 등급

		float		fExpMultiple;				// 경험치 개조 등급

		WORD		wReserved1;					// 사용 안함
		WORD		wReserved2;					// 사용 안함
		WORD		wReserved3;					// 사용 안함
		WORD		wReserved4;					// 사용 안함
		WORD		wReserved5;					// 사용 안함

		DWORD		dwFlags;				    // Trade 속성.
		DWORD		dwBuyPrice;					// 아이템 구입 가격.
		DWORD		dwSellPrice;				// 아이템 판매 가격

		EMEXITEM_TYPE	emItemType;					// 아이템 종류.

		//	착용 조건.
		EMBRIGHT	emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD		dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD		dwReqSchool;				//	착용할 수 있는 학원. 
		WORD		wReqLevelDW;				//	요구 Level 하한선
		WORD		wReqLevelUP;				//  요구 Level 상한선
		WORD		wReqPA;						//	요구 격투치.
		WORD		wReqSA;						//	요구 사격치.
		SCHARSTATS	sReqStats;					//	요구 케릭터 Stats 수치.

		WORD		wInvenSizeX;				//	인벤토리 사이즈.
		WORD		wInvenSizeY;				//	인벤토리 사이즈.

		SNATIVEID	sICONID;					//	아이콘 인덱스.

		std::string		strSelfBodyEffect;		//	자기 자신의 몸에 붓는 이팩트.
		std::string		strTargBodyEffect;		//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;		//	목표 지향 이팩트.

		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strComment;					//	아이템에 대한 간단한 설명.

		// PET
		std::string		strPetWearingFile;			//  팻의 형상파일. 

		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표

		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
	};


	struct SITEMBASIC_115
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFile[GLCI_NUM_5CLASS_EX];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
	};

	struct SITEMBASIC_116
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFile[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
	};

	struct SITEMBASIC_117
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strWearingFileLeft[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
	};

	struct SITEMBASIC_118
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_6CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data
	};

	struct SITEMBASIC_119
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_6CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_6CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data
	};

	struct SITEMBASIC_121
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMEXITEM_TYPE	emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwActivityReqType;	
		DWORD			dwReqActivityPoint;

		 /*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint;
	};

	struct SITEMBASIC_200
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwActivityReqType;	
		DWORD			dwReqActivityPoint;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint;
	};

	struct SITEMBASIC_201
	{

		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwActivityReqType;	
		DWORD			dwReqActivityPoint;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint;

		/*UserNum ItemReq, Juver, 2017/06/27 */
		DWORD			dwReqUserNum;
	};

	struct SITEMBASIC_202
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwReqActivityPoint;;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint; 

		/*UserNum ItemReq, Juver, 2017/06/27 */
		DWORD			dwReqUserNum;
	};

	struct SITEMBASIC_203
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwReqActivityPoint;;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint; 

		/*UserNum ItemReq, Juver, 2017/06/27 */
		DWORD			dwReqUserNum;

		/*item color, Juver, 2018/01/05 */
		bool			bItemColor;
		WORD			wItemColor1;
		WORD			wItemColor2;
	};

	struct SITEMBASIC_204
	{
		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwReqActivityPoint;;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint; 

		/*UserNum ItemReq, Juver, 2017/06/27 */
		DWORD			dwReqUserNum;

		/*item color, Juver, 2018/01/05 */
		bool			bItemColor;
		WORD			wItemColor1;
		WORD			wItemColor2;

		/*item wrapper, Juver, 2018/01/11 */
		SNATIVEID		sidWrapperBox;
	};

	struct SITEMBASIC
	{
		enum { VERSION = 0x0205 };

		SNATIVEID		sNativeID;					// 고유 ID. ( 아이템 설정 최초 생성시에 부여되는 고유 ID )
		SNATIVEID		sGroupID;					// ???
		EMITEMLEVEL		emLevel;					// 아이템 가치 등급.
		EMITEM_TYPE		emItemType;					// 아이템 종류.
		std::string		strName;					// 아이템의 이름.	
		float			fExpMultiple;				// 경험치 개조 등급
		WORD			wGradeAttack;				// 공격 개조 등급
		WORD			wGradeDefense;				// 방어 개조 등급
		DWORD			dwFlags;				    // Trade 속성.
		DWORD			dwBuyPrice;					// 아이템 구입 가격.
		DWORD			dwSellPrice;				// 아이템 판매 가격
		WORD			wReserved1;					// 사용 안함
		WORD			wReserved2;					// 사용 안함
		WORD			wReserved3;					// 사용 안함
		WORD			wReserved4;					// 사용 안함
		WORD			wReserved5;					// 사용 안함
		DWORD			dwReqCharClass;				//	착용할 수 있는 직업들. ( EMCHARCLASS Flags )
		DWORD			dwReqSchool;				//	착용할 수 있는 학원. 
		SCHARSTATS		sReqStats;					//	요구 케릭터 Stats 수치.
		WORD			wReqLevelDW;				//	요구 Level 하한선
		WORD			wReqLevelUP;				//  요구 Level 상한선
		WORD			wReqPA;						//	요구 격투치.
		WORD			wReqSA;						//	요구 사격치.
		EMBRIGHT		emReqBright;				//	캐릭 속성. ( 광/암 )
		DWORD			dwCoolTime;					// 아이템 사용 쿨타임
		EMCOOL_TYPE		emCoolType;					// 쿨타임 타입
		WORD			wInvenSizeX;				//	인벤토리 사이즈.
		WORD			wInvenSizeY;				//	인벤토리 사이즈.
		SNATIVEID		sICONID;					//	아이콘 인덱스.
		std::string		strFieldFile;				//	바닥 형상 파일.
		std::string		strInventoryFile;			//	인벤토리 형상 파일.
		std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
		std::string		strTargetEffect;			//	목표 지향 이팩트.
		std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
		std::string		strWearingFileRight[GLCI_NUM_8CLASS];	//	형상 파일. //pending dual piece
		std::string		strWearingFileLeft[GLCI_NUM_8CLASS];	//	형상 파일.
		std::string		strPetWearingFile;			//  팻의 형상파일. 
		std::string		strComment;					//	아이템에 대한 간단한 설명.
		SNATIVEID		sSubID;						// mid/sid ( 각종 MID/SID ) 
		WORD			wPosX;						// X좌표
		WORD			wPosY;						// Y좌표
		bool			bEnable;					//official unknown data

		/*activity point, Juver, 2017/08/23 */
		DWORD			dwReqActivityPoint;;

		/*contribution point, Juver, 2017/08/23 */
		DWORD			dwReqContributionPoint; 

		/*UserNum ItemReq, Juver, 2017/06/27 */
		DWORD			dwReqUserNum;

		/*item color, Juver, 2018/01/05 */
		bool			bItemColor;
		WORD			wItemColor1;
		WORD			wItemColor2;

		/*item wrapper, Juver, 2018/01/11 */
		SNATIVEID		sidWrapperBox;

		/*item transfer card, Juver, 2018/01/18 */
		bool			bItemTransfer;

		SITEMBASIC () 
			: sNativeID(false)
			, sGroupID(false)
			, emLevel(LEVEL_NORMAL)
			, emItemType(ITEM_SUIT)
			, fExpMultiple(1.0f)
			, wGradeAttack(0)
			, wGradeDefense(0)
			, dwFlags(TRADE_ALL)
			, dwBuyPrice(1)
			, dwSellPrice(1)
			, wReserved1(0)
			, wReserved2(0)
			, wReserved3(0)
			, wReserved4(0)
			, wReserved5(0)
			, dwReqCharClass(GLCC_NONE)
			, dwReqSchool(GLSCHOOL_ALL)
			, wReqLevelDW(0)
			, wReqLevelUP(0)
			, wReqPA(0)
			, wReqSA(0)
			, emReqBright(BRIGHT_BOTH)
			, dwCoolTime( 0 )
			, emCoolType( EMCOOL_ITEMID )
			, wInvenSizeX(1)
			, wInvenSizeY(1)
			, sICONID(0,0)
			, sSubID( false )
			, wPosX( 0 )
			, wPosY( 0 )
			, bEnable(true)
			, dwReqActivityPoint(0)		/*activity point, Juver, 2017/08/23 */
			, dwReqContributionPoint(0) /*contribution point, Juver, 2017/08/23 */
			, dwReqUserNum(0)			/*UserNum ItemReq, Juver, 2017/06/27 */

			/*item color, Juver, 2018/01/05 */
			, bItemColor(false)
			, wItemColor1(ITEMCOLOR_WHITE)
			, wItemColor2(ITEMCOLOR_WHITE)

			/*item wrapper, Juver, 2018/01/11 */
			, sidWrapperBox(false)

			/*item transfer card, Juver, 2018/01/18 */
			, bItemTransfer(false)
		{
		}

		void Assign ( SITEMBASIC_100 &sOld );
		void Assign ( SITEMBASIC_101 &sOld );
		void Assign ( SITEMBASIC_102 &sOld );

		bool LOAD_103 ( basestream &SFile );
		bool LOAD_104 ( basestream &SFIle );
		bool LOAD_105 ( basestream &SFile );
		bool LOAD_106 ( basestream &SFile );
		bool LOAD_107 ( basestream &SFile );
		bool LOAD_108 ( basestream &SFile );
		bool LOAD_109 ( basestream &SFile );
		bool LOAD_110 ( basestream &SFile );
		bool LOAD_111 ( basestream &SFile );
		bool LOAD_112 ( basestream &SFile );
		bool LOAD_113 ( basestream &SFile );
		bool LOAD_114 ( basestream &SFile );
		bool LOAD_115 ( basestream &SFile );
		bool LOAD_116 ( basestream &SFile );
		bool LOAD_117 ( basestream &SFile );
		bool LOAD_118 ( basestream &SFile );
		bool LOAD_119 ( basestream &SFile );
		bool LOAD_121 ( basestream &SFile );
		bool LOAD_122 ( basestream &SFile );
		bool LOAD_200 ( basestream &SFile );
		bool LOAD_201 ( basestream &SFile );
		bool LOAD_202 ( basestream &SFile );
		bool LOAD_203 ( basestream &SFile );
		bool LOAD_204 ( basestream &SFile );

		bool LOAD ( basestream &SFile );
		bool SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur  );

		SITEMBASIC& operator = ( const SITEMBASIC& rvalue );

		bool IsSALE ()			{ return (dwFlags&TRADE_SALE)!=NULL; }
		bool IsEXCHANGE ()		{ return (dwFlags&TRADE_EXCHANGE)!=NULL; }
		bool IsTHROW ()			{ return (dwFlags&TRADE_THROW)!=NULL; }
		bool IsEVENT ()			{ return (dwFlags&TRADE_EVENT_SGL)!=NULL; }
		bool IsDISGUISE ()		{ return (dwFlags&ITEM_DISGUISE)!=NULL; }
		bool IsCHANNEL_ALL ()	{ return (dwFlags&ITEM_CHANNEL_ALL)!=NULL; }
		bool IsGarbage()		{ return (dwFlags&TRADE_GARBAGE)!=NULL; }
		bool isHaveSkill()		{ return (dwFlags&ITEM_FLAG_EX_5)!=NULL; }
		bool IsCoolTime()		{ return ( dwCoolTime != 0 ); }
	};
};

#endif // GLITEM_BASIC_H_