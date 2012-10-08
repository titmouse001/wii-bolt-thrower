#include "Vessel.h"
#include <stdlib.h>
#include "Timer.h"
#include "HashString.h"

void Vessel::SetVel(f32 x, f32 y, f32 z)
{
	m_Vel.x = x;
	m_Vel.y = y;
	m_Vel.z = z;
}

void Vessel::AddPos(f32 x, f32 y, f32 z)
{
	m_Pos.x += ( x * m_SpeedFactor );  // factor will be kept at ONE for things like your ship
	m_Pos.y += ( y * m_SpeedFactor );
	m_Pos.z += ( z * m_SpeedFactor );
}

void Vessel::AddPos(guVector& rVector)
{
	m_Pos.x += ( rVector.x * m_SpeedFactor );
	m_Pos.y += ( rVector.y * m_SpeedFactor );
	m_Pos.z += ( rVector.z * m_SpeedFactor );
}


void Vessel::AddVel(f32 x, f32 y, f32 z)
{
	m_Vel.x += x;
	m_Vel.y += y;
	m_Vel.z += z;
}

void Vessel::AddVel(guVector& rVector)
{
	m_Vel.x += rVector.x;
	m_Vel.y += rVector.y ;
	m_Vel.z += rVector.z;
}

void Vessel::FactorVel() {
	FactorVel(GetGravityFactor());
}

void Vessel::FactorVel(float factor) {
	m_Vel.x *= factor;
	m_Vel.y *= factor;
	m_Vel.z *= factor;
}

void Vessel::AddVelToPos()
{
	AddPos(m_Vel);
}

void Vessel::SetDestinationPos(f32 x,f32 y , f32 z)
{
	m_Destination.x = x;
	m_Destination.y = y;
	m_Destination.z = z;
}

void Vessel::SetPos(f32 x,f32 y , f32 z)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_Pos.z = z;
}

void	Vessel::AddFacingDirection(float Value) 
{ 
	m_FacingDirection += Value; 

	if (m_FacingDirection < -M_PI) {
		m_FacingDirection += M_PI * 2.0f;
	}
	else if (m_FacingDirection > M_PI ){
		m_FacingDirection -= M_PI * 2.0f;
	}
}

void Vessel::AddTurrentDirection(float fValue) 
{ 	
	m_fTurrentDirection += fValue; 
	if (m_fTurrentDirection < -M_PI){
		m_fTurrentDirection += M_PI * 2.0f;
	}
	else if (m_fTurrentDirection > M_PI ){
		m_fTurrentDirection -= M_PI * 2.0f;
	}
}

// note: The radius param takes a squared value
bool  Vessel::InsideRadius(float center_x, float center_y, float radius)
{
	float XToCheck( GetX() - center_x );
	float YToCheck( GetY() - center_y );
	float square_dist ( (XToCheck * XToCheck) + (YToCheck * YToCheck) );
	return (square_dist <= radius);
}

// note: The radius param takes a squared value
bool  Vessel::InsideRadius(Vessel& rVessel, float radius)
{
	float XToCheck( GetX() - rVessel.GetX() );
	float YToCheck( GetY() - rVessel.GetY() );
	float square_dist ( (XToCheck * XToCheck) + (YToCheck * YToCheck) );
	return (square_dist <= radius);
}

void Vessel::SetFrameGroupWithRandomFrame(StartAndEndFrameInfo* pFameInfo, float FrameSpeed)
{
	SetFrameGroup(pFameInfo,FrameSpeed);
	SetFrame( GetFrameStart() + ( rand()% (int)(GetEndFrame() - GetFrameStart()) ) );
}

void Vessel::SetFrameGroup(StartAndEndFrameInfo* pFameInfo, float FrameSpeed)
{
	SetFrameStart( pFameInfo->StartFrame );
	SetEndFrame( pFameInfo->EndFrame );
	SetFrame( GetFrameStart() );
	SetFrameSpeed( FrameSpeed );  
}

f32 Vessel::GetTurnDirection(guVector* Vec)
{
	f32 DirectionToFaceTarget = M_PI - atan2( Vec->x - GetX(), Vec->y - GetY() );
	float diff = DirectionToFaceTarget - GetFacingDirection();
	if (diff > M_PI)  // shortest turn is always less than PI
		diff -= 2*M_PI; // get shorter turn direction
	else if (diff < -M_PI)
		diff += 2*M_PI; // get shorter turn direction
	return diff;
}

f32 Vessel::GetTurnDirectionForTurret(guVector* Vec)
{
	f32 DirectionToFaceTarget = M_PI - atan2( Vec->x - GetX(), Vec->y - GetY() );
	float diff = DirectionToFaceTarget - GetTurrentDirection();
	if (diff > M_PI)  // shortest turn is always less than PI
		diff -= 2*M_PI; // get shorter turn direction
	else if (diff < -M_PI)
		diff += 2*M_PI; // get shorter turn direction
	return diff;
}

void Vessel::AddShieldLevel(int Value) 
{ 
	int ShieldLevelMaxLimit = Singleton<WiiManager>::GetInstanceByPtr()->GetXmlVariable(HashString::PlayerMaxShieldLevel);
	m_iShieldLevel = std::min(m_iShieldLevel + Value,ShieldLevelMaxLimit); 
}

// **************
// Item3D support 
// **************

void Item3D::InitTimer() {
	m_pTimer = new Timer;  
}

void Item3D::SetTimerMillisecs(u32 t) { 
	m_pTimer->SetTimerMillisecs(t); 
}

bool Item3D::IsTimerDone() const { 
	return m_pTimer->IsTimerDone(); 
}



