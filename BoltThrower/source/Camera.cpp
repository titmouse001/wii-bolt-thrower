#include <math.h>

#include "Camera.h"
#include "WiiManager.h"
#include "Vessel.h"
#include "debug.h"
#include "CullFrustum\FrustumR.h"
#include "CullFrustum\Vec3.h"

// when... FOV set at 90 , giving a full left & right view. 
// As the camera height is known the other 45deg length must be the same.
// Note: Camera looks up to flip the view so it will work the same way as the 2d view.
//	CameraFactor, this affects the camera height, the real 3D world width/height of the screen

Camera::Camera() : m_FieldOfView( 45.0f ) , m_UsedLightMask(0)
{
}

void Camera::Init()
{
	//m_CameraHeightFor3DViewPort = CalculateCameraHeightFor2DViewPort();
	SetUpView();
}

void Camera::SetUpView()
{
	m_CameraHeightFor3DViewPort = CalculateCameraHeightFor2DViewPort();  // data member, reused 

	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );  // backbone stuff
	static const guVector UP = {0.0F, -1.0F, 0.0F}; 
	m_up = UP; 
	float CameraHeight = m_CameraHeightFor3DViewPort;
	SetCameraView( (Wii.GetScreenWidth()/2), (Wii.GetScreenHeight()/2), -(CameraHeight));
}

void Camera::CameraMovementLogic(Vessel* pPlayerVessel, float fFactor)
{
	float MoveToX = pPlayerVessel->GetX();
	float MoveToY = pPlayerVessel->GetY();

//	MoveToX += pPlayerVessel->GetVelX()*28.0f;
//	MoveToY += pPlayerVessel->GetVelY()*28.0f;

	AddCamX( ( MoveToX - GetCamX() ) * fFactor );
	AddCamY( ( MoveToY - GetCamY() ) * fFactor );
	SetCamZ(  m_CameraHeightFor3DViewPort );
	
	SetCameraView();
}

void Camera::SetCameraView() 
{
	m_look	= m_camera; 
	m_look.z = 0.0f;
	guLookAt(m_cameraMatrix, &m_camera,	&m_up, &m_look);
	Vec3 v1(m_camera.x,m_camera.y,m_camera.z);
	Vec3 v2(m_look.x,m_look.y,m_look.z);
	Vec3 v3(m_up.x,m_up.y,m_up.z);

	// *** This will update the frustum view ***
	Singleton<WiiManager>::GetInstanceByRef().GetFrustum()->setCamDef(v1,v2,v3);
}

void Camera::SetCameraView(float x, float y)  
{ 
	m_camera.x = x; 
	m_camera.y = y;  
	m_camera.z = m_CameraHeightFor3DViewPort;
	SetCameraView();
}

void Camera::SetCameraView(float x, float y, float z)  
{ 
	m_camera.x = x; 
	m_camera.y = y; 
	m_camera.z = z; 
	SetCameraView();
}

void Camera::SetCameraView(guVector& LookAt, guVector& Cam)
{
	guLookAt(m_cameraMatrix, &Cam,	&m_up, &LookAt);
}

void Camera::SetCameraView(f32 LookAtX, f32 LookAtY, f32 LookAtZ, f32 CamX, f32 CamY, f32 CamZ)
{
	m_look.x = LookAtX;
	m_look.y = LookAtY;
	m_look.z = LookAtZ;
	m_camera.x = CamX; 
	m_camera.y = CamY;  
	m_camera.z = CamZ;  

	SetCameraView(m_look, m_camera);
}

void Camera::ForceCameraView(float x, float y)  
{
	float CameraHeight = m_CameraHeightFor3DViewPort;

	guVector camera	= {x, y, CameraHeight }; 
	guVector look	= {x, y, 0.0f };
	guLookAt(m_cameraMatrix, &camera, &m_up, &look);

	// set frustum clipping
	Vec3 v1(camera.x,camera.y,camera.z);
	Vec3 v2(look.x,look.y,look.z);
	Vec3 v3(m_up.x,m_up.y,m_up.z);
	Singleton<WiiManager>::GetInstanceByRef().GetFrustum()->setCamDef(v1,v2,v3);
}

float Camera::CalculateCameraHeightFor2DViewPort()
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );  // backbone stuff

	f32 h = Wii.GetScreenHeight();
	float Fov = 45.0f;
	float triangle = 90.0f - (Fov * 0.5f); 
	float rads = triangle * (M_PI/180.0f);

	return -(tan(rads) * (h * 0.5)); 
}

// -------------------------------------------------------------
// Lighting
// -------------------------------------------------------------


void Camera::SetLightColour(float Colour)
{
	m_LightColour = (GXColor) { Colour, Colour, Colour, 0xFF };
}

void Camera::SetMaterialColour(u8 Colour, u8 Alpha)
{
	m_MaterialColour = (GXColor) { Colour, Colour, Colour, Alpha };
}

