#include <gccore.h>
//#include <math.h>
//#include "Util.h"
//#include "config.h"
//#include "ogc\lwp_watchdog.h"
//#include "ogc\system.h"
//#include "ogcsys.h"
//#include "wiiuse\wpad.h"
//#include <sstream>
//#include <iomanip>
#include "Draw_Util.h"

void Draw_Util::DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b)
{
	DrawRectangle(xpos, ypos, w, h,Alpha, r,g, b,r,g,b );
}

void Draw_Util::DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b,u8 r2, u8 g2, u8 b2  )
{	
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	float z=0; 

	GX_Begin(GX_QUADS, GX_VTXFMT3,4);		

	GX_Position3f32(xpos, ypos,z);		
	GX_Color4u8(r,g,b,Alpha);        

	GX_Position3f32(xpos+(w), ypos,z);
	GX_Color4u8(r,g,b,Alpha);     

	GX_Position3f32(xpos+(w), ypos+(h),z);         
	GX_Color4u8(r2,g2,b2,Alpha);     

	GX_Position3f32(xpos, ypos+(h),z);
	GX_Color4u8(r2,g2,b2,Alpha);  

	GX_End();
} 