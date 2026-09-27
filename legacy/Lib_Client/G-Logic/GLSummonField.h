#pragma once

#include "./GLSummon.h"
#include "./GLogicEx.h"
#include "./GLARoundSlot.h"
#include "./GLChar.h"

#include "../../Lib_Engine/G-Logic/GLList.h"


class GLSummonField : public GLSUMMON, public GLACTOR
{
private:
	typedef std::vector<STARGETID>	VEC_SK_TAR;
	typedef VEC_SK_TAR::iterator	VEC_SK_ITER;

	//	데미지 큐
	struct SSTRIKE
	{
		WORD wDivKey;
		bool bHit;

		SSTRIKE () : wDivKey(0), bHit(false) { }
		SSTRIKE ( WORD _wDivKey, bool _bHit ) : wDivKey(_wDivKey), bHit(_bHit) { }
	};
	typedef std::deque<SSTRIKE>	HITLIST;
	typedef HITLIST::iterator	HITLIST_ITER;


public:
	Actor				m_actorMove;

	D3DXVECTOR3			m_vDir;
	D3DXVECTOR3			m_vDirOrig;
	D3DXVECTOR3			m_vPos;
	D3DXMATRIX			m_matTrans;

	PGLCHAR				m_pOwner;

	D3DXVECTOR3			m_vTarPos;

	DWORD				m_dwActionFlag;
	float				m_fTIMER;

	DWORD				m_dwOwnerCharID; // 주인의 상태(OnLine/OffLine) 추적을 위해 사용

	SCROWATTACK*		m_pAttackProp;			//	공격 속성.

protected:
	GLARoundSlot		m_ARoundSlot;			//	피공격시에 자신의 주변에 몰려들수 있는 위치가 있는지 점검.

	float			   m_fIdleTime;

	bool				m_bValid;
	WORD				m_wAniSub;
	STARGETID			m_TargetID;
	float				m_fattTIMER;			//	공격 프래임.
	HITLIST				m_sHITARRAY;

	STARGETID			m_sAssault;				//	가장 최근에 자신을 공격한자.
	//	DAMAGELOG			m_cDamageLog;			//	대미지 로그.

	EMACTIONTYPE		m_Action;				//	행동.


public:
	/*continuous damage skill logic, Juver, 2017/06/10 */
	float				m_fContinuousDamageCheck;

	/*skill summon, Juver, 2017/10/09 */
	float				m_fSummonLife;
	float				m_fSummonDelay;
	float				m_fOwnerCheckTime;

private:
	void	PreStrikeProc ( BOOL bSkill, BOOL bLowSP );

	//	공격 프로세스.
	void	PreAttackProc ();
	BOOL	ShockProcess ( float fElapsedTime );
	BOOL	AttackProcess ( float fElapsedTime );
	void	AvoidProc ();
	void	DamageProc ( const int nDAMAGE, DWORD dwDamageFlag );
	DWORD	ToDamage ( const STARGETID &sTargetID, const int nDamage, const BOOL bShock );

	DWORD	FindOptAType ( const STARGETID &sTargetID, bool branvar );

	EMATT_RGTYPE GetAttackRangeType ()			{ if ( !m_pAttackProp ) return EMATT_SHORT; return m_pAttackProp->emAttRgType; }


public:
	GLLandMan*					m_pLandMan;
	SGLNODE<GLSummonField*>*	m_pLandNode;

	LANDQUADNODE*				m_pQuadNode;			//	쿼드 트리 노드. ( 직접 포인터 형을 지정하기 난감함. )
	SGLNODE<GLSummonField*>*	m_pCellNode;			//	셀 리스트 노드.

public:
	GLSummonField(void);
	~GLSummonField(void);

	HRESULT Create ( GLLandMan* pLandMan, PGLCHAR pOwner, PGLSUMMON pSummonData, D3DXVECTOR3 vtargetpos, SUMMON_DATA_SKILL *psummon_skill );
	HRESULT SetPosition ( GLLandMan* pLandMan );