void Camera::SetAmbientLight(float Colour)
{
	m_AmbientColour = (GXColor){ Colour, Colour, Colour, 0xFF };
}

//void Camera::DoLight(float x, float y, float z)
//{
//	SetLightColour(240);
//	SetMaterialColour(0xf0);
//	SetAmbientLight(0x0f);
//	LightOn3(x,y,z);
//}

void Camera::DoDefaultLight(float x, float y, float z)
{
	SetLightColour(240);
	SetMaterialColour(0xf0);
	SetAmbientLight(0x0f);
	DoLight(x,y,z);
}

void Camera::DoLight(float x, float y, float z)
{
	GX_SetChanAmbColor( GX_COLOR0A0, m_AmbientColour );
	
//	static const GXColor LightColour  = { 240, 240, 255, 0xFF };
//	static const GXColor MaterialColour = { 0xf0, 0xf0, 0xf0, 0xFF };

	guVector light_pos,light_look;
	guVector look, pos, light_dir;
	GXLightObj LightObject;
	light_pos.x = x;
	light_pos.y = y;
	light_pos.z = z;
	light_look.x = 0;
	light_look.y = 0;
	light_look.z = 0;
	guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	guVecMultiply (GetcameraMatrix(), &light_look, &look); 
	guVecSub (&look, &pos, &light_dir);
	GX_InitSpecularDirv (&LightObject, &light_dir) ;
	GX_InitLightColor (&LightObject, m_LightColour ) ;
	GX_InitLightShininess (&LightObject, 0.5f); 
	GX_LoadLightObj(&LightObject,GX_LIGHT0);
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
	GX_SetChanMatColor(GX_COLOR0A0,m_MaterialColour);
}

void Camera::SetLightOn2()
{
	SetLightColour(240);
	SetMaterialColour(0xf0);
	SetAmbientLight(0x0f);
	DoLight(1000,-500,-1000);

	//static const GXColor AmbientColour  = { 0x0f, 0x0f, 0x0f, 0xFF };
	//GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);

	//static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
	//static const GXColor MaterialColour = { 0xff, 0xff, 0xff, 0xFF };
	//guVector light_pos,light_look;
	//guVector look, pos, light_dir;
	//GXLightObj LightObject;
	//light_pos.x =  1000;
	//light_pos.y =  -500;
	//light_pos.z =  -1000;
	//light_look.x = 0;
	//light_look.y = 0;
	//light_look.z = 0;
	//guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	//guVecMultiply (GetcameraMatrix(), &light_look, &look); 
	//guVecSub (&look, &pos, &light_dir);
	//GX_InitSpecularDirv (&LightObject, &light_dir) ;
	//GX_InitLightColor (&LightObject, LightColour ) ;
	//GX_InitLightShininess (&LightObject, 0.5f);  // sets a relatively open angle 
	//GX_LoadLightObj(&LightObject,GX_LIGHT0);
	//GX_SetNumChans(1);
	//GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
	//GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);
}


void Camera::SetLightAlpha(u8 Alpha)
{
	SetLightColour(240);
	SetMaterialColour(0xf0,Alpha);
	SetAmbientLight(0x0f);
	DoLight(1000,-500,-1000);
}


void Camera::SetVesselLightOn(float x, float y, float z)
{
	DoLight(x,y,z);

	//static const GXColor AmbientColour  = { 144, 144, 144, 0xFF };
	//GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);

	//static const GXColor MaterialColour = { 0xff, 0xff, 0xff, 0xFF };
	//static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
	//guVector light_pos,light_look;
	//guVector look, pos, light_dir;
	//GXLightObj LightObject;
	//light_pos.x = x;
	//light_pos.y = y;
	//light_pos.z = z;
	//light_look.x = x;
	//light_look.y = y;
	//light_look.z = 0;
	//guVecMultiply (GetcameraMatrix(), &light_pos, &pos); 
	//guVecMultiply (GetcameraMatrix(), &light_look, &look); 
	//guVecSub (&look, &pos, &light_dir);
	//GX_InitSpecularDirv (&LightObject, &light_dir) ;
	//GX_InitLightColor (&LightObject, LightColour ) ;
	//GX_InitLightShininess (&LightObject, 0.5f);  // sets a relatively open angle 
	//GX_LoadLightObj(&LightObject,GX_LIGHT0);
	//GX_SetNumChans(1);
	//GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,GX_LIGHT0,GX_DF_CLAMP,GX_AF_SPEC);
	//GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);
}

void Camera::SetLightOn(int LightNumber, float x, float y, float z)
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
	GX_LoadLightObj(&LightObject,1<<LightNumber);
	GX_SetNumChans(1);

	m_UsedLightMask |= 1<<LightNumber;
	GX_SetChanCtrl(GX_COLOR0A0,GX_ENABLE,GX_SRC_REG,GX_SRC_REG,m_UsedLightMask,GX_DF_CLAMP,GX_AF_SPOT);

	GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
	GX_SetChanMatColor(GX_COLOR0A0,MaterialColour);
}

