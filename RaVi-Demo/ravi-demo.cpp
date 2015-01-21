// Realtime Raytracing Vizualizer "RaVi" Demo V0.37
// ------------------------------------------------
// (C) 01/14/2005                     Michael Granz

#include "stdafx.h"
#include "RaVi-Demo.h"

#include "MainFrm.h"

#include "general.h"
#include "materials.h"
#include "RaVi.h"

#ifdef _DEBUG
  #define new DEBUG_NEW
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#define HBlockSize 4096
#define QBlockSize 1024

extern CStatusBar *StatusBar;
       CMainFrame *Win;

extern RV_PrP    RV_LtObj;
extern vector   *RV_CamPtr;

short int        RV_fps = 0;

FILE     *GraphFile;
	   
short     Animation = 1;
short     Graph     = 0;
short     Demo      = 1;

float     wavlen = 555.0f;

float     C1=1.3f;
float     C2=0.3f;

MatPtr    MatList  = NULL;

Material  ENVIRONMENT;

matrix    ActM,ActB;
RV_PrP    Act1,Act2,Act3,Act4,Act5,Act6,Act7;

PHP       PHB[2000]; int PB;
QLP       QLB[2000]; int QB,QBp;

QLP       QLT; unsigned long int QLc=0;

int RV_Caustics_Samples=10000;
int RV_Caustics_Quality=   30;

int RV_GI_Samples=500000;
int RV_GI_Quality=  2500;

int RV_GIOnlyIndirect= 1;

QLP QAlloc()
{
  if(QBp>=QBlockSize){
	QB++; QBp=0;
	if(!QLB[QB]) QLB[QB]=new QLst[QBlockSize];
  }
  QLc++;
  return QLB[QB]+(QBp++);
}

float wlcoff(float cR, float cG, float cB, float v);

RGB_Color WL2RGB(float v)
{
  RGB_Color Color;
  Color.R = wlcoff(0.68292f, 0.0f, 0.0f, v);
  Color.G = wlcoff(0.0f, 1.00000f, 0.0f, v);
  Color.B = wlcoff(0.0f, 0.0f, 1.16667f, v);
  return Color;
}

void StoreHit(QLP QL, float xb, float yb, float l, PHP Hit, int Quality)
{
  if(!QL) return;
  if((QL->cnt < Quality) || (l < 0.001f)){
	Hit->nxt=QL->Hits; QL->Hits=Hit;
	QL->val = QL->val + (Hit->v * WL2RGB(Hit->w)); QL->cnt++;
  } else {
	float l2=0.5f*l;
	if(Hit->y<(yb+l2)){
	  if(Hit->x<(xb+l2)){
		if(!(QL->BL)){
          QL->BL=QAlloc(); QL->BL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
          QL->BR=QAlloc(); QL->BR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
          QL->TL=QAlloc(); QL->TL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
          QL->TR=QAlloc(); QL->TR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
		}
		StoreHit(QL->BL, xb   , yb   , l2, Hit, Quality);
	  } else {
		if(!(QL->BR)){
          QL->BL=QAlloc(); QL->BL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
          QL->BR=QAlloc(); QL->BR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
          QL->TL=QAlloc(); QL->TL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
          QL->TR=QAlloc(); QL->TR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
		}
		StoreHit(QL->BR, xb+l2, yb   , l2, Hit, Quality);
	  }
	} else {
	  if(Hit->x<(xb+l2)){
		if(!(QL->TL)){
          QL->BL=QAlloc(); QL->BL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
          QL->BR=QAlloc(); QL->BR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
          QL->TL=QAlloc(); QL->TL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
          QL->TR=QAlloc(); QL->TR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
		}
		StoreHit(QL->TL, xb,    yb+l2, l2, Hit, Quality);
	  } else {
		if(!(QL->TR)){
          QL->BL=QAlloc(); QL->BL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
          QL->BR=QAlloc(); QL->BR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
          QL->TL=QAlloc(); QL->TL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
          QL->TR=QAlloc(); QL->TR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
		}
		StoreHit(QL->TR, xb+l2, yb+l2, l2, Hit, Quality);
	  }
	}
    if(QL->cnt == Quality){
	  QL->cnt=Quality+1; QL->val=RGB_Color(0.0f, 0.0f, 0.0f);
	  PHP o,p=QL->Hits;
	  while(p){
		o=p->nxt; StoreHit(QL, xb, yb, l, p, Quality); p=o;
	  }
	}
  }
}

/*void KillScene()
{
  RV_PrP pp;
  while(RV_Scene){
    pp=RV_Scene; RV_Scene = RV_Scene->next;
    delete(pp);
  } 
  MatPtr mp;
  while(MatList){
    mp=MatList; MatList = MatList->next;
    delete(mp);
  }
}*/

void InitSphereScene()
{
  RV_Lights->p = vector(5.0f, -3.0f, 7.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;
  RV_Target    = vector(0.0f,  0.0f, 1.2f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=1.30f; C2=0.30f;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.8f, 0.2f);
  mat->Specularity=0.5f; mat->type=0; mat->name = strdup("Metal");

  RV_SimpleSphere *s = new RV_SimpleSphere; s->Mat=mat; Act1=s;
  s->position=vector(0.0f, 0.0f, 1.0f); s->flags = RV_ReceiveNoShadow;
  PrLP o = new(PrLst); o->Prim = s;
  
         mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.2f, 0.3f, 0.8f);
  mat->Specularity=0.0f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); Win->SetWindowText("RAVI-Demo: Bouncing Mirrorsphere"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,3.0f); RV_LtObj=NULL;
  o->next = new(PrLst); o->next->Prim = p;
  o->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
}