	HRESULT FrameMove( float fTime, float fElapsedTime );
	HRESULT DeleteDeviceObject ();

	BOOL  IsSTATE ( DWORD dwState ) const		{ return m_dwActionFlag&dwState; }
	void  SetSTATE ( DWORD dwStateFlag );
	void  ReSetSTATE ( DWORD dwStateFlag )				{ m_dwActionFlag &= ~dwStateFlag; }
	void  ReSetAllSTATE ()								{ m_dwActionFlag = 0; }

	void  TurnAction ( EMACTIONTYPE toAction );

	virtual DWORD GETCLIENTID ()					{ return m_pOwner->GETCLIENTID(); }

	void	SetValid ()									{ m_bValid = true; }
	void	ReSetValid ()								{ m_bValid = false; }
	bool	IsValid ()									{ return m_bValid; }

	void	CleanUp ();

	//void	AccountSkill ( SNATIVEID sSkillID );
	void	UpdateSkillState ( float fElapsedTime );

	HRESULT	UpdateTurnAction( float fElapsedTime );

	// Access
	void	SetPosition ( D3DXVECTOR3 vPos ) { m_vPos = vPos; }
	BOOL	IsReActionable ( const STARGETID &sTargetID, BOOL bENEMY = TRUE );

	// Attack
	bool	SetAttackTarget( STARGETID sAttackTargetID );
	bool	GetAttackTarget( STARGETID sTargetID );
	void	ResetAttackTarget();
	STARGETID& GetAttackTar()	{ return m_TargetID; }

	BOOL IsACTION ( EMACTIONTYPE emCur ) const	{ return m_Action == emCur; }
	BOOL IsRunning () const						{ return IsACTION(GLAT_MOVE) && IsSTATE(EM_SUMMONACT_ATTACK); };
	virtual BOOL IsAction ( EMACTIONTYPE emCur ) const	{ return IsACTION(emCur); }

	DWORD SELECT_SKILLSLOT ( SNATIVEID skill_id );

	virtual HRESULT SNDMSGAROUND ( NET_MSG_GENERIC *pnet )		{ return m_pOwner->SendMsgViewAround ( pnet ); }


	virtual EMCROW GetCrow () const			{ return CROW_SUMMON; }
	virtual DWORD GetCtrlID () const		{ return m_dwGUID; }

	virtual GLDWDATA GetHp () const			{ return GLDWDATA(GETHP(),GETMAXHP()); }
	// 소환수는 파티를 가질수 없지만 어쩔수 없이... ( 호출할 일 없음! )
	virtual DWORD GetPartyID () const				{ return 0; }

	virtual BOOL IsDie () const;
	virtual BOOL IsValidBody () const;
	virtual const D3DXVECTOR3& GetPosition () const	{ return m_vPos; }
	virtual WORD GetBodyRadius () const				{ return GETBODYRADIUS(); }

	virtual DWORD GETHOLDBLOW () const				{ return m_dwHOLDBLOW; }
	virtual const SRESIST& GETRESIST () const		{ return m_pSummonCrowData->m_sResist; }
	virtual WORD GetLevel () const					{ return GETLEVEL(); }

	virtual DWORD GetNowHP () const					{ return GETHP(); }
	virtual DWORD GetMaxHP () const					{ return GETMAXHP(); }
	virtual float GetMaxHpRate() const				{ return 0; }
	virtual void  SetMaxHpRate( float fRate )       {   }

	virtual WORD GetNowMP () const					{ return GETMP(); };
	virtual WORD GetMaxMP () const					{ return GETMAXMP(); }

	virtual WORD GetNowSP () const					{ return GETMAXSP(); }
	virtual WORD GetMaxSP () const					{ return GETMAXSP(); }

