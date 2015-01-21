
#include "stdafx.h"
#include <math.h>

#include "general.h"
#include "Materials.h"

float wrap(float c)
{
  if(c>=0.0f){
    return      fract(c);
  } else {
	return 1.0f-fract(c);
  }
}

float wlcoff(float cR, float cG, float cB, float v)
{
  float f;
  if(v>380.0){
	if(v>410.0){
	  if(v>470.0){
	    if(v>500.0){
	      if(v>530.5){
	        if(v>590.0){
	          if(v>660.0){
	            if(v<780.0){
				  f = cR*((780.0f-v)/(780.0f-660.0f));
				}
			  } else {
				f = max(cR,cG*((660.0f-v)/(660.0f-590.0f)));
			  }
			} else {
			  f = max(cG,cR*((v-530.0f)/(590.0f-530.0f)));
			}
		  } else {
			f = max(cG,cB*((530.0f-v)/(530.0f-500.0f)));
		  }
		} else {
		  f = max(cB,cG*((v-470.0f)/(500.0f-470.0f)));
		}
  	  } else {
	    f = max(cB,cR*((470.0f-v)/(470.0f-410.0f)));
	  }
	} else {
	  f = max(cR*((v-380.0f)/(410.0f-380.0f)),cB*((v-380.0f)/(410.0f-380.0f)));
	}
  }
  return f;
}

// fresnel formula straight approach by Michael:
/*float R_fnf(float n1, float n2, double ce1)
{
  if(fabs(n2-n1)<0.001) return 0.0f;
  double e1 = acos(fabs(ce1));
  if(e1<0.01f) return (float)sqr((n1-n2)/(n1+n2));
  double c1 = sin(e1) * n1/n2;
  if(c1 < 1.0){
    double e2 = asin(c1);
    return 0.5f*(float)( sqr(sin(e1-e2))/sqr(sin(e1+e2)) + sqr(tan(e1-e2))/sqr(tan(e1+e2)) );
  } else return 1.0f;
}*/

// fresnel formula optimized by Dmitry Omelchenko aka tigra:
float R_fnf(float n1, float n2, double ce1)
{
  double c12,z,A2B2,AB2,C2,D2,ZC12,face1;

  if(fabs(n2-n1)<0.001) return 0.0f;
  face1=fabs(ce1);
  if(acos(face1)<0.01f) return (float)sqr((n1-n2)/(n1+n2));
  z=1-face1*face1;
  c12=z*sqr(n1/n2);
  if(c12<1.0){
    ZC12=z*c12;
    A2B2=z+c12-ZC12-ZC12;
    AB2=sqrt((z-ZC12)*(c12-ZC12));AB2=AB2+AB2;
    C2=A2B2-AB2;D2=A2B2+AB2;
    return (float) (C2*(1+(1-D2)/(1-C2))/(D2+D2));
  }
  else return 1.0f;
}

float Material::IOR(float w)
{
  if(IOR_offset<0.0)
	return IOR_base;
  else
	return IOR_base + IOR_offset / (w - IOR_sub);
}

float Material::Diff(float w, float x, float y)
{
  if((Specularity > 0.99) || (Reflection.R < 0.0f)) return 0.0f;
  if(Texture){
	//if((CInt->x<0.0f) || (CInt->x>1.0f) || (CInt->y<0.0f) || (CInt->y>1.0f)) return 0.5f;
	if(Texture->tbits){
      unsigned char *bitptr = Texture->tbits + ((int)(wrap(y)*Texture->tinfo.bmiHeader.biHeight)) * ((Texture->tinfo.bmiHeader.biWidth*((Texture->tinfo.bmiHeader.biBitCount)>>3)+3) & 0xFFFFFFFC);
      return (1.0f - Specularity) * wlcoff(bitptr[3*((int)(wrap(x)*Texture->tinfo.bmiHeader.biWidth))+2]/255.0f, bitptr[3*((int)(wrap(x)*Texture->tinfo.bmiHeader.biWidth))+1]/255.0f, bitptr[3*((int)(wrap(x)*Texture->tinfo.bmiHeader.biWidth))]/255.0f, w);
	}
  }
  if(Reflection.G < 0.0f) return (1.0f - Specularity) * Reflection.R;
  return (1.0f - Specularity) * wlcoff(Reflection.R, Reflection.G, Reflection.B, w);
}

float Material::Spec(float w, float n1, float n2, double ce1)
{
  if(Specularity < 0.01) return 0.0f;
  if(Reflection.R < 0.0f) return R_fnf(n1,n2,ce1);
  if(Reflection.G < 0.0f) return Specularity * Reflection.R;
  return Specularity * wlcoff(Reflection.R, Reflection.G, Reflection.B, w);
}

float Material::Trns(float w, float n1, float n2, double ce1)
{
  if((Specularity < 0.01) || ((Transmission.R < 0.01f) && (Reflection.R >= 0.0f)) ) return 0.0f;
  if(Reflection.R < 0.0f) return 1.0f - R_fnf(n1,n2,ce1);
  return Transmission.R;
}

float Material::Thru(float w, float d)
{
  if(Absorption.G < 0.0f) if(Absorption.R==0.0f) return 1.0f; else return (float)pow((1.0f-Absorption.R), d/Ref_Length);
  return (float)pow((1.0f-wlcoff(Absorption.R, Absorption.G, Absorption.B, w)), d/Ref_Length);
}

