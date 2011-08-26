#include "Util3D.h"
#include "Singleton.h"
#include "WiiManager.h"
#include <gccore.h>


void Util3D::Trans(f32 xpos, f32 ypos)
{
	Mtx Matrix;
	guMtxIdentity(Matrix);
	guMtxTrans(Matrix,xpos, ypos, 0 );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),Matrix,Matrix);
	GX_LoadPosMtxImm (Matrix, GX_PNMTX0); 
}

void Util3D::Trans(f32 xpos, f32 ypos, f32 zpos)
{
	Mtx Matrix;
	guMtxIdentity(Matrix);
	guMtxTrans(Matrix,xpos, ypos, zpos );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),Matrix,Matrix);
	GX_LoadPosMtxImm (Matrix, GX_PNMTX0); 
}

void Util3D::TransRot(f32 xpos, f32 ypos, f32 z , f32 rad)
{
	Mtx FinalMatrix,TransMatrix;
	guMtxRotRad(TransMatrix,'Z',rad);  // Rotage
	guMtxTransApply(TransMatrix,TransMatrix,xpos, ypos, z );	// Position
	guMtxConcat(Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(),TransMatrix,FinalMatrix);
	GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
}

void Util3D::TransRot(f32 xpos, f32 ypos, f32 rad)
{
	Mtx FinalMatrix,TransMatrix;
	guMtxRotRad(TransMatrix,'Z',rad);  // Rotage
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


void Util3D::Identity()
{
	GX_LoadPosMtxImm (Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(), GX_PNMTX0); 
}

void Util3D::MatrixRotateX(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) );
	// X
	mt[0][0] =  1.0f;		mt[0][1] =  0.0f;			mt[0][2] =  0.0f;		mt[0][3] = 0.0f;
	mt[1][0] =  0.0f;		mt[1][1] =  CosValue;		mt[1][2] = -SinValue;	mt[1][3] = 0.0f;
	mt[2][0] =  0.0f;		mt[2][1] =  SinValue;		mt[2][2] =  CosValue;	mt[2][3] = 0.0;
};

void Util3D::MatrixRotateZ(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) ); 
	// Z
	mt[0][0] =  CosValue;	mt[0][1] = -SinValue;		mt[0][2] =  0.0f;		mt[0][3] = 0.0f;
	mt[1][0] =  SinValue;	mt[1][1] =  CosValue;		mt[1][2] =  0.0f;		mt[1][3] = 0.0f;
	mt[2][0] =  0.0f;		mt[2][1] =  0.0f;			mt[2][2] =  1.0f;		mt[2][3] = 0.0f;
};

void Util3D::MatrixRotateY(Mtx mt,f32 rad)
{
	f32 SinValue( sinf(rad) );
	f32 CosValue( cosf(rad) );
	// Y
	mt[0][0] =  CosValue;	mt[0][1] =  0.0f;			mt[0][2] =  SinValue;	mt[0][3] = 0.0f;
	mt[1][0] =  0.0f;		mt[1][1] =  1.0f;			mt[1][2] =  0.0f;		mt[1][3] = 0.0f;
	mt[2][0] = -SinValue;	mt[2][1] =  0.0f;			mt[2][2] =  CosValue;	mt[2][3] = 0.0f;
};