	virtual int GetDefense () const					{ return GETDEFENSE(); }
	virtual DWORD GetBonusExp () const				{ return DWORD(m_pSummonCrowData->m_dwBonusExp*GLCONST_CHAR::GETEXP_SCALE()); };

	virtual int GetAvoid () const					{ return GETAVOID(); }
	virtual EMBRIGHT GetBright () const				{ return GETBRIGHT(); }



	virtual DWORD ReceiveDamage ( const EMCROW emACrow, const DWORD dwAID, const DWORD dwDamage, const BOOL bShock );

	/*push pull skill logic, Juver, 2017/06/04 */
	/*dash skill logic, Juver, 2017/06/17 */
	virtual void ReceivePushPull ( const D3DXVECTOR3 &vMovePos, float fSpeed, bool bSkillMove, bool bReverseDir, bool bSkillDash, WORD wActionAnim );
	virtual void ReceiveTeleport ( const D3DXVECTOR3 &vMovePos ); //Teleport Skill Effect 

	virtual void STATEBLOW ( const SSTATEBLOW &sStateBlow );
	virtual void CURE_STATEBLOW ( DWORD dwCUREFLAG );
	virtual void VAR_BODY_POINT ( const EMCROW emACrow, const DWORD dwAID, const BOOL bPartySkill, int nvar_hp, int nvar_mp, int nvar_sp );
	virtual BOOL RECEIVE_SKILLFACT ( const SNATIVEID skill_id, const WORD wlevel, DWORD &dwSELECT, WORD _wCasterCrow, DWORD _dwCasterID );

	virtual float GETSTATE_DAMAGE () const			{ return m_fSTATE_DAMAGE; }

	virtual DAMAGE_SPEC GetDamageSpec() const		{ return m_sDamageSpec; }
	virtual DEFENSE_SKILL GetDefenseSkill() const	{ return m_sDefenseSkill; }

	virtual void DamageReflectionProc ( int nDAMAGE, STARGETID sACTOR );
	virtual void DefenseSkill( SNATIVEID sNativeID, WORD wLevel, STARGETID sACTOR ) { return;}

	/*ignore damage, Juver, 2017/12/12 */
	virtual BOOL IsIgnoreDamage () const			{ return (BOOL)m_bIgnoreDamage; }

	/*block back damage, Juver, 2017/12/12 */
	virtual BOOL IsBlockDamageBack () const			{ return (BOOL)m_bBlockBackDamage; }
	virtual void SetBlockDamageBack( bool bSet );
	virtual SINCREASEEFF* const GetIncreaseEff() { return &m_sIncreaseEff; }

	virtual void BUFF_REMOVE( DWORD dwBuffFlag );
	virtual void DO_STUN_ACTION ();


	GLARoundSlot& GetARoundSlot ()			{ return m_ARoundSlot; }
	BOOL IsFreeARSlot ()					{ return m_ARoundSlot.GetAmount()!=GLARoundSlot::SLOT_SIZE; }

	// 소환자보다 소환수가 거리가 더 짧은지 체크한다.
	BOOL IsOwnerShortDistance( D3DXVECTOR3 vCheckPos );

	virtual GLARoundSlot* GETSLOT ()				{ return &m_ARoundSlot; }
	virtual GLLandMan*			GetLandMan() { return m_pLandMan; }
	bool	 		MoveTo( const D3DXVECTOR3* pTarPos, bool bJump, DWORD dwFlags )		{ return S_OK; }


public:
	SDROPSUMMON ReqNetMsg_Drop ();

	void	MsgProcess ( NET_MSG_GENERIC* nmg );
	HRESULT	MsgGoto ( NET_MSG_GENERIC* nmg );
	HRESULT	MsgStop ( NET_MSG_GENERIC* nmg );
	HRESULT	MsgUpdateMoveState ( NET_MSG_GENERIC* nmg );
	HRESULT MsgUpdateState ( NET_MSG_GENERIC* nmg );


protected:
	typedef std::vector<DWORD>		VECPC;
	typedef VECPC::iterator			VECPC_ITER;

