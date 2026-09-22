#ifndef GLITEM_SUIT_H_
#define GLITEM_SUIT_H_

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


namespace ITEM
{
	GLITEM_ATT assign_att ( glold_102::GLITEM_ATT_102 emOld );
	GLITEM_ATT assign_att ( glold_103::GLITEM_ATT_103 emOld );
	GLITEM_ATT assign_att ( glold_108::GLITEM_ATT_108 emOld );

	struct SADDON
	{
		EMITEM_ADDON	emTYPE;		//	적용 타입.
		int				nVALUE;		//	적용 값.

		SADDON ()
			: emTYPE(EMADD_NONE)
			, nVALUE(0)
		{

		}
	};

	struct SVAR
	{
		EMITEM_VAR		emTYPE;		//	적용 타입.
		float			fVariate;	//	적용 값.

		SVAR ()
			: emTYPE(EMVAR_NONE)
			, fVariate(0.0f)
		{
		}
	};

	struct SVOL
	{
		EMITEM_VAR	emTYPE;		//	적용 타입.
		float		fVolume;	//	적용 값.

		SVOL ()
			: emTYPE(EMVAR_NONE)
			, fVolume(0)
		{
		}
	};

	struct SSATE_BLOW_100
	{
		EMSTATE_BLOW	emTYPE;		//	적용 타입.
		float			fRATE;		//	발생 확율.
		float			fLIFE;		//	지속 시간.
		float			fVALUE;		//	적용 값.
	};

	struct SSATE_BLOW
	{
		EMSTATE_BLOW	emTYPE;		//	적용 타입.
		float			fRATE;		//	발생 확율.
		float			fLIFE;		//	지속 시간.
		float			fVAR1;		//	적용 값 1.
		float			fVAR2;		//	적용 값 2.

		SSATE_BLOW ()
			: emTYPE(EMBLOW_NONE)
			, fRATE(0.0f)
			, fLIFE(0.0f)
			, fVAR1(0.0f)
			, fVAR2(0.0f)
		{
		}
	};

