#include "Camera.h"
#include "WiiManager.h"
#include "debug.h"
#include <math.h>


#include "CullFrustum\FrustumR.h"

#include "CullFrustum\Vec3.h"

// when... FOV set at 90 , giving a full left & right view. 
// As the camera height is known the other 45deg length must be the same.
// Note: Camera looks up to flip the view so it will work the same way as the 2d view.
//	CameraFactor, this affects the camera height, the real 3D world width/height of the screen

void Camera::InitialiseCamera()
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );  // backbone stuff
	static const guVector UP = {0.0F, -1.0F, 0.0F}; 
	m_up = UP; 

//	f32 w = Wii.GetScreenWidth();
	f32 h = Wii.GetScreenHeight();
	float Fov = 45.0f;
	float triangle = 90.0f - (Fov * 0.5f); 
	float rads = triangle * (M_PI/180.0f);
	float CameraHeight = tan(rads) * (h * 0.5); 

	//printf("Camera Height %f",CameraHeight);
	SetCameraView( (Wii.GetScreenWidth()/2), (Wii.GetScreenHeight()/2), -(CameraHeight));
}

////void Camera::CameraMovementLogic(float MoveToX, float MoveToY,float fFactor)
////{
////	CameraMovementLogic(MoveToX, MoveToY, GetCamZ(),fFactor);
////}
void Camera::CameraMovementLogic(float MoveToX, float MoveToY, float MoveToZ,float fFactor)
{
	//static float fFactor(0.075f);
	//static float fFactor(0.065f);
	AddCamX( ( MoveToX - GetCamX() ) * fFactor );
	AddCamY( ( MoveToY - GetCamY() ) * fFactor );
	AddCamZ( ( MoveToZ - GetCamZ() ) * fFactor );

	SetCameraView();
}

extern FrustumR frustum;

void Camera::SetCameraView()  //private
{
	m_look	= m_camera; 
	m_look.z = 0.0f;
	guLookAt(m_cameraMatrix, &m_camera,	&m_up, &m_look);

	Vec3 v1(m_camera.x,m_camera.y,m_camera.z);
	Vec3 v2(m_look.x,m_look.y,m_look.z);
	Vec3 v3(m_up.x,m_up.y,m_up.z);

	Singleton<WiiManager>::GetInstanceByRef().m_Frustum.setCamDef(v1,v2,v3);
}

void Camera::SetCameraView(float x, float y)  
{ 
	m_camera.x = x; 
	m_camera.y = y;  

	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );  
	f32 h = Wii.GetScreenHeight();
	float Fov = 45.0f;
	float triangle = 90.0f - (Fov * 0.5f); 
	float rads = triangle * (M_PI/180.0f);
	float CameraHeight = tan(rads) * (h * 0.5); 
	m_camera.z = -CameraHeight;

	SetCameraView();
}

void Camera::ForceCameraView(float x, float y)  
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );  
	f32 h = Wii.GetScreenHeight();
	float Fov = 45.0f;
	float triangle = 90.0f - (Fov * 0.5f); 
	float rads = triangle * (M_PI/180.0f);
	float CameraHeight = tan(rads) * (h * 0.5); 

	guVector camera	= {x, y, CameraHeight }; 
	guVector look	= {x, y, 0.0f };
	guLookAt(m_cameraMatrix, &camera, &m_up, &look);

	// set frustum clipping
	Vec3 v1(camera.x,camera.y,camera.z);
	Vec3 v2(look.x,look.y,look.z);
	Vec3 v3(m_up.x,m_up.y,m_up.z);
	Singleton<WiiManager>::GetInstanceByRef().m_Frustum.setCamDef(v1,v2,v3);
}

void Camera::SetCameraView(float x, float y, float z)  
{ 
	m_camera.x = x; 
	m_camera.y = y; 
	m_camera.z = z; 
	SetCameraView();
}


void Camera::SetCameraView(f32 LookAtX, f32 LookAtY, f32 LookAtZ, f32 CamX, f32 CamY, f32 CamZ)
{
	m_look.x = LookAtX;
	m_look.y = LookAtY;
	m_look.z = LookAtZ;
	m_camera.x = CamX; 
	m_camera.y = CamY;  
	m_camera.z = CamZ;  

	guLookAt(m_cameraMatrix, &m_camera,	&m_up, &m_look);
}


