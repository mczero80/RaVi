// Function Primitive for VModel & WinOSi
// --------------------------------------
// 09.06.1999               Michael Granz

#include "stdafx.h"
#include <math.h>
#include <string.h>
#include "general.h"
#include "Primitives.h"

double Var_X;
double Var_Y;
double VTime = 0.0;

double CalcExpr(Expr *Fn)
{
  if(Fn->type == ExpTyp_Number) return Fn->valu;
  if(Fn->type == ExpTyp_Var_X ) return Var_X;
  if(Fn->type == ExpTyp_Var_Y ) return Var_Y;
  if(Fn->type == ExpTyp_Time  ) return VTime;
  if(Fn->func == ExpFnc_plus  ) return CalcExpr(Fn->par1) + CalcExpr(Fn->par2);
  if(Fn->func == ExpFnc_minus ) return CalcExpr(Fn->par1) - CalcExpr(Fn->par2);
  if(Fn->func == ExpFnc_mult  ) return CalcExpr(Fn->par1) * CalcExpr(Fn->par2);
  if(Fn->func == ExpFnc_div   ) return CalcExpr(Fn->par1) / CalcExpr(Fn->par2);
  if(Fn->func == ExpFnc_sqr   ) return sqr (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_sqrt  ) return sqrt(CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_sin   ) return sin (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_cos   ) return cos (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_exp   ) return exp (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_log   ) return log (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_abs   ) return fabs(CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_sgn   ) return sgn (CalcExpr(Fn->par1));
  if(Fn->func == ExpFnc_atn   ) return atan(CalcExpr(Fn->par1));
  return 0.0;
}

// Parsing ---------------------------------------------------------

extern void GetLine (char *a, CFile *file);
extern void GetToken(char *a, CFile *file);

int Function::ParseToken(char *a, CFile *file)
{
  if(!stricmp(a,"Min")){
	GetToken(a,file); Min.x = (float)atof(a);
	GetToken(a,file); Min.y = (float)atof(a);
	GetToken(a,file); Min.z = (float)atof(a);
	return 0;
  }
  if(!stricmp(a,"Max")){
	GetToken(a,file); Max.x = (float)atof(a);
	GetToken(a,file); Max.y = (float)atof(a);
	GetToken(a,file); Max.z = (float)atof(a);
	return 0;
  }
  if(!stricmp(a,"StepSize")){
	GetToken(a,file); JS = (float)atof(a);
	return 0;
  }
  if(!stricmp(a,"Formula")){
	GetLine(a,file); func = ParseFunc(a);
	return 0;
  }
  return 1;
}

ExprPtr ParseFunc(char *func)
{
  int p,b; ExprPtr e=new Expr;
  // addition and subtraktion
  p=strlen(func)-1; b=0;
  while(p && ((func[p]!='+') || b) && ((func[p]!='-') || b)){
    if(func[p]=='(') b++;
    if(func[p]==')') b--;
    p--;
  }
  if(p){
    e->type=ExpTyp_Function; e->func = (func[p]=='+'? ExpFnc_plus : ExpFnc_minus);
    func[p]=0; e->par1=ParseFunc(func); e->par2=ParseFunc(func+p+1);
    return e;
  }
  // multiplication and division
  p=strlen(func)-1; b=0;
  while(p && ((func[p]!='*') || b) && ((func[p]!='/') || b)){
    if(func[p]=='(') b++;
    if(func[p]==')') b--;
    p--;
  }
  if(p){
    e->type=ExpTyp_Function; e->func = (func[p]=='*'? ExpFnc_mult : ExpFnc_div);
    func[p]=0; e->par1=ParseFunc(func); e->par2=ParseFunc(func+p+1);
    return e;
  }
  if(func[strlen(func)-1]=='²'){
    // square
    e->func = ExpFnc_sqr; func[strlen(func)-1]=0;
    e->par1 = ParseFunc(func);
  } else {
    if(strchr(func,'(')){
      // function
      e->type=ExpTyp_Function; func[strlen(func)-1]=0;
      if(*func == '('){
        delete e; return ParseFunc(func+1);
	  } else if(!strnicmp(func,"sqr",3)){
        e->func = ExpFnc_sqrt;
	  } else if(!strnicmp(func,"cos",3)){
        e->func = ExpFnc_cos;
	  } else if(!strnicmp(func,"sin",3)){
        e->func = ExpFnc_sin;
	  } else if(!strnicmp(func,"exp",3)){
        e->func = ExpFnc_exp;
	  } else if(!strnicmp(func,"log",3)){
        e->func = ExpFnc_log;
	  } else if(!strnicmp(func,"abs",3)){
        e->func = ExpFnc_abs;
	  } else if(!strnicmp(func,"sgn",3)){
        e->func = ExpFnc_sgn;
	  } else if(!strnicmp(func,"atn",3)){
        e->func = ExpFnc_atn;
	  }
      e->par1 = ParseFunc(func+4);
	} else {
      // constant or variable
             if(*func == 'x'){
        e->type=ExpTyp_Var_X;
      } else if(*func == 'y'){
        e->type=ExpTyp_Var_Y;
      } else if(*func == 't'){
        e->type=ExpTyp_Time;
      } else {
        e->type=ExpTyp_Number;
        e->valu=atof(func);
      }
    }
  }
  return e;
}

