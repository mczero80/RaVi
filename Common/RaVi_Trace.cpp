#include "stdafx.h"
#include "general.h"
#include "materials.h"

#include "RaVi.h"

vector  RV_Target;
vector  RV_Up;

vector *RV_CamPtr;
vector *RV_RayPtr;
IntPar *RV_IntPtr;
float  *RV_MDP;

#include "RaVi_Engine.h"

ushort    UseASM=0;

BSPplane *RV_SceneTree;
Material  RV_Environment;
RV_PrP    RV_LtObj=NULL;

extern "C" {
  ushort      cframe;
  RV_TriPara *bckdrp;
  LitPtr      RV_Lights=NULL;
  RGB_Color   Ambient = RGB_Color(0.4f, 0.4f, 0.4f);
}

float   ray_thresh;

extern "C" {
  vector RV_Cam;
  vector RV_Ray;
  IntPar RV_Int;
}

void RV_Init()
{
  RV_CamPtr = &RV_Cam;
  RV_RayPtr = &RV_Ray;
  RV_IntPtr = &RV_Int;

  ((float *)(RV_CamPtr))[3]=1.0f;
  ((float *)(RV_RayPtr))[3]=1.0f;

  ray_thresh=0.01f; if(CheckSSE()) UseASM=1;

  MatPtr mat=new Material; mat->next=NULL; mat->PBExp = 20.0f;
  mat->Reflection=RGB_Color(0.0f, 0.0f, 0.0f);
  mat->Specularity=0.0f; mat->type=0; mat->name = strdup("Backdrop");

  bckdrp = new RV_TriPara; bckdrp->type=1; bckdrp->flags=RV_CastNoShadow | RV_ReceiveNoShadow; bckdrp->Mat=mat;
  bckdrp->base=vector(0.0f, 0.0f, 0.0f);
  bckdrp->edg1=vector(1.0f, 0.0f, 0.0f);
  bckdrp->edg2=vector(0.0f, 1.0f, 0.0f);
  InitTP(bckdrp);
}

void RV_IntersectSimpleSphere0(Thrdata *Thread, RV_SimpleSphere *s)
{
  if(s->lastframe != cframe){
    s->pvec  = *RV_CamPtr - s->position;
    s->cterm = s->pvec.x*s->pvec.x + s->pvec.y*s->pvec.y + s->pvec.z*s->pvec.z - s->r2;
    s->lastframe = cframe;
  }
  float b = Thread->CamRay.x*s->pvec.x + Thread->CamRay.y*s->pvec.y + Thread->CamRay.z*s->pvec.z;
  float d = b*b-s->cterm;
  Thread->Cint.t  = ((d>=0.0f)? -b - sqrtf(d) : -1.0f);
}

extern "C" {
void RV_IntersectSimpleSphere1(RV_SimpleSphere *s, vector *Org, vector *Ray, IntPar *Ints)
{
  vector pvc = *Org - s->position;
  float b = Ray->x*pvc.x + Ray->y*pvc.y + Ray->z*pvc.z;
  float c = pvc .x*pvc.x + pvc .y*pvc.y + pvc .z*pvc.z - s->r2;
  float d = b*b-c;
  if(d<=0.0f) { Ints->t = -1.0f; return; }
  float t1 = -b - sqrtf(d);
  float t2 = -b + sqrtf(d);
  if(t1<t2){
    Ints->t = ((t1>0.001f)? t1 : t2);
  } else {
    Ints->t = ((t2>0.001f)? t2 : t1);
  }
}
}

extern "C" {
void RV_IntersectComplexSphere1(RV_ComplexSphere *s, vector *Org, vector *Ray, IntPar *Ints)
{
  vector pvc = *Org - s->position;
  float b = Ray->x*pvc.x + Ray->y*pvc.y + Ray->z*pvc.z;
  float c = pvc .x*pvc.x + pvc .y*pvc.y + pvc .z*pvc.z - s->r2;
  float d = b*b-c;
  if(d<=0.0f) { Ints->t = -1.0f; return; }
  vector TRay=vector(Ray->x*s->Tb.M11 + Ray->y*s->Tb.M21 + Ray->z*s->Tb.M31,
	                 Ray->x*s->Tb.M12 + Ray->y*s->Tb.M22 + Ray->z*s->Tb.M32,
	                 Ray->x*s->Tb.M13 + Ray->y*s->Tb.M23 + Ray->z*s->Tb.M33);
  vector TCam=(*Org) * s->Tb;
  float t1 = -b - sqrtf(d); vector IP1 = TCam+t1*TRay; float v1=ang(IP1.x, IP1.y); float u1=asinf(IP1.z);
  float t2 = -b + sqrtf(d); vector IP2 = TCam+t2*TRay; float v2=ang(IP2.x, IP2.y); float u2=asinf(IP2.z);
  if(s->vangle){
    if((u1 < s->ustart) || (u1 > s->ustop) || (v1 > s->vangle)){
      if((u2 < s->ustart) || (u2 > s->ustop) || (v2 > s->vangle)){
	    Ints->t = -1.0f; return;
	  } else {
        Ints->t=t2; Ints->u=u2; Ints->v=v2;
	  }
	} else {
	  if((u2 < s->ustart) || (u2 > s->ustop) || (v2 > s->vangle)){
        Ints->t=t1; Ints->u=u1; Ints->v=v1;
	  } else {
	    if(t1<t2){
	      if(t1>0.001){
            Ints->t=t1; Ints->u=u1; Ints->v=v1;
		  } else {
            Ints->t=t2; Ints->u=u2; Ints->v=v2;
		  }
		} else {
	      if(t2>0.001){
            Ints->t=t2; Ints->u=u2; Ints->v=v2;
		  } else {
            Ints->t=t1; Ints->u=u1; Ints->v=v1;
		  }
		}
	  }
	}
  } else {
	if(t1<t2){
	  if(t1>0.001){
        Ints->t=t1; Ints->u=u1; Ints->v=v1;
	  } else {
        Ints->t=t2; Ints->u=u2; Ints->v=v2;
	  }
	} else {
	  if(t2>0.001){
        Ints->t=t2; Ints->u=u2; Ints->v=v2;
	  } else {
        Ints->t=t1; Ints->u=u1; Ints->v=v1;
	  }
	}
  }
}
}

extern "C" {
  void RV_IntersectComplexSphere0(Thrdata *Thread, RV_ComplexSphere *s) { RV_IntersectComplexSphere1(s, RV_CamPtr, &(Thread->CamRay), &(Thread->Cint)); }
}

extern "C" {
void RV_IntersectEllipsoid1(RV_Ellipsoid *e, vector *Org, vector *Ray, IntPar *Ints)
{
  vector TRay=vector((*Ray).x*e->Tb.M11 + (*Ray).y*e->Tb.M21 + (*Ray).z*e->Tb.M31,
	                 (*Ray).x*e->Tb.M12 + (*Ray).y*e->Tb.M22 + (*Ray).z*e->Tb.M32,
	                 (*Ray).x*e->Tb.M13 + (*Ray).y*e->Tb.M23 + (*Ray).z*e->Tb.M33);
  vector TCam=(*Org) * e->Tb;
  float a = TRay.x*TRay.x + TRay.y*TRay.y + TRay.z*TRay.z;
  float b = TRay.x*TCam.x + TRay.y*TCam.y + TRay.z*TCam.z;
  float c = TCam.x*TCam.x + TCam.y*TCam.y + TCam.z*TCam.z-1.0f;
  float d = b*b-a*c;
  if(d<=0.0f){ Ints->t=-1.0f; return; }
  float t1 = (-b - sqrtf(d))/a; vector IP1 = TCam + t1 * TRay; float v1=ang(IP1.x, IP1.y); float u1=asinf(IP1.z);
  float t2 = (-b + sqrtf(d))/a; vector IP2 = TCam + t2 * TRay; float v2=ang(IP2.x, IP2.y); float u2=asinf(IP2.z);
  if((u1 < e->ustart) || (u1 > e->ustop) || (e->vangle && (v1 > e->vangle))){
    if((u2 < e->ustart) || (u2 > e->ustop) || (e->vangle && (v2 > e->vangle))){
	  Ints->t=-1.0f; return;
	} else {
      Ints->t=t2; Ints->IP=IP2; Ints->u=u2; Ints->v=v2;
	}
  } else {
    if((u2 < e->ustart) || (u2 > e->ustop) || (e->vangle && (v2 > e->vangle))){
      Ints->t=t1; Ints->IP=IP1; Ints->u=u1; Ints->v=v1;
	} else {
	  if(t1<t2){
	    if(t1>0.001f){
          Ints->t=t1; Ints->IP=IP1; Ints->u=u1; Ints->v=v1;
		} else {
          Ints->t=t2; Ints->IP=IP2; Ints->u=u2; Ints->v=v2;
		}
	  } else {
	    if(t2>0.001f){
          Ints->t=t2; Ints->IP=IP2; Ints->u=u2; Ints->v=v2;
		} else {
          Ints->t=t1; Ints->IP=IP1; Ints->u=u1; Ints->v=v1;
		}
	  }
	}
  }
}
}

extern "C" {
  void RV_IntersectEllipsoid0(Thrdata *Thread, RV_Ellipsoid *e) { RV_IntersectEllipsoid1(e, RV_CamPtr, &(Thread->CamRay), &(Thread->Cint)); }
}

void RV_IntersectTriPara0(Thrdata *Thread, RV_TriPara *p)
{
  float RN=(Thread->CamRay)%(p->N);
  if(RN && (!(p->flags & RV_SingleSided) || (RN<0.0f))){
    if(p->lastframe != cframe){
	   p->cpos       = (p->base - *RV_CamPtr)%(p->N);
       p->lastframe  = cframe;
	}
    Thread->Cint.t = p->cpos/RN;
    if(Thread->Cint.t>0.0f){
      Thread->Cint.IP = *RV_CamPtr + Thread->Cint.t * Thread->CamRay;
      switch(p->algn){
	    case 0: Thread->Cint.v = ((Thread->Cint.IP.y - p->base.y)                *p->edg1.x - (Thread->Cint.IP.x - p->base.x)*p->edg1.y) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.x - p->base.x - Thread->Cint.v*p->edg2.x) / p->edg1.x;
                break;
        case 1: Thread->Cint.v = ((Thread->Cint.IP.z - p->base.z)                *p->edg1.y - (Thread->Cint.IP.y - p->base.y)*p->edg1.z) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.y - p->base.y - Thread->Cint.v*p->edg2.y) / p->edg1.y;
                break;
        case 2: Thread->Cint.v = ((Thread->Cint.IP.z - p->base.z)                *p->edg1.x - (Thread->Cint.IP.x - p->base.x)*p->edg1.z) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.x - p->base.x - Thread->Cint.v*p->edg2.x) / p->edg1.x;
                break;
        case 3: Thread->Cint.v = ((Thread->Cint.IP.x - p->base.x)                *p->edg1.y - (Thread->Cint.IP.y - p->base.y)*p->edg1.x) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.y - p->base.y - Thread->Cint.v*p->edg2.y) / p->edg1.y;
                break;
        case 4: Thread->Cint.v = ((Thread->Cint.IP.y - p->base.y)                *p->edg1.z - (Thread->Cint.IP.z - p->base.z)*p->edg1.y) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.z - p->base.z - Thread->Cint.v*p->edg2.z) / p->edg1.z;
                break;
        case 5: Thread->Cint.v = ((Thread->Cint.IP.x - p->base.x)                *p->edg1.z - (Thread->Cint.IP.z - p->base.z)*p->edg1.x) * p->deno; if(Thread->Cint.v<0.0f){ Thread->Cint.t=-1.0f; return; }
                Thread->Cint.u =  (Thread->Cint.IP.z - p->base.z - Thread->Cint.v*p->edg2.z) / p->edg1.z;
                break;
	  }
      if(Thread->Cint.u>=0.0f){
        if(!(p->type)){
          if((Thread->Cint.u       +   Thread->Cint.v)>1.0f ) Thread->Cint.t=-1.0f;
		} else {
          if((Thread->Cint.u>1.0f) || (Thread->Cint.v >1.0f)) Thread->Cint.t=-1.0f;
		}
	  } else Thread->Cint.t=-1.0f;
	}
  } else Thread->Cint.t= -1.0f;
}