	struct SSUIT_100
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		BOOL		bBothHand;					//	양손 모두 장착가능.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_102::GLITEM_ATT_102	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		WORD		wReModelNum;				//	개조 가능 횟수.		( '0' 개조 불가 )

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW_100	sBLOW;					//	상태 이상 효과.
	};

	struct SSUIT_101
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		BOOL		bBothHand;					//	양손 모두 장착가능.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_102::GLITEM_ATT_102	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		WORD		wReModelNum;				//	개조 가능 횟수.		( '0' 개조 불가 )

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	struct SSUIT_102
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		bool		bBothHand;					//	양손 모두 장착가능.
		bool		bBig;						//	대형.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_102::GLITEM_ATT_102	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		WORD		wReModelNum;				//	개조 가능 횟수.		( '0' 개조 불가 )

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	struct SSUIT_103
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		bool		bBothHand;					//	양손 모두 장착가능.
		bool		bBig;						//	대형.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_103::GLITEM_ATT_103	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		WORD		wReModelNum;				//	개조 가능 횟수.		( '0' 개조 불가 )

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	struct SSUIT_104
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;
		bool		bBothHand;
		bool		bBig;
		EMITEM_HAND	emHand;

		glold_108::GLITEM_ATT_108	emAttack;
		WORD		wAttRange;
		WORD		wReqSP;

		short		nHitRate;
		short		nAvoidRate;

		GLPADATA	gdDamage;
		short		nDefense;

		SRESIST_101	sResist;

		WORD		wReModelNum;

		SADDON		sADDON[ADDON_SIZE];
		SVAR		sVARIATE[VARIATION_SIZE];
		SSATE_BLOW	sBLOW;
	};

	struct SSUIT_105
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;
		bool		bBothHand;
		bool		bBig;
		EMITEM_HAND	emHand;

		glold_108::GLITEM_ATT_108	emAttack;
		WORD		wAttRange;
		WORD		wReqSP;

		short		nHitRate;
		short		nAvoidRate;

		GLPADATA	gdDamage;
		short		nDefense;

		SRESIST		sResist;

		WORD		wReModelNum;				//	개조 가능 횟수.		( '0' 개조 불가 )

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	struct SSUIT_106
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_108::GLITEM_ATT_108	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		//	개조 가능 횟수.		( '0' 개조 불가 ),
		//	헤어, 얼굴 아이템에서는 디자인 번호.
		WORD		wReModelNum;

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	enum	EMHAND
	{
		EMHAND_BOTHHAND		= 0x0001,
		EMHAND_BIG			= 0x0002,
		EMHAND_BROOM		= 0x0004,
	};

	struct SSUIT_107
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.

		glold_108::GLITEM_ATT_108	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.

		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)

		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )

		SRESIST		sResist;					//	저항값.

		//	개조 가능 횟수.		( '0' 개조 불가 ),
		//	헤어, 얼굴 아이템에서는 디자인 번호.
		WORD		wReModelNum;

		//	특수 부가 효과들.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SVOL		sVOLUME;					//  변화량 효과. By 경대
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
	};

	struct SSUIT_108
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.
		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )
		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)
		SRESIST		sResist;					//	저항값.
		glold_108::GLITEM_ATT_108	emAttack;	//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SVOL		sVOLUME;					//  변화량 효과. By 경대
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		WORD		wReModelNum;
	};

	struct SSUIT_109
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.
		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )
		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)
		SRESIST		sResist;					//	저항값.
		GLITEM_ATT	emAttack;					//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SVOL		sVOLUME;					//  변화량 효과. By 경대
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		WORD		wReModelNum;
	};

	struct SSUIT_110
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.
		GLPADATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )
		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)
		SRESIST		sResist;					//	저항값.
		GLITEM_ATT	emAttack;					//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SVOL		sVOLUME;					//  변화량 효과. By 경대
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		WORD		wReModelNum;
	};

	//	착용 아이템, 관련 데이터.
	struct SSUIT
	{
		enum 
		{ 
			ADDON_SIZE = 5, 
			VARIATION_SIZE = 5,
			VOLUME_SIZE = 4,
			VERSION = 0x0112 
		};

		EMSUIT		emSuit;						//	아이템 착용 타입.
		DWORD		dwHAND;						//	손 도구 속성.
		EMITEM_HAND	emHand;						//	장착 손 위치.
		GLDWDATA	gdDamage;					//	대미지. ( +/- )
		short		nDefense;					//	방어. ( +/- )
		short		nHitRate;					//	명중율. ( +/- ) (%)
		short		nAvoidRate;					//	회피율. ( +/- ) (%)
		SRESIST		sResist;					//	저항값.
		GLITEM_ATT	emAttack;					//	공격속성.
		WORD		wAttRange;					//	공격 가능 거리.
		WORD		wReqSP;						//	공격시 필요한 SP.
		SVAR		sVARIATE[VARIATION_SIZE];					//	변화율 효과.
		SVOL		sVOLUME;					//  변화량 효과. By 경대
		SSATE_BLOW	sBLOW;						//	상태 이상 효과.
		SADDON		sADDON[ADDON_SIZE];			//	가산 효과.
		WORD		wReModelNum;

		DWORD		dummy_var_0;
		WORD		dummy_var_1;
		DWORD		dummy_var_2;

		SSUIT ()
			: emSuit(SUIT_HEADGEAR)
			, dwHAND(NULL)
			, emHand(HAND_RIGHT)
			, gdDamage(0,0)
			, nDefense(0)
			, nHitRate(0)
			, nAvoidRate(0)
			, emAttack(ITEMATT_NOTHING)
			, wAttRange(4)
			, wReqSP(0)
			, wReModelNum(0)

			, dummy_var_0(0)
			, dummy_var_1(0)
			, dummy_var_2(0)
		{
		}

		void Assign ( SSUIT_100 &Suit100 );
		void Assign ( SSUIT_101 &Suit101 );
		void Assign ( SSUIT_102 &Suit102 );
		void Assign ( SSUIT_103 &Suit103 );
		void Assign ( SSUIT_104 &Suit104 );
		void Assign ( SSUIT_105 &Suit105 );
		void Assign ( SSUIT_106 &Suit106 );
		void Assign ( SSUIT_107 &Suit107 );
		void Assign ( SSUIT_108 &Suit108 );
		void Assign ( SSUIT_109 &Suit109 );
		void Assign ( SSUIT_110 &Suit109 );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );

		void SETBOTHHAND ( bool bBOTH )		{ (bBOTH) ? (dwHAND|=EMHAND_BOTHHAND) : (dwHAND&=~EMHAND_BOTHHAND); }
		void SETBIG ( bool bBIG )			{ (bBIG) ? (dwHAND|=EMHAND_BIG) : (dwHAND&=~EMHAND_BIG); }
		void SETBROOM ( bool bBROOM )		{ (bBROOM) ? (dwHAND|=EMHAND_BROOM) : (dwHAND&=~EMHAND_BROOM); }

		bool IsBOTHHAND () const			{ return NULL!=(dwHAND&EMHAND_BOTHHAND); }
		bool IsBIG () const					{ return NULL!=(dwHAND&EMHAND_BIG); }
		bool IsBROOM () const				{ return NULL!=(dwHAND&EMHAND_BROOM); }
	};
};

namespace COMMENT
{
	extern std::string ADDON_NO[ITEM::SSUIT::ADDON_SIZE];
	extern std::string SVARON_NO[ITEM::SSUIT::VARIATION_SIZE];
};

#endif // GLITEM_SUIT_H_