// Sphere Primitive for VModel & WinOSi
// ------------------------------------
// 09.06.1999             Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

// Parsing ---------------------------------------------------------

extern void  GetToken (char *a, CFile *file);
extern float ReadAngle(char *a);

int Sphere::ParseToken(char *a, CFile *file)
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
  return 1;
}

// Extents ---------------------------------------------------------

void Sphere::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-1.0f, -1.0f, -1.0f);
  vector mx=vector( 1.0f,  1.0f,  1.0f);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Sphere::RndPnt (void){ return (vangle? rrdir(vangle,ustart,ustop) : rnddir())*TM; }

vector Sphere::PPoint (float u, float v){ return vector((float)(cos(v)*cos(u)), (float)(sin(v)*cos(u)), (float)sin(u)); }

// Normals ---------------------------------------------------------

vector Sphere::PNormal(float u, float v){ return vector((float)(cos(v)*cos(u)), (float)(sin(v)*cos(u)), (float)sin(u)); }

vector Sphere::VNormal(vector p) { return (flags & SURF_INVERSE)? -unit((p*Tb)*NM) : unit((p*Tb)*NM); }
