
#include "stdafx.h"
#include "..\RaVi-Demo\ChildView.h"

#include "general.h"
#include "materials.h"
#include "RaVi.h"
#include "RaVi_Engine.h"

short int RV_AntiAlias = 0;
short int RV_Caustics  = 0;
short int RV_Quality   = 1;
short int RV_Filter    = 0;

unsigned short int RV_Threads   = 1;

float RV_FoV = 0.5f;

short int ThrL0= 5;
short int ThrL1=24;
short int ThrL2=32;
short int ThrL3=48;
short int ThrL4=64;

inline unsigned long int max_int3(unsigned long int a, unsigned long int b, unsigned long int c)
{
  return (a>b)? ((a>c)? a : c) : ((b>c)? b : c);
}

inline unsigned long int BlendRGB2(unsigned long int a, unsigned long int b)
{
  return ((a & b) + (((a ^ b) & 0xFEFEFE)>>1));
}

inline unsigned long int BlendRGB3(unsigned long int a, unsigned long int b, unsigned long int c)
{
  return ((((a&0xFF0000)+(b&0xFF0000)+(c&0xFF0000))/3)&0xFF0000) | ((((a&0xFF00)+(b&0xFF00)+(c&0xFF00))/3)&0xFF00) | ((((a&0xFF)+(b&0xFF)+(c&0xFF))/3)&0xFF);
}

inline unsigned long int BlendRGB4(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d)
{
  return ((((a&0xFF0000)+(b&0xFF0000)+(c&0xFF0000)+(d&0xFF0000))>>2)&0xFF0000) | ((((a&0xFF00)+(b&0xFF00)+(c&0xFF00)+(d&0xFF00))>>2)&0xFF00) | ((((a&0xFF)+(b&0xFF)+(c&0xFF)+(d&0xFF))>>2)&0xFF);
}

inline unsigned long int DiffRGB2(unsigned long int a, unsigned long int b)
{
  // error C2668
  long absval1 = (a>>16)-(b>>16);
  long absval2 = ((a>>8)&255)-((b>>8)&255);
  long absval3 = (a&255)-(b&255);
  return max_int3(abs(absval1), abs(absval2), abs(absval3) );
}

inline unsigned long int DiffRGB4(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d)
{
	long absval1 = (a>>16)-(b>>16);
	long absval2 = ((a>>8)&255)-((b>>8)&255);
	long absval3 = (a&255)-(b&255);
	long absval4 = (c>>16)-(d>>16);
	long absval5 = ((c>>8)&255)-((d>>8)&255);
	long absval6 = (c&255)-(d&255);
	long absval7 = (a>>16)-(d>>16);
	long absval8 = ((a>>8)&255)-((d>>8)&255);
	long absval9 = (a&255)-(d&255);
	long absval10 = (c>>16)-(b>>16);
	long absval11 = ((c>>8)&255)-((b>>8)&255);
	long absval12 = (c&255)-(b&255);

	return abs(absval1) + abs(absval2) + abs(absval3)
	+ abs(absval4) + abs(absval5) + abs(absval6)
    + abs(absval7) + abs(absval8) + abs(absval9)
    + abs(absval10) + abs(absval11) + abs(absval12);
}

unsigned long int TLin[820];
unsigned long int MLin[820];
unsigned long int BLin[820];

