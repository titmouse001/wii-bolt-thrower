#ifndef Vessel_H
#define Vessel_H

#include "GCTypes.h"
#include "Util.h"
#include "ogc\lwp_watchdog.h"
#include "ogc\gu.h"
#include <math.h>

#include "hashlabel.h"

//class  HashLabel;
class  WiiManager;
class  Timer;

class  Item3D
{
public:
	Item3D()
	{
		m_Pos.x=0;
		m_Pos.y=0;
		m_Pos.z=0;
		m_Vel.x=0;
		m_Vel.y=0;
		m_Vel.z=0;
		m_Scale.x=1;
		m_Scale.y=1;
		m_Scale.z=1;
		m_Rotate.x=0;
		m_Rotate.y=0;
		m_Rotate.z=0;
		m_RotateAmount.x=0;
		m_RotateAmount.y=0;
		m_RotateAmount.z=0;
		m_Enable = true;
		m_LockOntoVesselIndex = 0;
	}

	guVector& GetPos() { return m_Pos ;}
	void SetPos(f32 x,f32 y , f32 z) { m_Pos.x = x; m_Pos.y = y; m_Pos.z = z; }
	void AddPos(f32 x,f32 y , f32 z) { m_Pos.x += x; m_Pos.y += y; m_Pos.z += z; }
	f32 GetX() const { return m_Pos.x; }
	f32 GetY() const { return m_Pos.y; }
	f32 GetZ() const { return m_Pos.z; }
	
	void SetVel(guVector& v) { m_Vel  = v; }
	void SetVel(f32 x,f32 y , f32 z) { m_Vel.x = x; m_Vel.y = y; m_Vel.z = z; }
	void AddVel(f32 x,f32 y , f32 z) { m_Vel.x += x; m_Vel.y += y; m_Vel.z += z; }
	void AddVel(guVector& v) { m_Vel.x += v.x; m_Vel.y += v.y; m_Vel.z += v.z; }

	void ReduceVel(f32 fFactor) { m_Vel.x *= fFactor; m_Vel.y *= fFactor; m_Vel.z *= fFactor; }

	void SetVelX(float Val) { m_Vel.x = Val; }
	void SetVelY(float Val) { m_Vel.y = Val; }
	void SetVelZ(float Val) { m_Vel.z = Val; }
	f32 GetVelX() const { return m_Vel.x; }
	f32 GetVelY() const { return m_Vel.y; }
	f32 GetVelZ() const { return m_Vel.z; }

	guVector& GetVel() { return m_Vel; }


	void AddVelToPos() { m_Pos.x += m_Vel.x; m_Pos.y += m_Vel.y; m_Pos.z +=  m_Vel.z; }
	void AddVelToPos(float fFactor) { m_Pos.x += m_Vel.x*fFactor; m_Pos.y += m_Vel.y*fFactor; m_Pos.z += m_Vel.z*fFactor; }

	void SetRotate(f32 x, f32 y , f32 z) { m_Rotate.x = x; m_Rotate.y = y; m_Rotate.z = z; }
	void Rotate() { m_Rotate.x += m_RotateAmount.x; m_Rotate.y += m_RotateAmount.y; m_Rotate.z +=  m_RotateAmount.z; }
	f32 GetRotateX() const { return m_Rotate.x; }
	f32 GetRotateY() const { return m_Rotate.y; }
	f32 GetRotateZ() const { return m_Rotate.z; }

	void SetRotateAmount(f32 x, f32 y , f32 z) { m_RotateAmount.x = x; m_RotateAmount.y = y; m_RotateAmount.z = z; }

	void DampRotation(float Value) { m_RotateAmount.x *= Value;	m_RotateAmount.y *= Value; m_RotateAmount.z *= Value; }

	void SetScale(f32 x, f32 y , f32 z)  { m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; }
	void MulScale(float Factor)  { m_Scale.x *= Factor; m_Scale.y *= Factor; m_Scale.z *= Factor; }
	const guVector& GetScale() const { return m_Scale; }
	f32 GetScaleX() const { return m_Scale.x; }
	f32 GetScaleY() const { return m_Scale.y; }
	f32 GetScaleZ() const { return m_Scale.z; }


