// Torus Primitive for VModel & WinOSi
// -----------------------------------
// 09.06.1999            Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

// Parsing ---------------------------------------------------------

extern void  GetToken (char *a, CFile *file);
extern float ReadAngle(char *a);

int Torus::ParseToken(char *a, CFile *file)
{
  if(!stricmp(a,"Angle")){
    GetToken(a, file); vangle = ReadAngle(a);
	return 0;
  }
  if(!stricmp(a,"Start")){
    GetToken(a, file); ustart = (float)atof(a);
    if(!vangle) vangle = 6.2831853F;
	return 0;
  }
  if(!stricmp(a,"Stop" )){
    GetToken(a, file); ustop  = (float)atof(a);
    if(!vangle) vangle = 6.2831853F;
	return 0;
  }
  if(!stricmp(a,"r1")){
    GetToken(a, file); R1 = (float)atof(a);
	return 0;
  }
  if(!stricmp(a,"r2" )){
    GetToken(a, file); R2 = (float)atof(a);
	return 0;
  }
  return 1;
}

// Extents ---------------------------------------------------------

void Torus::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-(R1+R2), -(R1+R2), -R2);
  vector mx=vector(  R1+R2 ,   R1+R2 ,  R2);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Torus::RndPnt(void){ return vector(0.0f, 0.0f, 0.0f); }

vector Torus::PPoint(float u, float v){ return vector((float)((R1+R2*cos(u))*cos(v)), (float)((R1+R2*cos(u))*sin(v)), R2*(float)sin(u)); }

// Normals ---------------------------------------------------------

vector Torus::PNormal(float u, float v){ return vector((float)(R2*cos(v)*cos(u)), (float)(R2*sin(v)*cos(u)), (float)(R2*sin(u))); }

vector Torus::VNormal(vector p){ return unit(p); }