extern "C" {
void RV_IntersectTriPara1(RV_TriPara *p, vector *Org, vector *Ray, IntPar *Ints)
{
  float RN = *Ray % (p->N);
  if(RN && (!(p->flags & RV_SingleSided) || (RN<0.0f))){
    Ints->t = ((p->base - *Org)%(p->N))/RN;
    if(Ints->t>0.0f){
      Ints->IP = *Org + Ints->t * *Ray;
      switch(p->algn){
	    case 0: Ints->v = ((Ints->IP.y - p->base.y)         *p->edg1.x - (Ints->IP.x - p->base.x)*p->edg1.y) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.x - p->base.x - Ints->v*p->edg2.x) / p->edg1.x;
                break;
        case 1: Ints->v = ((Ints->IP.z - p->base.z)         *p->edg1.y - (Ints->IP.y - p->base.y)*p->edg1.z) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.y - p->base.y - Ints->v*p->edg2.y) / p->edg1.y;
                break;
        case 2: Ints->v = ((Ints->IP.z - p->base.z)         *p->edg1.x - (Ints->IP.x - p->base.x)*p->edg1.z) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.x - p->base.x - Ints->v*p->edg2.x) / p->edg1.x;
                break;
        case 3: Ints->v = ((Ints->IP.x - p->base.x)         *p->edg1.y - (Ints->IP.y - p->base.y)*p->edg1.x) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.y - p->base.y - Ints->v*p->edg2.y) / p->edg1.y;
                break;
        case 4: Ints->v = ((Ints->IP.y - p->base.y)         *p->edg1.z - (Ints->IP.z - p->base.z)*p->edg1.y) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.z - p->base.z - Ints->v*p->edg2.z) / p->edg1.z;
                break;
        case 5: Ints->v = ((Ints->IP.x - p->base.x)         *p->edg1.z - (Ints->IP.z - p->base.z)*p->edg1.x) * p->deno; if(Ints->v<0.0f){ Ints->t=-1.0f; return; }
                Ints->u =  (Ints->IP.z - p->base.z - Ints->v*p->edg2.z) / p->edg1.z;
                break;
	  }
      if(Ints->u>=0.0f){
        if(!(p->type)){
          if((Ints->u       +   Ints->v)>1.0f ) Ints->t=-1.0f;
		} else {
          if((Ints->u>1.0f) || (Ints->v >1.0f)) Ints->t=-1.0f;
		}
	  } else Ints->t=-1.0f;
	}
  } else Ints->t= -1.0f;
}
}

extern "C" {
void RV_IntersectCylinder0(Thrdata *Thread, RV_Cylinder *c)
{
  vector TRay=vector(Thread->CamRay.x*c->Tb.M11 + Thread->CamRay.y*c->Tb.M21 + Thread->CamRay.z*c->Tb.M31,
                     Thread->CamRay.x*c->Tb.M12 + Thread->CamRay.y*c->Tb.M22 + Thread->CamRay.z*c->Tb.M32,
                     Thread->CamRay.x*c->Tb.M13 + Thread->CamRay.y*c->Tb.M23 + Thread->CamRay.z*c->Tb.M33);
  if(c->lastframe != cframe){
    c->TCam  = (*RV_CamPtr) * c->Tb;
    if(c->r_top){
      c->cterm = c->TCam.x*c->TCam.x + c->TCam.y*c->TCam.y - sqr(1.0f - c->r_top*c->TCam.z);
    } else {
      c->cterm = c->TCam.x*c->TCam.x + c->TCam.y*c->TCam.y -     1.0f;
    }
    c->lastframe = cframe;
  }
  float a,b,d;
  if(c->r_top){
    a = TRay.x*   TRay.x + TRay.y*   TRay.y - sqr             (       c->r_top*   TRay.z);
    b = TRay.x*c->TCam.x + TRay.y*c->TCam.y + c->r_top*TRay.z*(1.0f - c->r_top*c->TCam.z);
  } else {
    a = TRay.x*   TRay.x + TRay.y*   TRay.y;
    b = TRay.x*c->TCam.x + TRay.y*c->TCam.y;
  }
  d = b*b-a*c->cterm;
  if(d<=0.0f){ Thread->Cint.t=-1.0f; return; }
  if(c->angle || (c->flags & RV_Inverse)){
    float t1 = (-b - sqrtf(d))/a; vector IP1 = c->TCam + t1 * TRay; float v1=ang(IP1.x, IP1.y);
    float t2 = (-b + sqrtf(d))/a; vector IP2 = c->TCam + t2 * TRay; float v2=ang(IP2.x, IP2.y);
    if((IP1.z < 0.0f) || (IP1.z > 1.0f) || (c->angle && (v1 > c->angle))){
      if((IP2.z < 0.0f) || (IP2.z > 1.0f) || (c->angle && (v2 > c->angle))){
        Thread->Cint.t=-1.0f; return;
      } else {
        Thread->Cint.t=t2; Thread->Cint.IP=IP2; Thread->Cint.u=v2; Thread->Cint.v=IP2.z;
      }
    } else {
      if((IP2.z < 0.0f) || (IP2.z > 1.0f) || (c->angle && (v2 > c->angle))){
        Thread->Cint.t=t1; Thread->Cint.IP=IP1; Thread->Cint.u=v1; Thread->Cint.v=IP1.z;
      } else {
        if(t1<t2){
          if(t1>0.001f){
            Thread->Cint.t=t1; Thread->Cint.IP=IP1; Thread->Cint.u=v1; Thread->Cint.v=IP1.z;
          } else {
            Thread->Cint.t=t2; Thread->Cint.IP=IP2; Thread->Cint.u=v2; Thread->Cint.v=IP2.z;
          }
        } else {
          if(t2>0.001f){
            Thread->Cint.t=t2; Thread->Cint.IP=IP2; Thread->Cint.u=v2; Thread->Cint.v=IP2.z;
          } else {
            Thread->Cint.t=t1; Thread->Cint.IP=IP1; Thread->Cint.u=v1; Thread->Cint.v=IP1.z;
          }
        }
      }
    }
  } else {
    Thread->Cint.t  = (-b - sqrtf(d))/a;
    Thread->Cint.IP.z = c->TCam.z + Thread->Cint.t * TRay.z;
    if((Thread->Cint.IP.z<0.0f) || (Thread->Cint.IP.z>1.0f)){ Thread->Cint.t=-1.0f; return; }
    Thread->Cint.IP.x = c->TCam.x + Thread->Cint.t * TRay.x;
    Thread->Cint.IP.y = c->TCam.y + Thread->Cint.t * TRay.y;
  }
}
}

extern "C" {
void RV_IntersectCylinder1(RV_Cylinder *c, vector *Org, vector *Ray, IntPar *Ints)
{
  vector TRay=vector((*Ray).x*c->Tb.M11 + (*Ray).y*c->Tb.M21 + (*Ray).z*c->Tb.M31,
	                 (*Ray).x*c->Tb.M12 + (*Ray).y*c->Tb.M22 + (*Ray).z*c->Tb.M32,
	                 (*Ray).x*c->Tb.M13 + (*Ray).y*c->Tb.M23 + (*Ray).z*c->Tb.M33);
  vector TCam=(*Org) * c->Tb;
  float a,b,e,d;
  if(c->r_top){
    a = TRay.x*TRay.x + TRay.y*TRay.y - sqr             (       c->r_top*TRay.z);
    b = TRay.x*TCam.x + TRay.y*TCam.y + c->r_top*TRay.z*(1.0f - c->r_top*TCam.z);
    e = TCam.x*TCam.x + TCam.y*TCam.y - sqr             (1.0f - c->r_top*TCam.z);
  } else {
    a = TRay.x*TRay.x + TRay.y*TRay.y;
    b = TRay.x*TCam.x + TRay.y*TCam.y;
    e = TCam.x*TCam.x + TCam.y*TCam.y - 1.0f;
  }
  d = b*b-a*e;
  if(d<=0.0f){ Ints->t=-1.0f; return; }
  if(c->angle || (c->flags & RV_Inverse)){
    float t1 = (-b - sqrtf(d))/a; vector IP1 = TCam + t1 * TRay; float v1=ang(IP1.x, IP1.y);
    float t2 = (-b + sqrtf(d))/a; vector IP2 = TCam + t2 * TRay; float v2=ang(IP2.x, IP2.y);
    if((IP1.z < 0.0f) || (IP1.z > 1.0f) || (c->angle && (v1 > c->angle))){
      if((IP2.z < 0.0f) || (IP2.z > 1.0f) || (c->angle && (v2 > c->angle))){
	    Ints->t=-1.0f; return;
	  } else {
        Ints->t=t2; Ints->IP=IP2; Ints->u=v2; Ints->v=IP2.z;
	  }
	} else {
      if((IP2.z < 0.0f) || (IP2.z > 1.0f) || (c->angle && (v2 > c->angle))){
        Ints->t=t1; Ints->IP=IP1; Ints->u=v1; Ints->v=IP1.z;
	  } else {
	    if(t1<t2){
	      if(t1>0.001f){
            Ints->t=t1; Ints->IP=IP1; Ints->u=v1; Ints->v=IP1.z;
		  } else {
            Ints->t=t2; Ints->IP=IP2; Ints->u=v2; Ints->v=IP2.z;
		  }
		} else {
	      if(t2>0.001f){
            Ints->t=t2; Ints->IP=IP2; Ints->u=v2; Ints->v=IP2.z;
		  } else {
            Ints->t=t1; Ints->IP=IP1; Ints->u=v1; Ints->v=IP1.z;
		  }
		}
	  }
	}
  } else {
    Ints->t  = (-b - sqrtf(d))/a;
    Ints->IP.z = TCam.z + Ints->t * TRay.z;
    if((Ints->IP.z<0.0f) || (Ints->IP.z>1.0f)){ Ints->t=-1.0f; return; }
    Ints->IP.x = TCam.x + Ints->t * TRay.x;
    Ints->IP.y = TCam.y + Ints->t * TRay.y;
  }
}
}

// A coefficient smaller than SMALL_ENOUGH is considered to be zero (0.0).
#define SMALL_ENOUGH 1.0e-10

#define TWO_M_PI_3  2.0943951023931954923084
#define FOUR_M_PI_3 4.1887902047863909846168

static int solve_cubic(double *x, double *y)
{
  double Q, R, Q3, R2, sQ, d, an, theta;
  double A2, a0, a1, a2, a3;

  a0 = x[0];
  if(a0==0.0){
	return(0);
  } else {
    if(a0 != 1.0){
      a1 = x[1] / a0;
      a2 = x[2] / a0;
      a3 = x[3] / a0;
    } else {
      a1 = x[1];
      a2 = x[2];
      a3 = x[3];
    }
  }

  A2 =  a1 *  a1;   Q = (A2  -  3.0 * a2) /  9.0;
  R  = (a1 * (A2 - 4.5 * a2) + 13.5 * a3) / 27.0;
  Q3 = Q  * Q * Q;
  R2 = R  * R;
  d  = Q3 - R2;
  an = a1 / 3.0;

  if(d>=0.0){
    // Three real roots.
    d = R / sqrt(Q3); theta = acos(d) / 3.0; sQ = -2.0 * sqrt(Q);
    y[0] = sQ * cos(theta              ) - an;
    y[1] = sQ * cos(theta +  TWO_M_PI_3) - an;
    y[2] = sQ * cos(theta + FOUR_M_PI_3) - an;
    return(3);
  } else {
    sQ = pow(sqrt(R2 - Q3) + fabs(R), 1.0 / 3.0);
    if(R < 0){
      y[0] =  (sQ + Q / sQ) - an;
    } else {
      y[0] = -(sQ + Q / sQ) - an;
    }
    return(1);
  }
}