	DWORD	m_dwPC_AMOUNT;
	DWORD	m_dwCROW_AMOUNT;
	DWORD	m_dwSUMMON_AMOUNT;
	VECPC	m_vecPC_VIEW;
	VECPC	m_vecCROW_VIEW;
	VECPC	m_vecSUMMON_VIEW;

	WORD				m_wTARNUM;				//	타겟 숫자.
	STARID				m_sTARIDS[EMTARGET_NET];//	타겟들.
	D3DXVECTOR3			m_vTARPOS;				//	타겟 위치.

	HRESULT UpdateViewAround ();

	VEC_SK_TAR DetectTarget ( const D3DXVECTOR3 &vDetectPosA, const D3DXVECTOR3 &vDetectPosB, const DWORD dwMaxFind );
	VEC_SK_TAR DetectTarget ( const D3DXVECTOR3 &vDetectPos, const int nRange, const DWORD dwMaxFind );


	//	자기편에게 스킬 체크 ( 자기 위치에 영향 )
	BOOL SKT_SELFZONEOUR ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//  적들에게 스킬 체크 ( 자기 위치에 영향 )
	BOOL SKT_SELFZONEENERMY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//  모두에게 스킬 체크 ( 자기 위치에 영향 )
	BOOL SKT_SELFZONANYBODY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );

	//	자기편에게 스킬 체크 ( 대상 위치에 영향 )이고 자신 주위
	BOOL SKT_TARSPECOUR_SELF ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	적들에게 스킬 체크 ( 대상 위치에 영향 )이고 자신 주위
	BOOL SKT_TARSPECENERMY_SELF ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	모두에게 스킬 체크 ( 대상 위치에 영향 )이고 자신 주위
	BOOL SKT_TARSPECANYBODY_SELF ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );

	//	자기편에게 스킬 체크 ( 대상 위치에 영향 )이고 목표 주위
	BOOL SKT_TARSPECOUR ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	적들에게 스킬 체크 ( 대상 위치에 영향 )이고 목표 주위
	BOOL SKT_TARSPECENERMY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	모두에게 스킬 체크 ( 대상 위치에 영향 )이고 목표 주위
	BOOL SKT_TARSPECANYBODY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );

	//	'목표'를 향한 부채꼴 모양.
	BOOL SKT_TARSPECFANWIZE ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	// 적에게만 자기 위치에서 대상 위치까지. ( 화살 관통 )
	BOOL SKT_TARSELFTOSPEC ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );

	//	자기편에게 스킬 체크 ( 위치에 영향 )
	BOOL SKT_TARZONEOUR ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	적들에게 스킬 체크 ( 위치에 영향 )
	BOOL SKT_TARZONEENERMY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );
	//	모두에게 스킬 체크 ( 위치에 영향 )
	BOOL SKT_TARZONEANYBODY ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const GLSKILL &sSKILL, const WORD wLEVEL );

	BOOL SelectSkillTarget ( const STARGETID &sTargetID, const D3DXVECTOR3 *pvTarPos, const SNATIVEID &sCheckSkill, const WORD wLevel );

	void	SkillProc ( const BOOL bLowSP=FALSE );

public:
	BOOL	IsTargetReady();

	DWORD CALCDAMAGE_SKILL ( int& rResultDAMAGE, const DWORD dwGaeaID, const STARGETID &cTargetID, const GLLandMan* pLandMan,
		const GLSKILL* pSkill=NULL, DWORD dwskill_lev=0, DWORD dwWeatherFlag=NULL, DWORD dwDivCount=0 );
};

typedef GLSummonField* PGLSUMMONFIELD;

typedef CGLLIST<PGLSUMMONFIELD>	GLSUMMONLIST;
typedef SGLNODE<PGLSUMMONFIELD>	GLSUMMONNODE;