void Init3SpheresScene()
{
  MatPtr mat; RV_SimpleSphere *s;
  RV_Target    = vector(0.0f,  0.0f, 0.0f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=1.91f; C2=0.61f;
  RV_Lights->p = vector(5.0f, -3.0f, 7.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;
  
  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.8f, 0.4f);
  mat->Specularity=0.5f; mat->type=0; mat->name = strdup("Gold");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act1=s;
  s->position=vector(1.0f, 0.0f, 0.5f); s->flags = RV_ReceiveNoShadow;
  PrLP o = new(PrLst); o->Prim = s;
  
  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.8f, 0.8f, 0.7f);
  mat->Specularity=0.5f; mat->type=0; mat->name = strdup("Silver");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act2=s;
  s->position=vector(cosf(2.0944f), sinf(2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next = new(PrLst); o->next->Prim = s;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.6f, 0.5f);
  mat->Specularity=0.5f; mat->type=0; mat->name = strdup("Copper");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act3=s;
  s->position=vector(cosf(-2.0944f), sinf(-2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next = new(PrLst); o->next->next->Prim = s;
  
         mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.2f, 0.3f, 0.8f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); Win->SetWindowText("RAVI-Demo: 3 rotating Spheres"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,1.0f); RV_LtObj=NULL;
  o->next->next->next = new(PrLst); o->next->next->next->Prim = p;
  o->next->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
}

void InitEggScene()
{
  MatPtr mat; RV_ComplexSphere *b; RV_Ellipsoid *t;
  RV_Target    = vector(0.0f,  0.0f, 0.875f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=2.73f; C2=0.47f;
  RV_Lights->p = vector(5.0f, -3.0f, 7.000f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;
  
  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.2f, 0.9f, 0.8f); mat->Transmission=RGB_Color(-1.0f, -1.0f, -1.0f); mat->IOR_base=1.54f;
  mat->Specularity=0.70f; mat->type=0; mat->name = strdup("Glass");

  b = new RV_ComplexSphere; b->r2=0.5625f; b->vangle=6.2831853f; b->ustop=0.0f; b->Mat=mat; Act1=b;
  b->position=vector(0.0f, 0.0f, 0.75f); b->flags = RV_ReceiveNoShadow; b->Tb.InitIdentMat(); b->Tb.Scale(0.75f, 0.75f, 0.75f); b->Tb.Translate(0.0f, 0.0f, 0.75f); b->Tb=b->Tb.Inverse();
  PrLP o = new(PrLst); o->Prim = b;
  
  t = new RV_Ellipsoid; t->vangle=6.2831853f; t->ustart=0.0f; t->Mat=mat; Act2=t;
  t->sinfo = new RV_GenShd;
  t->flags = RV_ReceiveNoShadow;  t->Tb.InitIdentMat(); t->Tb.Scale(0.75f, 0.75f, 1.0f); t->Tb.Translate(0.0f, 0.0f, 0.75f); t->sinfo->NM=t->Tb.NTrans(); t->Tb=t->Tb.Inverse();
  o->next = new(PrLst); o->next->Prim = t;

         mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.6f, 0.5f, 0.4f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat; Act7=p;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); Win->SetWindowText("RAVI-Demo: Tumbling Egg"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,1.751f); RV_LtObj=NULL;
  o->next->next = new(PrLst); o->next->next->Prim = p;
  o->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
}

void InitSphereHeapScene()
{
  MatPtr mat; RV_SimpleSphere *s;
  RV_Target    = vector(0.0f,  0.0f, 1.0f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=0.24f; C2=0.59f;
  RV_Lights->p = vector(5.0f, -3.0f, 7.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;
  
  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.8f, 0.4f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Gold");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act1=s;
  s->position=vector( 1.0f, 0.0f, 1.5f); s->flags = RV_ReceiveNoShadow;
  PrLP o = new(PrLst); o->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector( 1.0f, 0.0f, 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next = new(PrLst); o->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector(-1.0f, 0.0f, 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next = new(PrLst); o->next->next->Prim = s;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.8f, 0.8f, 0.7f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Silver");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act2=s;
  s->position=vector( cosf(2.0944f),  sinf(2.0944f), 1.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next = new(PrLst); o->next->next->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector( cosf(2.0944f),  sinf(2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next = new(PrLst); o->next->next->next->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector(-cosf(2.0944f), -sinf(2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->Prim = s;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.6f, 0.5f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Copper");
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat; Act3=s;
  s->position=vector( cosf(-2.0944f),  sinf(-2.0944f), 1.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector( cosf(-2.0944f),  sinf(-2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.25f; s->Mat=mat;
  s->position=vector(-cosf(-2.0944f), -sinf(-2.0944f), 0.5f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->Prim = s;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.7f, 0.5f, 0.9f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Metal");
  s = new RV_SimpleSphere; s->r2=0.6f; s->Mat=mat;
  s->position=vector(0.0f, 0.0f, 1.0f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->Prim = s;
  s = new RV_SimpleSphere; s->r2=0.2f; s->Mat=mat;
  s->position=vector(0.0f, 0.0f, 1.6f); s->flags = RV_ReceiveNoShadow;
  o->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->Prim = s;
  o->next->next->next->next->next->next->next->next->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
  Win->SetWindowText("RAVI-Demo: 3 rotating Spheres"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,2.5f); RV_LtObj=NULL;
}

void InitTorusScene()
{
  RV_Lights->p = vector(5.0f, -3.0f, 7.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;
  RV_Target    = vector(0.0f,  0.0f, 1.2f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=2.36f; C2=0.53f;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.9f, 0.5f, 0.2f);
  mat->Specularity=0.5f; mat->type=Shader_Phong; mat->name = strdup("Plastic");

  RV_Torus *t = new RV_Torus; t->Mat=mat; Act1=t;
  ActM.InitIdentMat(); ActM.RotateX(0.7853981634f); ActM.Translate(0.0f, 0.0f, 1.2071f);
  t->Tb=ActM.Inverse(); t->sinfo = new RV_GenShd; t->sinfo->NM = ActM.NTrans();
  PrLP o = new(PrLst); o->Prim = t;

         mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.2f, 0.3f, 0.8f);
  mat->Specularity=0.0f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); Win->SetWindowText("RAVI-Demo: rotating Torus"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,2.5f); RV_LtObj=NULL;
  o->next = new(PrLst); o->next->Prim = p;
  o->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
}

void InitCubeScene()
{
  RV_Target    = vector(0.0f,  0.0f, 1.2f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=-0.82f; C2=0.53f;
  RV_Lights->p = vector(5.0f, -3.0f, 7.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.6f, 0.3f, 0.2f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); PrLP o = new(PrLst); o->Prim = p; Act7=p; //o->next = NULL;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.2f, 0.9f, 0.8f); mat->Transmission=RGB_Color(-1.0f, -1.0f, -1.0f); mat->IOR_base=1.54f;
  mat->Specularity=0.70f; mat->type=0; mat->name = strdup("Glass");

  /*RV_Cube *c = new RV_Cube; c->Mat=mat; c->flags = RV_ReceiveNoShadow; Act1=c;
  ActM.InitIdentMat(); ActM.Scale(1.5f, 1.5f, 1.5f); ActM.RotateX(0.78539f); ActM.RotateY(0.61548f); ActM.Translate(0.0f, 0.0f, 1.3f);
  InitCube(c, ActM); o->next = new(PrLst); o->next->Prim = c; o->next->next=NULL;*/

  ActM.InitIdentMat(); ActM.Scale(1.5f, 1.5f, 1.5f); ActM.RotateX(0.78539f); ActM.RotateY(0.61548f); matrix NM=ActM; ActM.Translate(0.0f, 0.0f, 1.3f);

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act1=p;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg2=vector( 1.0f,  0.0f,  0.0f)*NM;
  p->edg1=vector( 0.0f,  1.0f,  0.0f)*NM;
  InitTP(p); o->next = new(PrLst); o->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act2=p;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg2=vector( 0.0f,  1.0f,  0.0f)*NM;
  p->edg1=vector( 0.0f,  0.0f,  1.0f)*NM;
  InitTP(p); o->next->next = new(PrLst); o->next->next->Prim = p;
  
  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act3=p;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg2=vector( 0.0f,  0.0f,  1.0f)*NM;
  p->edg1=vector( 1.0f,  0.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next = new(PrLst); o->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act4=p;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector(-1.0f,  0.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f, -1.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next = new(PrLst); o->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act5=p;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f, -1.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f,  0.0f, -1.0f)*NM;
  InitTP(p); o->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->Prim = p;
  
  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow; p->Mat=mat; Act6=p;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f,  0.0f, -1.0f)*NM;
  p->edg2=vector(-1.0f,  0.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->Prim = p;
  InitTP(p); o->next->next->next->next->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
  Win->SetWindowText("RAVI-Demo: Spinning Glass Cube"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,2.6f); RV_LtObj=NULL;
}

void InitShadowScene()
{
  RV_Target    = vector(0.0f, 0.0f, 0.5f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=2.58f; C2=0.55f;
  RV_Lights->p = vector(1.0f, 0.0f, 1.0f); RV_Lights->v=1.00f; RV_Lights->Prim=NULL;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 0.0f;
  mat->Reflection=RGB_Color(5.0f, 5.0f, 5.0f);
  mat->Specularity=0.0f; mat->type=Shader_Phong; mat->name = strdup("Light");

  RV_SimpleSphere *s = new RV_SimpleSphere; s->r2=0.01f; s->Mat=mat; Act1=s; RV_LtObj=s;
  s->position=vector(1.0f, 0.0f, 1.0f); s->flags = (RV_CastNoShadow | RV_ReceiveNoShadow);

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.1f, 0.6f, 0.2f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_Chequered; p->Mat=mat; Act2=p;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); PrLP r = new(PrLst); r->Prim = p;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.9f, 0.8f, 0.7f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Clay");

    RV_Cylinder *c = new RV_Cylinder; c->Mat=mat;
  ActM.InitIdentMat(); ActM.Scale(0.25f, 0.25f, 1.5f); ActM.RotateX(1.5707963f); ActM.RotateZ(-0.5f); ActM.Translate(1.0f, -1.2f, 0.25f);
  c->Tb=ActM.Inverse(); c->sinfo = new RV_GenShd; c->sinfo->NM = ActM.NTrans();
  r->next = new(PrLst); r->next->Prim = c;

  RV_SimpleRing *d = new RV_SimpleRing; d->r_o=sqr(0.25f); d->Mat=mat;
  ActM.InitIdentMat(); ActM.Translate(0.0f, 0.0f, 1.5f); ActM.RotateX(1.5707963f); ActM.RotateZ(-0.5f); ActM.Translate(1.0f, -1.2f, 0.25f);
  d->center=vector(0.0f, 0.0f, 0.0f)*ActM; d->N=vector(0.0f, 0.0f, 1.0f)*ActM.NTrans();
  r->next->next = new(PrLst); r->next->next->Prim = d;

  d = new RV_SimpleRing; d->r_o=sqr(0.25f); d->Mat=mat;
  ActM.InitIdentMat(); ActM.RotateX(-1.5707963f); ActM.RotateZ(-0.5f); ActM.Translate(1.0f, -1.2f, 0.25f);
  d->center=vector(0.0f, 0.0f, 0.0f)*ActM; d->N=vector(0.0f, 0.0f, 1.0f)*ActM.NTrans();
  r->next->next->next = new(PrLst); r->next->next->next->Prim = d;
  r->next->next->next->next = NULL;

  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=1; RV_SceneTree->dval=0.7f;

  RV_SceneTree->rigt = new(BSPplane); RV_SceneTree->rigt->axis=0; RV_SceneTree->rigt->left=(BSPplane *)r;

  c = new RV_Cylinder; c->r_top=1.0f; c->Mat=mat;
  ActM.InitIdentMat(); ActM.Scale(0.45f, 0.45f, 2.0f); ActM.Translate(-1.3f, 0.0f, 0.0f);
  c->Tb=ActM.Inverse(); c->sinfo = new RV_GenShd; c->sinfo->NM = ActM.NTrans();
  PrLP l = new(PrLst); l->Prim = Act2;
  l->next = new(PrLst); l->next->Prim = c;
  l->next->next = NULL;

  RV_SceneTree->left = new(BSPplane); RV_SceneTree->left->axis=1; RV_SceneTree->left->dval=-0.65f;

  RV_SceneTree->left->left = new(BSPplane); RV_SceneTree->left->left->axis=0; RV_SceneTree->left->left->left=(BSPplane *)l;

  s = new RV_SimpleSphere; s->r2=0.25f; s->position=vector(0.0f, -1.0f, 0.5f); s->Mat=mat;
  l = new(PrLst); l->Prim = Act2;
  l->next = new(PrLst); l->next->Prim = s;

  p = new RV_TriPara; p->type=0; p->flags=RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.6f, 0.0f, 0.0f); p->edg1=vector(-0.3f, 0.0f, 1.0f); p->edg2=vector(-0.6f, 0.0f, 0.0f); InitTP(p);
  l->next->next = new(PrLst); l->next->next->Prim = p;
  l->next->next->next = NULL;

  p = new RV_TriPara; p->type=1; p->flags=RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.0f, 0.0f, 0.0f); p->edg2=vector( 0.0f, 0.8f, 0.0f); p->edg1=vector( 0.3f, 0.0f, 1.0f); InitTP(p);
  r = new(PrLst); r->Prim = Act2;
  r->next = new(PrLst); r->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.6f, 0.0f, 0.0f); p->edg1=vector( 0.0f, 0.8f, 0.0f); p->edg2=vector(-0.3f, 0.0f, 1.0f); InitTP(p);
  r->next->next = new(PrLst); r->next->next->Prim = p;
 
  p = new RV_TriPara; p->type=0; p->flags=RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.6f, 0.8f, 0.0f); p->edg1=vector(-0.6f, 0.0f, 0.0f); p->edg2=vector(-0.3f, 0.0f, 1.0f); InitTP(p);
  r->next->next->next = new(PrLst); r->next->next->next->Prim = p;
  r->next->next->next->next = NULL;

  RV_SceneTree->left->rigt = new(BSPplane); RV_SceneTree->left->rigt->axis=2; RV_SceneTree->left->rigt->dval=0.0f;

  RV_SceneTree->left->rigt->left = new(BSPplane); RV_SceneTree->left->rigt->left->axis=0; RV_SceneTree->left->rigt->left->left=(BSPplane *)l;

  RV_SceneTree->left->rigt->rigt = new(BSPplane); RV_SceneTree->left->rigt->rigt->axis=0; RV_SceneTree->left->rigt->rigt->left=(BSPplane *)r;

  Win->SetWindowText("RAVI-Demo: Moving Light"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,2.0f);
}

void InitAreaLightScene()
{
  RV_Target    = vector(0.00f, 0.00f, 0.50f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.5f; C1=1.97f; C2=0.67f;
  RV_Lights->p = vector(0.85f,-0.85f, 2.10f); RV_Lights->v=1.00f;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 0.0f;
  mat->Reflection=RGB_Color(5.0f, 5.0f, 5.0f);
  mat->Specularity=0.0f; mat->type=Shader_Phong; mat->name = strdup("Light");

  RV_TriPara *s = new RV_TriPara; s->type=1; s->flags=RV_CastNoShadow | RV_ReceiveNoShadow; s->Mat=mat; Act1=s; RV_LtObj=s; RV_Lights->Prim=s;
  s->base=vector( 1.0f,-1.0f, 2.0f);
  s->edg1=vector(-0.3f, 0.0f, 0.2f);
  s->edg2=vector( 0.0f, 0.3f, 0.0f);
  InitTP(s);

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.1f, 0.6f, 0.2f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Ground");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow; p->Mat=mat; Act2=p;
  p->base=vector(-2.0f,-2.0f, 0.0f);
  p->edg1=vector( 4.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 4.0f, 0.0f);
  InitTP(p); PrLP l = new(PrLst); l->Prim = p;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.9f, 0.8f, 0.7f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Clay");

  RV_Cylinder *c = new RV_Cylinder; c->Mat=mat;
  ActM.InitIdentMat(); ActM.Scale(0.25f, 0.25f, 1.0f);
  c->Tb=ActM.Inverse(); c->sinfo = new RV_GenShd; c->sinfo->NM = ActM.NTrans();
  l->next = new(PrLst); l->next->Prim = c;

  RV_SimpleRing *d = new RV_SimpleRing; d->r_o=sqr(0.25f); d->Mat=mat;
  ActM.InitIdentMat(); ActM.Translate(0.0f, 0.0f, 1.0f);
  d->center=vector(0.0f, 0.0f, 0.0f)*ActM; d->N=vector(0.0f, 0.0f, 1.0f)*ActM.NTrans();
  l->next->next = new(PrLst); l->next->next->Prim = d;

  l->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)l;

  Win->SetWindowText("RAVI-Demo: Area Light's Soft Shadow"); RV_BBMin=vector(-2.0f,-2.0f,-0.001f); RV_BBMax=vector(2.0f,2.0f,2.5f);
}

void InitCornellScene()
{
  RV_Target    = vector(0.0f,  0.0f, 1.50f); RV_Up = vector(0.0f,  0.0f, 1.0f); RV_FoV = 0.535f; C1=3.14159f; C2=0.2f;
  RV_Lights->p = vector(0.0f,  0.0f, 2.98f); RV_Lights->v=0.75f;

  MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 0.0f;
  mat->Reflection=RGB_Color(5.0f, 5.0f, 5.0f);
  mat->Specularity=0.0f; mat->type=Shader_Phong; mat->name = strdup("Light");

  RV_TriPara *s = new RV_TriPara; s->type=1; s->flags=RV_CastNoShadow | RV_ReceiveNoShadow; s->Mat=mat; RV_LtObj=s; RV_Lights->Prim=s;
  s->base=vector(-0.3f,-0.3f, 2.98f);
  s->edg1=vector( 0.0f, 0.6f, 0.00f);
  s->edg2=vector( 0.6f, 0.0f, 0.00f);
  InitTP(s); RV_Lights->area = s->edg1.y*s->edg2.x;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.75f, 0.75f, 0.75f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("White");

  RV_TriPara *p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-1.5f,-1.5f, 0.0f);
  p->edg1=vector( 3.0f, 0.0f, 0.0f);
  p->edg2=vector( 0.0f, 3.0f, 0.0f);
  InitTP(p); PrLP o = new(PrLst); o->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-1.5f, -1.5f, 0.0f);
  p->edg1=vector( 0.0f,  0.0f, 3.0f);
  p->edg2=vector( 3.0f,  0.0f, 0.0f);
  InitTP(p); o->next = new(PrLst); o->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-1.5f,  1.5f, 0.0f);
  p->edg1=vector( 3.0f,  0.0f, 0.0f);
  p->edg2=vector( 0.0f,  0.0f, 3.0f);
  InitTP(p); o->next->next = new(PrLst); o->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-1.5f,-1.5f, 3.0f);
  p->edg1=vector( 0.0f, 3.0f, 0.0f);
  p->edg2=vector( 3.0f, 0.0f, 0.0f);
  InitTP(p); o->next->next->next = new(PrLst); o->next->next->next->Prim = p;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.1f, 0.8f, 0.1f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Green");

  p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-1.5f, -1.5f, 0.0f);
  p->edg1=vector( 0.0f,  3.0f, 0.0f);
  p->edg2=vector( 0.0f,  0.0f, 3.0f);
  InitTP(p); o->next->next->next->next = new(PrLst); o->next->next->next->next->Prim = p;
  
  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.8f, 0.1f, 0.1f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Red");

  p = new RV_TriPara; p->type=1; p->flags=RV_CastNoShadow | RV_SingleSided; p->Mat=mat; Act7=p;
  p->base=vector( 1.5f, -1.5f, 0.0f);
  p->edg1=vector( 0.0f,  0.0f, 3.0f);
  p->edg2=vector( 0.0f,  3.0f, 0.0f);
  InitTP(p); o->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->Prim = p;

  mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 5.0f;
  mat->Reflection=RGB_Color(0.8f, 0.8f, 0.4f);
  mat->Specularity=0.2f; mat->type=Shader_Phong; mat->name = strdup("Yellow");

  ActM.InitIdentMat(); ActM.Scale(0.884f, 0.884f, 1.85f); ActM.RotateZ(-1.274f); matrix NM=ActM; ActM.Translate(0.471f, -0.375f, 0.925f);

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f, -1.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f,  0.0f, -1.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->Prim = p;
  
  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f,  0.0f, -1.0f)*NM;
  p->edg2=vector(-1.0f,  0.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg1=vector( 0.0f,  0.0f,  1.0f)*NM;
  p->edg2=vector( 0.0f,  1.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg1=vector( 1.0f,  0.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f,  0.0f,  1.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector(-1.0f,  0.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f, -1.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->Prim = p;

  ActM.InitIdentMat(); ActM.Scale(0.884f, 0.884f, 0.884f); ActM.RotateZ(4.416f); NM=ActM; ActM.Translate(-0.498f, 0.6f, 0.442f);

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f, -1.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f,  0.0f, -1.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->next->Prim = p;
  
  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector( 0.0f,  0.0f, -1.0f)*NM;
  p->edg2=vector(-1.0f,  0.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg1=vector( 0.0f,  0.0f,  1.0f)*NM;
  p->edg2=vector( 0.0f,  1.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector(-0.5f, -0.5f, -0.5f)*ActM;
  p->edg1=vector( 1.0f,  0.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f,  0.0f,  1.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->next->next->next->next->Prim = p;

  p = new RV_TriPara; p->type=1; p->flags=RV_ReceiveNoShadow | RV_SingleSided; p->Mat=mat;
  p->base=vector( 0.5f,  0.5f,  0.5f)*ActM;
  p->edg1=vector(-1.0f,  0.0f,  0.0f)*NM;
  p->edg2=vector( 0.0f, -1.0f,  0.0f)*NM;
  InitTP(p); o->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next = new(PrLst); o->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->Prim = p;
             o->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next = NULL;

  // set up BSP-tree
  RV_SceneTree = new(BSPplane); RV_SceneTree->axis=0; RV_SceneTree->left=(BSPplane *)o;
  Win->SetWindowText("RAVI-Demo: Cornell Box"); RV_BBMin=vector(-1.501f,-1.501f,-0.001f); RV_BBMax=vector(1.501f,1.501f,3.001f);
}

int SpecLightDepth=3;

RGB_Color PutCaustic(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar IntP, float em, float dist, int level, Thrdata *Thread)
{
  GetIPN(Org, Ray, &IntP); float ca = IntP.N % *Ray;
  MatPtr m = IntP.p->Mat; float v1=em; //if(m->Texture) GetTXY(&IntP);
  
  if((level>0) && (IntP.p->flags & RV_Chequered)){
	if(Thread->PHc>=HBlockSize){
	  if(!PHB[PB]){
	    PHB[PB] = new PHit[HBlockSize];
	  }
      Thread->PHB = PHB[PB]; PB++;
	  Thread->PHc = 0;
	}
	Thread->PHB[Thread->PHc].x=IntP.u;
	Thread->PHB[Thread->PHc].y=IntP.v;
	Thread->PHB[Thread->PHc].v=em;
	Thread->PHB[Thread->PHc].w=wavlen;
	Thread->PHB[Thread->PHc].nxt=NULL;
	Thread->PHB[Thread->PHc].Prim=IntP.p;
	Thread->PHc++;
  }
  
  // distribute diffuse reflected light
  /*if((float)rnd() < m->Diff(wavlen, 0.0f, 0.0f)){ // 0.75f){
    vector OutDir=crndhdir(IntP.N);
    RayTrace(Thread, IntP.p, NULL, mat, &(IntP.IP), &OutDir, em , 0.0f, &PutCaustic, level+1, 5);
  }*/

  // specular reflection/refraction
  if((level<SpecLightDepth) && (m->Specularity>ray_thresh)){
    float  spc = 0.0f, trn=0.0f;
	float  ca  = IntP.N % *Ray;
	vector OutDir;

	if(m->Transmission.R < 0.0f){
	  // fresnel reflection?
	  float n1,n2;
  	  if(ca<0.0f){
	    n1=mat->IOR(wavlen); n2=            m->IOR(wavlen);
	  } else {
        n1=m  ->IOR(wavlen); n2=RV_Environment.IOR(wavlen);
	  }
	  spc = m->Specularity * R_fnf(n1,n2,ca); trn = m->Specularity-spc;
	} else {
	  // simple specular reflection/refraction?
	  if((m->Specularity > ray_thresh) && (m->type!=Shader_Phong)){
	    spc = m->Specularity * ((m->Reflection.G < 0.0f)? m->Reflection.R : wlcoff(m->Reflection.R, m->Reflection.G, m->Reflection.B, wavlen));
        trn = m->Transmission.R;
	  }
	}

	// distribute specular reflected light
	if(v1*spc > ray_thresh) {
	  // mirror direction
	  OutDir = -unit(2.0f * ca * IntP.N - *Ray); //if(m->PBExp > 0.001f) OutDir = rndpdir(OutDir, 1.0f/m->PBExp);
	  RayTrace(Thread, IntP.p, NULL, &RV_Environment, &(IntP.IP), &OutDir, v1*spc, 0.0f, &PutCaustic, level+1, SpecLightDepth);
	}
    // distribute transmitted (refracted) light
	if(v1*trn > ray_thresh) {
	  if(ca<0.0f){
	    OutDir=refract(*Ray,  IntP.N, mat->IOR(wavlen),             m->IOR(wavlen));
	    RayTrace(Thread, IntP.p, NULL,                           m                    , &(IntP.IP), &OutDir, v1*trn, 0.0f, &PutCaustic, level+1, SpecLightDepth);
	  } else {
	    OutDir=refract(*Ray, -IntP.N, mat->IOR(wavlen), RV_Environment.IOR(wavlen));
	    RayTrace(Thread, IntP.p, NULL, ((OutDir % IntP.N)<0.0f)? m : &(RV_Environment), &(IntP.IP), &OutDir, v1*trn, 0.0f, &PutCaustic, level+1, SpecLightDepth);
	  }
	}
  }
  
  return RGB_Color(0.0f, 0.0f, 0.0f);
}

RGB_Color PutPhoton(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar IntP, float em, float dist, int level, Thrdata *Thread)
{
  GetIPN(Org, Ray, &IntP); float ca = IntP.N % *Ray;
  MatPtr m = IntP.p->Mat; float v1=em; //if(m->Texture) GetTXY(&IntP);
  
  if((level>0) || !RV_GIOnlyIndirect){
	if(Thread->PHc>=HBlockSize){
	  if(!PHB[PB]){
	    PHB[PB] = new PHit[HBlockSize];
	  }
      Thread->PHB = PHB[PB]; PB++;
	  Thread->PHc = 0;
	}
	Thread->PHB[Thread->PHc].x=IntP.u;
	Thread->PHB[Thread->PHc].y=IntP.v;
	Thread->PHB[Thread->PHc].v=em;
	Thread->PHB[Thread->PHc].w=wavlen;
	Thread->PHB[Thread->PHc].nxt=NULL;
	Thread->PHB[Thread->PHc].Prim=IntP.p;
	Thread->PHc++;
  }
  
  // distribute diffuse reflected light
  if((float)rnd() < m->Diff(wavlen, 0.0f, 0.0f)){ // 0.75f){
    vector OutDir=crndhdir(IntP.N);
    RayTrace(Thread, IntP.p, NULL, mat, &(IntP.IP), &OutDir, em , 0.0f, &PutPhoton, level+1, 5);
  }

  // specular reflection/refraction
  /*if((level<SpecLightDepth) && (m->Specularity>ray_thresh)){
    float  spc = 0.0f, trn=0.0f;
	float  ca  = IntP.N % *Ray;
	vector OutDir;

	if(m->Transmission.R < 0.0f){
	  // fresnel reflection?
	  float n1,n2;
  	  if(ca<0.0f){
	    n1=mat->IOR(wavlen); n2=            m->IOR(wavlen);
	  } else {
        n1=m  ->IOR(wavlen); n2=RV_Environment.IOR(wavlen);
	  }
	  spc = m->Specularity * R_fnf(n1,n2,ca); trn = m->Specularity-spc;
	} else {
	  // simple specular reflection/refraction?
	  if((m->Specularity > ray_thresh) && (m->type!=Shader_Phong)){
	    spc = m->Specularity * ((m->Reflection.G < 0.0f)? m->Reflection.R : wlcoff(m->Reflection.R, m->Reflection.G, m->Reflection.B, wavlen));
        trn = m->Transmission.R;
	  }
	}

	// distribute specular reflected light
	if(v1*spc > ray_thresh) {
	  // mirror direction
	  OutDir = -unit(2.0f * ca * IntP.N - *Ray); //if(m->PBExp > 0.001f) OutDir = rndpdir(OutDir, 1.0f/m->PBExp);
	  RayTrace(Thread, IntP.p, NULL, &RV_Environment, &(IntP.IP), &OutDir, v1*spc, 0.0f, &PutPhoton, level+1, SpecLightDepth);
	}
    // distribute transmitted (refracted) light
	if(v1*trn > ray_thresh) {
	  if(ca<0.0f){
	    OutDir=refract(*Ray,  IntP.N, mat->IOR(wavlen),             m->IOR(wavlen));
	    RayTrace(Thread, IntP.p, NULL,                           m                    , &(IntP.IP), &OutDir, v1*trn, 0.0f, &PutPhoton, level+1, SpecLightDepth);
	  } else {
	    OutDir=refract(*Ray, -IntP.N, mat->IOR(wavlen), RV_Environment.IOR(wavlen));
	    RayTrace(Thread, IntP.p, NULL, ((OutDir % IntP.N)<0.0f)? m : &(RV_Environment), &(IntP.IP), &OutDir, v1*trn, 0.0f, &PutPhoton, level+1, SpecLightDepth);
	  }
	}
  }*/
  
  return RGB_Color(0.0f, 0.0f, 0.0f);
}


vector      RVD_Ln, RVD_Lv, RVD_Lu;
extern int  ThrCnt;
float       PH_d;
Thrdata     ThD[4];

UINT Illuminate(LPVOID pParam)
{
  float xu,yv;

  if(RV_Threads==1){
    for(xu=-0.15f; xu<=+0.15f; xu+=PH_d){
      for(yv=-0.15f; yv<=+0.15f; yv+=PH_d){
	    vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
        RayTrace(&(ThD[0]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
	  }
	}
  } else if(RV_Threads==2){
	if((int)pParam==0){
      for(xu=-0.15f; xu<  0.00f; xu+=PH_d){
        for(yv=-0.15f; yv<=+0.15f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[0]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	} else {
      for(xu= 0.00f; xu<=+0.15f; xu+=PH_d){
        for(yv=-0.15f; yv<=+0.15f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[1]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	}
  } else if(RV_Threads==4){
	if((int)pParam==0){
      for(xu=-0.15f; xu<  0.00f; xu+=PH_d){
        for(yv=-0.15f; yv<  0.00f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[0]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	} else if((int)pParam==1){
      for(xu=-0.15f; xu<  0.00f; xu+=PH_d){
        for(yv= 0.00f; yv<=+0.15f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[1]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	} else if((int)pParam==2){
      for(xu= 0.00f; xu<=+0.15f; xu+=PH_d){
        for(yv=-0.15f; yv<  0.00f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[2]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	} else /*if((int)pParam==3)*/{
      for(xu= 0.00f; xu<=+0.15f; xu+=PH_d){
        for(yv= 0.00f; yv<=+0.15f; yv+=PH_d){
	      vector Dir = unit(RVD_Ln+xu*RVD_Lu+yv*RVD_Lv);
          RayTrace(&(ThD[3]), NULL, NULL, &RV_Environment, &(RV_Lights->p), &Dir, RV_Lights->v, 0.0f, &PutCaustic, 0, 3);
		}
	  }
	}
  }
  if(RV_Threads>1){ ThrCnt--; if(!ThrCnt) SetEvent(ThrFinished); }
  return 0;	// thread completed successfully
}

UINT Illuminate2(LPVOID pParam)
{
  int i; vector LPos, LDir;
  for(i=0; (PB*HBlockSize)<(RV_GI_Samples/*/RV_Threads*/); i++){
    wavlen=(float)(380.0+400.0*rnd()); // choose random wavelength from spectrum
    LPos = RV_RndPt(RV_Lights->Prim);
    LDir = crndhdir(RV_PtNrm(RV_Lights->Prim,LPos));
    RayTrace(&(ThD[(int)pParam]), RV_Lights->Prim, NULL, &RV_Environment, &LPos, &LDir, RV_Lights->v, 0.0f, &PutPhoton, 0, 3);
  }
  if(RV_Threads>1){ ThrCnt--; if(!ThrCnt) SetEvent(ThrFinished); }
  return 0;	// thread completed successfully
}

void InitQLT(BSPplane *tree)
{
  if(tree->axis == 0){
    PrLP p = (PrLP)tree->left;
    while(p){
	  if(p->Prim->type == 1){
		if(!(((RV_TriPara *)(p->Prim))->QLT)){
		  ((RV_TriPara *)(p->Prim))->QLT = new QLst;
		}
		((RV_TriPara *)(p->Prim))->QLT->val=RGB_Color(0.0f, 0.0f, 0.0f);
		((RV_TriPara *)(p->Prim))->QLT->cnt=0;
		((RV_TriPara *)(p->Prim))->QLT->BL=NULL;
		((RV_TriPara *)(p->Prim))->QLT->BR=NULL;
		((RV_TriPara *)(p->Prim))->QLT->TL=NULL;
		((RV_TriPara *)(p->Prim))->QLT->TR=NULL;
		((RV_TriPara *)(p->Prim))->QLT->Hits=NULL;
	  }
      p=p->next;
	}
  } else {
	InitQLT(tree->left);
	InitQLT(tree->rigt);
  }
}

extern RGB_Color GetShade4mxl(QLP QL, float xb, float yb, float l, float x, float y, int level, int maxlev);

RGB_Color SumVals(QLP QL)
{
  if(!QL) return RGB_Color(0.0f, 0.0f, 0.0f);
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
    QL->val=RGB_Color(0.0f, 0.0f, 0.0f);
    if(QL->BL) QL->val = QL->val+SumVals(QL->BL);
    if(QL->BR) QL->val = QL->val+SumVals(QL->BR);
    if(QL->TL) QL->val = QL->val+SumVals(QL->TL);
    if(QL->TR) QL->val = QL->val+SumVals(QL->TR);
  }
  return QL->val;
}

int GetMaxLev(QLP QL, int level)
{
  if(!QL) return 0;
  int maxlev=level;
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
    if(QL->BL) maxlev=max(maxlev, GetMaxLev(QL->BL, level+1));
    if(QL->BR) maxlev=max(maxlev, GetMaxLev(QL->BR, level+1));
    if(QL->TL) maxlev=max(maxlev, GetMaxLev(QL->TL, level+1));
    if(QL->TR) maxlev=max(maxlev, GetMaxLev(QL->TR, level+1));
  }
  return maxlev;
}

void SetupVals(QLP QLb, QLP QL, float xb, float yb, float l, int level, int maxlev)
{
  if(!QL) return;
  if((level<maxlev) && ((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR))){
    float l2=0.5f*l;
    if(QL->BL) SetupVals(QLb, QL->BL, xb   , yb   , l2, level+1, maxlev);
    if(QL->BR) SetupVals(QLb, QL->BR, xb+l2, yb   , l2, level+1, maxlev);
    if(QL->TL) SetupVals(QLb, QL->TL, xb,    yb+l2, l2, level+1, maxlev);
    if(QL->TR) SetupVals(QLb, QL->TR, xb+l2, yb+l2, l2, level+1, maxlev);
  } else {
    if(level==maxlev){
     float kcx=xb/(1.0f-l);
     float kcy=yb/(1.0f-l);
     QL->val = QL->val + -(l*l)*GetShade4mxl(QLb, 0.0f, 0.0f, 1.0f, kcx, kcy, 0, maxlev-1);
	}
  }
}

RGB_Color GetBL(QLP QL){ return ((QL->BL)? 4.0f*GetBL(QL->BL) : QL->val); }
RGB_Color GetBR(QLP QL){ return ((QL->BR)? 4.0f*GetBR(QL->BR) : QL->val); }
RGB_Color GetTL(QLP QL){ return ((QL->TL)? 4.0f*GetTL(QL->TL) : QL->val); }
RGB_Color GetTR(QLP QL){ return ((QL->TR)? 4.0f*GetTR(QL->TR) : QL->val); }

void SetupPatch(QLP QL, QLP B, QLP T, QLP L, QLP R, QLP BL, QLP BR, QLP TL, QLP TR, float l, int prst)
{
  if(!QL) return; int ct;

  if(!(prst & 1)){
            QL->vBL  = GetBL(QL); ct=1;
    if( L){ QL->vBL += GetBR( L); ct++; }
    if(B ){ QL->vBL += GetTL(B ); ct++; }
    if(BL){ QL->vBL += GetTR(BL); ct++; }
    QL->vBL /= (float)ct*l*l;
  }

  if(!(prst & 2)){
            QL->vBR =  GetBR(QL); ct=1;
    if( R){ QL->vBR += GetBL( R); ct++; }
    if(B ){ QL->vBR += GetTR(B ); ct++; }
    if(BR){ QL->vBR += GetTL(BR); ct++; }
    QL->vBR /= (float)ct*l*l;
  }

  if(!(prst & 4)){
            QL->vTL =  GetTL(QL); ct=1;
    if( L){ QL->vTL += GetTR( L); ct++; }
    if(T ){ QL->vTL += GetBL(T ); ct++; }
    if(TL){ QL->vTL += GetBR(TL); ct++; }
    QL->vTL /= (float)ct*l*l;
  }

  if(!(prst & 8)){
            QL->vTR =  GetTR(QL); ct=1;
    if( R){ QL->vTR += GetTL( R); ct++; }
    if(T ){ QL->vTR += GetBR(T ); ct++; }
    if(TR){ QL->vTR += GetBL(TR); ct++; }
    QL->vTR /= (float)ct*l*l;
  }

  if(QL->BL){
    int ps=0;
    if((B  && !(B ->TL)) || ((prst &  3)== 3)){ QL->BL->vBR = 0.5f*(QL->vBL + QL->vBR); QL->BL->vBL = QL->vBL; ps|= 3; }
	if(( L && !( L->BR)) || ((prst &  5)== 5)){ QL->BL->vTL = 0.5f*(QL->vBL + QL->vTL); QL->BL->vBL = QL->vBL; ps|= 5; }
	if((BL && !(BL->TR)) ||  (prst &  1)     ){                                         QL->BL->vBL = QL->vBL; ps|= 1; }
	SetupPatch(QL->BL, B? B->TL : NULL, QL->TL, L? L->BR : NULL, QL->BR, BL? BL->TR : NULL, B? B->TR : NULL, L? L->TR : NULL, QL->TR, 0.5f*l, ps);
  }
  if(QL->BR){
    int ps=0;
    if((B  && !(B ->TR)) || ((prst &  3)== 3)){ QL->BR->vBL = 0.5f*(QL->vBL + QL->vBR); QL->BR->vBR = QL->vBR; ps|= 3; }
	if(( R && !( R->BL)) || ((prst & 10)==10)){ QL->BR->vTR = 0.5f*(QL->vTR + QL->vBR); QL->BR->vBR = QL->vBR; ps|=10; }
	if((BR && !(BR->TL)) ||  (prst &  2)     ){                                         QL->BR->vBR = QL->vBR; ps|= 2; }
	SetupPatch(QL->BR, B? B->TR : NULL, QL->TR, QL->BL, R? R->BL : NULL, B? B->TL : NULL, BR? BR->TL : NULL, QL->TL, R? R->TL : NULL, 0.5f*l, ps);
  }
  if(QL->TL){
    int ps=0;
    if((T  && !(T ->BL)) || ((prst & 12)==12)){ QL->TL->vTR = 0.5f*(QL->vTL + QL->vTR); QL->TL->vTL = QL->vTL; ps|=12; }
	if(( L && !( L->TR)) || ((prst &  5)== 5)){ QL->TL->vBL = 0.5f*(QL->vTL + QL->vBL); QL->TL->vTL = QL->vTL; ps|= 5; }
	if((TL && !(TL->BR)) ||  (prst &  4)     ){                                         QL->TL->vTL = QL->vTL; ps|= 4; }
    SetupPatch(QL->TL, QL->BL, T? T->BL : NULL, L? L->TR : NULL, QL->TR, L? L->BR : NULL, QL->BR, TL? TL->BR : NULL, T? T->BR : NULL, 0.5f*l, ps);
  }
  if(QL->TR){
    int ps=0;
    if((T  && !(T ->BR)) || ((prst & 12)==12)){ QL->TR->vTL = 0.5f*(QL->vTL + QL->vTR); QL->TR->vTR = QL->vTR; ps|=12; }
	if(( R && !( R->TL)) || ((prst & 10)==10)){ QL->TR->vBR = 0.5f*(QL->vBR + QL->vTR); QL->TR->vTR = QL->vTR; ps|=10; }
	if((TR && !(TR->BL)) ||  (prst &  8)     ){                                         QL->TR->vTR = QL->vTR; ps|= 8; }
	SetupPatch(QL->TR, QL->BR, T? T->BR : NULL, QL->TL, R? R->TL : NULL, QL->BL, R? R->BL : NULL, T? T->BL : NULL, TR? TR->BL : NULL, 0.5f*l, ps);
  }
}

void FinQLT(BSPplane *tree)
{
  if(tree->axis == 0){
    PrLP p = (PrLP)tree->left;
    while(p){
	  if(p->Prim->type == 1){
        SumVals(((RV_TriPara *)(p->Prim))->QLT);
		if(RV_Caustics_Interpolation==4){
          int maxlev=GetMaxLev(((RV_TriPara *)(p->Prim))->QLT, 0);
		  for(int i=2; i<=maxlev; i++) SetupVals(((RV_TriPara *)(p->Prim))->QLT, ((RV_TriPara *)(p->Prim))->QLT, 0.0f, 0.0f, 1.0f, 0, i);
		}
		if(RV_Caustics_Interpolation==6){
          SetupPatch(((RV_TriPara *)(p->Prim))->QLT, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1.0f, 0);
		}
	  }
      p=p->next;
	}
  } else {
	FinQLT(tree->left);
	FinQLT(tree->rigt);
  }
}

void ShootCaustics()
{
  RVD_Ln = unit(vector(0.0f, 0.0f, 1.3f)-RV_Lights->p);
  if(RVD_Ln.x && RVD_Ln.y){
    RVD_Lv = unit(vector(0.0f, 0.0f, 1.0f)-(vector(0.0f, 0.0f, 1.0f)%RVD_Ln)*RVD_Ln);
  } else {
    RVD_Lv = vector(-1.0f, 0.0f, 0.0f);
  }
  RVD_Lu = RVD_Ln*RVD_Lv;

  PH_d=0.3f/sqrtf((float)RV_Caustics_Samples);
  int i,j; for(j=0; j<4; j++) ThD[j].PHc=HBlockSize+1; PB=0; wavlen=555.0f;
  
  if(RV_Threads==1){
	Illuminate((LPVOID)0);
  } else {
    ResetEvent(ThrFinished);
    ThrCnt=RV_Threads;
    for(i=0; i<RV_Threads; i++){
      AfxBeginThread(Illuminate, (LPVOID)i);
	}
    DWORD wr=WaitForSingleObject(ThrFinished, 2000);
    if(wr==WAIT_TIMEOUT){
      char msg[80]; sprintf(msg,"Time out, ThrCnt=%d",ThrCnt);
      MessageBox(NULL,msg,"Debug",MB_OK);
	}
  }

  // store photon hits in quadtree
  QLc=1; QLT=new QLst; QLT->val=RGB_Color(0.0f, 0.0f, 0.0f); QLT->cnt=0; QLT->BL=NULL; QLT->BR=NULL; QLT->TL=NULL; QLT->TR=NULL; QLT->Hits=NULL;
  InitQLT(RV_SceneTree); QBp=QBlockSize+1; QB=-1;
  for(i=0; i<PB; i++){
	int be=HBlockSize;
    for(j=0; j<RV_Threads; j++) {
	  if(ThD[j].PHB==PHB[i]) be=ThD[j].PHc;
	}
	for(j=0; j<be; j++){
      StoreHit(((RV_TriPara *)(PHB[i][j].Prim))->QLT, 0.0f, 0.0f, 1.0f, &(PHB[i][j]), RV_Caustics_Quality);
	}
  }

  /*QLP QL=((RV_TriPara *)(PHB[0][0].Prim))->QLT; QL->val=RGB_Color(2906.2f, 2906.2f, 2906.2f);
  QL->BL=QAlloc(); QL->BL->val=RGB_Color( 500.0f,  500.0f,  500.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL; //QL->BL->vBL= 500.0f;  QL->BL->vBR=375.0f; QL->BL->vTL=1250.0f; QL->BL->vTR=640.5f;
  QL->BR=QAlloc(); QL->BR->val=RGB_Color( 250.0f,  250.0f,  250.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL; //QL->BR->vBL= 375.0f;  QL->BR->vBR=250.0f; QL->BR->vTL= 640.5f; QL->BR->vTR=375.0f;
  QL->TL=QAlloc(); QL->TL->val=RGB_Color(1656.2f, 1656.2f, 1656.2f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL; //QL->TL->vBL=1250.0f;  QL->TL->vBR=640.5f; QL->TL->vTL=1500.0f; QL->TL->vTR=750.5f;
    QL->TL->BL=QAlloc(); QL->TL->BL->val=RGB_Color( 500.0f,  500.0f,  500.0f); QL->TL->BL->cnt=0; QL->TL->BL->BL=NULL; QL->TL->BL->BR=NULL; QL->TL->BL->TL=NULL; QL->TL->BL->TR=NULL; QL->TL->BL->Hits=NULL; //QL->TL->BL->vBL=312.5000f; QL->TL->BL->vBR=236.3125f; QL->TL->BL->vTL=437.50f; QL->TL->BL->vTR=406.2500f;
    QL->TL->BR=QAlloc(); QL->TL->BR->val=RGB_Color( 531.2f,  531.2f,  531.2f); QL->TL->BR->cnt=0; QL->TL->BR->BL=NULL; QL->TL->BR->BR=NULL; QL->TL->BR->TL=NULL; QL->TL->BR->TR=NULL; QL->TL->BR->Hits=NULL; //QL->TL->BR->vBL=236.3125f; QL->TL->BR->vBR=160.1250f; QL->TL->BR->vTL=406.25f; QL->TL->BR->vTR=173.8125f;
      QL->TL->BR->BL=QAlloc(); QL->TL->BR->BL->val=RGB_Color( 179.7f,  179.7f,  179.7f); QL->TL->BR->BL->cnt=0; QL->TL->BR->BL->BL=NULL; QL->TL->BR->BL->BR=NULL; QL->TL->BR->BL->TL=NULL; QL->TL->BR->BL->TR=NULL; QL->TL->BR->BL->Hits=NULL; //QL->TL->BR->BL->vBL= 59.0781250f; QL->TL->BR->BL->vBR= 49.5546875f; QL->TL->BR->BL->vTL= 80.3203125f; QL->TL->BR->BL->vTR=134.65f;
        QL->TL->BR->BL->BL=QAlloc(); QL->TL->BR->BL->BL->val=RGB_Color(  39.0f,   39.0f,   39.0f); QL->TL->BR->BL->BL->cnt=0; QL->TL->BR->BL->BL->BL=NULL; QL->TL->BR->BL->BL->BR=NULL; QL->TL->BR->BL->BL->TL=NULL; QL->TL->BR->BL->BL->TR=NULL; QL->TL->BR->BL->BL->Hits=NULL;
        QL->TL->BR->BL->BR=QAlloc(); QL->TL->BR->BL->BR->val=RGB_Color(  62.5f,   62.5f,   62.5f); QL->TL->BR->BL->BR->cnt=0; QL->TL->BR->BL->BR->BL=NULL; QL->TL->BR->BL->BR->BR=NULL; QL->TL->BR->BL->BR->TL=NULL; QL->TL->BR->BL->BR->TR=NULL; QL->TL->BR->BL->BR->Hits=NULL;
        QL->TL->BR->BL->TL=QAlloc(); QL->TL->BR->BL->TL->val=RGB_Color(  31.3f,   31.3f,   31.3f); QL->TL->BR->BL->TL->cnt=0; QL->TL->BR->BL->TL->BL=NULL; QL->TL->BR->BL->TL->BR=NULL; QL->TL->BR->BL->TL->TL=NULL; QL->TL->BR->BL->TL->TR=NULL; QL->TL->BR->BL->TL->Hits=NULL;
        QL->TL->BR->BL->TR=QAlloc(); QL->TL->BR->BL->TR->val=RGB_Color(  46.9f,   46.9f,   46.9f); QL->TL->BR->BL->TR->cnt=0; QL->TL->BR->BL->TR->BL=NULL; QL->TL->BR->BL->TR->BR=NULL; QL->TL->BR->BL->TR->TL=NULL; QL->TL->BR->BL->TR->TR=NULL; QL->TL->BR->BL->TR->Hits=NULL;
      QL->TL->BR->BR=QAlloc(); QL->TL->BR->BR->val=RGB_Color( 164.0f,  164.0f,  164.0f); QL->TL->BR->BR->cnt=0; QL->TL->BR->BR->BL=NULL; QL->TL->BR->BR->BR=NULL; QL->TL->BR->BR->TL=NULL; QL->TL->BR->BR->TR=NULL; QL->TL->BR->BR->Hits=NULL; //QL->TL->BR->BR->vBL= 49.5546875f; QL->TL->BR->BR->vBR= 40.0312500f; QL->TL->BR->BR->vTL=134.6500000f; QL->TL->BR->BR->vTR=41.7421875f;
      QL->TL->BR->TL=QAlloc(); QL->TL->BR->TL->val=RGB_Color( 125.0f,  125.0f,  125.0f); QL->TL->BR->TL->cnt=0; QL->TL->BR->TL->BL=NULL; QL->TL->BR->TL->BR=NULL; QL->TL->BR->TL->TL=NULL; QL->TL->BR->TL->TR=NULL; QL->TL->BR->TL->Hits=NULL; //QL->TL->BR->TL->vBL= 80.3203125f; QL->TL->BR->TL->vBR=134.6500000f; QL->TL->BR->TL->vTL=101.5625000f; QL->TL->BR->TL->vTR=72.5078125f;
      QL->TL->BR->TR=QAlloc(); QL->TL->BR->TR->val=RGB_Color(  62.5f,   62.5f,   62.5f); QL->TL->BR->TR->cnt=0; QL->TL->BR->TR->BL=NULL; QL->TL->BR->TR->BR=NULL; QL->TL->BR->TR->TL=NULL; QL->TL->BR->TR->TR=NULL; QL->TL->BR->TR->Hits=NULL; //QL->TL->BR->TR->vBL=134.6500000f; QL->TL->BR->TR->vBR= 41.7421875f; QL->TL->BR->TR->vTL= 72.5078125f; QL->TL->BR->TR->vTR=43.4531250f;
    QL->TL->TL=QAlloc(); QL->TL->TL->val=RGB_Color( 375.0f,  375.0f,  375.0f); QL->TL->TL->cnt=0; QL->TL->TL->BL=NULL; QL->TL->TL->BR=NULL; QL->TL->TL->TL=NULL; QL->TL->TL->TR=NULL; QL->TL->TL->Hits=NULL; //QL->TL->TL->vBL=437.5000f; QL->TL->TL->vBR=406.2500f; QL->TL->TL->vTL=375.00f; QL->TL->TL->vTR=312.5000f;
    QL->TL->TR=QAlloc(); QL->TL->TR->val=RGB_Color( 250.0f,  250.0f,  250.0f); QL->TL->TR->cnt=0; QL->TL->TR->BL=NULL; QL->TL->TR->BR=NULL; QL->TL->TR->TL=NULL; QL->TL->TR->TR=NULL; QL->TL->TR->Hits=NULL; //QL->TL->TR->vBL=406.2500f; QL->TL->TR->vBR=173.8125f; QL->TL->TR->vTL=312.50f; QL->TL->TR->vTR=187.5000f;
  QL->TR=QAlloc(); QL->TR->val=RGB_Color( 500.0f,  500.0f,  500.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL; //QL->TR->vBL=640.5f;  QL->TR->vBR=375.0f; QL->TR->vTL= 750.0f; QL->TR->vTR=500.0f;*/

  /*QLP QL=((RV_TriPara *)(PHB[0][0].Prim))->QLT;
  QL->BL=QAlloc(); QL->BL->val=RGB_Color(512.0f, 512.0f, 512.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
  QL->BR=QAlloc(); QL->BR->val=RGB_Color(512.0f, 512.0f, 512.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
  QL->TL=QAlloc(); QL->TL->val=RGB_Color(512.0f, 512.0f, 512.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
    QL->TL->BL=QAlloc(); QL->TL->BL->val=RGB_Color(128.0f, 128.0f, 128.0f); QL->TL->BL->cnt=0; QL->TL->BL->BL=NULL; QL->TL->BL->BR=NULL; QL->TL->BL->TL=NULL; QL->TL->BL->TR=NULL; QL->TL->BL->Hits=NULL;
    QL->TL->BR=QAlloc(); QL->TL->BR->val=RGB_Color(128.0f, 128.0f, 128.0f); QL->TL->BR->cnt=0; QL->TL->BR->BL=NULL; QL->TL->BR->BR=NULL; QL->TL->BR->TL=NULL; QL->TL->BR->TR=NULL; QL->TL->BR->Hits=NULL;
      QL->TL->BR->BL=QAlloc(); QL->TL->BR->BL->val=RGB_Color( 32.0f,  32.0f,  32.0f); QL->TL->BR->BL->cnt=0; QL->TL->BR->BL->BL=NULL; QL->TL->BR->BL->BR=NULL; QL->TL->BR->BL->TL=NULL; QL->TL->BR->BL->TR=NULL; QL->TL->BR->BL->Hits=NULL;
        QL->TL->BR->BL->BL=QAlloc(); QL->TL->BR->BL->BL->val=RGB_Color( 8.0f, 8.0f, 8.0f); QL->TL->BR->BL->BL->cnt=0; QL->TL->BR->BL->BL->BL=NULL; QL->TL->BR->BL->BL->BR=NULL; QL->TL->BR->BL->BL->TL=NULL; QL->TL->BR->BL->BL->TR=NULL; QL->TL->BR->BL->BL->Hits=NULL;
        QL->TL->BR->BL->BR=QAlloc(); QL->TL->BR->BL->BR->val=RGB_Color( 8.0f, 8.0f, 8.0f); QL->TL->BR->BL->BR->cnt=0; QL->TL->BR->BL->BR->BL=NULL; QL->TL->BR->BL->BR->BR=NULL; QL->TL->BR->BL->BR->TL=NULL; QL->TL->BR->BL->BR->TR=NULL; QL->TL->BR->BL->BR->Hits=NULL;
        QL->TL->BR->BL->TL=QAlloc(); QL->TL->BR->BL->TL->val=RGB_Color( 8.0f, 8.0f, 8.0f); QL->TL->BR->BL->TL->cnt=0; QL->TL->BR->BL->TL->BL=NULL; QL->TL->BR->BL->TL->BR=NULL; QL->TL->BR->BL->TL->TL=NULL; QL->TL->BR->BL->TL->TR=NULL; QL->TL->BR->BL->TL->Hits=NULL;
        QL->TL->BR->BL->TR=QAlloc(); QL->TL->BR->BL->TR->val=RGB_Color( 8.0f, 8.0f, 8.0f); QL->TL->BR->BL->TR->cnt=0; QL->TL->BR->BL->TR->BL=NULL; QL->TL->BR->BL->TR->BR=NULL; QL->TL->BR->BL->TR->TL=NULL; QL->TL->BR->BL->TR->TR=NULL; QL->TL->BR->BL->TR->Hits=NULL;
      QL->TL->BR->BR=QAlloc(); QL->TL->BR->BR->val=RGB_Color( 32.0f, 32.0f, 32.0f); QL->TL->BR->BR->cnt=0; QL->TL->BR->BR->BL=NULL; QL->TL->BR->BR->BR=NULL; QL->TL->BR->BR->TL=NULL; QL->TL->BR->BR->TR=NULL; QL->TL->BR->BR->Hits=NULL;
      QL->TL->BR->TL=QAlloc(); QL->TL->BR->TL->val=RGB_Color( 32.0f, 32.0f, 32.0f); QL->TL->BR->TL->cnt=0; QL->TL->BR->TL->BL=NULL; QL->TL->BR->TL->BR=NULL; QL->TL->BR->TL->TL=NULL; QL->TL->BR->TL->TR=NULL; QL->TL->BR->TL->Hits=NULL;
      QL->TL->BR->TR=QAlloc(); QL->TL->BR->TR->val=RGB_Color( 32.0f, 32.0f, 32.0f); QL->TL->BR->TR->cnt=0; QL->TL->BR->TR->BL=NULL; QL->TL->BR->TR->BR=NULL; QL->TL->BR->TR->TL=NULL; QL->TL->BR->TR->TR=NULL; QL->TL->BR->TR->Hits=NULL;
    QL->TL->TL=QAlloc(); QL->TL->TL->val=RGB_Color(128.0f, 128.0f, 128.0f); QL->TL->TL->cnt=0; QL->TL->TL->BL=NULL; QL->TL->TL->BR=NULL; QL->TL->TL->TL=NULL; QL->TL->TL->TR=NULL; QL->TL->TL->Hits=NULL;
    QL->TL->TR=QAlloc(); QL->TL->TR->val=RGB_Color(128.0f, 128.0f, 128.0f); QL->TL->TR->cnt=0; QL->TL->TR->BL=NULL; QL->TL->TR->BR=NULL; QL->TL->TR->TL=NULL; QL->TL->TR->TR=NULL; QL->TL->TR->Hits=NULL;
  QL->TR=QAlloc(); QL->TR->val=RGB_Color(512.0f, 512.0f, 512.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;*/

  /*QLP QL=((RV_TriPara *)(PHB[0][0].Prim))->QLT; QL->val=RGB_Color(2906.2f, 2906.2f, 2906.2f);
  QL->BL=QAlloc(); QL->BL->val=RGB_Color( 512.0f,  512.0f,  512.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
  QL->BR=QAlloc(); QL->BR->val=RGB_Color( 512.0f,  512.0f,  512.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
  QL->TL=QAlloc(); QL->TL->val=RGB_Color( 656.0f,  656.0f,  656.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
    QL->TL->BL=QAlloc(); QL->TL->BL->val=RGB_Color( 132.0f,  132.0f,  132.0f); QL->TL->BL->cnt=0; QL->TL->BL->BL=NULL; QL->TL->BL->BR=NULL; QL->TL->BL->TL=NULL; QL->TL->BL->TR=NULL; QL->TL->BL->Hits=NULL;
    QL->TL->BR=QAlloc(); QL->TL->BR->val=RGB_Color( 132.0f,  132.0f,  132.0f); QL->TL->BR->cnt=0; QL->TL->BR->BL=NULL; QL->TL->BR->BR=NULL; QL->TL->BR->TL=NULL; QL->TL->BR->TR=NULL; QL->TL->BR->Hits=NULL;
    QL->TL->TL=QAlloc(); QL->TL->TL->val=RGB_Color( 196.0f,  196.0f,  196.0f); QL->TL->TL->cnt=0; QL->TL->TL->BL=NULL; QL->TL->TL->BR=NULL; QL->TL->TL->TL=NULL; QL->TL->TL->TR=NULL; QL->TL->TL->Hits=NULL;
    QL->TL->TR=QAlloc(); QL->TL->TR->val=RGB_Color( 196.0f,  196.0f,  196.0f); QL->TL->TR->cnt=0; QL->TL->TR->BL=NULL; QL->TL->TR->BR=NULL; QL->TL->TR->TL=NULL; QL->TL->TR->TR=NULL; QL->TL->TR->Hits=NULL;
  QL->TR=QAlloc(); QL->TR->val=RGB_Color( 656.0f,  656.0f,  656.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
    QL->TR->BL=QAlloc(); QL->TL->BL->val=RGB_Color( 132.0f,  132.0f,  132.0f); QL->TL->BL->cnt=0; QL->TL->BL->BL=NULL; QL->TL->BL->BR=NULL; QL->TL->BL->TL=NULL; QL->TL->BL->TR=NULL; QL->TL->BL->Hits=NULL;
    QL->TR->BR=QAlloc(); QL->TL->BR->val=RGB_Color( 132.0f,  132.0f,  132.0f); QL->TL->BR->cnt=0; QL->TL->BR->BL=NULL; QL->TL->BR->BR=NULL; QL->TL->BR->TL=NULL; QL->TL->BR->TR=NULL; QL->TL->BR->Hits=NULL;
    QL->TR->TL=QAlloc(); QL->TL->TL->val=RGB_Color( 196.0f,  196.0f,  196.0f); QL->TL->TL->cnt=0; QL->TL->TL->BL=NULL; QL->TL->TL->BR=NULL; QL->TL->TL->TL=NULL; QL->TL->TL->TR=NULL; QL->TL->TL->Hits=NULL;
    QL->TR->TR=QAlloc(); QL->TL->TR->val=RGB_Color( 196.0f,  196.0f,  196.0f); QL->TL->TR->cnt=0; QL->TL->TR->BL=NULL; QL->TL->TR->BR=NULL; QL->TL->TR->TL=NULL; QL->TL->TR->TR=NULL; QL->TL->TR->Hits=NULL;*/

  /*QLP QL=((RV_TriPara *)(PHB[0][0].Prim))->QLT; QL->val=RGB_Color(2906.2f, 2906.2f, 2906.2f);
  QL->BL=QAlloc(); QL->BL->val=RGB_Color( 100.0f,  100.0f,  100.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL;
  QL->BR=QAlloc(); QL->BR->val=RGB_Color( 512.0f,  512.0f,  512.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL;
    QL->BR->BL=QAlloc(); QL->BR->BL->val=RGB_Color(  50.0f,   50.0f,   50.0f); QL->BR->BL->cnt=0; QL->BR->BL->BL=NULL; QL->BR->BL->BR=NULL; QL->BR->BL->TL=NULL; QL->BR->BL->TR=NULL; QL->BR->BL->Hits=NULL;
    QL->BR->BR=QAlloc(); QL->BR->BR->val=RGB_Color( 190.0f,  190.0f,  190.0f); QL->BR->BR->cnt=0; QL->BR->BR->BL=NULL; QL->BR->BR->BR=NULL; QL->BR->BR->TL=NULL; QL->BR->BR->TR=NULL; QL->BR->BR->Hits=NULL;
    QL->BR->TL=QAlloc(); QL->BR->TL->val=RGB_Color( 100.0f,  100.0f,  100.0f); QL->BR->TL->cnt=0; QL->BR->TL->BL=NULL; QL->BR->TL->BR=NULL; QL->BR->TL->TL=NULL; QL->BR->TL->TR=NULL; QL->BR->TL->Hits=NULL;
    QL->BR->TR=QAlloc(); QL->BR->TR->val=RGB_Color( 190.0f,  190.0f,  190.0f); QL->BR->TR->cnt=0; QL->BR->TR->BL=NULL; QL->BR->TR->BR=NULL; QL->BR->TR->TL=NULL; QL->BR->TR->TR=NULL; QL->BR->TR->Hits=NULL;
  QL->TL=QAlloc(); QL->TL->val=RGB_Color( 656.0f,  656.0f,  656.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL;
    QL->TL->BL=QAlloc(); QL->TL->BL->val=RGB_Color( 220.0f,  220.0f,  220.0f); QL->TL->BL->cnt=0; QL->TL->BL->BL=NULL; QL->TL->BL->BR=NULL; QL->TL->BL->TL=NULL; QL->TL->BL->TR=NULL; QL->TL->BL->Hits=NULL;
    QL->TL->BR=QAlloc(); QL->TL->BR->val=RGB_Color( 160.0f,  160.0f,  160.0f); QL->TL->BR->cnt=0; QL->TL->BR->BL=NULL; QL->TL->BR->BR=NULL; QL->TL->BR->TL=NULL; QL->TL->BR->TR=NULL; QL->TL->BR->Hits=NULL;
    QL->TL->TL=QAlloc(); QL->TL->TL->val=RGB_Color( 240.0f,  240.0f,  240.0f); QL->TL->TL->cnt=0; QL->TL->TL->BL=NULL; QL->TL->TL->BR=NULL; QL->TL->TL->TL=NULL; QL->TL->TL->TR=NULL; QL->TL->TL->Hits=NULL;
    QL->TL->TR=QAlloc(); QL->TL->TR->val=RGB_Color( 200.0f,  200.0f,  200.0f); QL->TL->TR->cnt=0; QL->TL->TR->BL=NULL; QL->TL->TR->BR=NULL; QL->TL->TR->TL=NULL; QL->TL->TR->TR=NULL; QL->TL->TR->Hits=NULL;
  QL->TR=QAlloc(); QL->TR->val=RGB_Color( 656.0f,  656.0f,  656.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL;
    QL->TR->BL=QAlloc(); QL->TR->BL->val=RGB_Color( 180.0f,  180.0f,  180.0f); QL->TR->BL->cnt=0; QL->TR->BL->BL=NULL; QL->TR->BL->BR=NULL; QL->TR->BL->TL=NULL; QL->TR->BL->TR=NULL; QL->TR->BL->Hits=NULL;
    QL->TR->BR=QAlloc(); QL->TR->BR->val=RGB_Color( 230.0f,  230.0f,  230.0f); QL->TR->BR->cnt=0; QL->TR->BR->BL=NULL; QL->TR->BR->BR=NULL; QL->TR->BR->TL=NULL; QL->TR->BR->TR=NULL; QL->TR->BR->Hits=NULL;
    QL->TR->TL=QAlloc(); QL->TR->TL->val=RGB_Color( 205.0f,  205.0f,  205.0f); QL->TR->TL->cnt=0; QL->TR->TL->BL=NULL; QL->TR->TL->BR=NULL; QL->TR->TL->TL=NULL; QL->TR->TL->TR=NULL; QL->TR->TL->Hits=NULL;
    QL->TR->TR=QAlloc(); QL->TR->TR->val=RGB_Color( 170.0f,  170.0f,  170.0f); QL->TR->TR->cnt=0; QL->TR->TR->BL=NULL; QL->TR->TR->BR=NULL; QL->TR->TR->TL=NULL; QL->TR->TR->TR=NULL; QL->TR->TR->Hits=NULL;*/

  //int maxlev=GetMaxLev(QL, 0); for(i=2; i<=maxlev; i++) SetupVals(QL, QL, 0.0f, 0.0f, 1.0f, 0, i);
  if((RV_Caustics_Interpolation==4) || (RV_Caustics_Interpolation==5) || (RV_Caustics_Interpolation==6)) FinQLT(RV_SceneTree);

/*void StoreHit(QLP QL, float xb, float yb, float l, PHP Hit, int Quality)
{
  if(!QL) return;
  if((QL->cnt < Quality) || (l < 0.001f)){
	Hit->nxt=QL->Hits; QL->Hits=Hit;
	QL->val = QL->val + (Hit->v * WL2RGB(Hit->w)); QL->cnt++;
  } else {
	float l2=0.5f*l;
	if(Hit->y<(yb+l2)){
	  if(Hit->x<(xb+l2)){
		if(!(QL->BL)){ QL->BL=QAlloc(); QL->BL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BL->cnt=0; QL->BL->BL=NULL; QL->BL->BR=NULL; QL->BL->TL=NULL; QL->BL->TR=NULL; QL->BL->Hits=NULL; }
		StoreHit(QL->BL, xb   , yb   , l2, Hit, Quality);
	  } else {
		if(!(QL->BR)){ QL->BR=QAlloc(); QL->BR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->BR->cnt=0; QL->BR->BL=NULL; QL->BR->BR=NULL; QL->BR->TL=NULL; QL->BR->TR=NULL; QL->BR->Hits=NULL; }
		StoreHit(QL->BR, xb+l2, yb   , l2, Hit, Quality);
	  }
	} else {
	  if(Hit->x<(xb+l2)){
		if(!(QL->TL)){ QL->TL=QAlloc(); QL->TL->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TL->cnt=0; QL->TL->BL=NULL; QL->TL->BR=NULL; QL->TL->TL=NULL; QL->TL->TR=NULL; QL->TL->Hits=NULL; }
		StoreHit(QL->TL, xb,    yb+l2, l2, Hit, Quality);
	  } else {
		if(!(QL->TR)){ QL->TR=QAlloc(); QL->TR->val=RGB_Color(0.0f, 0.0f, 0.0f); QL->TR->cnt=0; QL->TR->BL=NULL; QL->TR->BR=NULL; QL->TR->TL=NULL; QL->TR->TR=NULL; QL->TR->Hits=NULL; }
		StoreHit(QL->TR, xb+l2, yb+l2, l2, Hit, Quality);
	  }
	}
    if(QL->cnt == Quality){
	  QL->cnt=Quality+1; QL->val=RGB_Color(0.0f, 0.0f, 0.0f);
	  PHP o,p=QL->Hits;
	  while(p){
		o=p->nxt; StoreHit(QL, xb, yb, l, p, Quality); p=o;
	  }
	}
  }
}*/

}

void ShootLight()
{
  int i,j; for(j=0; j<4; j++) ThD[j].PHc=HBlockSize+1; PB=0;
  
  if(RV_Threads==1){
	Illuminate2((LPVOID)0);
  } else {
    ResetEvent(ThrFinished);
    ThrCnt=RV_Threads;
    for(i=0; i<RV_Threads; i++){
      AfxBeginThread(Illuminate2, (LPVOID)i);
	}
    DWORD wr=WaitForSingleObject(ThrFinished, 2000);
    if(wr==WAIT_TIMEOUT){
      char msg[80]; sprintf(msg,"Time out, ThrCnt=%d",ThrCnt);
      MessageBox(NULL,msg,"Debug",MB_OK);
	}
  }

  // store photon hits in quadtree
  QLc=1; QLT=new QLst; QLT->val=RGB_Color(0.0f, 0.0f, 0.0f); QLT->cnt=0; QLT->BL=NULL; QLT->BR=NULL; QLT->TL=NULL; QLT->TR=NULL; QLT->Hits=NULL;
  InitQLT(RV_SceneTree); QBp=QBlockSize+1; QB=-1;
  for(i=0; i<PB; i++){
	int be=HBlockSize;
    for(j=0; j<RV_Threads; j++) {
	  if(ThD[j].PHB==PHB[i]) be=ThD[j].PHc;
	}
	for(j=0; j<be; j++){
      StoreHit(((RV_TriPara *)(PHB[i][j].Prim))->QLT, 0.0f, 0.0f, 1.0f, &(PHB[i][j]), RV_GI_Quality);
	}
  }
  if((RV_Caustics_Interpolation==4) || (RV_Caustics_Interpolation==5) || (RV_Caustics_Interpolation==6)) FinQLT(RV_SceneTree);
}

/////////////////////////////////////////////////////////////////////////////
// cRaViApp

BEGIN_MESSAGE_MAP(cRaViApp, CWinApp)
	//{{AFX_MSG_MAP(cRaViApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_UPDATE_COMMAND_UI(ID_DEMO_ANIM, OnUpdateAnimation)
	ON_COMMAND(ID_DEMO_ANIM, OnAnimationToggle)
	ON_COMMAND(ID_QUALITY_1by1, OnQuality_1by1)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_1by1, OnUpdateQuality_1by1)
	ON_COMMAND(ID_QUALITY_SUBSAMPLING, OnQualitySubsampling)
	ON_COMMAND(ID_DEMO_CUBE, OnDemoCube)
	ON_COMMAND(ID_DEMO_MIRRORSPHERE, OnMirrorSphere)
	ON_UPDATE_COMMAND_UI(ID_DEMO_MIRRORSPHERE, OnUpdateDemoMirrSph)
	ON_UPDATE_COMMAND_UI(ID_DEMO_CUBE, OnUpdateDemoCube)
	ON_COMMAND(ID_DEMO_3SPHERES, OnDemo3Spheres)
	ON_UPDATE_COMMAND_UI(ID_DEMO_3SPHERES, OnUpdate3Spheres)
	ON_COMMAND(ID_DEMO_SHADOWS, OnDemoShadows)
	ON_UPDATE_COMMAND_UI(ID_DEMO_SHADOWS, OnUpdateShadows)
	ON_COMMAND(ID_SHADER_NORMALS, OnShaderNormals)
	ON_COMMAND(ID_SHADER_PHONG, OnShaderPhong)
	ON_COMMAND(ID_SHADER_REFLECTION, OnShaderReflection)
	ON_UPDATE_COMMAND_UI(ID_SHADER_NORMALS, OnUpdateShaderNormals)
	ON_UPDATE_COMMAND_UI(ID_SHADER_PHONG, OnUpdateShaderPhong)
	ON_UPDATE_COMMAND_UI(ID_SHADER_REFLECTION, OnUpdateShaderReflection)
	ON_COMMAND(ID_SHADER_DEPTH, OnShaderDepth)
	ON_UPDATE_COMMAND_UI(ID_SHADER_DEPTH, OnUpdateShaderDepth)
	ON_COMMAND(ID_FILTER, OnFilter)
	ON_UPDATE_COMMAND_UI(ID_FILTER, OnUpdateFilter)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_SUBSAMPLING, OnUpdateQualitySub)
	ON_COMMAND(ID_DEMO_TORUS, OnDemoTorus)
	ON_UPDATE_COMMAND_UI(ID_DEMO_TORUS, OnUpdateDemoTorus)
	ON_COMMAND(ID_SHADER_UV, OnShaderUV)
	ON_UPDATE_COMMAND_UI(ID_SHADER_UV, OnUpdateShaderUV)
	ON_COMMAND(ID_SHADER_SIMPLE, OnShaderSimple)
	ON_UPDATE_COMMAND_UI(ID_SHADER_SIMPLE, OnUpdateShaderSimple)
	ON_COMMAND(ID_DEMO_SPHEREHEAP, OnDemoSphereHeap)
	ON_UPDATE_COMMAND_UI(ID_DEMO_SPHEREHEAP, OnUpdateDemoSphereHeap)
	ON_COMMAND(ID_DEMO_SHADOWS3, OnDemoAreaLight)
	ON_UPDATE_COMMAND_UI(ID_DEMO_SHADOWS3, OnUpdateDemoAreaLight)
	ON_COMMAND(ID_RES480, OnRes480)
	ON_COMMAND(ID_RES320, OnRes320)
	ON_COMMAND(ID_RES512, OnRes512)
	ON_COMMAND(ID_RES640, OnRes640)
	ON_COMMAND(ID_RES800, OnRes800)
	ON_COMMAND(ID_SingleThread, OnSingleThread)
	ON_COMMAND(ID_DualThread, OnDualThread)
	ON_COMMAND(ID_QuadThread, OnQuadThread)
	ON_UPDATE_COMMAND_UI(ID_SingleThread, OnUpdateSingleThread)
	ON_UPDATE_COMMAND_UI(ID_DualThread, OnUpdateDualThread)
	ON_COMMAND(ID_SAMP_INT16, OnSampInt16)
	ON_COMMAND(ID_SAMP_4, OnSampInt4)
	ON_COMMAND(ID_SAMP_RAW, OnSampRaw16)
	ON_COMMAND(ID_SAMP_STD, OnSampStd)
	ON_COMMAND(ID_SAMP_FULL, OnSampFull)
	ON_COMMAND(ID_QUALITY_AD20, OnQualityAd20)
	ON_COMMAND(ID_QUALITY_AD10, OnQualityAd10)
	ON_COMMAND(ID_QUALITY_AD30, OnQualityAd30)
	ON_COMMAND(ID_QUALITY_AD25, OnQualityAd25)
	ON_COMMAND(ID_QUALITY_AD15, OnQualityAd15)
	ON_COMMAND(ID_QUALITY_AD5, OnQualityAd05)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD20, OnUpdateQualityAd20)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD10, OnUpdateQualityAd10)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD15, OnUpdateQualityAd15)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD25, OnUpdateQualityAd25)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD30, OnUpdateQualityAd30)
	ON_UPDATE_COMMAND_UI(ID_QUALITY_AD5, OnUpdateQualityAd05)
	ON_COMMAND(ID_AA_none, OnAA_None)
	ON_COMMAND(ID_AA_good, OnAA_Good)
	ON_COMMAND(ID_AA_best, OnAA_Best)
	ON_UPDATE_COMMAND_UI(ID_AA_none, OnUpdateAA_None)
	ON_UPDATE_COMMAND_UI(ID_AA_good, OnUpdateAA_Good)
	ON_UPDATE_COMMAND_UI(ID_AA_best, OnUpdateAA_Best)
	ON_UPDATE_COMMAND_UI(ID_QuadThread, OnUpdateQuadThread)
	ON_COMMAND(ID_DEMO_EGG, OnDemoEgg)
	ON_UPDATE_COMMAND_UI(ID_DEMO_EGG, OnUpdateDemoEgg)
	ON_COMMAND(ID_CAUSTICS, OnCaustics)
	ON_UPDATE_COMMAND_UI(ID_CAUSTICS, OnUpdateCaustics)
	ON_COMMAND(ID_CaIn_None, OnCaInNone)
	ON_COMMAND(ID_CaIn_Kernel, OnCaInKernel)
	ON_COMMAND(ID_CaIn_Other, OnCaInOther)
	ON_UPDATE_COMMAND_UI(ID_CaIn_None, OnUpdateCaInNone)
	ON_UPDATE_COMMAND_UI(ID_CaIn_Kernel, OnUpdateCaInKernel)
	ON_UPDATE_COMMAND_UI(ID_CaIn_Other, OnUpdateCaInOther)
	ON_COMMAND(ID_Caln_Other2, OnCalnOther2)
	ON_UPDATE_COMMAND_UI(ID_Caln_Other2, OnUpdateCalnOther2)
	ON_COMMAND(ID_CaSa_Coarse, OnCaSaCoarse)
	ON_COMMAND(ID_CaSa_Hig, OnCaSaHig)
	ON_COMMAND(ID_CaSa_Med, OnCaSaMed)
	ON_UPDATE_COMMAND_UI(ID_CaSa_Coarse, OnUpdateCaSaCoarse)
	ON_UPDATE_COMMAND_UI(ID_CaSa_Hig, OnUpdateCaSaHig)
	ON_UPDATE_COMMAND_UI(ID_CaSa_Med, OnUpdateCaSaMed)
	ON_COMMAND(ID_CaQu, OnCaQu)
	ON_UPDATE_COMMAND_UI(ID_CaQu, OnUpdateCaQu)
	ON_COMMAND(ID_CaQu_Noisy, OnCaQuNoisy)
	ON_UPDATE_COMMAND_UI(ID_CaQu_Noisy, OnUpdateCaQuNoisy)
	ON_COMMAND(ID_CaQu_Smooth, OnCaQuSmooth)
	ON_UPDATE_COMMAND_UI(ID_CaQu_Smooth, OnUpdateCaQuSmooth)
	ON_COMMAND(ID_CaDisp, OnCaDisp)
	ON_UPDATE_COMMAND_UI(ID_CaDisp, OnUpdateCaDisp)
	ON_COMMAND(ID_TEST_ASM, OnTestAsm)
	ON_UPDATE_COMMAND_UI(ID_TEST_ASM, OnUpdateTestAsm)
	ON_COMMAND(ID_DEMO_CORNELL, OnDemoCornell)
	ON_UPDATE_COMMAND_UI(ID_DEMO_CORNELL, OnUpdateDemoCornell)
	ON_COMMAND(ID_GISa_Med, OnGISaMed)
	ON_UPDATE_COMMAND_UI(ID_GISa_Med, OnUpdateGISaMed)
	ON_COMMAND(ID_GISa_Fine, OnGISaFine)
	ON_UPDATE_COMMAND_UI(ID_GISa_Fine, OnUpdateGISaFine)
	ON_COMMAND(ID_GISa_Coarse, OnGISaCoarse)
	ON_UPDATE_COMMAND_UI(ID_GISa_Coarse, OnUpdateGISaCoarse)
	ON_COMMAND(ID_GIQu_Med, OnGIQuMed)
	ON_UPDATE_COMMAND_UI(ID_GIQu_Med, OnUpdateGIQuMed)
	ON_COMMAND(ID_GIQu_Noisy, OnGIQuNoisy)
	ON_UPDATE_COMMAND_UI(ID_GIQu_Noisy, OnUpdateGIQuNoisy)
	ON_COMMAND(ID_GIQu_Smooth, OnGIQuSmooth)
	ON_UPDATE_COMMAND_UI(ID_GIQu_Smooth, OnUpdateGIQuSmooth)
	ON_COMMAND(ID_GICalc, OnGICalc)
	ON_UPDATE_COMMAND_UI(ID_GICalc, OnUpdateGICalc)
	ON_COMMAND(ID_GIDisp, OnGIDisp)
	ON_UPDATE_COMMAND_UI(ID_GIDisp, OnUpdateGIDisp)
	ON_COMMAND(ID_Caln_Diff, OnCalnDiff)
	ON_UPDATE_COMMAND_UI(ID_Caln_Diff, OnUpdateCalnDiff)
	ON_COMMAND(ID_Caln_Stack, OnCalnStack)
	ON_UPDATE_COMMAND_UI(ID_Caln_Stack, OnUpdateCalnStack)
	ON_COMMAND(ID_Caln_PreBiPatch, OnCalnPreBiPatch)
	ON_UPDATE_COMMAND_UI(ID_Caln_PreBiPatch, OnUpdateCalnPreBiPatch)
	ON_COMMAND(ID_GIOnlyIndirect, OnGIOnlyIndirect)
	ON_UPDATE_COMMAND_UI(ID_GIOnlyIndirect, OnUpdateGIOnlyIndirect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cRaViApp Konstruktion

cRaViApp::cRaViApp()
{
	// ZU ERLEDIGEN: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance platzieren
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige cRaViApp-Objekt

cRaViApp theApp;

/////////////////////////////////////////////////////////////////////////////
// cRaViApp Initialisierung

BOOL cRaViApp::InitInstance()
{
  // Standardinitialisierung
  // Wenn Sie diese Funktionen nicht nutzen und die Größe Ihrer fertigen 
  //  ausführbaren Datei reduzieren wollen, sollten Sie die nachfolgenden
  //  spezifischen Initialisierungsroutinen, die Sie nicht benötigen, entfernen.

  #ifdef _AFXDLL
    Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
  #else
    Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
  #endif

  // Ändern des Registrierungsschlüssels, unter dem unsere Einstellungen gespeichert sind.
  // ZU ERLEDIGEN: Sie sollten dieser Zeichenfolge einen geeigneten Inhalt geben
  // wie z.B. den Namen Ihrer Firma oder Organisation.
  SetRegistryKey(_T("Local AppWizard-Generated Applications"));

  // Dieser Code erstellt ein neues Rahmenfensterobjekt und setzt dies
  // dann als das Hauptfensterobjekt der Anwendung, um das Hauptfenster zu erstellen.

  CMainFrame* pFrame = new CMainFrame;
  m_pMainWnd = pFrame; Win=pFrame;

  // Rahmen mit Ressourcen erstellen und laden

  pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

  // Das einzige Fenster ist initialisiert und kann jetzt angezeigt und aktualisiert werden.
  pFrame->ShowWindow(SW_SHOW);
  pFrame->UpdateWindow();

  ThrFinished=CreateEvent(NULL, true, false, NULL);
  RV_Init(); RV_Threads=1; RV_Quality=2; RV_AntiAlias=0; RV_Filter=0;

  RV_Lights = new RV_Light; RV_Lights->next=NULL; RV_Lights->Prim=NULL; RV_Lights->v=1.0f;

  int i;
  for(i=0; i<500; i++) PHB[i]=NULL;
  for(i=0; i<500; i++) QLB[i]=NULL;

  rndinit(time(NULL)); InitSphereScene(); cframe=0;

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// cRaViApp Nachrichten-Handler


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg-Dialog für Info über Anwendung

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialogdaten
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// Überladungen für virtuelle Funktionen, die vom Anwendungs-Assistenten erzeugt wurden
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	//{{AFX_MSG(CAboutDlg)
		// Keine Nachrichten-Handler
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No Message-Handler
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void cRaViApp::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// cRaViApp-Nachrichtenbehandlungsroutinen

DWORD ts,tc,te, wt=0; short int cnt=0; char fd[80]; float cfps;

extern unsigned long int IRCnt;
extern short         int ThrL0;
extern short         int ThrL1;
extern short         int ThrL2;
extern short         int ThrL3;
extern short         int ThrL4;

BOOL cRaViApp::OnIdle(LONG lCount) 
{
  if(!cnt) ts = GetTickCount(); // start timer at first frame
  DWORD tn=GetTickCount(); if((tn-te) > 20) wt+=(tn-te);
  rndinit(12345);
  // calculate scene animation and camera movement
  float stp; double v,w; *RV_CamPtr = 7.0f*vector((float)(sin(C1)*cos(C2)), -(float)cos(C1)*(float)cos(C2), (float)sin(C2) ); IRCnt=0;
  if(Animation){
	matrix NM;
	switch(Demo){
	  case 1: ((RV_SimpleSphere *)Act1)->position.z = 1.0f+(float)fabs(sin(0.002*(double)GetTickCount()));
              break;
	  case 2: w = 0.001*(double)GetTickCount();
              ((RV_SimpleSphere *)Act1)->position=vector((float)cos(w       ), (float)sin(w       ), 0.5f);
              ((RV_SimpleSphere *)Act2)->position=vector((float)cos(w+2.0944), (float)sin(w+2.0944), 0.5f);
              ((RV_SimpleSphere *)Act3)->position=vector((float)cos(w-2.0944), (float)sin(w-2.0944), 0.5f);
			  break;
      case 3: ActM.InitIdentMat(); ActM.Scale(1.5f, 1.5f, 1.5f); ActM.RotateX(0.78539f); ActM.RotateY(0.61548f); ActM.RotateZ(-(float)fmod(0.001*(double)GetTickCount(),6.283185307179586476925286766559)); NM=ActM; ActM.Translate(0.0f, 0.0f, 1.3f);
              ((RV_TriPara *)Act1)->base=vector(-0.5f, -0.5f, -0.5f)*ActM; ((RV_TriPara *)Act1)->edg2=vector( 1.0f,  0.0f,  0.0f)*NM; ((RV_TriPara *)Act1)->edg1=vector( 0.0f,  1.0f,  0.0f)*NM; InitTP((RV_TriPara *)Act1);
              ((RV_TriPara *)Act2)->base=vector(-0.5f, -0.5f, -0.5f)*ActM; ((RV_TriPara *)Act2)->edg2=vector( 0.0f,  1.0f,  0.0f)*NM; ((RV_TriPara *)Act2)->edg1=vector( 0.0f,  0.0f,  1.0f)*NM; InitTP((RV_TriPara *)Act2);
              ((RV_TriPara *)Act3)->base=vector(-0.5f, -0.5f, -0.5f)*ActM; ((RV_TriPara *)Act3)->edg2=vector( 0.0f,  0.0f,  1.0f)*NM; ((RV_TriPara *)Act3)->edg1=vector( 1.0f,  0.0f,  0.0f)*NM; InitTP((RV_TriPara *)Act3);
              ((RV_TriPara *)Act4)->base=vector( 0.5f,  0.5f,  0.5f)*ActM; ((RV_TriPara *)Act4)->edg1=vector(-1.0f,  0.0f,  0.0f)*NM; ((RV_TriPara *)Act4)->edg2=vector( 0.0f, -1.0f,  0.0f)*NM; InitTP((RV_TriPara *)Act4);
              ((RV_TriPara *)Act5)->base=vector( 0.5f,  0.5f,  0.5f)*ActM; ((RV_TriPara *)Act5)->edg1=vector( 0.0f, -1.0f,  0.0f)*NM; ((RV_TriPara *)Act5)->edg2=vector( 0.0f,  0.0f, -1.0f)*NM; InitTP((RV_TriPara *)Act5);
              ((RV_TriPara *)Act6)->base=vector( 0.5f,  0.5f,  0.5f)*ActM; ((RV_TriPara *)Act6)->edg1=vector( 0.0f,  0.0f, -1.0f)*NM; ((RV_TriPara *)Act6)->edg2=vector(-1.0f,  0.0f,  0.0f)*NM; InitTP((RV_TriPara *)Act6);
			  break;
	  case 4: ActM.InitIdentMat();
              ActM.Translate(2.0f, -1.0f, 0.0f); ActM.RotateZ(-(float)fmod(0.00025*(double)GetTickCount(),6.283185307179586476925286766559));
              ActM.RotateY(-0.30000f); ActM.Translate(0.0f,  0.0f, 3.0f);
			  RV_Lights->p=vector(0.0f, 0.0f, 0.0f)*ActM; ((RV_SimpleSphere *) Act1)->position=RV_Lights->p;
			  break;
	  case 5: ActM.InitIdentMat(); ActM.RotateX(0.7853981634f); ActM.Translate(0.0f, 0.0f, 1.2071f);
              ActM.RotateZ(-(float)fmod(0.001*(double)GetTickCount(),6.283185307179586476925286766559));
              ((RV_Torus *)Act1)->Tb=ActM.Inverse(); ((RV_Torus *)Act1)->sinfo->NM = ActM.NTrans();
		      break;
	  case 6: w = 0.001*(double)GetTickCount();
              ((RV_SimpleSphere *)Act1)->position=vector((float)cos(w       ), (float)sin(w       ), 1.5f);
              ((RV_SimpleSphere *)Act2)->position=vector((float)cos(w+2.0944), (float)sin(w+2.0944), 1.5f);
              ((RV_SimpleSphere *)Act3)->position=vector((float)cos(w-2.0944), (float)sin(w-2.0944), 1.5f);
			  break;
	  case 8: v = /*-0.415874f;*/ -fmod(0.0003*(double)GetTickCount(),6.283185307179586476925286766559);
		      w = /*-0.486933f;*/   cos(0.0015*(double)GetTickCount());
		      ActM.InitIdentMat(); ActM.Scale(0.75f, 0.75f, 0.75f); ActM.RotateX((float)w); ActM.Translate(0.0f, 0.75f*(float)w, 0.75f); ActM.RotateZ((float)v);
              ((RV_ComplexSphere *)Act1)->Tb=ActM.Inverse(); ((RV_ComplexSphere *)Act1)->position=vector(0.0f, 0.0f, 0.0f)*ActM;
	          ActM.InitIdentMat(); ActM.Scale(0.75f, 0.75f, 1.00f); ActM.RotateX((float)w); ActM.Translate(0.0f, 0.75f*(float)w, 0.75f); ActM.RotateZ((float)v);
              ((RV_Ellipsoid *)Act2)->Tb=ActM.Inverse(); ((RV_Ellipsoid *)Act2)->sinfo->NM = ActM.NTrans();
		      break;
	}
  }
  // shoot light-rays into the scene to fill the photon maps
  if(RV_Caustics) ShootCaustics();
  if(RV_GI)       ShootLight();
  // render scene to device independant bitmap
  RV_RenderImage(ViewPort->BMPbits, ViewPort->XRes, ViewPort->YRes, 1); cframe++;
  // blit rendering to window
  CDC *dc = ViewPort->GetDC();
  SetDIBitsToDevice(dc->m_hDC, 0, 0, ViewPort->XRes, ViewPort->YRes, 1, 1, 1, ViewPort->YRes+1, ViewPort->BMPbits, &(ViewPort->BMPinfo), DIB_RGB_COLORS);
  ViewPort->ReleaseDC(dc);
  // display frame counter at statusbar
  DWORD tc=GetTickCount()-ts; cnt++;
  if(tc>250){ // update counter 4 times per second
	cfps=1000.0f/(float)(tc-wt)*(float)cnt;
	if(RV_fps){
	  stp=((float)RV_fps-cfps)*0.05f*ThrL1; if((stp<1.0f) && (stp>0.0f)) stp=1.0f; if((stp>-1.0f) && (stp<0.0f)) stp=-1.0f;
	  if(stp<=-ThrL1) stp=(float)(-ThrL1)/3.0f;
	  if(stp>256-ThrL1) stp=(float)(256-ThrL1);
	  ThrL1 += (int)stp; ThrL0=min(ThrL1/6,8);
	  ThrL2=4*ThrL1/3; ThrL3=4*ThrL2/3; ThrL4=4*ThrL3/3;
	}
    sprintf(fd,"%5.2f fps",cfps);
    //sprintf(fd,"%5.2f fps, QLc=%d, PHc=%d",cfps,QLc,PHc);
    //sprintf(fd,"C1=%5.2f, C2=%5.2f",C1,C2);
	/*if(RV_fps){
      sprintf(fd,"%5.2f fps, %d, %d, %d, %f",cfps, ThrL0, ThrL1, ThrL2, stp);
	} else {
      sprintf(fd,"%5.2f fps, %d, %d, %d",cfps, ThrL0, ThrL1, ThrL2);
	}*/
    StatusBar->SetPaneText(0,fd,TRUE); cnt=0; wt=0;
  }  
  //if(Graph){ fprintf(GraphFile,"%3d\t%6.2f\n",ThrL1, (float)IRCnt/768.0f); ThrL1++; ThrL2=4*ThrL1/3; }
  te=GetTickCount();
  return 1; // indicate we need more idle time
}

void cRaViApp::OnUpdateAnimation(CCmdUI* pCmdUI) { pCmdUI->SetCheck(Animation  ); }
void cRaViApp::OnAnimationToggle() { Animation = !Animation; if(!Animation) RV_Caustics=0; }

void cRaViApp::OnUpdateFilter(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_Filter  ); } 
void cRaViApp::OnFilter()          { RV_Filter    = !RV_Filter;  }


void cRaViApp::OnGICalc() { RV_GI = !RV_GI; if(RV_GI) RV_GI_Display=1; } 
void cRaViApp::OnUpdateGICalc(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_GI); pCmdUI->Enable(Demo==9); }

void cRaViApp::OnGIDisp() { RV_GI_Display = !RV_GI_Display; }
void cRaViApp::OnUpdateGIDisp(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_GI_Display); pCmdUI->Enable(Demo==9); }

void cRaViApp::OnUpdateCaustics(CCmdUI* pCmdUI)  { pCmdUI->SetCheck(RV_Caustics); pCmdUI->Enable((Demo==3)||(Demo==8)); }
void cRaViApp::OnCaustics()        { RV_Caustics = !RV_Caustics; if(RV_Caustics) RV_Caustics_Display=1; }

void cRaViApp::OnUpdateCaDisp(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_Caustics_Display); pCmdUI->Enable((Demo==3)||(Demo==8));}
void cRaViApp::OnCaDisp()          { RV_Caustics_Display = !RV_Caustics_Display; } 

void cRaViApp::OnQuality_1by1()       { RV_Quality=1; }
void cRaViApp::OnQualitySubsampling() { RV_Quality=2; }

void cRaViApp::OnUpdateQuality_1by1(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Quality==1); }
void cRaViApp::OnUpdateQualitySub(CCmdUI* pCmdUI)   { pCmdUI->SetCheck(RV_Quality==2); }

void cRaViApp::OnTestAsm() { UseASM = !UseASM; }
void cRaViApp::OnUpdateTestAsm(CCmdUI* pCmdUI) { pCmdUI->SetCheck(UseASM==1); }

void cRaViApp::OnCaInNone()       { RV_Caustics_Interpolation=0; }
void cRaViApp::OnCaInKernel()     { RV_Caustics_Interpolation=1; }
void cRaViApp::OnCaInOther()      { RV_Caustics_Interpolation=2; }
void cRaViApp::OnCalnOther2()     { RV_Caustics_Interpolation=3; } 
void cRaViApp::OnCalnDiff()       { RV_Caustics_Interpolation=4; } 
void cRaViApp::OnCalnStack()      { RV_Caustics_Interpolation=5; } 
void cRaViApp::OnCalnPreBiPatch() { RV_Caustics_Interpolation=6; }

void cRaViApp::OnUpdateCaInNone(CCmdUI* pCmdUI)       { pCmdUI->SetCheck(RV_Caustics_Interpolation==0); }
void cRaViApp::OnUpdateCaInKernel(CCmdUI* pCmdUI)     { pCmdUI->SetCheck(RV_Caustics_Interpolation==1); }
void cRaViApp::OnUpdateCaInOther(CCmdUI* pCmdUI)      { pCmdUI->SetCheck(RV_Caustics_Interpolation==2); }
void cRaViApp::OnUpdateCalnOther2(CCmdUI* pCmdUI)     { pCmdUI->SetCheck(RV_Caustics_Interpolation==3); } 
void cRaViApp::OnUpdateCalnDiff(CCmdUI* pCmdUI)       { pCmdUI->SetCheck(RV_Caustics_Interpolation==4); }
void cRaViApp::OnUpdateCalnStack(CCmdUI* pCmdUI)      { pCmdUI->SetCheck(RV_Caustics_Interpolation==5); }
void cRaViApp::OnUpdateCalnPreBiPatch(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Caustics_Interpolation==6); }

void cRaViApp::OnSingleThread() { RV_Threads=1; }
void cRaViApp::OnDualThread()   { RV_Threads=2; }
void cRaViApp::OnQuadThread()   { RV_Threads=4; }

void cRaViApp::OnUpdateSingleThread(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Threads==1); }
void cRaViApp::OnUpdateDualThread(CCmdUI* pCmdUI)   { pCmdUI->SetCheck(RV_Threads==2); }
void cRaViApp::OnUpdateQuadThread(CCmdUI* pCmdUI)   { pCmdUI->SetCheck(RV_Threads==4); }

void cRaViApp::OnMirrorSphere()   { InitSphereScene();     RV_Caustics_Interpolation=1; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=1; }
void cRaViApp::OnDemo3Spheres()   { Init3SpheresScene();   RV_Caustics_Interpolation=1; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=2; }
void cRaViApp::OnDemoCube()       { InitCubeScene();       RV_Caustics_Interpolation=6; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=3; }
void cRaViApp::OnDemoShadows()    { InitShadowScene();     RV_Caustics_Interpolation=0; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=4; }
void cRaViApp::OnDemoTorus()      { InitTorusScene();      RV_Caustics_Interpolation=1; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=5; }
void cRaViApp::OnDemoSphereHeap() { InitSphereHeapScene(); RV_Caustics_Interpolation=1; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=6; }
void cRaViApp::OnDemoAreaLight()  { InitAreaLightScene();  RV_Caustics_Interpolation=0; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=0; RV_GI_Display=0; Demo=7; }
void cRaViApp::OnDemoEgg()        { InitEggScene();        RV_Caustics_Interpolation=1; RV_Caustics=1; RV_Caustics_Display=1; RV_GI=0; RV_GI_Display=0; Demo=8; }
void cRaViApp::OnDemoCornell()    { InitCornellScene();    RV_Caustics_Interpolation=6; RV_Caustics=0; RV_Caustics_Display=0; RV_GI=1; RV_GI_Display=1; Demo=9; } 

void cRaViApp::OnUpdateDemoMirrSph(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(Demo==1); }
void cRaViApp::OnUpdate3Spheres(CCmdUI* pCmdUI)       { pCmdUI->SetCheck(Demo==2); }
void cRaViApp::OnUpdateDemoCube(CCmdUI* pCmdUI)       { pCmdUI->SetCheck(Demo==3); }
void cRaViApp::OnUpdateShadows(CCmdUI* pCmdUI)        { pCmdUI->SetCheck(Demo==4); }
void cRaViApp::OnUpdateDemoTorus(CCmdUI* pCmdUI)      { pCmdUI->SetCheck(Demo==5); }
void cRaViApp::OnUpdateDemoSphereHeap(CCmdUI* pCmdUI) { pCmdUI->SetCheck(Demo==6); }
void cRaViApp::OnUpdateDemoAreaLight(CCmdUI* pCmdUI)  { pCmdUI->SetCheck(Demo==7); }
void cRaViApp::OnUpdateDemoEgg(CCmdUI* pCmdUI)        { pCmdUI->SetCheck(Demo==8); }
void cRaViApp::OnUpdateDemoCornell(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(Demo==9); } 

void cRaViApp::OnShaderPhong()      { RV_Shader = &Shade;           }
void cRaViApp::OnShaderSimple()     { RV_Shader = &ShadeSimple;     } 
void cRaViApp::OnShaderNormals()    { RV_Shader = &ShadeNormals;    }
void cRaViApp::OnShaderReflection() { RV_Shader = &ShadeReflection;	}
void cRaViApp::OnShaderDepth()      { RV_Shader = &ShadeDepth;      } 
void cRaViApp::OnShaderUV()         { RV_Shader = &ShadeUV;         } 

void cRaViApp::OnUpdateShaderPhong(CCmdUI* pCmdUI)      { pCmdUI->SetCheck(RV_Shader == &Shade          ); }
void cRaViApp::OnUpdateShaderSimple(CCmdUI* pCmdUI)     { pCmdUI->SetCheck(RV_Shader == &ShadeSimple    ); } 
void cRaViApp::OnUpdateShaderNormals(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_Shader == &ShadeNormals   ); }
void cRaViApp::OnUpdateShaderReflection(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Shader == &ShadeReflection); }
void cRaViApp::OnUpdateShaderDepth(CCmdUI* pCmdUI)      { pCmdUI->SetCheck(RV_Shader == &ShadeDepth     ); } 
void cRaViApp::OnUpdateShaderUV(CCmdUI* pCmdUI)         { pCmdUI->SetCheck(RV_Shader == &ShadeUV        ); } 

extern CMainFrame *FrmWin;

void SizeWin(int w, int h) 
{
  int cx = w+2*GetSystemMetrics(SM_CXSIZEFRAME)+4;
  int cy = h+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYMENU)+22;
  RECT win; FrmWin->GetWindowRect(&win);
  FrmWin->MoveWindow(win.left, win.top, cx, cy, TRUE);	
}

void cRaViApp::OnRes320() { SizeWin(320,240); }
void cRaViApp::OnRes480() { SizeWin(480,360); }
void cRaViApp::OnRes512() { SizeWin(512,384); }
void cRaViApp::OnRes640() { SizeWin(640,480); }
void cRaViApp::OnRes800() { SizeWin(800,600); }

void cRaViApp::OnSampRaw16() { ThrL0=9999; ThrL1=300; ThrL2=300; ThrL3=300; ThrL4=300; RV_fps=0; }
void cRaViApp::OnSampInt16() { ThrL0=  -1; ThrL1=300; ThrL2=300; ThrL3=300; ThrL4=300; RV_fps=0; }
void cRaViApp::OnSampInt4()  { ThrL0=  -1; ThrL1= -1; ThrL2=300; ThrL3=300; ThrL4=300; RV_fps=0; }
void cRaViApp::OnSampFull()  { ThrL0=  -1; ThrL1= -1; ThrL2= -1; ThrL3= 48; ThrL4= 64; RV_fps=0; } 
void cRaViApp::OnSampStd()   { ThrL0=   5; ThrL1= 24; ThrL2= 32; ThrL3= 48; ThrL4= 64; RV_fps=0; }

void cRaViApp::OnQualityAd05() { RV_fps= 5; } 
void cRaViApp::OnQualityAd10() { RV_fps=10; } 
void cRaViApp::OnQualityAd15() { RV_fps=15; }
void cRaViApp::OnQualityAd20() { RV_fps=20; }
void cRaViApp::OnQualityAd25() { RV_fps=25; }
void cRaViApp::OnQualityAd30() { RV_fps=30; } 

void cRaViApp::OnUpdateQualityAd05(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps ==  5); }
void cRaViApp::OnUpdateQualityAd10(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps == 10); }
void cRaViApp::OnUpdateQualityAd15(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps == 15); }
void cRaViApp::OnUpdateQualityAd20(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps == 20); }
void cRaViApp::OnUpdateQualityAd25(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps == 25); }
void cRaViApp::OnUpdateQualityAd30(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_fps == 30); }

void cRaViApp::OnAA_None() { RV_AntiAlias = 0; } 
void cRaViApp::OnAA_Good() { RV_AntiAlias = 1; }
void cRaViApp::OnAA_Best() { RV_AntiAlias = 2; }

void cRaViApp::OnUpdateAA_None(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_AntiAlias == 0); } 
void cRaViApp::OnUpdateAA_Good(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_AntiAlias == 1); }
void cRaViApp::OnUpdateAA_Best(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_AntiAlias == 2); }

void cRaViApp::OnGIOnlyIndirect() { RV_GIOnlyIndirect = !RV_GIOnlyIndirect; }

void cRaViApp::OnUpdateGIOnlyIndirect(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_GIOnlyIndirect); }