static int solve_quartic(double *x, double *results)
{
  double cubic[4], roots[3];
  double c12, z, p, q, q1, q2, r, d1, d2;
  double c0, c1, c2, c3, c4;

  // Make sure the quartic has a leading coefficient of 1.0
  c0 = x[0];
  if(c0 != 1.0){
    c1 = x[1] / c0;
    c2 = x[2] / c0;
    c3 = x[3] / c0;
    c4 = x[4] / c0;
  } else {
    c1 = x[1];
    c2 = x[2];
    c3 = x[3];
    c4 = x[4];
  }

  // Compute the cubic resolvant
  c12 =               c1  * c1;
  p   = -0.37500000 * c12 + c2;
  q   =  0.12500000 * c12 * c1  - 0.5000 * c1  * c2 + c3;
  r   = -0.01171875 * c12 * c12 + 0.0625 * c12 * c2 - 0.25 * c1 * c3 + c4;

  cubic[0] =  1.0;
  cubic[1] = -0.5 * p;
  cubic[2] = -r;
  cubic[3] =  0.5 * r * p - 0.125 * q * q;

  int i = solve_cubic(cubic, roots);

  if(i>0) z = roots[0]; else return 0;

  d1 = 2.0 * z - p; if(d1 < 0.0) if(d1 > -SMALL_ENOUGH) d1 = 0.0; else return 0;

  if(d1 < SMALL_ENOUGH){
    d2 = z * z - r;
    if(d2 < 0.0) return 0;
    d2 = sqrt(d2);
  } else {
    d1 = sqrt(d1);
    d2 = 0.5 * q / d1;
  }

  // Set up useful values for the quadratic factors
  q1 =    d1 * d1;
  q2 = -0.25 * c1; i = 0;

  // Solve the first quadratic
  p = q1 - 4.0 * (z - d2);
  if(p==0.0){
    results[i++] = -0.5 * d1 - q2;
  } else {
    if(p > 0){
      p = sqrt(p);
      results[i++] = -0.5 * (d1 + p) + q2;
      results[i++] = -0.5 * (d1 - p) + q2;
    }
  }

  // Solve the second quadratic
  p = q1 - 4.0 * (z + d2);
  if(p==0.0){
    results[i++] = 0.5 * d1 - q2;
  } else {
    if(p > 0){
      p = sqrt(p);
      results[i++] = 0.5 * (d1 + p) + q2;
      results[i++] = 0.5 * (d1 - p) + q2;
    }
  }

  return i;
}

int insector(float a, float start, float stop)
{
  if(start>=0.0f){
	return ((a>=start) && (a<=stop));
  } else {
	if(stop>=0.0f){
	  return (((a-6.2831853f)>=start) || ( a            <=stop));
	} else {
	  return (((a-6.2831853f)>=start) && ((a-6.2831853f)<=stop));
	}
  }
}

extern "C" {
void RV_IntersectTorus1(RV_Torus *t, vector *Org, vector *Ray, IntPar *Ints)
{
  vector TRay=vector((*Ray).x*t->Tb.M11 + (*Ray).y*t->Tb.M21 + (*Ray).z*t->Tb.M31,
	                 (*Ray).x*t->Tb.M12 + (*Ray).y*t->Tb.M22 + (*Ray).z*t->Tb.M32,
	                 (*Ray).x*t->Tb.M13 + (*Ray).y*t->Tb.M23 + (*Ray).z*t->Tb.M33);
  vector TCam=(*Org) * t->Tb;

  double c[5], r[4], Depth[4]; float v1,v2; vector IP,N,N1,N2;

  float len=Betrag(TRay); TRay=unit(TRay);
  
  double r12  = sqr(t->R1);
  double r22  = sqr(t->R2);

  double Py2  = TCam.z * TCam.z;
  double Dy2  = TRay.z * TRay.z;
  double PDy2 = TCam.z * TRay.z;

  double k1   = TCam.x * TCam.x + TCam.y * TCam.y + Py2 - r12 - r22;
  double k2   = TCam.x * TRay.x + TCam.y * TRay.y + PDy2;

  c[0] = 1.0;
  c[1] = 4.0 *  k2;
  c[2] = 2.0 * (k1 + 2.0 * (k2 * k2 + r12 * Dy2));
  c[3] = 4.0 * (k2 * k1  + 2.0 * r12 * PDy2);
  c[4] =  k1 *  k1 + 4.0 * r12 * (Py2 - r22);

  int n = solve_quartic(c, r); int i=0; while(n--) Depth[i++] = r[n] / len;
  
  if(i){
	doubsort(i,Depth);
    Ints->t=(float)Depth[0]; IP = TCam + Ints->t * TRay*len;
	v1 = ang(IP.x, IP.y); N1=IP-vector(t->R1*cosf(v1), t->R1*sinf(v1), 0.0f);
	v2 = v1+3.1415927f;   N2=IP-vector(t->R1*cosf(v2), t->R1*sinf(v2), 0.0f);
	if(fabs(Betrag(N1) - t->R2) < fabs(Betrag(N2) - t->R2)){ Ints->v=v1; Ints->N=unit(N1); } else { Ints->v=v2; Ints->N=unit(N2); }
	Ints->u = IP.z<0.0f? 6.2831853f-(float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f)) : (float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f));
	if((Ints->t < 0.001f) || (t->vangle && ((Ints->v > t->vangle) || !insector(Ints->u, t->ustart, t->ustop)))){
	  if(i<2) { Ints->t = -1.0f; return; }
      Ints->t=(float)Depth[1]; IP = TCam + Ints->t * TRay*len;
	  v1 = ang(IP.x, IP.y); N1=IP-vector(t->R1*cosf(v1), t->R1*sinf(v1), 0.0f);
	  v2 = v1+3.1415927f;   N2=IP-vector(t->R1*cosf(v2), t->R1*sinf(v2), 0.0f);
	  if(fabs(Betrag(N1) - t->R2) < fabs(Betrag(N2) - t->R2)){ Ints->v=v1; Ints->N=unit(N1); } else { Ints->v=v2; Ints->N=unit(N2); }
	  Ints->u = IP.z<0.0f? 6.2831853f-(float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f)) : (float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f));
	  if((Ints->t < 0.001f) || (t->vangle && ((Ints->v > t->vangle) || !insector(Ints->u, t->ustart, t->ustop)))){
        if(i<3) { Ints->t = -1.0f; return; }
        Ints->t=(float)Depth[2]; IP = TCam + Ints->t * TRay*len;
	    v1 = ang(IP.x, IP.y); N1=IP-vector(t->R1*cosf(v1), t->R1*sinf(v1), 0.0f);
	    v2 = v1+3.1415927f;   N2=IP-vector(t->R1*cosf(v2), t->R1*sinf(v2), 0.0f);
	    if(fabs(Betrag(N1) - t->R2) < fabs(Betrag(N2) - t->R2)){ Ints->v=v1; Ints->N=unit(N1); } else { Ints->v=v2; Ints->N=unit(N2); }
	    Ints->u = IP.z<0.0f? 6.2831853f-(float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f)) : (float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f));
	    if((Ints->t < 0.001f) || (t->vangle && ((Ints->v > t->vangle) || !insector(Ints->u, t->ustart, t->ustop)))){
          if(i<3) { Ints->t = -1.0f; return; }
          Ints->t=(float)Depth[3]; IP = TCam + Ints->t * TRay*len;
	      v1 = ang(IP.x, IP.y); N1=IP-vector(t->R1*cosf(v1), t->R1*sinf(v1), 0.0f);
	      v2 = v1+3.1415927f;   N2=IP-vector(t->R1*cosf(v2), t->R1*sinf(v2), 0.0f);
	      if(fabs(Betrag(N1) - t->R2) < fabs(Betrag(N2) - t->R2)){ Ints->v=v1; Ints->N=unit(N1); } else { Ints->v=v2; Ints->N=unit(N2); }
	      Ints->u = IP.z<0.0f? 6.2831853f-(float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f)) : (float)acs(Ints->N % vector(cosf(Ints->v), sinf(Ints->v), 0.0f));
	      if(t->vangle && ((Ints->v > t->vangle) || !insector(Ints->u, t->ustart, t->ustop))) { Ints->t = -1.0f; return; }
	    }
	  }
	}
  } else Ints->t = -1.0f;
}
}

extern "C" {
  void RV_IntersectTorus0(Thrdata *Thread, RV_Torus *c) { RV_IntersectTorus1(c, RV_CamPtr, &(Thread->CamRay), &(Thread->Cint)); }
}

extern "C" {
void RV_IntersectParaboloid1(RV_Paraboloid *p, vector *Org, vector *Ray, IntPar *Ints)
{
  vector TRay=vector((*Ray).x*p->Tb.M11 + (*Ray).y*p->Tb.M21 + (*Ray).z*p->Tb.M31,
	                 (*Ray).x*p->Tb.M12 + (*Ray).y*p->Tb.M22 + (*Ray).z*p->Tb.M32,
	                 (*Ray).x*p->Tb.M13 + (*Ray).y*p->Tb.M23 + (*Ray).z*p->Tb.M33);
  vector TCam=(*Org) * p->Tb;
  float a = TRay.x*TRay.x + TRay.y*TRay.y;
  float b = TRay.x*TCam.x + TRay.y*TCam.y - 0.5f*TRay.z;
  float c = TCam.x*TCam.x + TCam.y*TCam.y -      TCam.z;
  float d = b*b-a*c;
  if(d<=0.0f){ Ints->t=-1.0f; return; }
  float t1 = (-b - sqrtf(d))/a; vector IP1 = TCam + t1 * TRay; float v1=ang(IP1.x, IP1.y);
  float t2 = (-b + sqrtf(d))/a; vector IP2 = TCam + t2 * TRay; float v2=ang(IP2.x, IP2.y);
  if((IP1.z > p->height) || (p->angle && (v1 > p->angle))){
    if((IP2.z > p->height) || (p->angle && (v2 > p->angle))){
	  Ints->t=-1.0f; return;
	} else {
      Ints->t=t2; Ints->IP=IP2; Ints->v=v2;
	}
  } else {
    if((IP2.z > p->height) || (p->angle && (v2 > p->angle))){
      Ints->t=t1; Ints->IP=IP1; Ints->v=v1;
	} else {
	  if(t1<t2){
	    if(t1>0.001f){
          Ints->t=t1; Ints->IP=IP1; Ints->v=v1;
		} else {
          Ints->t=t2; Ints->IP=IP2; Ints->v=v2;
		}
	  } else {
	    if(t2>0.001f){
          Ints->t=t2; Ints->IP=IP2; Ints->v=v2;
		} else {
          Ints->t=t1; Ints->IP=IP1; Ints->v=v1;
		}
	  }
	}
  }
  Ints->u=Ints->IP.z;
}
}

extern "C" {
  void RV_IntersectParaboloid0(Thrdata *Thread, RV_Paraboloid *p) { RV_IntersectParaboloid1(p, RV_CamPtr, &(Thread->CamRay), &(Thread->Cint)); }
}