	float  GetSqaureRadius(float center_x, float center_y)
	{
		return fabs((GetX()-center_x)*(GetX()-center_x) ) + ((GetY()-center_y)*(GetY()-center_y)) ;
	}

	bool  InsideRadius(float center_x, float center_y, float radius) const 
	{
		float square_dist = ((GetX()-center_x)*(GetX()-center_x) ) + ((GetY()-center_y)*(GetY()-center_y)) ;
		return ( fabs(square_dist) < (radius) );
	}
	bool  InsideRadius(Item3D& rItem, float radius) const
	{
		// note: The radius param takes a squared value
		float XToCheck(rItem.GetX());
		float YToCheck(rItem.GetY());
		float square_dist = ((GetX()-XToCheck)*(GetX()-XToCheck) ) + ((GetY()-YToCheck)*(GetY()-YToCheck)) ;
		return ( fabs(square_dist) < radius );
	}

	void SetEnable(bool State) { m_Enable = State; }
	bool GetEnable() { return m_Enable; }

	int GetLockOntoVesselIndex() const { return m_LockOntoVesselIndex; }
	void SetLockOntoVesselIndex(int Value) { m_LockOntoVesselIndex = Value;}

	HashLabel GetLockOntoVesselType()  { return m_LockOntoVesselType; }
	void SetLockOntoVesselType(HashLabel Value) { m_LockOntoVesselType = Value;}

	// timer section
	void InitTimer();
	void SetTimerMillisecs(u32 t);
	bool IsTimerDone() const;

private:
	guVector	m_Pos;
	guVector	m_Vel;
	guVector	m_Rotate;
	guVector	m_RotateAmount;
	guVector	m_Scale;
	bool		m_Enable;

	int			m_LockOntoVesselIndex;
	HashLabel	m_LockOntoVesselType;

	Timer*		m_pTimer;
};

enum EDetailLevelFor3D { High, Medium, Low, Auto } ;

class  MoonItem3D : public Item3D
{
public:
	MoonItem3D() : m_AmountOfRocks(0), m_eDetailLevel(Auto) {;}

	u32 GetAmountOfRocks() { return  m_AmountOfRocks; } 
	void SetAmountOfRocks(u32 Value) { m_AmountOfRocks = Value; }

	void SetDetailLevel(EDetailLevelFor3D Value = Auto) { m_eDetailLevel=Value; }
	EDetailLevelFor3D GetDetailLevel()	{ return m_eDetailLevel; }

private:
	u32		m_AmountOfRocks;
	EDetailLevelFor3D 	m_eDetailLevel;
};

class  TurretItem3D : public Item3D
{
public:
	TurretItem3D()
	{
		m_WorkingTarget.x=0;
		m_WorkingTarget.y=0;
		m_WorkingTarget.z=0;
		m_CurrentTarget.x=0;
		m_CurrentTarget.y=0;
		m_CurrentTarget.z=0;
	}

	void SetWorkingTarget(guVector& WorkingTarget) { m_WorkingTarget = WorkingTarget; }
	void SetCurrentTarget(guVector& CurrentTarget) { m_CurrentTarget = CurrentTarget; }
	guVector& GetWorkingTarget() { return m_WorkingTarget; }
	guVector& GetCurrentTarget() { return m_CurrentTarget; }

private:
	guVector m_WorkingTarget;
	guVector m_CurrentTarget;

};

class Vessel
{
public:
	Vessel() :	m_FireRate(222.0f), m_BulletSpeedFactor(0.10f), m_fTurrentDirection(0.0f),
				m_SpeedFactor(1.0f), m_iEndFrame(0), m_fFrameStart(0),m_fFrame(0), m_fFrameSpeed(0.025f), m_iShieldLevel(1), 
				m_FacingDirection(0), m_Spin(0.0f), m_Gravity(0.995f), m_iFuelValue(0), m_Alpha(255),// m_bGoingBoom(false),
				m_fCurrentScaleFactor(1.0f),m_fScaleToFactor(1.0f), m_fScaleToFactorSpeed(0.20f)
	{
		m_Vel.x=0.0f;
		m_Vel.y=0.0f;
		m_Vel.z=0.0f;
	}

