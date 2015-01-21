// Paraboloid Primitive for VModel & WinOSi
// ----------------------------------------
// 09.06.1999                 Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

// Parsing ---------------------------------------------------------

extern void  GetToken (char *a, CFile *file);
extern float ReadAngle(char *a);

int Paraboloid::ParseToken(char *a, CFile *file)
{
  if(!stricmp(a,"Angle")){
    GetToken(a, file); angle = ReadAngle(a);
	return 0;
  }
  if(!stricmp(a,"Height")){
    GetToken(a, file); height = (float)atof(a);
	return 0;
  }
  return 1;
}

// Extents ---------------------------------------------------------

void Paraboloid::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-1.0f, -1.0f,   0.0f);
  vector mx=vector( 1.0f,  1.0f, height);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Paraboloid::RndPnt (void){ return vector(0.0f, 0.0f, 0.0f); }

vector Paraboloid::PPoint (float u, float v){ return u * vector((float)cos(v), (float)sin(v), u); }

// Normals ---------------------------------------------------------

vector Paraboloid::PNormal(float u, float v){ return vector(2.0f*u*(float)cos(v), 2.0f*u*(float)sin(v), -1.0f); }

vector Paraboloid::VNormal(vector p){ return unit(p); }
