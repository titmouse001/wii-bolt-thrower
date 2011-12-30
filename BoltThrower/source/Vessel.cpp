#include "Vessel.h"
#include <stdlib.h>
#include "HashLabel.h"
#include "WiiManager.h"
#include "Timer.h"

void Vessel::SetVel(f32 x, f32 y, f32 z)
{
	m_Vel.x = x;
	m_Vel.y = y;
	m_Vel.z = z;
}

void Vessel::AddPos(f32 x, f32 y, f32 z)
{
	m_Pos.x += ( x * m_SpeedFactor );
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

void Vessel::VelReduce()
{
	//static const float Gravity(0.99f);
	m_Vel.x *= GetGravity();
	m_Vel.y *= GetGravity();
	m_Vel.z *= GetGravity();
}

void Vessel::AddVelToPos()
{
	AddPos(m_Vel);
}

void Vessel::SetPos(f32 x,f32 y , f32 z)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_Pos.z = z;
}

void	Vessel::AddFacingDirection(float Value) 
{ 
	static const float PI(3.14159265f);

	m_FacingDirection += Value; 

	if (m_FacingDirection < -PI)
	{
		m_FacingDirection += PI * 2.0f;
	}
	else if (m_FacingDirection > PI )
	{
		m_FacingDirection -= PI * 2.0f;
	}

	//m_LastValueAddedToFacingDirection = Value;
}

void Vessel::AddTurrentDirection(float fValue) 
{ 	
	m_fTurrentDirection += fValue; 
	if (m_fTurrentDirection < -M_PI)
	{
		m_fTurrentDirection += M_PI * 2.0f;
	}
	else if (m_fTurrentDirection > M_PI )
	{
		m_fTurrentDirection -= M_PI * 2.0f;
	}
}

// note: The radius param takes a squared value
bool  Vessel::InsideRadius(float center_x, float center_y, float radius)
{
	//float square_dist = ((GetX()-center_x)*(GetX()-center_x) ) + ((GetY()-center_y)*(GetY()-center_y)) ;
	//return ( fabs(square_dist) < (radius) );
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

////
////void Vessel::DetonationSpin()
////{
////	// blows up and spins off into space
////	SetFuel(100 + rand()%50);
////	SetGravity(0.9975f);
////	AddVel( ((rand()%100)-50) * 0.025f , ((rand()%100)-50) * 0.025f, 5 );
////	SetSpin( (1000 - (rand()%2000 )) * 0.00025f );
////	SetGoingBoom(true);
////}

void Vessel::SetFrameGroupWithRandomFrame(HashLabel FrameName, float FrameSpeed)
{
	WiiManager* pWii( Singleton<WiiManager>::GetInstanceByPtr() );

	SetFrameStart( pWii->m_FrameEndStartConstainer[FrameName].StartFrame );
	SetEndFrame( pWii->m_FrameEndStartConstainer[FrameName].EndFrame );

	SetFrame( GetFrameStart() + ( rand()% (int)(GetEndFrame() - GetFrameStart()) ) );
	SetFrameSpeed( FrameSpeed );  
}


void Vessel::SetFrameGroup(HashLabel FrameName, float FrameSpeed)
{
	WiiManager* pWii( Singleton<WiiManager>::GetInstanceByPtr() );

	SetFrameStart( pWii->m_FrameEndStartConstainer[FrameName].StartFrame );
	SetEndFrame( pWii->m_FrameEndStartConstainer[FrameName].EndFrame );
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

void Item3D::InitTimer() 	{ m_pTimer = new Timer;  }
void Item3D::SetTimerMillisecs(u32 t) { m_pTimer->SetTimerMillisecs(t); }
bool Item3D::IsTimerDone() const { return m_pTimer->IsTimerDone(); }

