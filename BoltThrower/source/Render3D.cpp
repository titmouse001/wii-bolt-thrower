#include "Render3D.h"
#include "DEBUG.h"
#include "Image.h"
#include "Util.h"
#include "WiiManager.h"
#include <string>
#include <gccore.h>
#include <math.h>
//#include <vector>

//#define HACK_COLOR_OR_TEXTURE (0)

//
// To keep this simple only one texture is used per 3d object
// the loading code does support more, but the display does not.
//
// FUTURE STUFF: * To support multiple texures maybe fit them into one large texture and change the UV's - good for display lists
//				 * Supporting multiple texutes for non display list (coded display) would be less of a problem.
//				 * Don't like the idea of separate textures for a display list - just a bad idea

void Render3D::CreateDisplayList(std::string ModelName)
{
	// Work arround... here we use far more memory than we really need, once GX_EndDispList is called
	// we have the real size we can re-alloc copy the display list and then free up this working memory.
	static const int MAXDISPLAYSIZE(1024*(256*4));
	void* TempMem = memalign(32,MAXDISPLAYSIZE);
	if (TempMem==NULL) 
		ExitPrintf("Display List failed - part1");

	float Radius = 0;

	memset(TempMem,0,MAXDISPLAYSIZE);
	DCInvalidateRange(TempMem,MAXDISPLAYSIZE);
	GX_BeginDispList(TempMem,MAXDISPLAYSIZE);
	for (std::vector<Object3D>::iterator  AllModelsIter = GetObjectBegin();AllModelsIter!=GetObjectEnd(); ++AllModelsIter)
	{
		if (AllModelsIter->GetName() == ModelName) 
		{

			//printf ("\n\n%s has %d points in model\n", AllModelsIter->GetName().c_str(),AllModelsIter->GetTotalPointsInObject());
			//GX_Begin(GX_LINESTRIP, GX_VTXFMT5,	AllModelsIter->GetTotalPointsInObject() );
			
//			if (HACK_COLOR_OR_TEXTURE) // pos,nrm,col - colour+alpha with normals
				GX_Begin(GX_TRIANGLES, GX_VTXFMT4,	AllModelsIter->GetTotalPointsInObject() );
//			else // pos,nrm,tex - texture with normals
//				GX_Begin(GX_TRIANGLES, GX_VTXFMT5,	AllModelsIter->GetTotalPointsInObject() );
		

			for ( VectorOfPolyXYZ::iterator iter(AllModelsIter->GetPolyBegin()) ; iter!=AllModelsIter->GetPolyEnd() ; ++iter)
			{
				for ( VectorOfPointXYZ::iterator iterPoints(iter->GetPointBegin()) ; iterPoints!=iter->GetPointEnd() ; ++iterPoints)
				{
					GX_Position3f32(iterPoints->Getx(),iterPoints->Gety(), iterPoints->Getz()); 
					// use inverted norms
					GX_Normal3f32(iterPoints->GetNormX(),iterPoints->GetNormY(), iterPoints->GetNormZ()); 
					
//					if (HACK_COLOR_OR_TEXTURE) // pos,nrm,col - colour+alpha with normals
						GX_Color4u8(255,255,255,255);
//					else // pos,nrm,tex - texture with normals
						GX_TexCoord2f32(iterPoints->Getu(),iterPoints->Getv());
				}
			}
			GX_End();
			Radius = AllModelsIter->GetRadius();
			break;
		}
	}

	int dispSize = GX_EndDispList();
	int size = dispSize+31; // +31 - read somewhere this was needed as a work around... not tried it without yet so may not even be true.

	//printf("%s: 3D buffer memory %d for copying %d",ModelName.c_str() , MAXDISPLAYSIZE , size);
	if (size > MAXDISPLAYSIZE)
		ExitPrintf("More buffer memory needed, using %d but need %d", MAXDISPLAYSIZE , size);

	void* dispList = memalign(32,size);
	if (dispList==NULL) 
		ExitPrintf("Display List failed - part2");

	memset(dispList,0,size);
	memcpy(dispList,TempMem,dispSize);
	//	printf ("\nAllocated %d bytes for display list\n\n\n",dispSize);
	DCFlushRange((void*)dispList, dispSize);
	free(TempMem);
	AddModel( (HashLabel)ModelName, dispList, dispSize, Radius ); 

	CurrentMemoryUsedForDisplayList += size;
	//printf("CurrentMemoryUsedForDisplayList: %d", CurrentMemoryUsedForDisplayList);


	// clean up ... remove any data thats no longer needed, as we have created a display list
	for (std::vector<Object3D>::iterator Iter(GetObjectBegin());Iter!=GetObjectEnd(); ++Iter)
	{
		if (Iter->GetName() == ModelName) 
		{
//need flag to keep this sometimes
//			Iter->ClearPloyContainer();
//			Iter->ClearTextureFullFileNamesContainer();
			break;
		}
	}
}

