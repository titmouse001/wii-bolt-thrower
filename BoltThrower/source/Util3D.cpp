#include "Util3D.h"
#include "Singleton.h"
#include "WiiManager.h"
#include "camera.h"
#include <gccore.h>


void Util3D::Trans(f32 xpos, f32 ypos)
{
	Mtx Matrix, Final;
	guMtxIdentity(Matrix);
	guMtxTrans(Matrix,xpos, ypos, 0 );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),Matrix,Final);
	GX_LoadPosMtxImm (Final, GX_PNMTX0); 
}

void Util3D::Trans(f32 xpos, f32 ypos, f32 zpos)
{
	Mtx Matrix, Final;
	guMtxIdentity(Matrix);
	guMtxTrans(Matrix,xpos, ypos, zpos );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),Matrix,Final);
	GX_LoadPosMtxImm (Final, GX_PNMTX0); 
}

void Util3D::TransRot(f32 xpos, f32 ypos, f32 z , f32 rad)
{
	Mtx FinalMatrix,TransMatrix;
	MatrixRotateZ(TransMatrix,rad);
	//guMtxRotRad(TransMatrix,'Z',rad);  // Rotage
	guMtxTransApply(TransMatrix,TransMatrix,xpos, ypos, z );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),TransMatrix,FinalMatrix);
	GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
}

void Util3D::TransRot(f32 xpos, f32 ypos, f32 rad)
{
	Mtx FinalMatrix,TransMatrix;
	MatrixRotateZ(TransMatrix,rad);
	//guMtxRotRad(TransMatrix,'Z',rad);  // Rotage
	guMtxTransApply(TransMatrix,TransMatrix,xpos, ypos, 0.0f );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),TransMatrix,FinalMatrix);
	GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
}

void Util3D::TransScale(f32 xpos, f32 ypos, f32 zpos , f32 scale)
{
	Mtx FinalMatrix,TransMatrix;
	guMtxScale(TransMatrix,scale,scale,scale);
	guMtxTransApply(TransMatrix,TransMatrix,xpos, ypos, zpos );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),TransMatrix,FinalMatrix);
	GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
}


void Util3D::CameraIdentity()
{
	GX_LoadPosMtxImm (Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(), GX_PNMTX0); 
}

void Util3D::MatrixRotateX(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) );
	//sincosf(rad,&SinValue,&CosValue);
	// X
	mt[0][0] =  1.0f;		mt[0][1] =  0.0f;			mt[0][2] =  0.0f;		mt[0][3] = 0.0f;
	mt[1][0] =  0.0f;		mt[1][1] =  CosValue;		mt[1][2] = -SinValue;	mt[1][3] = 0.0f;
	mt[2][0] =  0.0f;		mt[2][1] =  SinValue;		mt[2][2] =  CosValue;	mt[2][3] = 0.0;
};

void Util3D::MatrixRotateZ(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) ); 
	//sincosf(rad,&SinValue,&CosValue);
	// Z
	mt[0][0] =  CosValue;	mt[0][1] = -SinValue;		mt[0][2] =  0.0f;		mt[0][3] = 0.0f;
	mt[1][0] =  SinValue;	mt[1][1] =  CosValue;		mt[1][2] =  0.0f;		mt[1][3] = 0.0f;
	mt[2][0] =  0.0f;		mt[2][1] =  0.0f;			mt[2][2] =  1.0f;		mt[2][3] = 0.0f;
};

void Util3D::MatrixRotateY(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) );
	//sincosf(rad,&SinValue,&CosValue);
	// Y
	mt[0][0] =  CosValue;	mt[0][1] =  0.0f;			mt[0][2] =  SinValue;	mt[0][3] = 0.0f;
	mt[1][0] =  0.0f;		mt[1][1] =  1.0f;			mt[1][2] =  0.0f;		mt[1][3] = 0.0f;
	mt[2][0] = -SinValue;	mt[2][1] =  0.0f;			mt[2][2] =  CosValue;	mt[2][3] = 0.0f;
};

//
//
//
//void Util3D::MatrixConcat(Mtx a,Mtx b,Mtx ab)
//{
//    ab[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0];
//    ab[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1];
//    ab[0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2];
//    ab[0][3] = a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3];
//
//    ab[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0];
//    ab[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1];
//    ab[1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2];
//    ab[1][3] = a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3];
//
//    ab[2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0];
//    ab[2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1];
//    ab[2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2];
//    ab[2][3] = a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3];
//}
//
//
//void Util3D::TransApply(Mtx m,f32 x,f32 y,f32 z)
//{
//	m[0][3] += x;
//	m[1][3] += y;
//	m[2][3] += z;
//}
//
//void Util3D::ScaleApply(Mtx m,f32 xS,f32 yS,f32 zS)
//{
//	m[0][0] *= xS;     m[0][1] *= xS;
//	m[0][2] *= xS;     m[0][3] *= xS;
//
//	m[1][0] *= yS;     m[1][1] *= yS;
//	m[1][2] *= yS;     m[1][3] *= yS;
//
//	m[2][0] *= zS;     m[2][1] *= zS;
//	m[2][2] *= zS;     m[2][3] *= zS;
//}


//  -ffast-math



////
//float Util3D::sine(float x) 
//{ 
//	return sin(x);
//
//    static const float B = 4.0f/M_PI; 
//    static const float C = -4.0f/(M_PI*M_PI); 
//    float y = B * x + C * x * abs(x); 
////    #ifdef EXTRA_PRECISION 
//    //  const float Q = 0.775; 
//	const float P = 0.225f; 
//	y = P * (y * abs(y) - y) + y;   // Q * y + P * y * abs(y) 
////    #endif 
//	return y;
//}
////
////float Util3D::cosine(float x) 
////{     
////	return cos(x);
////
////	//return sine(x + (M_PI / 2.0f)); 
////} 