void DoFilter(byte *BMPbits, int Width, int Height)
{
  byte              *bitptr1;
  byte              *bitptr2;
  long unsigned int  ScLen;
  int                x,y;
  
  long unsigned int  cr,dg;

  long unsigned int *LBPt = TLin;
  long unsigned int *LBPm = MLin;
  long unsigned int *LBPb = BLin;

  ScLen= Width*4;
    
  bitptr1=BMPbits;
  // Initiate Linebuffer
  memcpy(LBPm+1,bitptr1,ScLen);
  bitptr2 = bitptr1; bitptr1 += ScLen;
  // extend borders
  LBPm[0]=LBPm[1]; LBPm[Width+1]=LBPm[Width];
  memcpy(LBPb,LBPm,4*(Width+2));

  for(y=0;y<Height;y++){
    // scroll line pointers
	if(LBPt==TLin){
      LBPt=MLin; LBPm=BLin; LBPb=TLin;
	} else if(LBPt==MLin){
      LBPt=BLin; LBPm=TLin; LBPb=MLin;
	} else {
      LBPt=TLin; LBPm=MLin; LBPb=BLin;
	}
    // copy line into buffer
    if(y<(Height-1)){
      memcpy(LBPb+1,bitptr1,ScLen);
      // extend borders
      LBPb[0]=LBPb[1]; LBPb[Width+1]=LBPb[Width];
    }
    for(x=0; x<Width; x++){
	  cr = BlendRGB4(LBPt[x+1],LBPm[x  ],LBPm[x+2],LBPb[x+1]);
	  dg = BlendRGB4(LBPt[x  ],LBPt[x+2],LBPb[x  ],LBPb[x+2]);
	  ((unsigned long int *)bitptr2)[x] = BlendRGB2(LBPm[x+1], BlendRGB3(cr, cr, dg));
    }
    bitptr2 = bitptr1; bitptr1 += ScLen;
  }
}

unsigned long int *fbf;

vector VPorg1, XD, YD;
int    XRs1, cd,cd1,cd2;

unsigned long int cc,bc,cr,br;

unsigned long int DoSubAA(Thrdata *Thread, float x, float y, unsigned long int br, unsigned long int tl, unsigned long int bl, unsigned long int tr)
{
  unsigned long int cc,bc,cr;
  cd =   DiffRGB2(br, bl);
  if(cd>ThrL4){
    Thread->CamRay = unit(VPorg1 + (y)      *YD + (x-0.25f)*XD);
    bc = Clamp2ABGR(RayTrace0(Thread));
  } else {
    bc = BlendRGB2(br, bl);
  }
  cd = DiffRGB2(br, tr);
  if(cd>ThrL4){
    Thread->CamRay = unit(VPorg1 + (y-0.25f)*YD + (x)      *XD);
    cr = Clamp2ABGR(RayTrace0(Thread));
  } else {
    cr = BlendRGB2(br, tr);
  }
  cd = DiffRGB2(br, tl);
  if(cd>ThrL4){
    Thread->CamRay = unit(VPorg1 + (y-0.25f)*YD + (x-0.25f)*XD);
    cc = Clamp2ABGR(RayTrace0(Thread));
  } else {
    cc = BlendRGB2(br, tl);
  }
  return BlendRGB4(br,cc,bc,cr);
}