extern "C" {
void RV_IntersectSimpleRing0(Thrdata *Thread, RV_SimpleRing *r)
{
  if(r->lastframe != cframe){
	 r->cpos       = (r->center-(*RV_CamPtr))%(r->N);
     r->lastframe  = cframe;
  }
  float RN=(Thread->CamRay)%(r->N);
  if(RN){
    Thread->Cint.t = r->cpos/RN;
    if(Thread->Cint.t>0.0f){
      Thread->Cint.IP = *RV_CamPtr + Thread->Cint.t * Thread->CamRay;
	  float r2=sqr(Thread->Cint.IP.x-r->center.x)+sqr(Thread->Cint.IP.y-r->center.y)+sqr(Thread->Cint.IP.z-r->center.z);
	  if(r2>r->r_o) Thread->Cint.t= -1.0f;
	} else Thread->Cint.t= -1.0f;
  } else Thread->Cint.t= -1.0f;
}
}

extern "C" {
void RV_IntersectSimpleRing1(RV_SimpleRing *r, vector *Org, vector *Ray, IntPar *Ints)
{
  float RN=(*Ray)%(r->N);
  if(RN){
    Ints->t = ((r->center-(*Org))%(r->N))/RN;
    if(Ints->t>0.0f){
      Ints->IP = *Org + Ints->t * *Ray;
	  float r2=sqr(Ints->IP.x-r->center.x)+sqr(Ints->IP.y-r->center.y)+sqr(Ints->IP.z-r->center.z);
	  if((r2>r->r_o) || (r2<r->r_i)) Ints->t= -1.0f;
	} else Ints->t= -1.0f;
  } else Ints->t= -1.0f;
}
}

extern "C" {
void RV_IntersectComplexRing1(RV_ComplexRing *r, vector *Org, vector *Ray, IntPar *Ints)
{
  float RN=(*Ray)%(r->N);
  if(RN){
    Ints->t = ((r->center-(*Org))%(r->N))/RN;
    if(Ints->t>0.0f){
      Ints->IP = *Org + Ints->t * *Ray;
	  Ints->u  = sqr(Ints->IP.x-r->center.x)+sqr(Ints->IP.y-r->center.y)+sqr(Ints->IP.z-r->center.z);
	  if((Ints->u > r->r_o) || (Ints->u < r->r_i)) { Ints->t= -1.0f; return; }
	  vector OP = Ints->IP * r->Tb; Ints->v = ang(OP.x, OP.y);
	  if(r->angle && (Ints->v > r->angle)) { Ints->t= -1.0f; return; }
	} else Ints->t= -1.0f;
  } else Ints->t= -1.0f;
}
}

extern "C" {
  void RV_IntersectComplexRing0(Thrdata *Thread, RV_ComplexRing *r) { RV_IntersectComplexRing1(r, RV_CamPtr, &(Thread->CamRay), &(Thread->Cint)); }
}

#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

extern "C" {
  vector RV_BBMin;
  vector RV_BBMax;
  vector             BBcandidatePlane;
  unsigned short int BBinside,BBquadrant[3];
  unsigned short int BBlastframe=42;
}

void RV_IntersectBoundingBox0c(Thrdata *Thread)
{
  vector             maxT;
  int                whichPlane;

  // Find candidate planes - only needed once per frame
  if(BBlastframe != cframe){
	BBlastframe  = cframe; BBinside=true;

    if(       RV_CamPtr->x < RV_BBMin.x) {
      BBquadrant[0] = LEFT;  BBcandidatePlane.x = RV_BBMin.x; BBinside=false;
	} else if(RV_CamPtr->x > RV_BBMax.x) {
      BBquadrant[0] = RIGHT; BBcandidatePlane.x = RV_BBMax.x; BBinside=false;
	} else {
      BBquadrant[0] = MIDDLE;
	}

    if(       RV_CamPtr->y < RV_BBMin.y) {
      BBquadrant[1] = LEFT;  BBcandidatePlane.y = RV_BBMin.y; BBinside=false;
	} else if(RV_CamPtr->y > RV_BBMax.y) {
      BBquadrant[1] = RIGHT; BBcandidatePlane.y = RV_BBMax.y; BBinside=false;
	} else {
      BBquadrant[1] = MIDDLE;
	}

    if(       RV_CamPtr->z < RV_BBMin.z) {
      BBquadrant[2] = LEFT;  BBcandidatePlane.z = RV_BBMin.z; BBinside=false;
	} else if(RV_CamPtr->z > RV_BBMax.z) {
      BBquadrant[2] = RIGHT; BBcandidatePlane.z = RV_BBMax.z; BBinside=false;
	} else {
      BBquadrant[2] = MIDDLE;
	}
  }

  if(BBinside){ Thread->Cint.t = 1.0f; Thread->Cint.IP = *RV_CamPtr; return; }

  // calculate distances to candidate planes
  if(BBquadrant[0] != MIDDLE && Thread->CamRay.x) maxT.x = (BBcandidatePlane.x - RV_CamPtr->x) / Thread->CamRay.x; else	maxT.x = -1.0f;
  if(BBquadrant[1] != MIDDLE && Thread->CamRay.y) maxT.y = (BBcandidatePlane.y - RV_CamPtr->y) / Thread->CamRay.y; else	maxT.y = -1.0f;
  if(BBquadrant[2] != MIDDLE && Thread->CamRay.z) maxT.z = (BBcandidatePlane.z - RV_CamPtr->z) / Thread->CamRay.z; else	maxT.z = -1.0f;

  // get largest of the maxT's for final choice of intersection
  whichPlane = 0; Thread->Cint.t = maxT.x;
  if(Thread->Cint.t < maxT.y){ whichPlane = 1; Thread->Cint.t = maxT.y; }
  if(Thread->Cint.t < maxT.z){ whichPlane = 2; Thread->Cint.t = maxT.z; }

  // check final candidate actually inside box
  if(Thread->Cint.t < 0.0f) return;

  if(whichPlane != 0) {
	Thread->Cint.IP.x = RV_CamPtr->x + Thread->Cint.t * Thread->CamRay.x;
    if(Thread->Cint.IP.x < RV_BBMin.x || Thread->Cint.IP.x > RV_BBMax.x){ Thread->Cint.t = -1.0f; return; }
  } else {
    Thread->Cint.IP.x = BBcandidatePlane.x;
  }
  if(whichPlane != 1) {
	Thread->Cint.IP.y = RV_CamPtr->y + Thread->Cint.t * Thread->CamRay.y;
    if(Thread->Cint.IP.y < RV_BBMin.y || Thread->Cint.IP.y > RV_BBMax.y){ Thread->Cint.t = -1.0f; return; }
  } else {
    Thread->Cint.IP.y = BBcandidatePlane.y;
  }
  if(whichPlane != 2) {
	Thread->Cint.IP.z = RV_CamPtr->z + Thread->Cint.t * Thread->CamRay.z;
    if(Thread->Cint.IP.z < RV_BBMin.z || Thread->Cint.IP.z > RV_BBMax.z){ Thread->Cint.t = -1.0f; return; }
  } else {
    Thread->Cint.IP.z = BBcandidatePlane.z;
  }
}

void RV_IntersectBoundingBox1(vector *Org, vector *Ray, IntPar *Cint)
{
  vector  maxT;
  vector  candidatePlane;
  int     whichPlane;
  char    quadrant[3];
  bool    inside=true;

  // Find candidate planes
  if(       Org->x < RV_BBMin.x) {
    quadrant[0] = LEFT;  candidatePlane.x = RV_BBMin.x; inside=false;
  } else if(Org->x > RV_BBMax.x) {
    quadrant[0] = RIGHT; candidatePlane.x = RV_BBMax.x; inside=false;
  } else {
    quadrant[0] = MIDDLE;
  }

  if(       Org->y < RV_BBMin.y) {
    quadrant[1] = LEFT;  candidatePlane.y = RV_BBMin.y; inside=false;
  } else if(Org->y > RV_BBMax.y) {
    quadrant[1] = RIGHT; candidatePlane.y = RV_BBMax.y; inside=false;
  } else {
    quadrant[1] = MIDDLE;
  }

  if(       Org->z < RV_BBMin.z) {
    quadrant[2] = LEFT;  candidatePlane.z = RV_BBMin.z; inside=false;
  } else if(Org->z > RV_BBMax.z) {
    quadrant[2] = RIGHT; candidatePlane.z = RV_BBMax.z; inside=false;
  } else {
    quadrant[2] = MIDDLE;
  }

  if(inside){ Cint->t = 1.0f; Cint->IP = *Org; return; }

  // calculate distances to candidate planes
  if(quadrant[0] != MIDDLE && Ray->x) maxT.x = (candidatePlane.x - Org->x) / Ray->x; else	maxT.x = -1.0f;
  if(quadrant[1] != MIDDLE && Ray->y) maxT.y = (candidatePlane.y - Org->y) / Ray->y; else	maxT.y = -1.0f;
  if(quadrant[2] != MIDDLE && Ray->z) maxT.z = (candidatePlane.z - Org->z) / Ray->z; else	maxT.z = -1.0f;

  // get largest of the maxT's for final choice of intersection
  whichPlane = 0; Cint->t = maxT.x;
  if(Cint->t < maxT.y){ whichPlane = 1; Cint->t = maxT.y; }
  if(Cint->t < maxT.z){ whichPlane = 2; Cint->t = maxT.z; }

  // check final candidate actually inside box
  if(Cint->t < 0.0f) return;

  if(whichPlane != 0) {
	Cint->IP.x = Org->x + Cint->t * Ray->x;
    if(Cint->IP.x < RV_BBMin.x || Cint->IP.x > RV_BBMax.x){ Cint->t = -1.0f; return; }
  } else {
    Cint->IP.x = candidatePlane.x;
  }
  if(whichPlane != 1) {
	Cint->IP.y = Org->y + Cint->t * Ray->y;
    if(Cint->IP.y < RV_BBMin.y || Cint->IP.y > RV_BBMax.y){ Cint->t = -1.0f; return; }
  } else {
    Cint->IP.y = candidatePlane.y;
  }
  if(whichPlane != 2) {
	Cint->IP.z = Org->z + Cint->t * Ray->z;
    if(Cint->IP.z < RV_BBMin.z || Cint->IP.z > RV_BBMax.z){ Cint->t = -1.0f; return; }
  } else {
    Cint->IP.z = candidatePlane.z;
  }
}

void GetIPN(vector *Org, vector *Ray, IntPar *I)
{
  if(I->p->type < 2){
    if(((RV_TriPara *)(I->p))->sinfo && ((RV_TriPara *)(I->p))->sinfo->nrm){
      // phong interpolation
      I->N = unit(((RV_TriPara *)(I->p))->sinfo->nrm[0] + I->u*(((RV_TriPara *)(I->p))->sinfo->nrm[1] - ((RV_TriPara *)(I->p))->sinfo->nrm[0]) + I->v*(((RV_TriPara *)(I->p))->sinfo->nrm[2] - ((RV_TriPara *)(I->p))->sinfo->nrm[0]));
     } else {
      I->N  = ((RV_TriPara *)(I->p))->N;
     }
  } else if(I->p->type == 2){
    I->IP = *Org + I->t * *Ray;
    I->N  = unit(I->IP - ((RV_SimpleSphere *)(I->p))->position);
  } else if(I->p->type == 3){
    I->N  = unit(vector(I->IP.x, I->IP.y, (1.0f - I->IP.z)*((RV_Cylinder *)I->p)->r_top)*((RV_Cylinder *)I->p)->sinfo->NM);
    I->IP = *Org + I->t * *Ray;
  } else if(I->p->type == 4){
    I->IP = *Org + I->t * *Ray;
    I->N  = unit( (I->i<4)? ((I->i<2)? ((RV_Cube *)(I->p))->axis1: ((RV_Cube *)(I->p))->axis2) : ((RV_Cube *)(I->p))->axis3 );
    if((I->i) & 1) I->N = -(I->N);
  } else if(I->p->type == 5){
    I->N  = ((RV_SimpleRing *)(I->p))->N;
  } else if(I->p->type == 6){
    I->N  = unit(I->N * ((RV_Torus *)I->p)->sinfo->NM);
    I->IP = *Org + I->t * *Ray;
  } else if(I->p->type == 7){
    I->IP = *Org + I->t * *Ray;
    I->N  = unit(I->IP - ((RV_ComplexSphere *)(I->p))->position);
  } else if(I->p->type == 8){
    I->N  = unit(vector(2.0f*I->IP.x, 2.0f*I->IP.y, -1.0f) * ((RV_Paraboloid *)I->p)->sinfo->NM);
    I->IP = *Org + I->t * *Ray;
  } else if(I->p->type == 9){
    I->N  = unit(I->IP * ((RV_Ellipsoid *)I->p)->sinfo->NM);
    I->IP = *Org + I->t * *Ray;
  } else if(I->p->type == 10){
    I->N  = ((RV_ComplexRing *)(I->p))->N;
  }
}

