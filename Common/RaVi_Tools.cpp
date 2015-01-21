
#include "stdafx.h"

#include "general.h"

#include "Materials.h"
#include "RaVi.h"

void InitTP(RV_TriPara *p){
  float de[6];
  p->N = unit(p->edg1 * p->edg2); // calculate surface normal-vector
  float dxy = p->edg2.y*p->edg1.x - p->edg2.x*p->edg1.y;
  float dyz = p->edg2.z*p->edg1.y - p->edg2.y*p->edg1.z;
  float dxz = p->edg2.z*p->edg1.x - p->edg2.x*p->edg1.z;
  de[0] = fabsf(p->edg1.x *  dxy);
  de[1] = fabsf(p->edg1.y *  dyz);
  de[2] = fabsf(p->edg1.x *  dxz);
  de[3] = fabsf(p->edg1.y * -dxy);
  de[4] = fabsf(p->edg1.z * -dyz);
  de[5] = fabsf(p->edg1.z * -dxz);
  p->algn=0; for(int i=1; i<6; i++) if(de[i]>de[p->algn]) p->algn=i;
  switch(p->algn){
    case 0: p->deno = 1.0f/ dxy; break;
    case 1: p->deno = 1.0f/ dyz; break;
    case 2: p->deno = 1.0f/ dxz; break;
    case 3: p->deno = 1.0f/-dxy; break;
    case 4: p->deno = 1.0f/-dyz; break;
    case 5: p->deno = 1.0f/-dxz; break;
  }
}

void InitCube(RV_Cube *c, matrix & TM){
  c->top   = vector( 0.5f,  0.5f,  0.5f)*TM;
  c->base  = vector(-0.5f, -0.5f, -0.5f)*TM;
  matrix NM= TM.NTrans();
  c->axis1 = vector( 1.0f,  0.0f,  0.0f)*NM;
  c->axis2 = vector( 0.0f,  1.0f,  0.0f)*NM;
  c->axis3 = vector( 0.0f,  0.0f,  1.0f)*NM;
}

vector RV_RndPt(RV_PrP p)
{
  if(p->type == 0){
    float r1=(float)rnd();
    float r2=(float)rnd();
    return ((RV_TriPara *)p)->base + (((r1+r2)>1.0f)? (1.0f-r1)*((RV_TriPara *)p)->edg1 + (1.0f-r2)*((RV_TriPara *)p)->edg2 : r1*((RV_TriPara *)p)->edg1 + r2*((RV_TriPara *)p)->edg2);
  } else if(p->type == 1){
    return ((RV_TriPara *)p)->base + (float)rnd()*((RV_TriPara *)p)->edg1 + (float)rnd()*((RV_TriPara *)p)->edg2;
  } else if(p->type == 2){
    return ((RV_SimpleSphere *)p)->position + sqrtf(((RV_SimpleSphere *)p)->r2) * rnddir();
  } else if(p->type == 3){
    float a = ( ((RV_Cylinder *)p)->angle? ((RV_Cylinder *)p)->angle : TwoPi) * (float)rnd();
	float h = (float)rnd();
	float r = 1.0f - h*((RV_Cylinder *)p)->r_top;
    return vector(r*cosf(a), r*sinf(a), h) * ((RV_Cylinder *)p)->Tb.Inverse();
  } else if(p->type == 5){
	float a = TwoPi*(float)rnd();
	float r = ((RV_SimpleRing *)p)->r_i + (((RV_SimpleRing *)p)->r_o - ((RV_SimpleRing *)p)->r_i)*(float)rnd();
	vector u = XfromZ(((RV_SimpleRing *)p)->N);
    return ((RV_SimpleRing *)p)->center + r*cosf(a)*u + r*sinf(a)*(((RV_SimpleRing *)p)->N*u);
  } else if(p->type == 7){
	if(p->flags & RV_SurfCosZ){
	  return crrdir(((RV_ComplexSphere *)p)->vangle, ((RV_ComplexSphere *)p)->ustart, ((RV_ComplexSphere *)p)->ustop) * ((RV_ComplexSphere *)p)->sinfo->NM;
	} else {
	  return  rrdir(((RV_ComplexSphere *)p)->vangle, ((RV_ComplexSphere *)p)->ustart, ((RV_ComplexSphere *)p)->ustop) * ((RV_ComplexSphere *)p)->sinfo->NM;
	}
  } else return vector(0.0f, 0.0f, 0.0f);
}

vector RV_PtNrm(RV_PrP p, vector pt)
{
  vector N = vector(0.0f, 0.0f, 1.0f);
  if((p->type == 0) || (p->type == 1)){
    N = ((RV_TriPara *)p)->N;
  } else if(p->type == 5){
    N = ((RV_SimpleRing *)p)->N;
  } else if(p->type == 3){
	vector P = pt * ((RV_Cylinder *)p)->Tb;
    N = unit(vector(P.x, P.y, (1.0f - P.z)*((RV_Cylinder *)p)->r_top)*((RV_Cylinder *)p)->sinfo->NM);
  } else if(p->type == 2){
    N = unit(pt-((RV_SimpleSphere  *)p)->position);
  } else if(p->type == 7){
	N = unit(pt-((RV_ComplexSphere *)p)->position);
  }
  return (p->flags & RV_Inverse)? -N : N;
}