// Intersection ----------------------------------------------------

/*float Function::IntFac(vector Org, vector Ray, int algn)
{
  vector B,N;
  switch(algn){
    case 0: B=Max; N=vector( 1.0f,  0.0f,  0.0f); break;
    case 1: B=Max; N=vector( 0.0f,  1.0f,  0.0f); break;
    case 2: B=Max; N=vector( 0.0f,  0.0f,  1.0f); break;
    case 3: B=Min; N=vector(-1.0f,  0.0f,  0.0f); break;
    case 4: B=Min; N=vector( 0.0f, -1.0f,  0.0f); break;
    case 5: B=Min; N=vector( 0.0f,  0.0f, -1.0f); break;
  }
  if((Ray%N)==0.0f) return -1.0f;
  float t = ((B-Org)%N)/(Ray%N); if(t<=0.0f) return -1.0f;
  vector iP = Org+t*Ray;
  switch(algn){
    case 0:
    case 3: return ((iP.y>=Min.y) && (iP.y<=Max.y) && (iP.z>=Min.z) && (iP.z<=Max.z))? t : -1.0f; break;
    case 1: 
    case 4: return ((iP.x>=Min.x) && (iP.x<=Max.x) && (iP.z>=Min.z) && (iP.z<=Max.z))? t : -1.0f; break;
    case 2:
    case 5: return ((iP.x>=Min.x) && (iP.x<=Max.x) && (iP.y>=Min.y) && (iP.y<=Max.y))? t : -1.0f; break;
  }
  return -1.0f;
}

double Function::frec(float t1, float t2, vector Org, vector Ray, int level)
{
  vector P1 = Org+t1*Ray; float v1=PPoint(P1.x, P1.y).z;
  vector P2 = Org+t2*Ray; float v2=PPoint(P2.x, P2.y).z;
  if(((v1>P1.z) && (v2>P2.z)) || ((v1<P1.z) && (v2<P2.z))) return -1.0;
  float tm;
  if(level<3){
	tm=0.5f*(t1+t2);
  } else {
	tm = (v1 - t1*(v2-v1)/(t2-t1) - Org.z) / (Ray.z - (v2-v1)/(t2-t1));
    if((tm<t1) || (tm>t2)) tm=0.5f*(t1+t2);
  }
  if(level<5){
    double t1n=frec(t1, tm, Org, Ray, level+1);
    double t2n=frec(tm, t2, Org, Ray, level+1);
	if(t1n<0.0){
	  if(t2n<0.0) return -1.0; else return t2n;
	} else {
	  if(t2n<0.0) return t1n; else return min(t1n,t2n);
	}
  } else {
	return tm;
  }
}

double Function::ChkE(int top, float t1, float t2, double v1, double v2, vector Org, vector Ray, int level)
{
  float t0=0.5f*(t1+t2); float d0=t2-t1; double a;
  vector P1  = Org+ t1          *Ray;
  vector P2  = Org+ t2          *Ray;
  vector P00 = Org+ t0          *Ray; double v00=PPoint(P00.x,P00.y).z;
  vector P01 = Org+(t0-0.01f*d0)*Ray; double v01=PPoint(P01.x,P01.y).z;
  vector P02 = Org+(t0+0.01f*d0)*Ray; double v02=PPoint(P02.x,P02.y).z;
  if(((P1.z < v1) && (P00.z > v00)) || ((P1.z > v1) && (P00.z < v00))) return frec(t1, t0, Org, Ray, 0);
  if(((P2.z < v2) && (P00.z > v00)) || ((P2.z > v2) && (P00.z < v00))) return frec(t0, t2, Org, Ray, 0);
  if((fabs(P00.z-v00)/(max(fabs(v00-v1),fabs(v00-v2))/Betrag(P2-P1))<5.0) && (level<20)){
	if(top? (v01<v02) : (v01>v02)){
	  a=ChkE(top, t0, t2, v00, v2, Org, Ray, level+1);
    } else {
	  a=ChkE(top, t1, t0, v1, v00, Org, Ray, level+1);
    }
	if(a>-1.0) return a;
  }
  return -1.0;
}

float Function::Intersect(vector Org, vector Ray)
{
  Ray=Ray*Tb-vector(Tb.M41, Tb.M42, Tb.M43);
  Org=Org*Tb;
  float t, t1, t2=-1.0f;
  t1 = IntFac(Org, Ray, 0);
  t  = IntFac(Org, Ray, 1); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFac(Org, Ray, 2); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFac(Org, Ray, 3); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFac(Org, Ray, 4); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  t  = IntFac(Org, Ray, 5); if(t>0.0f) if(t1<=0.0f) t1=t; else t2=t;
  if((t1>-1.0f) || (t2>-1.0f)){
    if((t1>-1.0f) && (t2<0.0f)) t2=0.01f;
    if((t2>-1.0f) && (t1<0.0f)) t1=0.01f;
	if(t2<t1) swap(t1,t2);
	double a; float tJS = JS/Betrag(Ray);
    vector P1 = Org+t1*Ray; double v1=PPoint(P1.x,P1.y).z, v2=v1, v3=42.0;
    int b = (P1.z > v1);
	while((b? (P1.z > v1) : (P1.z < v1)) && (t1<t2)){
	  t1+=tJS; P1 = Org+t1*Ray; v1=PPoint(P1.x,P1.y).z;
	  if(v3!=42.0){
	    if((v1<v2) && (v2>v3)){
		  a=ChkE(1, t1-2.0f*tJS, t1, v3, v1, Org, Ray, 0);
		  if((a>-1.0) && (a<=t2)) goto rt;
		}
		if((v1>v2) && (v2<v3)){
		  a=ChkE(0, t1-2.0f*tJS, t1, v3, v1, Org, Ray, 0);
		  if((a>-1.0) && (a<=t2)) goto rt;
		}
	  }
  	  v3=v2; v2=v1;
    }
	if(t1<t2){
	  a=frec(t1-tJS, t1, Org, Ray, 0);
	} else {
	  a=frec(t1-tJS, t2, Org, Ray, 0);
	}
rt: CInt.t=(float)a;
	CInt.IP = Org+CInt.t*Ray; CInt.u = CInt.IP.x; CInt.v = CInt.IP.y;
    vector Dx=vector(CInt.IP.x+0.01f, CInt.IP.y, PPoint(CInt.IP.x+0.01f, CInt.IP.y).z) - vector(CInt.IP.x-0.01f, CInt.IP.y, PPoint(CInt.IP.x-0.01f, CInt.IP.y).z);
    vector Dz=vector(CInt.IP.x, CInt.IP.y+0.01f, PPoint(CInt.IP.x, CInt.IP.y+0.01f).z) - vector(CInt.IP.x, CInt.IP.y-0.01f, PPoint(CInt.IP.x, CInt.IP.y-0.01f).z);
	if(txc){
	  CInt.x = bilinear((CInt.u-Min.x)/(Max.x-Min.x), (CInt.v-Min.y)/(Max.y-Min.y), txc[3].x, txc[2].x, txc[1].x, txc[0].x);
	  CInt.y = bilinear((CInt.u-Min.x)/(Max.x-Min.x), (CInt.v-Min.y)/(Max.y-Min.y), txc[3].y, txc[2].y, txc[1].y, txc[0].y);
	} else {
	  CInt.x = (CInt.u-Min.x)/(Max.x-Min.x);
      CInt.y = (CInt.v-Min.y)/(Max.y-Min.y);
	}
    CInt.IP=CInt.IP*TM; CInt.N=-unit((Dz*Dx)*NM); if(flags & SURF_INVERSE) CInt.N=-CInt.N;
	return CInt.t;
  }
  return -1.0f;
}*/