void GetTXY(IntPar *I)
{
  if(I->p->type == 0){
    if(((RV_TriPara *)(I->p))->sinfo && ((RV_TriPara *)(I->p))->sinfo->txc){
	  I->x = ((RV_TriPara *)(I->p))->sinfo->txc[0].x + I->u*(((RV_TriPara *)(I->p))->sinfo->txc[1].x-((RV_TriPara *)(I->p))->sinfo->txc[0].x) + I->v*(((RV_TriPara *)(I->p))->sinfo->txc[2].x-((RV_TriPara *)(I->p))->sinfo->txc[0].x);
	  I->y = ((RV_TriPara *)(I->p))->sinfo->txc[0].y + I->u*(((RV_TriPara *)(I->p))->sinfo->txc[1].y-((RV_TriPara *)(I->p))->sinfo->txc[0].y) + I->v*(((RV_TriPara *)(I->p))->sinfo->txc[2].y-((RV_TriPara *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->u;
	  I->y = I->v;
	}
  } else if(I->p->type == 1){
    if(((RV_TriPara *)(I->p))->sinfo && ((RV_TriPara *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->u, I->v, ((RV_TriPara *)(I->p))->sinfo->txc[3].x, ((RV_TriPara *)(I->p))->sinfo->txc[1].x, ((RV_TriPara *)(I->p))->sinfo->txc[2].x, ((RV_TriPara *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->u, I->v, ((RV_TriPara *)(I->p))->sinfo->txc[3].y, ((RV_TriPara *)(I->p))->sinfo->txc[1].y, ((RV_TriPara *)(I->p))->sinfo->txc[2].y, ((RV_TriPara *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->u;
	  I->y = I->v;
	}
  } else if(I->p->type == 3){
	I->u /= ((RV_Cylinder *)I->p)->angle;
    if(((RV_Cylinder *)(I->p))->sinfo && ((RV_Cylinder *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->v, I->u, ((RV_Cylinder *)(I->p))->sinfo->txc[3].x, ((RV_Cylinder *)(I->p))->sinfo->txc[2].x, ((RV_Cylinder *)(I->p))->sinfo->txc[1].x, ((RV_Cylinder *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->v, I->u, ((RV_Cylinder *)(I->p))->sinfo->txc[3].y, ((RV_Cylinder *)(I->p))->sinfo->txc[2].y, ((RV_Cylinder *)(I->p))->sinfo->txc[1].y, ((RV_Cylinder *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->u;
	  I->y = I->v;
    }
  } else if(I->p->type == 6){
	I->v /= (((RV_Torus *)I->p)->vangle? ((RV_Torus *)I->p)->vangle : 6.2831853f);
	if((((RV_Torus *)I->p)->ustart < 0.0f) && ((I->u-6.2831853f) >= ((RV_Torus *)I->p)->ustart)){
	  I->u = ((I->u-6.2831853f)-((RV_Torus *)I->p)->ustart)/(((RV_Torus *)I->p)->ustop-((RV_Torus *)I->p)->ustart);
	} else {
	  I->u = ( I->u            -((RV_Torus *)I->p)->ustart)/(((RV_Torus *)I->p)->ustop-((RV_Torus *)I->p)->ustart);
	}
    if(((RV_Torus *)(I->p))->sinfo && ((RV_Torus *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->v, I->u, ((RV_Torus *)(I->p))->sinfo->txc[3].x, ((RV_Torus *)(I->p))->sinfo->txc[1].x, ((RV_Torus *)(I->p))->sinfo->txc[2].x, ((RV_Torus *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->v, I->u, ((RV_Torus *)(I->p))->sinfo->txc[3].y, ((RV_Torus *)(I->p))->sinfo->txc[1].y, ((RV_Torus *)(I->p))->sinfo->txc[2].y, ((RV_Torus *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->v;
	  I->y = I->u;
    }
  } else if(I->p->type == 7){
	I->v /= (((RV_ComplexSphere *)I->p)->vangle? ((RV_ComplexSphere *)I->p)->vangle : 6.2831853f);
    I->u  = (I->u-((RV_ComplexSphere *)I->p)->ustart)/(((RV_ComplexSphere *)I->p)->ustop-((RV_ComplexSphere *)I->p)->ustart);
    if(((RV_ComplexSphere *)(I->p))->sinfo && ((RV_ComplexSphere *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->u, I->v, ((RV_ComplexSphere *)(I->p))->sinfo->txc[3].x, ((RV_ComplexSphere *)(I->p))->sinfo->txc[2].x, ((RV_ComplexSphere *)(I->p))->sinfo->txc[1].x, ((RV_ComplexSphere *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->u, I->v, ((RV_ComplexSphere *)(I->p))->sinfo->txc[3].y, ((RV_ComplexSphere *)(I->p))->sinfo->txc[2].y, ((RV_ComplexSphere *)(I->p))->sinfo->txc[1].y, ((RV_ComplexSphere *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->v;
	  I->y = I->u;
    }
  } else if(I->p->type == 8){
	I->v /= (((RV_Paraboloid *)I->p)->angle? ((RV_Paraboloid *)I->p)->angle : 6.2831853f);
    I->u  = sqrtf(I->u);
    if(((RV_Paraboloid *)(I->p))->sinfo && ((RV_Paraboloid *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->u, I->v, ((RV_Paraboloid *)(I->p))->sinfo->txc[3].x, ((RV_Paraboloid *)(I->p))->sinfo->txc[2].x, ((RV_Paraboloid *)(I->p))->sinfo->txc[1].x, ((RV_Paraboloid *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->u, I->v, ((RV_Paraboloid *)(I->p))->sinfo->txc[3].y, ((RV_Paraboloid *)(I->p))->sinfo->txc[2].y, ((RV_Paraboloid *)(I->p))->sinfo->txc[1].y, ((RV_Paraboloid *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->v;
	  I->y = I->u;
    }
  } else if(I->p->type == 9){
	I->v /= (((RV_Ellipsoid *)I->p)->vangle? ((RV_Ellipsoid *)I->p)->vangle : 6.2831853f);
    I->u  = (I->u-((RV_Ellipsoid *)I->p)->ustart)/(((RV_Ellipsoid *)I->p)->ustop-((RV_Ellipsoid *)I->p)->ustart);
    if(((RV_Ellipsoid *)(I->p))->sinfo && ((RV_Ellipsoid *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->u, I->v, ((RV_Ellipsoid *)(I->p))->sinfo->txc[3].x, ((RV_Ellipsoid *)(I->p))->sinfo->txc[2].x, ((RV_Ellipsoid *)(I->p))->sinfo->txc[1].x, ((RV_Ellipsoid *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->u, I->v, ((RV_Ellipsoid *)(I->p))->sinfo->txc[3].y, ((RV_Ellipsoid *)(I->p))->sinfo->txc[2].y, ((RV_Ellipsoid *)(I->p))->sinfo->txc[1].y, ((RV_Ellipsoid *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->v;
	  I->y = I->u;
    }
  } else if(I->p->type == 10){
	I->v /= (((RV_ComplexRing *)I->p)->angle? ((RV_ComplexRing *)I->p)->angle : 6.2831853f);
	float r_i=sqrtf(((RV_ComplexRing *)I->p)->r_i);
	float r_o=sqrtf(((RV_ComplexRing *)I->p)->r_o);
	I->u  = (sqrtf(I->u)-r_i)/(r_o-r_i);
    if(((RV_ComplexRing *)(I->p))->sinfo && ((RV_ComplexRing *)(I->p))->sinfo->txc){
	  I->x = bilinear(I->u, I->v, ((RV_ComplexRing *)(I->p))->sinfo->txc[3].x, ((RV_ComplexRing *)(I->p))->sinfo->txc[2].x, ((RV_ComplexRing *)(I->p))->sinfo->txc[1].x, ((RV_ComplexRing *)(I->p))->sinfo->txc[0].x);
	  I->y = bilinear(I->u, I->v, ((RV_ComplexRing *)(I->p))->sinfo->txc[3].y, ((RV_ComplexRing *)(I->p))->sinfo->txc[2].y, ((RV_ComplexRing *)(I->p))->sinfo->txc[1].y, ((RV_ComplexRing *)(I->p))->sinfo->txc[0].y);
	} else {
	  I->x = I->v;
	  I->y = I->u;
    }
  }
}

float R_fnf(float n1, float n2, double ce1);
float wlcoff(float cR, float cG, float cB, float v);
RGB_Color CallShade(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar Cmin, float em, float dist, int level, Thrdata *Thread);
extern float wavlen;

RGB_Color ShadeDepth(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  GetIPN(Org, Ray, &IntP);
  float d = 1.0f-((distance(*Org,IntP.IP)-4.17f)/5.66f);
  return RGB_Color(d, d, d);
}

RGB_Color SIMD_ShNorm(vector *Org, vector Ray, float t, RV_PrP p)
{
  vector N;
  if(p->type<2){
    N=((RV_TriPara *)(p))->N;
  } else {
    vector IP = *Org + t*Ray; N  = unit(IP - ((RV_SimpleSphere *)p)->position);
  }
  return RGB_Color(0.5f*(1.0f+N.x), 0.5f*(1.0f+N.y), 0.5f*(1.0f+N.z));
}

RGB_Color ShadeNormals(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  GetIPN(Org, Ray, &IntP);
  return RGB_Color(0.5f*(1.0f+IntP.N.x), 0.5f*(1.0f+IntP.N.y), 0.5f*(1.0f+IntP.N.z));
}

RGB_Color ShadeReflection(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  GetIPN(Org, Ray, &IntP);
  vector L =  unit(RV_Lights->p - IntP.IP);
  vector R = -unit(2.0f*(IntP.N % L)*(IntP.N)-L);
  return RGB_Color(0.5f*(1.0f+R.x), 0.5f*(1.0f+R.y), 0.5f*(1.0f+R.z));
}

short int RV_Caustics_Interpolation=1;
short int RV_Caustics_Display=0;

float GetHit(QLP QL, float xb, float yb, float l, float x, float y)
{
  float l2=0.5f*l;
  if(y<(yb+l2)){
    if(x<(xb+l2)){
	  return ((QL->BL)? GetHit(QL->BL, xb   , yb   , l2, x, y) : QL->val/(l*l));
	} else {
	  return ((QL->BR)? GetHit(QL->BR, xb+l2, yb   , l2, x, y) : QL->val/(l*l));
	}
  } else {
    if(x<(xb+l2)){
      return ((QL->TL)? GetHit(QL->TL, xb,    yb+l2, l2, x, y) : QL->val/(l*l));
	} else {
      return ((QL->TR)? GetHit(QL->TR, xb+l2, yb+l2, l2, x, y) : QL->val/(l*l));
	}
  }
}

/*float GetShade2(QLP QL, float xb, float yb, float l, float x, float y)
{
  float l2=0.5f*l;
  if(y<(yb+l2)){
    if(x<(xb+l2)){
      if(QL->BL){
        if((QL->BL->BL) || (QL->BL->BR) || (QL->BL->TL) || (QL->BL->TR)) return GetShade2(QL->BL, xb   , yb   , l2, x, y);
	  }
	} else {
      if(QL->BR){
        if((QL->BR->BL) || (QL->BR->BR) || (QL->BR->TL) || (QL->BR->TR)) return GetShade2(QL->BR, xb+l2, yb   , l2, x, y);
	  }
	}
  } else {
    if(x<(xb+l2)){
      if(QL->TL){
        if((QL->TL->BL) || (QL->TL->BR) || (QL->TL->TL) || (QL->TL->TR)) return GetShade2(QL->TL, xb,    yb+l2, l2, x, y);
	  }
	} else {
      if(QL->TR){
        if((QL->TR->BL) || (QL->TR->BR) || (QL->TR->TL) || (QL->TR->TR)) return GetShade2(QL->TR, xb+l2, yb+l2, l2, x, y);
	  }
	}
  }
  float vbl = ((QL->BL)? QL->BL->val/(l2*l2): 0.0f);
  float vbr = ((QL->BR)? QL->BR->val/(l2*l2): 0.0f);
  float vtl = ((QL->TL)? QL->TL->val/(l2*l2): 0.0f);
  float vtr = ((QL->TR)? QL->TR->val/(l2*l2): 0.0f);
  return bilinear(1.0f-(x-xb)/l, 1.0f-(y-yb)/l, vbl, vtl, vbr, vtr);
}*/


float CalcF(float x, float y, float w)
{
  float xl=1.0f-fabsf(x)/w; if(xl<0.0f) return 0.0f;
  float yl=1.0f-fabsf(y)/w; if(yl<0.0f) return 0.0f;
  return max(xl*yl, 0.0f);
}

float GetShade1(QLP QL, float *ks, float xb, float yb, float l, float x, float y)
{
  float c=0.0f, l2=0.5f*l;
  float dx=x-xb, dy=y-yb;
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
	float l1=0.5f*l2, l3=1.5f*l2;
	float ax1=fabsf(dx-l1), ax2=fabsf(dx-l3);
	float ay1=fabsf(dy-l1), ay2=fabsf(dy-l3);
    if((QL->BL) && (ax1<l2) && (ay1<l2)) c += GetShade1(QL->BL, ks, xb   , yb   , l2, x, y);
    if((QL->BR) && (ax2<l2) && (ay1<l2)) c += GetShade1(QL->BR, ks, xb+l2, yb   , l2, x, y);
    if((QL->TL) && (ax1<l2) && (ay2<l2)) c += GetShade1(QL->TL, ks, xb,    yb+l2, l2, x, y);
    if((QL->TR) && (ax2<l2) && (ay2<l2)) c += GetShade1(QL->TR, ks, xb+l2, yb+l2, l2, x, y);
    return c;
  }
  c=CalcF(dx-l2, dy-l2, l); *ks += c;
  return QL->val/(l*l) * c;
}

float GetShade2(QLP QL, float *ks, float xb, float yb, float l, float x, float y)
{
  float c=0.0f, l2=0.5f*l;
  float dx=x-xb, dy=y-yb;
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
	float l1=0.5f*l2, l3=1.5f*l2;
	float ax1=fabsf(dx-l1), ax2=fabsf(dx-l3);
	float ay1=fabsf(dy-l1), ay2=fabsf(dy-l3);
    if((QL->BL) && (ax1<l) && (ay1<l)) c += GetShade2(QL->BL, ks, xb   , yb   , l2, x, y);
    if((QL->BR) && (ax2<l) && (ay1<l)) c += GetShade2(QL->BR, ks, xb+l2, yb   , l2, x, y);
    if((QL->TL) && (ax1<l) && (ay2<l)) c += GetShade2(QL->TL, ks, xb,    yb+l2, l2, x, y);
    if((QL->TR) && (ax2<l) && (ay2<l)) c += GetShade2(QL->TR, ks, xb+l2, yb+l2, l2, x, y);
    return c;
  }
  c=CalcF(dx-l2, dy-l2, 2.0f*l); *ks += c;
  return QL->val/(l*l) * c;
}


float GetShade3(QLP QL, float *ks, float xb, float yb, float l, float x, float y)
{
  float c=0.0f, l2=0.5f*l;
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
    if(QL->BL) c += GetShade3(QL->BL, ks, xb   , yb   , l2, x, y);
    if(QL->BR) c += GetShade3(QL->BR, ks, xb+l2, yb   , l2, x, y);
    if(QL->TL) c += GetShade3(QL->TL, ks, xb,    yb+l2, l2, x, y);
    if(QL->TR) c += GetShade3(QL->TR, ks, xb+l2, yb+l2, l2, x, y);
    return c;
  }
  float kcx=xb+l2;
  float kcy=yb+l2;
  float knx=(kcx-0.25f*l)/(1.0f-l2);
  float kny=(kcy-0.25f*l)/(1.0f-l2);
  c=CalcF(x-knx, y-kny, l); *ks += c;
  return QL->val/(l*l) * c;
}

/*float GetShade4(QLP QL, float *ks, int *kc, float xb, float yb, float l, float x, float y)
{
  float c=0.0f, l2=0.5f*l;
  float dx=x-xb, dy=y-yb;
  if((QL->BL) || (QL->BR) || (QL->TL) || (QL->TR)){
	float l1=0.5f*l2, l3=1.5f*l2;
	float ax1=fabsf(dx-l1), ax2=fabsf(dx-l3);
	float ay1=fabsf(dy-l1), ay2=fabsf(dy-l3);
    if((ax1<l2) && (ay1<l2)){
      if(QL->BL) c += GetShade4(QL->BL, ks, kc, xb   , yb   , l2, x, y); else { *ks += CalcF((x-xb   )-0.5f*l2, (y-yb   )-0.5f*l2, l2); *kc += 1; }
	}
    if((ax2<l2) && (ay1<l2)){
      if(QL->BR) c += GetShade4(QL->BR, ks, kc, xb+l2, yb   , l2, x, y); else { *ks += CalcF((x-xb-l2)-0.5f*l2, (y-yb   )-0.5f*l2, l2); *kc += 1; }
	}
    if((ax1<l2) && (ay2<l2)){
      if(QL->TL) c += GetShade4(QL->TL, ks, kc, xb,    yb+l2, l2, x, y); else { *ks += CalcF((x-xb   )-0.5f*l2, (y-yb-l2)-0.5f*l2, l2); *kc += 1; }
	}
    if((ax2<l2) && (ay2<l2)){
      if(QL->TR) c += GetShade4(QL->TR, ks, kc, xb+l2, yb+l2, l2, x, y); else { *ks += CalcF((x-xb-l2)-0.5f*l2, (y-yb-l2)-0.5f*l2, l2); *kc += 1; }
	}
    return c;
  }
  c=CalcF(dx-l2, dy-l2, l); // apply interpolating Granz-Filter
  *ks += c; *kc += 1;
  return QL->val/(l*l) * c; // apply interpolating Granz-Filter
}*/

/*float GetShade3(QLP QL, float *ks, float xb, float yb, float l, float x, float y)
{
  float c=0.0f, l2=0.5f*l;
  if(QL->BL){
    if((QL->BL->BL) || (QL->BL->BR) || (QL->BL->TL) || (QL->BL->TR)) c += GetShade3(QL->BL, ks, xb   , yb   , l2, x, y); else c += QL->BL->val/(l*l) * CalcF(x-(xb  ), y-(yb  ),l);
  }
  if(QL->BR){
    if((QL->BR->BL) || (QL->BR->BR) || (QL->BR->TL) || (QL->BR->TR)) c += GetShade3(QL->BR, ks, xb+l2, yb   , l2, x, y); else c += QL->BR->val/(l*l) * CalcF(x-(xb+l), y-(yb  ),l);
  }
  if(QL->TL){
    if((QL->TL->BL) || (QL->TL->BR) || (QL->TL->TL) || (QL->TL->TR)) c += GetShade3(QL->TL, ks, xb,    yb+l2, l2, x, y); else c += QL->TL->val/(l*l) * CalcF(x-(xb  ), y-(yb+l),l);
  }
  if(QL->TR){
    if((QL->TR->BL) || (QL->TR->BR) || (QL->TR->TL) || (QL->TR->TR)) c += GetShade3(QL->TR, ks, xb+l2, yb+l2, l2, x, y); else c += QL->TR->val/(l*l) * CalcF(x-(xb+l), y-(yb+l),l);
  }
  *ks=1.0f; return c;
}*/

extern short Demo;

RGB_Color ShadeSimple(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  GetIPN(Org, Ray, &IntP);
  vector L = unit(RV_Lights->p - IntP.IP);
  float  s = (IntP.p->flags & RV_ReceiveNoShadow)? 1.0f : 1.0f-ShadowTest(IntP.p, RV_Lights->Prim, &(IntP.IP), &L, distance(RV_Lights->p, IntP.IP));
  float NL; if(s){ NL = s*(L % IntP.N); if(NL<0.0f) NL=0.0f; } else NL=0.0f;
  RGB_Color C; 
  if(RV_Caustics_Display && (IntP.p->flags & RV_Chequered)){
	float sl=0.2f/(float)RV_Caustics_Samples; if(Demo==3) sl*=3.0f;
	if(RV_Caustics_Interpolation==1){
	  float ks=0.0f; float c=GetShade1(QLT, &ks, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v); if(ks>0.5f) c =c/ks;
	  NL += sl * c;
	} else if(RV_Caustics_Interpolation==2){
	  float ks=0.0f; float c=GetShade2(QLT, &ks, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v); if(ks>1.0f) c =c/ks;
	  NL += sl * c;
	} else if(RV_Caustics_Interpolation==3){
	  float ks=0.0f; float c=GetShade3(QLT, &ks, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v); if(ks>1.0f) c =c/ks;
	  //float ks=0.0f; int kc=0;
	  //float c=GetShade3(QLT, &ks, &kc, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v);
	  //if(ks>1.0f) c =c/ks;
	  //if(ks>0.5f) c =c/ks; else if(ks>0.001f) c=c*2.0f;
	  //if(fabsf(1.0f-ks)>0.5f) c*=1000.0f;
	  //if(ks<0.5f) c*=10000.0f;
	  NL += sl * c;
	} else {
	  NL += sl * GetHit(QLT, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v);
	}
  }
  C.R = (Ambient.R + NL) * IntP.p->Mat->Reflection.R;
  C.G = (Ambient.G + NL) * IntP.p->Mat->Reflection.G;
  C.B = (Ambient.B + NL) * IntP.p->Mat->Reflection.B;
  return C;
}

extern float wrap(float c);

RGB_Color ShadeUV(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  GetIPN(Org, Ray, &IntP); GetTXY(&IntP);
  return RGB_Color(wrap(IntP.x), wrap(IntP.y), 0.5f);
}

RGB_Color Shade(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level)
{
  const int max_raydepth=3;

  GetIPN(Org, Ray, &IntP); MatPtr m = IntP.p->Mat;
  float NL=0.0f,RV=0.0f; RGB_Color C,Cs;
  LitPtr l=RV_Lights;
  while(l){
    vector L = unit(l->p - IntP.IP);
    float  s = (IntP.p->flags & RV_ReceiveNoShadow)? 1.0f : 1.0f-ShadowTest(IntP.p, l->Prim, &(IntP.IP), &L, distance(l->p, IntP.IP));
    if(s){
      float NL1 = L % IntP.N;
      if(level==0){
        vector R  = -unit(2.0f*NL1*(IntP.N)-L);
        float RV1 = R % unit(IntP.IP - *Org); if(RV1<0.0f) RV1=0.0f; RV1 = (float)pow(RV1, (m->PBExp? m->PBExp : 30.0f));
        RV += s * l->v * RV1;
      }
      if(NL1>0.0f) NL += s * l->v * NL1;      
    }
    l=l->next;
  }
  if(RV_Caustics_Display && (IntP.p->flags & RV_Chequered)){
	float sl=0.2f/(float)RV_Caustics_Samples; if(Demo==3) sl*=3.0f;
	if(RV_Caustics_Interpolation==1){
	  float ks=0.0f; float c=GetShade1(QLT, &ks, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v); if(ks>0.5f) c =c/ks;
	  Cs = sl * c * RGB_Color(0.2f, 0.9f, 0.8f);
	} else if(RV_Caustics_Interpolation==2){
	  float ks=0.0f; float c=GetShade2(QLT, &ks, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v); if(ks>1.0f) c =c/ks;
	  Cs = sl * c * RGB_Color(0.2f, 0.9f, 0.8f);
	} else {
	  Cs = sl * GetHit(QLT, 0.0f, 0.0f, 1.0f, IntP.u, IntP.v) * RGB_Color(0.2f, 0.9f, 0.8f);
	}
  } else Cs = RGB_Color(0.0f, 0.0f, 0.0f);
  if(m->Texture){
    GetTXY(&IntP);
    if(m->Texture->tbits){
      unsigned char *bitptr = m->Texture->tbits + ((int)(wrap(IntP.y)*m->Texture->tinfo.bmiHeader.biHeight)) * ((m->Texture->tinfo.bmiHeader.biWidth*((m->Texture->tinfo.bmiHeader.biBitCount)>>3)+3) & 0xFFFFFFFC);
      unsigned long  offset = 3*                  ((int)(wrap(IntP.x)*m->Texture->tinfo.bmiHeader.biWidth));
      C.R = (bitptr[offset+2]/255.0f); C.G = (bitptr[offset+1]/255.0f); C.B = (bitptr[offset]/255.0f);
    } else {
      C = RGB_Color(wrap(IntP.x), wrap(IntP.y), 0.5f);
    }
  } else {
    if((IntP.p->flags & RV_Chequered) && (((int)ceil(2.0f*IntP.IP.x)^(int)ceil(2.0f*IntP.IP.y)) & 1)){
      C = RGB_Color(0.8f, 0.8f, 0.8f);
    } else {
      if(m->Reflection.G<0.0f){
        C = RGB_Color(m->Reflection.R, m->Reflection.R, m->Reflection.R);
      } else {
        C = m->Reflection;
      }
    }
  }
  C.R = (Ambient.R + NL + Cs.R) * (1.0f - m->Specularity) * C.R + m->Specularity*RV;
  C.G = (Ambient.G + NL + Cs.G) * (1.0f - m->Specularity) * C.G + m->Specularity*RV;
  C.B = (Ambient.B + NL + Cs.B) * (1.0f - m->Specularity) * C.B + m->Specularity*RV;

  // specular reflection/refraction
  if((level<max_raydepth) && (m->Specularity>ray_thresh)){
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

    // collect specular reflected light
    if(spc > ray_thresh) {
      // mirror direction
      OutDir = -unit(2.0f * ca * IntP.N - *Ray); //if(m->PBExp > 0.001f) OutDir = rndpdir(OutDir, 1.0f/m->PBExp);
      C += spc * RayTrace(NULL, IntP.p, NULL, &RV_Environment, &(IntP.IP), &OutDir, 1.0f, 0.0f, &CallShade, level+1, max_raydepth);
    }
    // collect transmitted (refracted) light
    if(trn > ray_thresh) {
      if(ca<0.0f){
        OutDir=refract(*Ray,  IntP.N, mat->IOR(wavlen),             m->IOR(wavlen));
        C += trn * RayTrace(NULL, IntP.p, NULL,                           m                    , &(IntP.IP), &OutDir, 1.0f, 0.0f, &CallShade, level+1, max_raydepth);
      } else {
        OutDir=refract(*Ray, -IntP.N, mat->IOR(wavlen), RV_Environment.IOR(wavlen));
        C += trn * RayTrace(NULL, IntP.p, NULL, ((OutDir % IntP.N)<0.0f)? m : &(RV_Environment), &(IntP.IP), &OutDir, 1.0f, 0.0f, &CallShade, level+1, max_raydepth);
      }
    }
  }
  return C;
}

RGB_Color (*RV_Shader)(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level) = &Shade;

RGB_Color CallShade(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar Cmin, float em, float dist, int level, Thrdata *Thread)
{
  return RV_Shader(Org, Ray, Cmin, mat, level);
}

extern "C" {
  void RV_DispatchIntersectAll0C(Thrdata *Thread, RV_PrP p)
  {
    if(p->flags & RV_OnlyIndirect){ Thread->Cint.t = -1.0f; return; }
    if(p->lastray == Thread->CRay){ Thread->Cint.t = -1.0f; return; } else p->lastray = Thread->CRay;
    if((p->type==1) || (p->type==0)){
      RV_IntersectTriPara0(Thread, (RV_TriPara *)p);
	} else if(p->type== 2){
      RV_IntersectSimpleSphere0(Thread, (RV_SimpleSphere *)p);
	} else if(p->type== 3){
      RV_IntersectCylinder0(Thread, (RV_Cylinder *)p);
	} else if(p->type== 5){
      RV_IntersectSimpleRing0(Thread, (RV_SimpleRing *)p);
	} else if(p->type== 6){
      RV_IntersectTorus0(Thread, (RV_Torus *)p);
	} else if(p->type== 7){
      RV_IntersectComplexSphere0(Thread, (RV_ComplexSphere *)p);
	} else if(p->type== 8){
      RV_IntersectParaboloid0(Thread, (RV_Paraboloid *)p);
	} else if(p->type== 9){
      RV_IntersectEllipsoid0(Thread, (RV_Ellipsoid *)p);
	} else if(p->type==10){
      RV_IntersectComplexRing0(Thread, (RV_ComplexRing *)p);
	} else {
      Thread->Cint.t = -1.0f;
	}
  }
}

void RV_DispatchIntersectAll1C(RV_PrP p, vector *Org, vector *Ray, IntPar *Ints)
{
  //if(p->lastray == cray){ Ints->t = -1.0f; return; } else p->lastray = cray;
  if((p->type==1) || (p->type==0)){
    RV_IntersectTriPara1((RV_TriPara *)p, Org, Ray, Ints);
  } else if(p->type== 2){
    RV_IntersectSimpleSphere1((RV_SimpleSphere *)p, Org, Ray, Ints);
  } else if(p->type== 3){
    RV_IntersectCylinder1((RV_Cylinder *)p, Org, Ray, Ints);
  } else if(p->type== 5){
    RV_IntersectSimpleRing1((RV_SimpleRing *)p, Org, Ray, Ints);
  } else if(p->type== 6){
    RV_IntersectTorus1((RV_Torus *)p, Org, Ray, Ints);
  } else if(p->type== 7){
    RV_IntersectComplexSphere1((RV_ComplexSphere *)p, Org, Ray, Ints);
  } else if(p->type== 8){
    RV_IntersectParaboloid1((RV_Paraboloid *)p, Org, Ray, Ints);
  } else if(p->type== 9){
    RV_IntersectEllipsoid1((RV_Ellipsoid *)p, Org, Ray, Ints);
  } else if(p->type==10){
    RV_IntersectComplexRing1((RV_ComplexRing *)p, Org, Ray, Ints);
  } else {
    Ints->t = -1.0f;
  }
}

void TraceTree0(Thrdata *Thread, BSPplane *tree, vector HP, vector BBMin, vector BBMax)
{
  switch(tree->axis){
    case 0:
    if(UseASM){
      RV_TraceList0(Thread, tree);
	} else {
      PrLP p = (PrLP)tree->left;
      while(p){
        RV_DispatchIntersectAll0C(Thread, p->Prim);
        if(Thread->Cint.t>=0.0f){
          if((Thread->Cmin.t<0.0f) || (Thread->Cint.t<Thread->Cmin.t)){ Thread->Cint.p = p->Prim; Thread->Cmin = Thread->Cint; }
        }
        p=p->next;
      }
	}
    break;
    case 1:
      if(HP.x < tree->dval){
        TraceTree0(Thread, tree->left, HP, BBMin, vector(tree->dval, BBMax.y, BBMax.z));
        if((Thread->Cmin.t>=0.0f) && (HP.x > Thread->Cmin.IP.x)) return;
        if(Thread->CamRay.x > 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->x)/Thread->CamRay.x) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree0(Thread, tree->rigt, PH, vector(tree->dval, BBMin.y, BBMin.z), BBMax);
          }
        }
      } else {
        TraceTree0(Thread, tree->rigt, HP, vector(tree->dval, BBMin.y, BBMin.z), BBMax);
        if((Thread->Cmin.t>=0.0f) && (HP.x < Thread->Cmin.IP.x)) return;
        if(Thread->CamRay.x < 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->x)/Thread->CamRay.x) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree0(Thread, tree->left, PH, BBMin, vector(tree->dval, BBMax.y, BBMax.z));
          }
        }
      }
    break;
    case 2:
      if(HP.y < tree->dval){
        TraceTree0(Thread, tree->left, HP, BBMin, vector(BBMax.x, tree->dval, BBMax.z));
        if((Thread->Cmin.t>=0.0f) && (HP.y > Thread->Cmin.IP.y)) return;
        if(Thread->CamRay.y > 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->y)/Thread->CamRay.y) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree0(Thread, tree->rigt, PH, vector(BBMin.x, tree->dval, BBMin.z), BBMax);
		  }
		}
	  } else {
	    TraceTree0(Thread, tree->rigt, HP, vector(BBMin.x, tree->dval, BBMin.z), BBMax);
	    if((Thread->Cmin.t>=0.0f) && (HP.y < Thread->Cmin.IP.y)) return;
        if(Thread->CamRay.y < 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->y)/Thread->CamRay.y) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree0(Thread, tree->left, PH, BBMin, vector(BBMax.x, tree->dval, BBMax.z));
		  }
		}
	  }
	break;
	case 3:
	  if(HP.z < tree->dval){
        TraceTree0(Thread, tree->left, HP, BBMin, vector(BBMax.x, BBMax.y, tree->dval));
	    if((Thread->Cmin.t>=0.0f) && (HP.z > Thread->Cmin.IP.z)) return;
        if(Thread->CamRay.z > 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->z)/Thread->CamRay.z) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree0(Thread, tree->rigt, PH, vector(BBMin.x, BBMin.y, tree->dval), BBMax);
		  }
		}
	  } else {
	    TraceTree0(Thread, tree->rigt, HP, vector(BBMin.x, BBMin.y, tree->dval), BBMax);
	    if((Thread->Cmin.t>=0.0f) && (HP.z < Thread->Cmin.IP.z)) return;
        if(Thread->CamRay.z < 0.0f){
          vector PH = *RV_CamPtr + ((tree->dval - RV_CamPtr->z)/Thread->CamRay.z) * Thread->CamRay;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
	        TraceTree0(Thread, tree->left, PH, BBMin, vector(BBMax.x, BBMax.y, tree->dval));
		  }
		}
	  }
	break;
  }
}

void TraceTree1(BSPplane *tree, vector HP, vector *Org, vector *Ray, IntPar *Cmin, RV_PrP CLtOb, RV_PrP OrgOb, float Shf, vector BBMin, vector BBMax)
{
  switch(tree->axis){
    case 0:
		{
        IntPar Cint; PrLP p = (PrLP)tree->left;
        while(p){
	      if(!Shf || !(p->Prim->flags & RV_CastNoShadow) ){
            if(UseASM) RV_DispatchIntersectAll1(p->Prim, Org, Ray, &Cint); else RV_DispatchIntersectAll1C(p->Prim, Org, Ray, &Cint);
            if((p->Prim == OrgOb) && (Cint.t<0.0001f)) Cint.t=-1.0f;
		    if(Shf && (Cint.t   > Shf  )) Cint.t=-1.0f;
		    if(Shf && (p->Prim == CLtOb)) Cint.t=-1.0f;
            if(Cint.t>=0.0f){
              if((Cmin->t<0.0f) || (Cint.t<Cmin->t)){ Cint.p=p->Prim; *Cmin=Cint; }
              if(Shf) return;
			}
		  }
         p=p->next;
		}
		}
	break;
	case 1:
	  if(HP.x < tree->dval){
	    TraceTree1(tree->left, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(tree->dval, BBMax.y, BBMax.z));
	    if((Cmin->t>=0.0f) && ((HP.x > Cmin->IP.x) || Shf)) return;
        if(Ray->x > 0.0f){
          vector PH = HP + ((tree->dval - HP.x)/Ray->x) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
	        TraceTree1(tree->rigt, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(tree->dval, BBMin.y, BBMin.z), BBMax);
		  }
		}
	  } else {
	    TraceTree1(tree->rigt, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(tree->dval, BBMin.y, BBMin.z), BBMax);
	    if((Cmin->t>=0.0f) && ((HP.x < Cmin->IP.x) || Shf)) return;
        if(Ray->x < 0.0f){
          vector PH = HP + ((tree->dval - HP.x)/Ray->x) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
	        TraceTree1(tree->left, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(tree->dval, BBMax.y, BBMax.z));
		  }
		}
	  }
	break;
	case 2:
	  if(HP.y < tree->dval){
	    TraceTree1(tree->left, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(BBMax.x, tree->dval, BBMax.z));
	    if((Cmin->t>=0.0f) && ((HP.y > Cmin->IP.y) || Shf)) return;
        if(Ray->y > 0.0f){
          vector PH = HP + ((tree->dval - HP.y)/Ray->y) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree1(tree->rigt, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(BBMin.x, tree->dval, BBMin.z), BBMax);
		  }
		}
	  } else {
	    TraceTree1(tree->rigt, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(BBMin.x, tree->dval, BBMin.z), BBMax);
	    if((Cmin->t>=0.0f) && ((HP.y < Cmin->IP.y) || Shf)) return;
        if(Ray->y < 0.0f){
          vector PH = HP + ((tree->dval - HP.y)/Ray->y) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
	        TraceTree1(tree->left, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(BBMax.x, tree->dval, BBMax.z));
		  }
		}
	  }
	break;
	case 3:
	  if(HP.z < tree->dval){
	    TraceTree1(tree->left, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(BBMax.x, BBMax.y, tree->dval));
	    if((Cmin->t>=0.0f) && ((HP.z > Cmin->IP.z) || Shf)) return;
        if(Ray->z > 0.0f){
          vector PH = HP + ((tree->dval - HP.z)/Ray->z) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
            TraceTree1(tree->rigt, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(BBMin.x, BBMin.y, tree->dval), BBMax);
		  }
		}
	  } else {
	    TraceTree1(tree->rigt, HP, Org, Ray, Cmin, CLtOb, OrgOb, Shf, vector(BBMin.x, BBMin.y, tree->dval), BBMax);
	    if((Cmin->t>=0.0f) && ((HP.z < Cmin->IP.z) || Shf)) return;
        if(Ray->z < 0.0f){
          vector PH = HP + ((tree->dval - HP.z)/Ray->z) * *Ray;
          if((PH.x>BBMin.x)&&(PH.x<BBMax.x)&&(PH.y>BBMin.y)&&(PH.y<BBMax.y)&&(PH.z>BBMin.z)&&(PH.z<BBMax.z)){
	        TraceTree1(tree->left, PH, Org, Ray, Cmin, CLtOb, OrgOb, Shf, BBMin, vector(BBMax.x, BBMax.y, tree->dval));
		  }
		}
	  }
	break;
  }
}

float DivQuad(float xb, float yb, float ll, RV_PrP OrgObj, RV_TriPara *LtObj, vector *Org, float v1, float v2, float v3, float v4, int level)
{
  IntPar Cint; vector TR; float btr;
  float v0,v12,v23,v34,v14;
  if(fabs(v2-v1)>0.1f){
    TR = LtObj->base + (xb + 0.5f*ll)*LtObj->edg1 + (yb          )*LtObj->edg2 - *Org; btr=Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); v12 = (Cint.t>0.0f)? 1.0f : 0.0f;
  } else v12=0.5f*(v1+v2);
  if(fabs(v3-v4)>0.1f){
    TR = LtObj->base + (xb + 0.5f*ll)*LtObj->edg1 + (yb +      ll)*LtObj->edg2 - *Org; btr=Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); v34 = (Cint.t>0.0f)? 1.0f : 0.0f;
  } else v34=0.5f*(v3+v4);
  if(fabs(v3-v2)>0.1f){
    TR = LtObj->base + (xb +      ll)*LtObj->edg1 + (yb + 0.5f*ll)*LtObj->edg2 - *Org; btr=Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); v23 = (Cint.t>0.0f)? 1.0f : 0.0f;
  } else v23=0.5f*(v3+v2);
  if(fabs(v4-v1)>0.1f){
    TR = LtObj->base + (xb          )*LtObj->edg1 + (yb + 0.5f*ll)*LtObj->edg2 - *Org; btr=Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); v14 = (Cint.t>0.0f)? 1.0f : 0.0f;
  } else v14=0.5f*(v1+v4);
  if((fabs(v12-v34)>0.1f) || (fabs(v23-v14)>0.1f)){
    TR = LtObj->base + (xb + 0.5f*ll)*LtObj->edg1 + (yb + 0.5f*ll)*LtObj->edg2 - *Org; btr=Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); v0  = (Cint.t>0.0f)? 1.0f : 0.0f;
  } else v0=0.25f*(v12+v34+v23+v14);
  if(level<2){
	float q1=DivQuad(xb        ,yb        ,0.5f*ll,OrgObj,LtObj,Org,v1,v12,v0,v14,level+1);
	float q2=DivQuad(xb+0.5f*ll,yb        ,0.5f*ll,OrgObj,LtObj,Org,v12,v2,v23,v0,level+1);
	float q3=DivQuad(xb+0.5f*ll,yb+0.5f*ll,0.5f*ll,OrgObj,LtObj,Org,v0,v23,v3,v34,level+1);
	float q4=DivQuad(xb        ,yb+0.5f*ll,0.5f*ll,OrgObj,LtObj,Org,v14,v0,v34,v4,level+1);
    return (q1+q2+q3+q4)/4.0f;
  }
  return (v14+v1+v12+v2+v23+v3+v34+v4+v0)/9.0f;
}

float ShadowTest(RV_PrP OrgObj, RV_PrP LtObj, vector *Org, vector *Ray, float dist)
{
  IntPar Cint; 
  if(LtObj && (LtObj->type==1)){
    vector TR; float btr;
    TR = ((RV_TriPara *)(LtObj))->base                                                                 - *Org; btr = Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); float v1 = (Cint.t>0.0f)? 1.0f : 0.0f;
    TR = ((RV_TriPara *)(LtObj))->base + ((RV_TriPara *)(LtObj))->edg1                                 - *Org; btr = Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); float v2 = (Cint.t>0.0f)? 1.0f : 0.0f;
    TR = ((RV_TriPara *)(LtObj))->base + ((RV_TriPara *)(LtObj))->edg1 + ((RV_TriPara *)(LtObj))->edg2 - *Org; btr = Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); float v3 = (Cint.t>0.0f)? 1.0f : 0.0f;
    TR = ((RV_TriPara *)(LtObj))->base                                 + ((RV_TriPara *)(LtObj))->edg2 - *Org; btr = Betrag(TR); TR=TR/btr;
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, &TR, &Cint, LtObj, OrgObj, 0.999f*btr, RV_BBMin, RV_BBMax); float v4 = (Cint.t>0.0f)? 1.0f : 0.0f;
    return DivQuad(0.0f, 0.0f, 1.0f, OrgObj, (RV_TriPara *)LtObj, Org, v1, v2, v3, v4, 0);
    //return (v1+v2+v3+v4)/4.0f;
  } else {
    Cint.t=-1.0f; TraceTree1(RV_SceneTree, *Org, Org, Ray, &Cint, LtObj, OrgObj, 0.999f*dist, RV_BBMin, RV_BBMax);
    if(Cint.t>0.0f) return 1.0f; else return 0.0f;
  }
}

void RayTrace4(void)
{
  //RV_MDP[52]=-1.0f; RV_MDP[53]=-1.0f; RV_MDP[54]=-1.0f; RV_MDP[55]=-1.0f;

  //RV_SIMD_TrLst0((PrLP)RV_SceneTree->left);
}

unsigned long int IRCnt;

RGB_Color RayTrace0(Thrdata *Thread)
{
  RGB_Color col=RGB_Color(0.0f, 0.0f, 0.0f); // default return value

  Thread->Cmin.t=-1.0f; Thread->CRay++; IRCnt++;

  if(RV_LtObj){
    if(UseASM) RV_DispatchIntersectAll0(Thread, RV_LtObj); else RV_DispatchIntersectAll0C(Thread, RV_LtObj);
	Thread->Cint.p=RV_LtObj; Thread->Cmin = Thread->Cint;
  }
  
  if(UseASM) RV_IntersectBoundingBox0(Thread); else RV_IntersectBoundingBox0c(Thread);
  if(Thread->Cint.t>0.0f) TraceTree0(Thread, RV_SceneTree, Thread->Cint.IP, RV_BBMin, RV_BBMax);

  if(Thread->Cmin.t>0.0f){
    col=RV_Shader(RV_CamPtr, &(Thread->CamRay), Thread->Cmin, &RV_Environment, 0);
  }

  return col;
  //return RayTrace(Thread, NULL, NULL, &RV_Environment, RV_CamPtr, &(Thread->CamRay), 1.0f, 0.0f, &CallShade, 0, 1);
  //return RGB_Color(fabs(RayVec.x),fabs(RayVec.y),fabs(RayVec.z));
}

RGB_Color RayTrace(Thrdata  *Thread,           // pointer to data structure of working thread
				   RV_PrP    OrgObj,           // object where ray comes from
                   void      Init(vector Org), // initialization on ray origin
                   MatPtr    mat,              // material where ray is in
                   vector   *Org,              // start-position of ray
                   vector   *Ray,              // direction of ray
                   float     em,               // energy carried by ray
                   float     dist,             // accumulated distance from ray emitter
                   RGB_Color shade(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar Cmin, float em, float dist, int level, Thrdata *Thread), // shader function
                   int       level,            // current recursion level
                   int       max_raydepth)     // maximum recursion level
{
  IntPar Cint, Cmin; Cmin.t=-1.0f;

  if(Init!=NULL) Init(*Org);      // ray initialization
  
  RGB_Color col = RGB_Color(0.0f, 0.0f, 0.0f); // default return value (background color)

  if(RV_LtObj){
	if(UseASM) RV_DispatchIntersectAll1(RV_LtObj, Org, Ray, &Cint); else RV_DispatchIntersectAll1C(RV_LtObj, Org, Ray, &Cint);
	Cint.p=RV_LtObj; Cmin=Cint;
  }

  RV_IntersectBoundingBox1(Org, Ray, &Cint); if(Cint.t>0.0f) TraceTree1(RV_SceneTree, Cint.IP, Org, Ray, &Cmin, NULL, OrgObj, 0.0f, RV_BBMin, RV_BBMax);

  if(Cmin.t>0.0f){
    // diffuse shading at intersection point
	col  = shade(OrgObj, mat, Org, Ray, Cmin, em, dist, level, Thread);
	// absorbtion
	//col *= mat->Thru(wavlen, dt); 
  }
  return col;
}