void AAQ(Thrdata *Thread, int x, int y)
{
  unsigned long int sub_rb, sub_mb, sub_rm, sub_mm;
  char q1,q2,q3;

  cd = DiffRGB2(*(fbf+(y)*XRs1+(x)), *(fbf+(y)*XRs1+(x-1)) );
  if(cd>ThrL3){
    Thread->CamRay = unit(VPorg1 + ((float)(y))*YD + ((float)(x)-0.5f)*XD);
    sub_mb = Clamp2ABGR(RayTrace0(Thread)); q1=1;
  } else {
    sub_mb = *(fbf+(y)*XRs1+(x)); q1=0;
  }
  cd = DiffRGB2(*(fbf+(y)*XRs1+(x)), *(fbf+(y-1)*XRs1+(x)) );
  if(cd>ThrL3){
    Thread->CamRay = unit(VPorg1 + ((float)(y)-0.5f)*YD + ((float)(x))*XD);
    sub_rm = Clamp2ABGR(RayTrace0(Thread)); q2=1;
  } else {
    sub_rm = *(fbf+(y)*XRs1+(x)); q2=0;
  }
  cd = max(DiffRGB2(sub_mb, *(fbf+(y-1)*XRs1+(x  ))),
           DiffRGB2(sub_rm, *(fbf+(y  )*XRs1+(x-1))) );
  if(cd>ThrL3){
    Thread->CamRay = unit(VPorg1 + ((float)(y)-0.5f)*YD + ((float)(x)-0.5f)*XD);
    sub_mm = Clamp2ABGR(RayTrace0(Thread)); q3=1;
  } else {
    sub_mm = *(fbf+(y)*XRs1+(x)); q3=0;
  }
  if(RV_AntiAlias>1){
           sub_rb = DoSubAA(Thread, ((float)x)     , ((float)y)     , *(fbf+y*XRs1+x), sub_mm, sub_mb, sub_rm);
    if(q1) sub_mb = DoSubAA(Thread, ((float)x)-0.5f, ((float)y)     , sub_mb, BlendRGB2(*(fbf+(y  )*XRs1+(x-1)), *(fbf+(y-1)*XRs1+(x-1))), *(fbf+(y  )*XRs1+(x-1)), sub_mm );
    if(q2) sub_rm = DoSubAA(Thread, ((float)x)     , ((float)y)-0.5f, sub_rm, BlendRGB2(*(fbf+(y-1)*XRs1+(x  )), *(fbf+(y-1)*XRs1+(x-1))), sub_mm, *(fbf+(y-1)*XRs1+(x  )) );
    if(q3) sub_mm = DoSubAA(Thread, ((float)x)-0.5f, ((float)y)-0.5f, sub_mm, *(fbf+(y-1)*XRs1+(x-1)), BlendRGB2(*(fbf+(y  )*XRs1+(x-1)), *(fbf+(y-1)*XRs1+(x-1))), BlendRGB2(*(fbf+(y-1)*XRs1+(x  )), *(fbf+(y-1)*XRs1+(x-1))) );
    *(fbf+(y)*XRs1+(x)) = BlendRGB4(sub_rb,              sub_mm, sub_mb, sub_rm);
  } else {
    *(fbf+(y)*XRs1+(x)) = BlendRGB4(*(fbf+(y)*XRs1+(x)), sub_mm, sub_mb, sub_rm);
  }
}

void Quadrant2(Thrdata *Thread, int x, int y, char q)
{
  char q1,q2,q3;
  if(!(q & 1)){
    cd = DiffRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y  )*XRs1+(x-2)) );
    if(cd>ThrL2){
      Thread->CamRay = unit(VPorg1 + ((float)(y  ))*YD + ((float)(x-1))*XD);
      *(fbf+(y  )*XRs1+(x-1)) = Clamp2ABGR(RayTrace0(Thread)); q1=1;
	} else {
      *(fbf+(y  )*XRs1+(x-1)) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y  )*XRs1+(x-2)) ); q1=0;
	}
  }
  if(!(q & 2)){
    cd = DiffRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x  )) );
    if(cd>ThrL2){
      Thread->CamRay = unit(VPorg1 + ((float)(y-1))*YD + ((float)(x  ))*XD);
      *(fbf+(y-1)*XRs1+(x  )) = Clamp2ABGR(RayTrace0(Thread)); q2=1;
	} else {
      *(fbf+(y-1)*XRs1+(x  )) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x  )) ); q2=0;
	}
  }
  cd = max(DiffRGB2(*(fbf+(y-1)*XRs1+(x  )), *(fbf+(y-1)*XRs1+(x-2))),
	       DiffRGB2(*(fbf+(y  )*XRs1+(x-1)), *(fbf+(y-2)*XRs1+(x-1))) );
  if(cd>ThrL2){
    Thread->CamRay = unit(VPorg1 + ((float)(y-1))*YD + ((float)(x-1))*XD);
    *(fbf+(y-1)*XRs1+(x-1)) = Clamp2ABGR(RayTrace0(Thread)); q3=1;
  } else {
    *(fbf+(y-1)*XRs1+(x-1)) = BlendRGB4(*(fbf+(y-1)*XRs1+(x  )), *(fbf+(y-1)*XRs1+(x-2)), *(fbf+(y  )*XRs1+(x-1)), *(fbf+(y-2)*XRs1+(x-1)) ); q3=0;
  }
  if(RV_AntiAlias){
           AAQ(Thread, x  ,y  );
    if(q1) AAQ(Thread, x-1,y  );
    if(q2) AAQ(Thread, x  ,y-1);
    if(q3) AAQ(Thread, x-1,y-1);
  }
}

