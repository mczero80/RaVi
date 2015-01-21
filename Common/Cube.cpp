// Cube Primitive for VModel & WinOSi
// ----------------------------------
// 09.06.1999           Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

/*float IntFace(vector Org, vector Ray, int algn)
{
  vector B,N;
  switch(algn){
    case 0: B=vector( 0.5f,  0.5f,  0.5f); N=vector( 1.0f,  0.0f,  0.0f); break;
    case 1: B=vector( 0.5f,  0.5f,  0.5f); N=vector( 0.0f,  1.0f,  0.0f); break;
    case 2: B=vector( 0.5f,  0.5f,  0.5f); N=vector( 0.0f,  0.0f,  1.0f); break;
    case 3: B=vector(-0.5f, -0.5f, -0.5f); N=vector(-1.0f,  0.0f,  0.0f); break;
    case 4: B=vector(-0.5f, -0.5f, -0.5f); N=vector( 0.0f, -1.0f,  0.0f); break;
    case 5: B=vector(-0.5f, -0.5f, -0.5f); N=vector( 0.0f,  0.0f, -1.0f); break;
  }
  if((Ray%N)==0.0f) return -1.0f;
  double t = ((B-Org)%N)/(Ray%N); if(t<=0.0) return -1.0f;
  vector iP = Org+(float)t*Ray;
  switch(algn){
    case 0:
    case 3: return ((iP.y>=-0.5f) && (iP.y<=0.5f) && (iP.z>=-0.5f) && (iP.z<=0.5f))? (float)t : -1.0f; break;
    case 1: 
    case 4: return ((iP.x>=-0.5f) && (iP.x<=0.5f) && (iP.z>=-0.5f) && (iP.z<=0.5f))? (float)t : -1.0f; break;
    case 2:
    case 5: return ((iP.x>=-0.5f) && (iP.x<=0.5f) && (iP.y>=-0.5f) && (iP.y<=0.5f))? (float)t : -1.0f; break;
  }
  return -1.0f;
}*/

// Parsing ---------------------------------------------------------

int Cube::ParseToken(char *a, CFile *file) { return 1; }

// Intersection ----------------------------------------------------

/*float Cube::Intersect(vector Org, vector Ray)
{
  Ray=Ray*Tb-vector(Tb.M41, Tb.M42, Tb.M43);
  Org=Org*Tb;
  float t, t1, t2=-1.0f;
  t1 = IntFace(Org, Ray, 0);
  t  = IntFace(Org, Ray, 1); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFace(Org, Ray, 2); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFace(Org, Ray, 3); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFace(Org, Ray, 4); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFace(Org, Ray, 5); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  CInt.t=min(t1,t2); vector D=Org+CInt.t*Ray; CInt.IP=D*TM;
  if((fabs(D.y)>=fabs(D.x)) && (fabs(D.y)>=fabs(D.z))){
	CInt.N = (D.y>0.0f)? vector(0.0f, 1.0f, 0.0f) : vector(0.0f, -1.0f, 0.0f);
  } else {
    if((fabs(D.x)>=fabs(D.y)) && (fabs(D.x)>=fabs(D.z))){
	  CInt.N = (D.x>0.0f)? vector(1.0f, 0.0f, 0.0f) : vector(-1.0f, 0.0f,  0.0f);
    } else {
	  CInt.N = (D.z>0.0f)? vector(0.0f, 0.0f, 1.0f) : vector( 0.0f, 0.0f, -1.0f);
    }
  }
  CInt.N = unit(CInt.N*NM);
  return CInt.t;
}*/

// Extents ---------------------------------------------------------

void Cube::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-0.5f, -0.5f, -0.5f);
  vector mx=vector( 0.5f,  0.5f,  0.5f);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Cube::RndPnt (void){
  int side = (int)(rnd()*6.0f);
  switch(side) {
    case 0:
      return vector((float)rnd()-0.5f, (float)rnd()-0.5f,              0.5f)*TM;
    case 1:
      return vector((float)rnd()-0.5f, (float)rnd()-0.5f,             -0.5f)*TM;
    case 2:
      return vector(             0.5f, (float)rnd()-0.5f, (float)rnd()-0.5f)*TM;
    case 3:
      return vector(            -0.5f, (float)rnd()-0.5f, (float)rnd()-0.5f)*TM;
    case 4:
      return vector((float)rnd()-0.5f,              0.5f, (float)rnd()-0.5f)*TM;
    case 5:
      return vector((float)rnd()-0.5f,             -0.5f, (float)rnd()-0.5f)*TM;
  }
  return vector(0.0f, 0.0f, 0.0f);
}

vector Cube::PPoint (float u, float v){ return vector(0.0f, 0.0f, 0.0f); }

// Normals ---------------------------------------------------------

vector Cube::PNormal(float u, float v){ return vector(0.0f, 0.0f, 1.0f); }

vector Cube::VNormal(vector p) {
  vector D=p*Tb-vector(0.5f, 0.5f, 0.5f);
  if((fabs(D.y)>=fabs(D.x)) && (fabs(D.y)>=fabs(D.z))){
	return unit((D.y>0.0f)? vector(0.0f, 1.0f, 0.0f)*NM : vector(0.0f, -1.0f, 0.0f)*NM);
  } else {
    if((fabs(D.x)>=fabs(D.y)) && (fabs(D.x)>=fabs(D.z))){
	  return unit((D.x>0.0f)? vector(1.0f, 0.0f, 0.0f)*NM : vector(-1.0f, 0.0f,  0.0f)*NM);
    } else {
	  return unit((D.z>0.0f)? vector(0.0f, 0.0f, 1.0f)*NM : vector( 0.0f, 0.0f, -1.0f)*NM);
    }
  }
}