	Vessel(guVector& Pos, guVector& Velocity, float StartFrame, float EndFrame, float FrameSpeed, float Gravity) :
				m_FireRate(222.0f), m_BulletSpeedFactor(0.10f), m_fTurrentDirection(0.0f),
				m_SpeedFactor(1.0f),
				m_iEndFrame(EndFrame), 
				m_fFrameStart(StartFrame),
				m_fFrame(StartFrame), 
				m_fFrameSpeed(FrameSpeed), 
				m_iShieldLevel(1), m_FacingDirection(0), m_Spin(0.0f), 
				m_Gravity(Gravity), 
				m_iFuelValue(0), m_Alpha(255), //m_bGoingBoom(false),
				m_fCurrentScaleFactor(1.0f),m_fScaleToFactor(1.0f), m_fScaleToFactorSpeed(0.20f)
	{
		m_Pos = Pos;
		m_Vel = Velocity;
	}

	Vessel(Item3D* Item, float Gravity) :
				m_FireRate(222.0f), m_BulletSpeedFactor(0.10f), m_fTurrentDirection(0.0f),
				m_SpeedFactor(1.0f),
			//	m_iEndFrame(EndFrame), 
			//	m_fFrameStart(StartFrame),
			//	m_fFrame(StartFrame), 
			//	m_fFrameSpeed(FrameSpeed), 
				m_iShieldLevel(1), m_FacingDirection(0), m_Spin(0.0f), 
				m_Gravity(Gravity), 
				m_iFuelValue(0), m_Alpha(255), //m_bGoingBoom(false),
				m_fCurrentScaleFactor(1.0f),m_fScaleToFactor(1.0f), m_fScaleToFactorSpeed(0.20f)
	{
		m_Pos = Item->GetPos();
		m_Vel = Item->GetVel();
	}


	void  Init();

	void SetPos(f32 x,f32 y , f32 z);
	void SetZ(f32 z) {m_Pos.z = z;}

	void AddPos(guVector& rVector);
	void AddPos(f32 x, f32 y, f32 z);
	void AddVel(guVector& rVector);
	void AddVel(f32 x, f32 y, f32 z);
	void SetVel(f32 x, f32 y, f32 z);
	void AddVelToPos();

	void VelReduce();
	guVector& GetPos() { return m_Pos; }
	void SetPos(guVector& v ) { SetPos(v.x,v.y,v.z); }
	f32 GetX() const { return m_Pos.x; }
	f32 GetY() const { return m_Pos.y; }
	f32 GetZ() const { return m_Pos.z; }

	guVector& GetVel() { return m_Vel; }
	f32 GetVelX() const { return m_Vel.x; }
	f32 GetVelY() const { return m_Vel.y; }
	void SetVel(guVector& Vel ) { m_Vel = Vel; }
	void SetVelX(float Val) { m_Vel.x = Val; }
	void SetVelY(float Val) { m_Vel.y = Val; }

	void SetFacingDirection(float Value) { m_FacingDirection = Value; }
	void AddFacingDirection(float Value) ;
	f32 GetFacingDirection() const { return m_FacingDirection; }

	bool  InsideRadius(float center_x, float center_y, float radius);
	bool  InsideRadius(Vessel& rVessel, float radius);

	float GetSpin() const { return m_Spin; }
	void SetSpin(float fValue)  { m_Spin = fValue; }	

	float GetGravity() const { return m_Gravity; }
	void SetGravity(float fValue) { m_Gravity = fValue;}

	int GetFuel() const { return m_iFuelValue; }
	void SetFuel(int iFuel) { m_iFuelValue = iFuel;}
	int ReduceFuel(int iFuel=1) { m_iFuelValue -= iFuel; return m_iFuelValue; }

	u8 GetAlpha() { return m_Alpha; }
	void SetAlpha(u8 Value) { m_Alpha = Value;}