// Extents ---------------------------------------------------------

void Function::WorldExtents(vector & min, vector & max)
{
  CStretch(min, max, Min, Max);
}

// Points ----------------------------------------------------------

vector Function::RndPnt (void){ return vector(0.0f, 0.0f, 0.0f); }

vector Function::PPoint (float u, float v){ Var_X = u; Var_Y = v; return vector(u, v, (float)CalcExpr(func)); }

// Normals ---------------------------------------------------------

vector Function::PNormal(float u, float v){
  Var_X=u-0.01f; Var_Y=v; float z1=(float)CalcExpr(func);
  Var_X=u+0.01f; Var_Y=v; float z2=(float)CalcExpr(func);
  Var_X=u; Var_Y=v-0.01f; float z3=(float)CalcExpr(func);
  Var_X=u; Var_Y=v+0.01f; float z4=(float)CalcExpr(func);
  if(flags & SURF_INVERSE){
    return -unit((vector(u+0.01f, v, z2)-vector(u-0.01f, v, z1)) * (vector(u, v+0.01f, z4)-vector(u, v-0.01f, z3)));
  } else {
    return  unit((vector(u+0.01f, v, z2)-vector(u-0.01f, v, z1)) * (vector(u, v+0.01f, z4)-vector(u, v-0.01f, z3)));
  }
}

vector Function::VNormal(vector p) {
  return unit(p);
}