//
//void Camera::SetSpotLight( guVector pos, guVector lookat, 
//								 f32 angAttn0, f32 angAttn1, f32 angAttn2, 
//								 f32 distAttn0, f32 distAttn1, f32 distAttn2) 
//{      
//	static const GXColor AmbientColour  = { 0x0f, 0x0f, 0x0f, 0xFF };
//	GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
//
//
//	GXLightObj lobj;      
//	guVector lpos = (guVector){ pos.x, pos.y, pos.z };    
//	guVector ldir = (guVector){ lookat.x-pos.x, lookat.y-pos.y, lookat.z-pos.z };  
//	guVecNormalize(&ldir);       
//
//	guVecMultiplySR( GetcameraMatrix(), &ldir,&ldir);   
//	guVecMultiply( GetcameraMatrix(), &lpos, &lpos);   
//	GX_InitLightDirv( &lobj, &ldir);    
//	GX_InitLightPosv( &lobj, &lpos);    
//
//	static const GXColor LightColour  = { 255, 255, 255, 0xFF };
//	GX_InitLightColor( &lobj, LightColour );      
//
//	//this is just for code readers, wanting to know how to use direct cut off    
//	//GX_InitLightSpot(&lobj, 0<angle<90, GX_SP_FLAT);     
//	GX_InitLightAttn(&lobj, angAttn0, angAttn1, angAttn2, distAttn0, distAttn1, distAttn2);   
//	GX_LoadLightObj(&lobj, GX_LIGHT0 );   // light zero 
//
//	// Turn light ON      
//	GX_SetNumChans(1);  
//	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, 
//					GX_LIGHT0,   // light zero
//					GX_DF_CLAMP, GX_AF_SPOT); 
//}  
//
//void Camera::SetLightSpec(u8 num, guVector dir, f32 shy) 
//{    
//	Mtx mr,mv;   
//	GXLightObj MyLight;
////	guVector ldir = {dir.x, dir.y, dir.z}; 
////	GRRLIB_Settings.lights |= (1<<num);   
//	guMtxInverse(GetcameraMatrix(),mr);   
//	guMtxTranspose(mr,mv);  
//	guVecMultiplySR(mv, &dir,&dir); 
//	GX_InitSpecularDirv(&MyLight, &dir);    
//	GX_InitLightShininess(&MyLight, shy); 
//	// between 4 and 255 !!!    
//	
//	static const GXColor LightColour  = { 0xff, 0xff, 0xff, 0xFF };
//
//	GX_InitLightColor(&MyLight, LightColour );   
//	GX_LoadLightObj(&MyLight, (1<<num));   
//	/////////////////////// Turn light ON ////////////////////////////////////////////////   
//	GX_SetNumChans(2);   
//	// use two color channels   
//	GX_SetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);  
//	GX_SetChanCtrl(GX_COLOR1, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_NONE, GX_AF_SPEC);  
//	GX_SetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);   
//	GX_SetChanCtrl(GX_ALPHA1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);    
//	GX_SetNumTevStages(2);      GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0 ); 
//	GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR1A1 );    
//	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR); 
//	GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV );     
//	GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_CPREV );   
//	/////////////////////// Define Material and Ambient color and draw object /////////////////////////////////////    
//	GX_SetChanAmbColor(GX_COLOR1, (GXColor){0x00,0x00,0x00,0xFF});  // specular ambient forced to black     
//
//	static const GXColor SpecColour  = { 0xff, 0x0, 0x0, 0xFF };
//	GX_SetChanMatColor(GX_COLOR1, SpecColour ); 
//}  
//
//

void  Camera::SetLightDiff(u8 num, guVector pos, f32 distattn, f32 brightness) 
{
    GXLightObj MyLight;
    //guVector lpos = {pos.x, pos.y, pos.z};
    guVecMultiply(GetcameraMatrix(), &pos, &pos);
    GX_InitLightPos(&MyLight, pos.x, pos.y, pos.z);
	GX_InitLightColor(&MyLight, (GXColor) { 0xff, 0xff, 0xff, 0xff }); //does this do anything???
    GX_InitLightSpot(&MyLight, 0.0f, GX_SP_OFF);
    GX_InitLightDistAttn(&MyLight, distattn, brightness, GX_DA_MEDIUM); // DistAttn = 20.0  &  Brightness=1.0f (full)
    GX_LoadLightObj(&MyLight, (1<<num));
    GX_SetNumChans(1);
	m_UsedLightMask |= (1<<num);
    GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, m_UsedLightMask, GX_DF_CLAMP, GX_AF_SPOT);
}

void Camera::SetLightOff() 
{
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR); 
	GX_SetNumTevStages(1);
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}