	float GetCurrentScaleFactor()					{ return m_fCurrentScaleFactor;   }
	void  SetCurrentScaleFactor(float fValue)		{ m_fCurrentScaleFactor = fValue; }
	float GetScaleToFactor()						{ return m_fScaleToFactor;   }
	void  SetScaleToFactor(float fValue)			{ m_fScaleToFactor = fValue; }
	float AddCurrentScaleFactor(float fValue)		{ return m_fCurrentScaleFactor += fValue; }
	
	void  SetScaleToFactorSpeed(float fValue)		{ m_fScaleToFactorSpeed = fValue; }
	float GetScaleToFactorSpeed()					{ return m_fScaleToFactorSpeed; }

	float GetTurrentDirection() { return m_fTurrentDirection; }
	void SetTurrentDirection( float fValue) { m_fTurrentDirection = fValue; }
	void AddTurrentDirection(float fValue);

	int GetShieldLevel() const { return m_iShieldLevel; }
	void SetShieldLevel(int Value) { m_iShieldLevel = Value; }
	void AddShieldLevel(int Value) { m_iShieldLevel += Value; }
	bool IsShieldOk() const { return m_iShieldLevel > 0; }
	bool HasShieldFailed() const { return m_iShieldLevel <= 0; }

	float GetFrame() const { return m_fFrame; }
	void SetFrame(float Value) { m_fFrame = Value; }
	void  AddFrame(float Value) { m_fFrame+=Value; }
	void  AddFrame() { m_fFrame+=m_fFrameSpeed; }

	float GetFrameSpeed() const { return m_fFrameSpeed; }
	int GetEndFrame() const { return m_iEndFrame; }
	float GetFrameStart() const { return m_fFrameStart; }

	int GetFireRate() const { return m_FireRate; }
	void SetFireRate(int Value) { m_FireRate = Value; }

	float GetBulletSpeedFactor() const { return m_BulletSpeedFactor; }
	void SetBulletSpeedFactor(float Value) { m_BulletSpeedFactor = Value; }

	float GetSpeedFactor() const { return m_SpeedFactor; }
	void SetSpeedFactor(float Value) { m_SpeedFactor = Value; }


	//void DetonationSpin();
	void SetFrameGroupWithRandomFrame(HashLabel FrameName, float FrameSpeed);
	void SetFrameGroup(HashLabel FrameName, float FrameSpeed=0);
	
	f32 GetTurnDirection(guVector* Vec);
	f32 GetTurnDirectionForTurret(guVector* Vec);

	void SetRadius(float Value) { m_Radius = Value; }
	float GetRadius() const { return m_Radius; }

private:
	void SetEndFrame(int Value) { m_iEndFrame = Value; }
	void SetFrameStart(float Value) { m_fFrameStart = Value; }
	void SetFrameSpeed(float Value) { m_fFrameSpeed = Value; }

	// gun ship section
	int			m_FireRate;
	float		m_BulletSpeedFactor;
	float		m_fTurrentDirection;

	// factor for movement speed
	float		m_SpeedFactor;

	int			m_iEndFrame;
	float		m_fFrameStart;
	float		m_fFrame;
	float		m_fFrameSpeed;
	int			m_iShieldLevel;

	guVector	m_Pos;
	guVector	m_Vel;
	float		m_FacingDirection;
	float		m_Spin;
	float		m_Gravity;
	int			m_iFuelValue;
	u8			m_Alpha;  

	//explosions
	float		m_fCurrentScaleFactor; 
	float		m_fScaleToFactor;
	float		m_fScaleToFactorSpeed;

	float		m_Radius;

};


class PlayerVessel : public Vessel
{
public:
	PlayerVessel() : m_PickUpTotal(0.0f) {;}
	void AddToPickUpTotal(int Value) { m_PickUpTotal += Value; }
	int GetPickUpTotal() { return m_PickUpTotal; }
	void ClearPickUpTotal() { m_PickUpTotal=0.0f; }
private:
	int m_PickUpTotal;
};


class  Item3DChronometry : public Item3D
{
public:
	void SetCountdownSeconds(u32 Value)	{ m_Timer = Util::timer_gettime() + secs_to_ticks(Value) ;}
	bool IsCountdownFinished()	{return Util::timer_gettime() > m_Timer;}

private:
	u64 m_Timer;
};

#endif
