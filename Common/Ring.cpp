// Ring Primitive for VModel & WinOSi
// ----------------------------------
// 09.06.1999           Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

// Parsing ---------------------------------------------------------

extern void  GetToken (char *a, CFile *file);
extern float ReadAngle(char *a);

int Ring::ParseToken(char *a, CFile *file)
{
  if(!stricmp(a,"Angle")){
    GetToken(a, file); angle = ReadAngle(a);
	return 0;
  }
  if(!stricmp(a,"Ri")){
    GetToken(a, file); r_i = (float)atof(a);
	return 0;
  }
  return 1;
}

// Extents ---------------------------------------------------------

void Ring::WorldExtents(vector & min, vector & max)
{
  vector mn=vector(-1.0f, -1.0f, 0.0f);
  vector mx=vector( 1.0f,  1.0f, 0.0f);
  CStretch(min, max, mn, mx);
}

// Points ----------------------------------------------------------

vector Ring::RndPnt (void)
{
  //double b = TwoPi*rnd(); double a=ri+(ra-ri)*rnd();
  //return vector(a*cos(b), 0.0f, a*sin(b))*TM;
  return vector(0.0f, 0.0f, 0.0f);
}

vector Ring::PPoint (float u, float v){ return vector(u*(float)cos(v), u*(float)sin(v), 0.0f); }

// Normals ---------------------------------------------------------

vector Ring::PNormal(float u, float v){ return vector(0.0f, 0.0f, 1.0f); }

vector Ring::VNormal(vector p)        { return vector(0.0f, 0.0f, 1.0f); }
