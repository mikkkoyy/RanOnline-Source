#include "StdAfx.h"
#include "QBoxButtonIcon.h"

CQBoxButtonIcon::CQBoxButtonIcon(void) :
	CBasicButton()
	, m_emType( QUESTION_SIZE )
	, m_pImageSpeedUp( NULL )
	, m_pImageCrazyTime( NULL )
	, m_pImagePowerUp( NULL )
	, m_pImageExpTime( NULL )
	, m_pImageLucky( NULL )
	, m_pImageSpeedUpMax( NULL )
	, m_pImageMadnessTime( NULL )
	, m_pImagePowerUpMax( NULL )
{
}

CQBoxButtonIcon::~CQBoxButtonIcon(void)
{
}

void CQBoxButtonIcon::CreateSubControl ()
{
	m_pImageSpeedUp = new CUIControl;
	m_pImageSpeedUp->CreateSub ( this, "Q_BOX_SPEED_UP" );
	m_pImageSpeedUp->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageSpeedUp );

	m_pImageCrazyTime = new CUIControl;
	m_pImageCrazyTime->CreateSub ( this, "Q_BOX_CRAZY_TIME" );
	m_pImageCrazyTime->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageCrazyTime );

	m_pImagePowerUp = new CUIControl;
	m_pImagePowerUp->CreateSub ( this, "Q_BOX_POWER_UP" );
	m_pImagePowerUp->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImagePowerUp );

	m_pImageExpTime = new CUIControl;
	m_pImageExpTime->CreateSub ( this, "Q_BOX_EXP_TIME" );
	m_pImageExpTime->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageExpTime );

	m_pImageLucky = new CUIControl;
	m_pImageLucky->CreateSub ( this, "Q_BOX_LUCKY" );
	m_pImageLucky->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageLucky );

	m_pImageSpeedUpMax = new CUIControl;
	m_pImageSpeedUpMax->CreateSub ( this, "Q_BOX_SPEED_UP_MAX" );
	m_pImageSpeedUpMax->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageSpeedUpMax );

	m_pImageMadnessTime = new CUIControl;
	m_pImageMadnessTime->CreateSub ( this, "Q_BOX_MADNESS_TIME" );
	m_pImageMadnessTime->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImageMadnessTime );

	m_pImagePowerUpMax = new CUIControl;
	m_pImagePowerUpMax->CreateSub ( this, "Q_BOX_POWER_UP_MAX" );
	m_pImagePowerUpMax->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pImagePowerUpMax );
}

void CQBoxButtonIcon::SetType( EMITEM_QUESTION emType )
{
	m_emType = emType;

	m_pImageSpeedUp->SetVisibleSingle( m_emType == QUESTION_SPEED_UP );
	m_pImageCrazyTime->SetVisibleSingle( m_emType == QUESTION_CRAZY );
	m_pImagePowerUp->SetVisibleSingle( m_emType == QUESTION_ATTACK_UP );
	m_pImageExpTime->SetVisibleSingle( m_emType == QUESTION_EXP_UP || m_emType == QUESTION_EXP_GET );
	m_pImageLucky->SetVisibleSingle( m_emType == QUESTION_LUCKY );
	m_pImageSpeedUpMax->SetVisibleSingle( m_emType == QUESTION_SPEED_UP_M );
	m_pImageMadnessTime->SetVisibleSingle( m_emType == QUESTION_MADNESS );
	m_pImagePowerUpMax->SetVisibleSingle( m_emType == QUESTION_ATTACK_UP_M );
}