void Camera::SetLightOn3(float x, float y, float z)
{
    static const GXColor LightColour  = { 240, 240, 255, 0xFF };
	static const GXColor AmbientColour  = { 130, 130, 130, 0xFF };
    static const GXColor MaterialColour = { 0xf0, 0xf0, 0xf0, 0xFF };

	guVector light_pos,light_look;
	guVector look, pos, light_dir;
	GXLightObj LightObject;

	light_pos.x =    25000;
	light_pos.y =	 45000;
	light_pos.z =  -100000;

	light_look.x = 0;
	light_look.y = 0;
	light_look.z = 0;

	guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	guVecMultiply (GetcameraMatrix(), &light_look, &look); 

	guVecSub (&look, &pos, &light_dir);
	GX_InitSpecularDirv (&LightObject, &light_dir) ;
	GX_InitLightColor (&LightObject, LightColour ) ;
	GX_InitLightShininess (&LightObject, 0.5f); 

	GX_LoadLightObj(&LightObject,GX_LIGHT0);
	GX_SetNumChans(1);

	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
    GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
    GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);
}


void Camera::SetLightOn2()
{
    static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
	static const GXColor AmbientColour  = { 0x0f, 0x0f, 0x0f, 0xFF };
    static const GXColor MaterialColour = { 0xff, 0xff, 0xff, 0xFF };

	guVector light_pos,light_look;
	guVector look, pos, light_dir;
	GXLightObj LightObject;

	light_pos.x =  1000;
	light_pos.y =  -500;
	light_pos.z =  -1000;

	light_look.x = 0;
	light_look.y = 0;
	light_look.z = 0;

	guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	guVecMultiply (GetcameraMatrix(), &light_look, &look); 

	
	guVecSub (&look, &pos, &light_dir);
	GX_InitSpecularDirv (&LightObject, &light_dir) ;
	GX_InitLightColor (&LightObject, LightColour ) ;
	GX_InitLightShininess (&LightObject, 0.5f);  // sets a relatively open angle 


	GX_LoadLightObj(&LightObject,GX_LIGHT0);
	GX_SetNumChans(1);

	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
    GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
    GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);

}




void Camera::SetVesselLightOn(float x, float y, float z)
{
    static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
	static const GXColor AmbientColour  = { 144, 144, 144, 0xFF };
    static const GXColor MaterialColour = { 0xff, 0xff, 0xff, 0xFF };

	guVector light_pos,light_look;
	guVector look, pos, light_dir;
	GXLightObj LightObject;

	light_pos.x = x;
	light_pos.y = y;
	light_pos.z = z;

	light_look.x = x;
	light_look.y = y;
	light_look.z = 0;

	guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	guVecMultiply (GetcameraMatrix(), &light_look, &look); 

	guVecSub (&look, &pos, &light_dir);
	GX_InitSpecularDirv (&LightObject, &light_dir) ;
	GX_InitLightColor (&LightObject, LightColour ) ;
	GX_InitLightShininess (&LightObject, 0.5f);  // sets a relatively open angle 

	GX_LoadLightObj(&LightObject,GX_LIGHT0);
	GX_SetNumChans(1);

	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
    GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
    GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);

}

void Camera::SetLightOn(float x, float y, float z)
{
    static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
	static const GXColor AmbientColour  = { 0x2f, 0x2f, 0x2f, 0xFF };
    static const GXColor MaterialColour = { 0xff, 0xff, 0xff, 0xFF };

	guVector LightPos;
	LightPos.x = x;//   250000.0f;
	LightPos.y = y;//   250000.0f;
	LightPos.z = z;// -1000000.0f;

	guVecMultiply(GetcameraMatrix(), &LightPos, &LightPos);

	GXLightObj LightObject;
	GX_InitLightPos(&LightObject,LightPos.x,LightPos.y,LightPos.z);
	GX_InitLightColor(&LightObject,LightColour);
	GX_InitLightSpot(&LightObject, 0.0f, GX_SP_OFF);
	GX_InitLightDistAttn(&LightObject, 1.0f, 1.0f, GX_DA_OFF);
	
	GX_LoadLightObj(&LightObject,GX_LIGHT0);
	GX_SetNumChans(1);

	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPOT);
    GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
    GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);
}

void Camera::SetLightOff() 
{
    GX_SetNumTevStages(1);
 //   GX_SetTevOp  (GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}