void cRaViApp::OnCaSaCoarse() { RV_Caustics_Samples =  2500; }  
void cRaViApp::OnCaSaMed()    { RV_Caustics_Samples = 10000; }
void cRaViApp::OnCaSaHig()    { RV_Caustics_Samples = 40000; } 

void cRaViApp::OnUpdateCaSaCoarse(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Caustics_Samples ==  2500); }
void cRaViApp::OnUpdateCaSaMed(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_Caustics_Samples == 10000); }
void cRaViApp::OnUpdateCaSaHig(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_Caustics_Samples == 40000); }

void cRaViApp::OnGISaFine()   { RV_GI_Samples =  2000000; }
void cRaViApp::OnGISaMed()    { RV_GI_Samples =   500000; }
void cRaViApp::OnGISaCoarse() { RV_GI_Samples =   125000; }

void cRaViApp::OnUpdateGISaFine(CCmdUI* pCmdUI)   { pCmdUI->SetCheck(RV_GI_Samples == 2000000); }
void cRaViApp::OnUpdateGISaMed(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_GI_Samples ==  500000); }
void cRaViApp::OnUpdateGISaCoarse(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_GI_Samples ==  125000); }

void cRaViApp::OnGIQuNoisy()  { RV_GI_Quality = 1000; }
void cRaViApp::OnGIQuMed()    { RV_GI_Quality = 2500; }
void cRaViApp::OnGIQuSmooth() { RV_GI_Quality = 5000; }

void cRaViApp::OnUpdateGIQuNoisy(CCmdUI* pCmdUI)  { pCmdUI->SetCheck(RV_GI_Quality == 1000); }
void cRaViApp::OnUpdateGIQuMed(CCmdUI* pCmdUI)    { pCmdUI->SetCheck(RV_GI_Quality == 2500); }
void cRaViApp::OnUpdateGIQuSmooth(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_GI_Quality == 5000); }

void cRaViApp::OnCaQuNoisy()  { RV_Caustics_Quality = 20; }
void cRaViApp::OnCaQu()       { RV_Caustics_Quality = 30; }
void cRaViApp::OnCaQuSmooth() { RV_Caustics_Quality = 40; }

void cRaViApp::OnUpdateCaQuNoisy(CCmdUI* pCmdUI)  { pCmdUI->SetCheck(RV_Caustics_Quality == 20); }
void cRaViApp::OnUpdateCaQu(CCmdUI* pCmdUI)       { pCmdUI->SetCheck(RV_Caustics_Quality == 30); }
void cRaViApp::OnUpdateCaQuSmooth(CCmdUI* pCmdUI) { pCmdUI->SetCheck(RV_Caustics_Quality == 40); }