int     Rx,Ry;
int     ThrCnt;
int     tXRs,tYRs,tBrd;
HANDLE  ThrFinished;

extern  Thrdata ThD[4];

//CRITICAL_SECTION nxpx;

UINT RenderPixels(LPVOID pParam)
{
  int        x,y;
  RGB_Color  Col;

  if(RV_Threads==1){
	for(y=0; y<tYRs; y++){
	  for(x=tBrd; x<tXRs; x++){
        ThD[0].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
        Col = RayTrace0(&(ThD[0]));
        *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
	  }
	}
  } else if(RV_Threads==2){
	if((int)pParam==0){
	  for(y=0; y<tYRs; y++){
	    for(x=tBrd; x<(tXRs/2); x++){
          ThD[0].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[0]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	} else {
	  for(y=0; y<tYRs; y++){
	    for(x=(tXRs/2); x<tXRs; x++){
          ThD[1].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[1]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	}
  } else if(RV_Threads==4){
	if((int)pParam==0){
	  for(y=0; y<(tYRs/2); y++){
	    for(x=tBrd; x<(tXRs/2); x++){
          ThD[0].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[0]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	} else if((int)pParam==1){
	  for(y=0; y<(tYRs/2); y++){
	    for(x=(tXRs/2); x<tXRs; x++){
          ThD[1].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[1]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	} else if((int)pParam==2){
	  for(y=(tYRs/2); y<tYRs; y++){
	    for(x=tBrd; x<(tXRs/2); x++){
          ThD[2].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[2]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	} else if((int)pParam==3){
	  for(y=(tYRs/2); y<tYRs; y++){
	    for(x=(tXRs/2); x<tXRs; x++){
          ThD[3].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
          Col = RayTrace0(&(ThD[3]));
          *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	}
  }
  ThrCnt--; if(!ThrCnt) SetEvent(ThrFinished);
  return 0;	// thread completed successfully
}


/*UINT RenderPixels(LPVOID pParam)
{
  int        x,y;
  RGB_Color  Col;

  while(Ry<tYRs){;
	EnterCriticalSection(&nxpx);
    x=Rx; y=Ry;	if((Rx++)>=tXRs){ Rx=tBrd; Ry++; }
	LeaveCriticalSection(&nxpx);
    Col = RayTrace0(unit(VPorg1 + ((float)y)*YD + ((float)x)*XD));
	//Col.G = 0.25f*((int)pParam);
    *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
  }
  ThrCnt--; if(!ThrCnt) SetEvent(ThrFinished);
  return 0;	// thread completed successfully
}*/

UINT RenderSuperPixels(LPVOID pParam)
{
  char q00,q01,q10,q11;
  int  x,y;

  for(y=((Thrdata *)pParam)->ys+4; y<((Thrdata *)pParam)->ye+1; y+=4){
    for(x=((Thrdata *)pParam)->xs+4; x<((Thrdata *)pParam)->xe+1; x+=4){
	  ((Thrdata *)pParam)->CamRay = unit(VPorg1 + ((float)y)*YD + ((float)x)*XD);
	  *(fbf+y*XRs1+x) = Clamp2ABGR(RayTrace0((Thrdata *)pParam));
      cd = DiffRGB4(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-4)*XRs1+(x-4)), *(fbf+(y  )*XRs1+(x-4)), *(fbf+(y-4)*XRs1+(x  )) );
	  if(abs(cd)>ThrL0){
	    q00=0; q01=0; q10=0; q11=0;
        cd = DiffRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y  )*XRs1+(x-4)) );
        if(cd>ThrL1){
	      ((Thrdata *)pParam)->CamRay = unit(VPorg1 + ((float)(y  ))*YD + ((float)(x-2))*XD);
		  *(fbf+(y  )*XRs1+(x-2)) = Clamp2ABGR(RayTrace0((Thrdata *)pParam));
		} else {
          *(fbf+(y  )*XRs1+(x-2)) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y  )*XRs1+(x-4)) );
          *(fbf+(y  )*XRs1+(x-1)) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y  )*XRs1+(x-2)) );
          *(fbf+(y  )*XRs1+(x-3)) = BlendRGB2(*(fbf+(y  )*XRs1+(x-2)), *(fbf+(y  )*XRs1+(x-4)) );
		  q00|=1; q10|=1;
		}
        cd = DiffRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-4)*XRs1+(x  )) );
        if(cd>ThrL1){
		  ((Thrdata *)pParam)->CamRay = unit(VPorg1 + ((float)(y-2))*YD + ((float)(x  ))*XD);
		  *(fbf+(y-2)*XRs1+(x  )) = Clamp2ABGR(RayTrace0((Thrdata *)pParam));
		} else {
          *(fbf+(y-2)*XRs1+(x  )) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-4)*XRs1+(x  )) );
          *(fbf+(y-1)*XRs1+(x  )) = BlendRGB2(*(fbf+(y  )*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x  )) );
          *(fbf+(y-3)*XRs1+(x  )) = BlendRGB2(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-4)*XRs1+(x  )) );
		  q00|=2; q01|=2;
		}
        cd1 = DiffRGB2(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x-4)) );
		cd2 = DiffRGB2(*(fbf+(y  )*XRs1+(x-2)), *(fbf+(y-4)*XRs1+(x-2)) );
		if((cd1<=ThrL1) && (cd2<=ThrL1)){
          *(fbf+(y-2)*XRs1+(x-2)) = BlendRGB4(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x-4)), *(fbf+(y  )*XRs1+(x-2)), *(fbf+(y-4)*XRs1+(x-2)) );
          *(fbf+(y-2)*XRs1+(x-1)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x-2)) );
          *(fbf+(y-2)*XRs1+(x-3)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x-2)), *(fbf+(y-2)*XRs1+(x-4)) );
          *(fbf+(y-3)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x-2)), *(fbf+(y-4)*XRs1+(x-2)) );
          *(fbf+(y-1)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y  )*XRs1+(x-2)), *(fbf+(y-2)*XRs1+(x-2)) );
		  q10|=2; q01|=1; q11|=3;
		} else if(cd1<=ThrL1) {
          *(fbf+(y-2)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x-4)) );
          *(fbf+(y-2)*XRs1+(x-1)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x  )), *(fbf+(y-2)*XRs1+(x-2)) );
          *(fbf+(y-2)*XRs1+(x-3)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x-2)), *(fbf+(y-2)*XRs1+(x-4)) );
		  q11|=1; q01|=1;
		} else if(cd2<=ThrL1) {
          *(fbf+(y-2)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y  )*XRs1+(x-2)), *(fbf+(y-4)*XRs1+(x-2)) );
          *(fbf+(y-3)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y-2)*XRs1+(x-2)), *(fbf+(y-4)*XRs1+(x-2)) );
          *(fbf+(y-1)*XRs1+(x-2)) = BlendRGB2(*(fbf+(y  )*XRs1+(x-2)), *(fbf+(y-2)*XRs1+(x-2)) );
		  q11|=2; q10|=2;
		} else {
		  ((Thrdata *)pParam)->CamRay = unit(VPorg1 + ((float)(y-2))*YD + ((float)(x-2))*XD);
		  *(fbf+(y-2)*XRs1+(x-2)) = Clamp2ABGR(RayTrace0((Thrdata *)pParam));
		}
        Quadrant2((Thrdata *)pParam, x-2,y-2,q11);
		Quadrant2((Thrdata *)pParam, x  ,y-2,q01);
		Quadrant2((Thrdata *)pParam, x-2,y  ,q10);
		Quadrant2((Thrdata *)pParam, x  ,y  ,q00);
	  } else {
	    unsigned long int *pixels;
		unsigned long int  color = *(fbf+y*XRs1+x);
		for(int yi=0; yi<4; yi++){
		  pixels=fbf+(y-yi)*XRs1;
		  if(yi) pixels[x  ]=color;
		         pixels[x-1]=color;
		         pixels[x-2]=color;
		         pixels[x-3]=color;
		}
	  }
    }
  }
  ThrCnt--; if(!ThrCnt) SetEvent(ThrFinished);
  return 0;	// thread completed successfully
}

