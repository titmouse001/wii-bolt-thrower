#ifndef Camera_H
#define Camera_H

#include "GCTypes.h"
#include "ogc/gx.h"
#include <gccore.h>

using namespace std;

class Vessel;

class Camera
{

public:
	
	Camera();

	void Init();
	void SetUpView();

	void SetCameraView(float x, float y)  ;
	void SetCameraView(float x, float y, float z)  ;
	void SetCameraView(guVector& LookAt, guVector& Cam);
	void SetCameraView(f32 LookAtX, f32 LookAtY, f32 LookAtZ, f32 CamX, f32 CamY, f32 CamZ);

	void ForceCameraView(float x, float y);

	float GetCamX() const { return m_camera.x; }
	float GetCamY() const { return m_camera.y; }
	float GetCamZ() const {  return m_camera.z; }

	void SetCamX(float Value) {  m_camera.x = Value; }
	void SetCamY(float Value) {  m_camera.y = Value; }
	void SetCamZ(float Value) {  m_camera.z = Value; }
	
	void AddCamX(float Value) {  m_camera.x += Value; }
	void AddCamY(float Value) {  m_camera.y += Value; }
	void AddCamZ(float Value) {  m_camera.z += Value; }

	void CameraMovementLogic(Vessel* pVessel,float fFactor = 0.065f);
		
	Mtx&  GetcameraMatrix() { return m_cameraMatrix; }

	float GetFOV() const { return m_FieldOfView; }
	void SetFOV(float fValue) { m_FieldOfView = fValue; }

	void SetLightOn(int LightNumber=1, float x = 250000.0f, float y = 250000.0f, float z = -1000000.0f);
	void SetLightOn2();
	void SetLightAlpha(u8 Alpha);
	void SetLightOff();

	//void SetSpotLight( guVector pos, guVector lookat, 
	//							 f32 angAttn0, f32 angAttn1, f32 angAttn2, 
	//							 f32 distAttn0, f32 distAttn1, f32 distAttn2) ;

	//void SetLightSpec(u8 num, guVector dir, f32 shy) ;

	void  SetLightDiff(u8 num, guVector pos, f32 distattn, f32 brightness);

	void SetVesselLightOn(float x, float y, float z);

	void StoreCameraView()	{ m_StoredCamera = m_camera; }
	void RecallCameraView()	{ m_camera = m_StoredCamera; SetCameraView(); }
	
	float GetCameraHeightFor2DViewPort() { return m_CameraHeightFor3DViewPort; } 

	//---------------------------------
	// lights
	
	void	DoDefaultLight(float x = 25000.0f, float y = 45000.0f, float z = -100000.0f);
	void	DoLight(float x = 25000.0f, float y = 45000.0f, float z = -100000.0f);

	void		SetAmbientLight(float Colour);
	void		SetLightColour(float Colour);
	void		SetMaterialColour(u8 Colour, u8 Alpha=255);
	GXColor		m_LightColour;
	GXColor		m_MaterialColour;
	GXColor		m_AmbientColour;

private:

	float	CalculateCameraHeightFor2DViewPort();

	void	SetCameraView();

	Mtx			m_cameraMatrix;
	guVector	m_camera;
	guVector	m_up;
	guVector	m_look;
	guVector	m_StoredCamera;

	float	m_CameraHeightFor3DViewPort;
	float	m_FieldOfView;
	u32		m_UsedLightMask;
};


#endif