void Render3D::AddModel(HashLabel ModelName, void* DispList, int DispListSize, float Radius)
{
	DisplayListInfo Info;
	Info.m_dispList = DispList;
	Info.m_dispSize = DispListSize;
	Info.m_Radius = Radius;
	m_DispayListContainer[ModelName] = Info;
}

void Render3D::RenderModel(HashLabel ModelName, Mtx& ModelView)
{	
	RenderModelPreStage(ModelName);
	RenderModelMinimal(ModelName,ModelView);
}

void Render3D::RenderModelHardNorms(HashLabel ModelName, Mtx& ModelView)
{	
	RenderModelPreStage(ModelName);
	RenderModelMinimalHardNorms(ModelName,ModelView);
}

void Render3D::RenderModelPreStage(HashLabel ModelName)
{	
	SetTextureFromModelName(ModelName, GX_PNMTX0);

	//GX_SetNumTexGens(1);
	//GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
//	if (HACK_COLOR_OR_TEXTURE)
	{
		//GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	}
	//else
	{
		GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	}
}

void Render3D::RenderModelMinimal(HashLabel ModelName, Mtx& ModelView)
{	
	GX_LoadPosMtxImm(ModelView, GX_PNMTX0);
	Mtx mat;
	guMtxInverse(ModelView,mat);
	guMtxTranspose(mat,ModelView);
	GX_LoadNrmMtxImm(ModelView, GX_PNMTX0);
	DisplayListInfo& Info( m_DispayListContainer[ModelName] );
	GX_CallDispList(Info.m_dispList,Info.m_dispSize);
}

void Render3D::RenderModelMinimalHardNorms(HashLabel ModelName, Mtx& ModelView)
{	
	GX_LoadPosMtxImm(ModelView, GX_PNMTX0);
	GX_LoadNrmMtxImm(ModelView, GX_PNMTX0);
	DisplayListInfo& Info( m_DispayListContainer[ModelName] );
	GX_CallDispList(Info.m_dispList,Info.m_dispSize);
}

void Render3D::SetTextureFromModelName(HashLabel Name, int GX_TexMap)
{
	for (std::vector<Object3D>::iterator AllModelsIter( GetObjectBegin()); AllModelsIter!=GetObjectEnd(); ++AllModelsIter)
	{
		if ((HashLabel)AllModelsIter->GetName() == Name) 
		{
			GX_LoadTexObj( AllModelsIter->GetTexture()->GetHardwareTextureInfo(), GX_TexMap);   
			return;
		}
	}

	// DEBUG ... NEVER HIT (YET!)
	//for (std::vector<Object3D>::iterator  AllModelsIter = GetObjectBegin(); AllModelsIter!=GetObjectEnd(); ++AllModelsIter)
	//{
	//	printf ( "From HERE: %s", AllModelsIter->GetName().c_str() ); 
	//}
	//ExitPrintf("Texture not found");
}

void Render3D::LoadTextures(Object3D& Obj3D)
{
	if (Obj3D.IsFullFileNameEmpty())
	{
		Obj3D.AddTexture( new Image(16, 16) );  // missing texture... lets give it one - the missing texture is easily spotted
	}
	else
	{
		for (VectorOfStrings::const_iterator iter(Obj3D.GetFullFileNameBegin()) ; iter!=Obj3D.GetFullFileNameEnd(); ++iter)
		{
			std::string FileNameWithPath = *iter;
			std::string basename ( FileNameWithPath.substr( FileNameWithPath.find_last_of( '/' ) +1 ) );  

			// check for route, e.g. "C:thing.tga"
			basename = basename.substr( basename.find_last_of( ':' ) +1 );  
						


			std::string LongFileName( WiiFile::GetGamePath() + "lwo/textures/" + basename );
			const HashLabel Hash((HashLabel)LongFileName);

			Image* pImage(NULL);
			std::vector<Object3D>::iterator  AllModelsIter(GetObjectBegin());
			do 
			{
				for (vector<Image*>::iterator CheckIter(AllModelsIter->GetImageDataBegin()); CheckIter!=AllModelsIter->GetImageDataEnd(); ++CheckIter)
				{
					if ( (*CheckIter)->GetFileName() == Hash)
					{
						pImage = *CheckIter;  // found a matching image... just use it, no need to load it into memory
						break;  // found it - all done
					}
				}
				++AllModelsIter;
			} while ((AllModelsIter!=GetObjectEnd()) && (pImage==NULL));

			if (pImage == NULL)  // if nothing found then it doesn't exist, will need to load it as a new thing
			{
				pImage = new Image( LongFileName );
			}
			Obj3D.AddTexture( pImage );
		}
	}
}

//void CalcNormal(guVector* InVec1, guVector* InVec2, guVector* InVec3, guVector* OutVec)
//{
//	guVector Tempv1,Tempv2;
//	guVecSub(InVec3, InVec1, &Tempv1);
//	guVecSub(InVec3, InVec2, &Tempv2);
//	guVecCross(&Tempv2,&Tempv1,OutVec);
//	guVecNormalize(OutVec);
//}

Object3D* Render3D::Add3DObject(std::string FullFileName, bool bSmooth3DObject, int IndexLayerForBones)
{
	unsigned int failID;
	int failpos;


	lwObject* obj ( lwGetObject( FullFileName.c_str(), &failID, &failpos ) );
	if (obj == NULL)
	{
		ExitPrintf("lwGetObject failed:%s",FullFileName.c_str());
	}

	// Fist step, Store all texture file names that are used inside this 3dObject
	// A texture index is stored for each poly to ref this
	Object3D  TempWorking3DObject;
	lwClip* pClip( obj->clip );
	while (pClip!=NULL)
	{
		TempWorking3DObject.AddFullFileName( pClip->source.still.name  ); 
		pClip = pClip->next;
	} 

	// Second step, looping through the polygons storing points, Uv's and colours.
	lwLayer* pLayer( obj->layer ); // holds things like surface name , colours, index into textures
	//printf ("%f %f %f %f %f %f",pLayer->bbox[0],pLayer->bbox[1],pLayer->bbox[2],pLayer->bbox[3],pLayer->bbox[4],pLayer->bbox[5]);
	float Radius(pLayer->bbox[0]);


	//?????WHAT ABOUT MEMORY REQUIREMENTS FOR 3D STUIFF?????
	for ( int indexLayers = 0; indexLayers < obj->nlayers; indexLayers++ ) 
	{
		//printf("polygon.count: %d",pLayer->polygon.count);

		if (IndexLayerForBones!=-1)
		{
			if (indexLayers != IndexLayerForBones)
			{
				pLayer = pLayer->next;
				continue;
			}
		}

		//printf("processing LAYER: %d",indexLayers);
		//keep stuffing until there are no layers left....

		for (int PolygonNum(0); PolygonNum < pLayer->polygon.count ; PolygonNum++)  // look in all polygons
		{
			lwPolygon* pPoly ( &pLayer->polygon.pol[PolygonNum] );

			std::string Sname = pPoly->surf->name;
			std::string Vmapname;
			if (pPoly->surf->color.tex !=NULL)
			{
				Vmapname = pPoly->surf->color.tex->param.imap.vmap_name;
			}

			int cindex = 0;
			if (pPoly->surf->color.tex!=NULL)
			{
				cindex = pPoly->surf->color.tex->param.imap.cindex - 1;  // cindex starts from 1, we need our index to start from zero
			}

			PolyXYZ TempPoly;
			TempPoly.ClearPoints();


			////----------------------
			////TEST CODE - calc norms - this is just to check that the incoming data is what I think it is!
			//std::vector<guVector> Pointcontainer;
			//Pointcontainer.clear();
			//for ( int k = 0; k < pPoly->nverts; k++ ) 
			//{
			////	printf("%d",pPoly->nverts);
			//	/* point index, position, number of vmads and vmaps */
			//	int n1 ( pPoly->v[ k ].index );
			//	lwPoint* pt = &pLayer->point.pt[ n1 ];

			//	//PointXYZ pointdata ( pt->pos[ 0 ], pt->pos[ 1 ], pt->pos[ 2 ] );  // ahhhhhhhhhhhhhhhhhhhhhhhhhhhhh...took make ages to work this one out
			//	PointXYZ pointdata ( pt->pos[ 2 ], pt->pos[ 1 ], pt->pos[ 0 ] ); // poly faces camera using this order

			//	guVector v;
			//	v.x=pointdata.Getx();
			//	v.y=pointdata.Gety();
			//	v.z=pointdata.Getz();
			//	Pointcontainer.push_back(v);
			//}

			////calc normal - tested works -same values as LW pumps out but I'ven not saved in LW with smooth normals - if even possible)
			//guVector v1,v2,res;
			//guVecSub(&Pointcontainer[2], &Pointcontainer[0], &v1);
			//guVecSub(&Pointcontainer[2], &Pointcontainer[1], &v2);
			//guVecCross(&v2,&v1,&res);
			//guVecNormalize(&res);
			////---------------------


			for ( int k = 0; k < pPoly->nverts; k++ ) 
			{
				/* point index, position, number of vmads and vmaps */
				int n1 ( pPoly->v[ k ].index );
				lwPoint* pt = &pLayer->point.pt[ n1 ];

				//PointXYZ pointdata ( pt->pos[ 0 ], pt->pos[ 1 ], pt->pos[ 2 ] );  // ahhhhhhhhhhhhhhhhhhhhhhhhhhhhh...took make ages to work this one out
				PointXYZ pointdata ( pt->pos[ 2 ], pt->pos[ 1 ], pt->pos[ 0 ] ); // poly faces camera using this order

				pointdata.SetColour( (unsigned char) (pPoly->surf->color.rgb[2] * 255.0f), 
					(unsigned char) (pPoly->surf->color.rgb[1] * 255.0f), 
					(unsigned char) (pPoly->surf->color.rgb[1] * 255.0f) );

				if (bSmooth3DObject)
				{
					pointdata.SetNorms( pt->pos[ 2 ], pt->pos[ 1 ], pt->pos[ 0 ] );
				}
				else
				{
					// load or calculate gives same results...  
					//printf("new %f %f %f",res.x,res.y,res.z);
					//printf("old %f %f %f",pPoly->v[k].norm[2]),( pPoly->v[k].norm[1]),( pPoly->v[k].norm[0]);
					//pointdata.SetNorms(res.x,res.y,res.z);

					// normals are already loaded with the 3d object, so might as well use them
					pointdata.SetNorms(( pPoly->v[k].norm[2]),( pPoly->v[k].norm[1]),( pPoly->v[k].norm[0]));
				}

				// bugger and again ... looks like I need to flip these normals.. above looks good now
					//pointdata.SetNorms(( pPoly->v[k].norm[0]),( pPoly->v[k].norm[1]),( pPoly->v[k].norm[2]));



				for ( int j = 0; j < pt->nvmaps; j++ )  /* vmaps for this vertex */
				{
					lwVMap* vmap = pt->vm[ j ].vmap;
					if ( vmap->perpoly == 0 )
					{
						int n = pt->vm[ j ].index;
						/* vmap values */
						pointdata.SetUV(vmap->val[ n ][ 0 ],  1.0f - vmap->val[ n ][ 1 ]);
					}
				}

				/* Over write vmaps using these vmads - corrects things like textured balls at edges*/
				for (int j = 0; j < pPoly->v[ k ].nvmaps; j++ ) 
				{
					lwVMap* vmap = pPoly->v[ k ].vm[ j ].vmap;
					if (Vmapname == vmap->name)
					{
						int n = pPoly->v[ k ].vm[ j ].index;
						/* vmads but same as vmap values */
						pointdata.SetUV(vmap->val[ n ][ 0 ], 1.0f - vmap->val[ n ][ 1 ]);

						break;
					}
				}
				TempPoly.AddPoint( pointdata );
				TempPoly.SetTextureIndex( cindex ); //,Vmapname);

				// ready to add one completed polygon?
				if (TempPoly.GetTotalPoints() == 3)  // Tri poly
				{
					TempWorking3DObject.AddPloy( TempPoly );

					// Found 3 points,  Tri poly completed.
					// From now on we only need 1 point added per loop -  note: points are in a clockwise order
					TempPoly.RemovePoint(1); 


				//	if (IndexLayerForBones!=-1)
				//		printf("PolygonNum %d",PolygonNum);
				}
			}
		}
		pLayer = pLayer->next;
	}

	lwFreeObject(obj);
	Object3D& obj3D = AddObjectToList(TempWorking3DObject);

	// [+BONE] becomes [BONE] at some point when stored in mem
	//if (!FullFileName.find("[+BONE]")!=string::npos)  // "[+BONE]" filename convention for spotting lwo with bones
		LoadTextures( obj3D );

	obj3D.SetTotalPointsInObject( obj3D.CalcTotalPointsInObject() );
	obj3D.SetRadius( fabs(Radius) );

	return &obj3D;
}

int Object3D::CalcTotalPointsInObject()
{
	int Total(0);
	for ( VectorOfPolyXYZ::const_iterator iter(GetPolyBegin()) ; iter!=GetPolyEnd() ; ++iter)
	{
		Total += iter->GetTotalPoints();
	}

	return Total;
}

void Render3D::DrawModelPoints(std::string ModelName, float Rot, float dist)
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );

	for (std::vector<Object3D>::iterator  AllModelsIter = GetObjectBegin();AllModelsIter!=GetObjectEnd(); ++AllModelsIter)
	{
		if (AllModelsIter->GetName() == ModelName) 
		{
			for ( VectorOfPolyXYZ::iterator iter(AllModelsIter->GetPolyBegin()) ; iter!=AllModelsIter->GetPolyEnd() ; ++iter)
			{
				for ( VectorOfPointXYZ::iterator iterPoints(iter->GetPointBegin()) ; iterPoints!=iter->GetPointEnd() ; ++iterPoints)
				{
					Mtx Model,mat;
					guMtxTrans(Model,iterPoints->Getx(),iterPoints->Gety(),iterPoints->Getz());
					guMtxRotRad(mat,'y', Rot);
					guMtxConcat(mat,Model,Model);
					guMtxTransApply(Model,Model,0,0,dist);
					guMtxConcat(Wii.GetCamera()->GetcameraMatrix(),Model,Model);
					RenderModel(HashString::ShieldGenerator,Model);
				}
			}
		}
	}
}


VectorOfPointXYZ Render3D::GetModelPointsFromLayer(std::string ModelName)
{
	VectorOfPointXYZ Points;
	Points.clear();
	Object3D* pModel(GetModel(ModelName));
	if (pModel!=NULL)
	{
		for ( VectorOfPolyXYZ::iterator iter(pModel->GetPolyBegin()) ; iter!=pModel->GetPolyEnd() ; ++iter)
		{
			VectorOfPointXYZ::iterator iterPoints(iter->GetPointBegin());
			PointXYZ p= *iterPoints;
			Points.push_back(p);
		}
	}
	return Points;
}


Object3D* Render3D::GetModel(std::string ModelName)
{
	for (std::vector<Object3D>::iterator  AllModelsIter = GetObjectBegin();AllModelsIter!=GetObjectEnd(); ++AllModelsIter)
	{
		if (AllModelsIter->GetName() == ModelName) 
		{
			return &(*AllModelsIter);
		}
	}
	return NULL;
}