/*CRITICAL_SECTION CRc;
ushort           CRay;

ushort NewRay(void)
{
  ushort nr;
  EnterCriticalSection(&CRc);
  nr = ++CRay;
  LeaveCriticalSection(&CRc);
  return nr;
}*/

void RV_RenderImage(byte *BMPbits, int XRes, int YRes, int border)
{
  int        x,y;
  RGB_Color  Col;
  vector     Ray;

  fbf = (unsigned long int *)BMPbits; XRs1=XRes+border;

  //InitializeCriticalSection(&CRc); CRay=0;

  vector N  = unit(RV_Target - *RV_CamPtr);
  vector V  = unit(RV_Up-(RV_Up%N)*N);
  vector U  = N*V;

  float  b  = (float)tan(RV_FoV/2.0f);
  float  bx = XRes<YRes? b : b*((float)XRes/(float)YRes)    ;
  float  by = XRes<YRes?     b*((float)YRes/(float)XRes) : b;

  VPorg1 = N - bx*U - by*V;
  YD     = 2.0f/(float)YRes*by*V;
  XD     = 2.0f/(float)XRes*bx*U;
  
  ThD[0].CRay=0;

  if(RV_Quality==1){
	if(RV_Threads==1){
      for(y=0; y<YRes; y++){
	    Ray = VPorg1 + ((float)y)*YD;
        for(x=border; x<XRs1; x++){
		  //if((x==XRs1/7) && (y==YRes/2)){
		  /*if((x==62) && (y==49) && (RV_Caustics_Interpolation==2)){
		    Col=RGB_Color(0.0f, 0.0f, 0.0f);
		  }*/
		  ThD[0].CamRay = unit(Ray + ((float)x)*XD); Col = RayTrace0(&(ThD[0]));
	      *(fbf+y*XRs1+x)=Clamp2ABGR(Col);
		}
	  }
	} else {
	  ResetEvent(ThrFinished); //InitializeCriticalSection(&nxpx);
	  Rx=0; Ry=0; tXRs=XRes; tYRs=YRes; tBrd=border; ThrCnt=RV_Threads;
      ThD[1].CRay=10000; ThD[2].CRay=20000; ThD[3].CRay=30000;
	  for(int i=0; i<RV_Threads; i++){
        AfxBeginThread(RenderPixels, (LPVOID)i);
	  }
	  DWORD wr=WaitForSingleObject(ThrFinished, 2000);
	  //DeleteCriticalSection(&nxpx);
	  if(wr==WAIT_TIMEOUT){
	    char msg[80]; sprintf(msg,"Time out, ThrCnt=%d",ThrCnt);
	    MessageBox(NULL,msg,"Debug",MB_OK);
	  }
	}
  } else if(RV_Quality==2){
    ThD[0].CamRay = unit(VPorg1); Col = RayTrace0(&(ThD[0])); *(fbf)=Clamp2ABGR(Col);
	for(x=4; x<XRes+1; x+=4){
      ThD[0].CamRay = unit(VPorg1 + ((float)x)*XD); Col = RayTrace0(&(ThD[0]));
	  *(fbf+ x   )=Clamp2ABGR(Col);
      *(fbf+(x-2))=BlendRGB2(*(fbf+(x  )), *(fbf+(x-4)) );
      *(fbf+(x-1))=BlendRGB2(*(fbf+(x  )), *(fbf+(x-2)) );
      *(fbf+(x-3))=BlendRGB2(*(fbf+(x-2)), *(fbf+(x-4)) );
    }
	for(y=4; y<YRes+1; y+=4){
      ThD[0].CamRay = unit(VPorg1 + ((float)y)*YD); Col = RayTrace0(&(ThD[0]));
	  *(fbf+ y   *XRs1)=Clamp2ABGR(Col);
      *(fbf+(y-2)*XRs1)=BlendRGB2(*(fbf+(y  )*XRs1), *(fbf+(y-4)*XRs1) );
      *(fbf+(y-1)*XRs1)=BlendRGB2(*(fbf+(y  )*XRs1), *(fbf+(y-2)*XRs1) );
      *(fbf+(y-3)*XRs1)=BlendRGB2(*(fbf+(y-2)*XRs1), *(fbf+(y-4)*XRs1) );
	}
	if(RV_Threads>1){
      unsigned long int *fb2 = fbf+XRes/2; 
	  for(y=4; y<YRes+1; y+=4){
        ThD[0].CamRay = unit(VPorg1 + ((float)y)*YD + ((float)(XRes/2))*XD); Col = RayTrace0(&(ThD[0]));
	    *(fb2+ y   *XRs1)=Clamp2ABGR(Col);
        *(fb2+(y-2)*XRs1)=BlendRGB2(*(fb2+(y  )*XRs1), *(fb2+(y-4)*XRs1) );
        *(fb2+(y-1)*XRs1)=BlendRGB2(*(fb2+(y  )*XRs1), *(fb2+(y-2)*XRs1) );
        *(fb2+(y-3)*XRs1)=BlendRGB2(*(fb2+(y-2)*XRs1), *(fb2+(y-4)*XRs1) );
	  }
	  if(RV_Threads>2){
        fb2 = fbf+YRes/2*XRs1; 
	    for(x=4; x<XRes+1; x+=4){
          ThD[0].CamRay = unit(VPorg1 + ((float)(YRes/2))*YD + ((float)x)*XD); Col = RayTrace0(&(ThD[0]));
	      *(fb2+ x   )=Clamp2ABGR(Col);
          *(fb2+(x-2))=BlendRGB2(*(fb2+(x  )), *(fb2+(x-4)) );
          *(fb2+(x-1))=BlendRGB2(*(fb2+(x  )), *(fb2+(x-2)) );
          *(fb2+(x-3))=BlendRGB2(*(fb2+(x-2)), *(fb2+(x-4)) );
		}
	  }
	}
	if(RV_Threads>1){
	  ResetEvent(ThrFinished); ThrCnt=RV_Threads;
	  if(RV_Threads==2){
	    ThD[0].xs=     0; ThD[0].ys=     0; ThD[0].xe=XRes/2; ThD[0].ye=YRes;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[0])));
	    ThD[1].xs=XRes/2; ThD[1].ys=     0; ThD[1].xe=XRes  ; ThD[1].ye=YRes  ; ThD[1].CRay=10000;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[1])));
	  } else if(RV_Threads==4){
	    ThD[0].xs=     0; ThD[0].ys=     0; ThD[0].xe=XRes/2; ThD[0].ye=YRes/2;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[0])));
	    ThD[1].xs=XRes/2; ThD[1].ys=     0; ThD[1].xe=XRes  ; ThD[1].ye=YRes/2; ThD[1].CRay=10000;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[1])));
	    ThD[2].xs=     0; ThD[2].ys=YRes/2; ThD[2].xe=XRes/2; ThD[2].ye=YRes  ; ThD[2].CRay=20000;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[2])));
	    ThD[3].xs=XRes/2; ThD[3].ys=YRes/2; ThD[3].xe=XRes  ; ThD[3].ye=YRes  ; ThD[3].CRay=30000;
        AfxBeginThread(RenderSuperPixels, (LPVOID)(&(ThD[3])));
	  } 
	  DWORD wr=WaitForSingleObject(ThrFinished, 2000);
	  if(wr==WAIT_TIMEOUT){
	    char msg[80]; sprintf(msg,"Time out, ThrCnt=%d",ThrCnt);
	    MessageBox(NULL,msg,"Debug",MB_OK);
	  }
	} else {
	  ThD[0].xs=0; ThD[0].ys=0; ThD[0].xe=XRes; ThD[0].ye=YRes;
	  RenderSuperPixels((LPVOID)(&(ThD[0])));
	}
  }
  if(RV_Filter) DoFilter(BMPbits, XRes, YRes);
  //DeleteCriticalSection(&CRc);
}
