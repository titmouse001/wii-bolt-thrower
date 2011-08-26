#ifndef Camera_H
#define Camera_H

#include "GCTypes.h"
#include "ogc/gx.h"
#include <gccore.h>

using namespace std;

class Camera
{

public:
	
	Camera() : m_FieldOfView( 45.0f ) {;}

	void InitialiseCamera();

	void SetCameraView(float x, float y)  ;
	void SetCameraView(float x, float y, float z)  ;
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

	void CameraMovementLogic(float MoveToX,float MoveToY,float MoveToZ,float fFactor = 0.065f);
	//void CameraMovementLogic(float MoveToX,float MoveToY,float fFactor = 0.065f);
		
	Mtx&  GetcameraMatrix() { return m_cameraMatrix; }

	float GetFOV() const { return m_FieldOfView; }
	void SetFOV(float fValue) { m_FieldOfView = fValue; }

	void SetLightOn3(float x = 250000.0f, float y = 250000.0f, float z = -1000000.0f);
	void SetLightOn(float x = 250000.0f, float y = 250000.0f, float z = -1000000.0f);
	void SetLightOn2();
	void SetLightOff();

	void SetVesselLightOn(float x, float y, float z);

	void StoreCameraView()	{ m_StoredCamera = m_camera; }
	void RecallCameraView()	{ m_camera = m_StoredCamera; SetCameraView(); }


private:

	void SetCameraView();

	Mtx			m_cameraMatrix;
	guVector	m_camera;
	guVector	m_up;
	guVector	m_look;
	guVector	m_StoredCamera;


	float		m_FieldOfView;


};


#endif