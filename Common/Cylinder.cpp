// Cylinder Primitive for VModel & WinOSi
// --------------------------------------
// 09.06.1999               Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

// Parsing ---------------------------------------------------------

extern void  GetToken (char *a, CFile *file);
extern float ReadAngle(char *a);

int Cylinder::ParseToken(char *a, CFile *file)
{
  if(!stricmp(a,"Angle")){
    GetToken(a, file); angle = ReadAngle(a);
	return 0;
  }
  if(!stricmp(a,"r_top")){
    GetToken(a, file); r_top = (float)atof(a);
	return 0;
  }
  return 1;
}

// Extents ---------------------------------------------------------

void Cylinder::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-1.0f, -1.0f, 0.0f);
  vector mx=vector( 1.0f,  1.0f, 1.0f);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Cylinder::RndPnt (void)
{
  double b = 6.2831853*rnd();
  return vector((float)cos(b), (float)rnd(), (float)sin(b))*TM;
}

vector Cylinder::PPoint (float u, float v){ return vector((u*r_top+(1.0f-u))*(float)cos(v), (u*r_top+(1.0f-u))*(float)sin(v), u); }

// Normals ---------------------------------------------------------

vector Cylinder::PNormal(float u, float v)
{
  return (flags & SURF_INVERSE)? -vector((float)cos(v), (float)sin(v), 1.0f-r_top ) : vector((float)cos(v), (float)sin(v), 1.0f-r_top );
}

vector Cylinder::VNormal(vector p)
{
  vector P=p*Tb;
  return (flags & SURF_INVERSE)? -unit(vector(P.x, 0.0f, P.z)*NM) : unit(vector(P.x, 0.0f, P.z)*NM);
}
