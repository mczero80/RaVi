
#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

#include "RaVi.h"
#include "RaVi_Tree.h"

#ifdef _DEBUG
  #define new DEBUG_NEW
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

extern long int sSMem;
extern long int sSObj;

bool IsParallelogram(vector p1, vector p2, vector p3, vector p4)
{
  vector p3p = p2+p4-p1;
  if(distance(p3,p3p) < (0.01f*distance(p3,p1))) return true;
  return false;
}

PrLP Prim2RV(PrimPtr p)
{
  PrLP o, n = NULL;
  while(p){
    if(p->idnt==POLYMESH){
      PolyMesh *t = (PolyMesh *)p;
      if(((t->pmflags & SURF_ROUNDED) || (t->pmflags & SURF_GROUPED)) && (!t->nrm)){
        if(!t->mesh.snm) t->CalcSurfaceNormals();
        t->CalcSmoothingNormals();
	  }
	  int fi,i,j,k;
	  vector base;
      RV_TriPara *rvt;
	  for(fi=0,i=0; i<(t->nf); i++){
		k=(t->pnv)? t->pnv[i] : 3;
        base=t->pts[t->pfv[fi]]*t->TM;
		if((k==4) && IsParallelogram(base, t->pts[t->pfv[fi+1]]*t->TM, t->pts[t->pfv[fi+2]]*t->TM, t->pts[t->pfv[fi+3]]*t->TM)){
		  rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL;
		  if(t->nm > 1) rvt->Mat = t->mtl[t->mat[i]]; else rvt->Mat = t->Mat;
		  rvt->flags=((t->pmflags & SURF_DOUBLESIDED)? 0 : RV_SingleSided);
		  if(t->pmflags & SURF_INVISIBLE) rvt->flags |= RV_Invisible;
          o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
		  rvt->edg1 = t->pts[t->pfv[fi+3]]*t->TM - base;
		  rvt->edg2 = t->pts[t->pfv[fi+1]]*t->TM - base;
          rvt->base = base; InitTP(rvt);
	      sSMem += sizeof(RV_TriPara); sSObj++;
		  if(t->nrm || t->txc){
		    rvt->sinfo = new RV_TrPShd; sSMem += sizeof(RV_TrPShd);
			if(t->nrm){
			  rvt->sinfo->nrm    = new vector[4]; sSMem += sizeof(vector[4]);
			  rvt->sinfo->nrm[0] = t->nrm[(t->pft? t->pft[fi  ] : t->pfv[fi  ])];
			  rvt->sinfo->nrm[1] = t->nrm[(t->pft? t->pft[fi+3] : t->pfv[fi+3])];
			  rvt->sinfo->nrm[2] = t->nrm[(t->pft? t->pft[fi+1] : t->pfv[fi+1])];
			  rvt->sinfo->nrm[3] = t->nrm[(t->pft? t->pft[fi+2] : t->pfv[fi+2])];
			} else rvt->sinfo->nrm = NULL;
			if(t->txc){
			  rvt->sinfo->txc    = new vector[4]; sSMem += sizeof(vector[4]);
			  rvt->sinfo->txc[0] = t->txc[(t->pft? t->pft[fi  ] : t->pfv[fi  ])];
			  rvt->sinfo->txc[1] = t->txc[(t->pft? t->pft[fi+3] : t->pfv[fi+3])];
			  rvt->sinfo->txc[2] = t->txc[(t->pft? t->pft[fi+1] : t->pfv[fi+1])];
			  rvt->sinfo->txc[3] = t->txc[(t->pft? t->pft[fi+2] : t->pfv[fi+2])];
			} else rvt->sinfo->txc = NULL;
		  }
		} else {
		  for(j=1; j<(k-1); j++){
		    rvt = new RV_TriPara; rvt->type=0; rvt->sinfo=NULL;
		    if(t->nm > 1) rvt->Mat = t->mtl[t->mat[i]]; else rvt->Mat = t->Mat;
			rvt->flags=((t->pmflags & SURF_DOUBLESIDED)? 0 : RV_SingleSided);
            o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
		    rvt->edg1 = t->pts[t->pfv[fi+j+1]]*t->TM - base;
		    rvt->edg2 = t->pts[t->pfv[fi+j  ]]*t->TM - base;
            rvt->base = base; InitTP(rvt);
	        sSMem += sizeof(RV_TriPara); sSObj++;
			if(t->nrm || t->txc){
			  rvt->sinfo = new RV_TrPShd; sSMem += sizeof(RV_TrPShd);
			  if(t->nrm){
			    rvt->sinfo->nrm    = new vector[3]; sSMem += sizeof(vector[3]);
			    rvt->sinfo->nrm[0] = t->nrm[(t->pfn? t->pfn[fi    ] : t->pfv[fi    ])];
			    rvt->sinfo->nrm[1] = t->nrm[(t->pfn? t->pfn[fi+j+1] : t->pfv[fi+j+1])];
			    rvt->sinfo->nrm[2] = t->nrm[(t->pfn? t->pfn[fi+j  ] : t->pfv[fi+j  ])];
			  } else rvt->sinfo->nrm = NULL;
			  if(t->txc){
			    rvt->sinfo->txc    = new vector[3]; sSMem += sizeof(vector[3]);
			    rvt->sinfo->txc[0] = t->txc[(t->pft? t->pft[fi    ] : t->pfv[fi    ])];
			    rvt->sinfo->txc[1] = t->txc[(t->pft? t->pft[fi+j+1] : t->pfv[fi+j+1])];
			    rvt->sinfo->txc[2] = t->txc[(t->pft? t->pft[fi+j  ] : t->pfv[fi+j  ])];
			  } else rvt->sinfo->txc = NULL;
			}
		  }
		}
		fi+=k;
	  }
	} else if(p->idnt==SPHERE){
      Sphere *s = (Sphere *)p;
	  // test if sphere is scaled uniformly
	  float sx = Betrag(vector(1.0f, 0.0f, 0.0f) * s->TM - vector(s->TM.M41, s->TM.M42, s->TM.M43));
	  float sy = Betrag(vector(0.0f, 1.0f, 0.0f) * s->TM - vector(s->TM.M41, s->TM.M42, s->TM.M43));
	  float sz = Betrag(vector(0.0f, 0.0f, 1.0f) * s->TM - vector(s->TM.M41, s->TM.M42, s->TM.M43));
 	  float sa = 0.001f*(sx+sy+sz);
	  // if not, create Ellipsoid primitive
      if((fabs(sx-sy)>sa) || (fabs(sy-sz)>sa) || (fabs(sz-sx)>sa)){
        RV_Ellipsoid *rvs;
	    rvs = new RV_Ellipsoid; rvs->Mat=s->Mat; rvs->vangle=s->vangle; rvs->ustart=s->ustart; rvs->ustop=s->ustop;
	    rvs->flags=s->flags; rvs->Tb = s->Tb; rvs->sinfo = new RV_GenShd; rvs->sinfo->NM = s->NM; rvs->sinfo->txc = s->txc;
        o = new(PrLst); o->Prim = rvs; o->next=n; n=o;
	    sSMem += sizeof(RV_Ellipsoid)+sizeof(RV_GenShd); sSObj++;
	  } else {
	    if(s->vangle){
          RV_ComplexSphere *rvs;
	      rvs = new RV_ComplexSphere; rvs->Mat=s->Mat; rvs->vangle=s->vangle; rvs->ustart=s->ustart; rvs->ustop=s->ustop;
	      rvs->r2 = sqr(Betrag(vector(1.0f, 0.0f, 0.0f)*(s->TM)-vector(s->TM.M41, s->TM.M42, s->TM.M43)));
	      rvs->position=vector(s->TM.M41, s->TM.M42, s->TM.M43); rvs->flags=s->flags; rvs->Tb = s->Tb;
          #ifdef WINOSI
		  rvs->sinfo = new RV_GenShd; sSMem += sizeof(RV_GenShd); rvs->sinfo->NM = s->TM; rvs->sinfo->txc = s->txc;
          #else
		  if(s->txc){ rvs->sinfo = new RV_GenShd; sSMem += sizeof(RV_GenShd); rvs->sinfo->txc = s->txc; }
          #endif
          o = new(PrLst); o->Prim = rvs; o->next=n; n=o;
	      sSMem += sizeof(RV_ComplexSphere); sSObj++;
		} else {
          RV_SimpleSphere *rvs;
	      rvs = new RV_SimpleSphere; rvs->Mat=s->Mat;
	      rvs->r2 = sqr(Betrag(vector(1.0f, 0.0f, 0.0f)*(s->TM)-vector(s->TM.M41, s->TM.M42, s->TM.M43)));
	      rvs->position=vector(s->TM.M41, s->TM.M42, s->TM.M43); rvs->flags=s->flags;
          o = new(PrLst); o->Prim = rvs; o->next=n; n=o;
	      sSMem += sizeof(RV_SimpleSphere); sSObj++;
		}
	  }
	} else if(p->idnt==CYLINDER){
      Cylinder *c = (Cylinder *)p;
      RV_Cylinder *rvc;
	  rvc = new RV_Cylinder; rvc->Mat=c->Mat; rvc->flags=c->flags;
	  rvc->sinfo = new RV_GenShd; rvc->sinfo->NM = c->NM; rvc->sinfo->txc = c->txc;
	  rvc->Tb = c->Tb; rvc->angle = c->angle; rvc->r_top = 1.0f-c->r_top;
      o = new(PrLst); o->Prim = rvc; o->next=n; n=o;
	  sSMem += sizeof(RV_Cylinder)+sizeof(RV_GenShd); sSObj++;
	} else if(p->idnt==TORUS){
      Torus *t = (Torus *)p;
      RV_Torus *rvt;
	  rvt = new RV_Torus; rvt->Mat=t->Mat; rvt->flags=t->flags;
	  rvt->sinfo = new RV_GenShd; rvt->sinfo->NM = t->NM; rvt->sinfo->txc = t->txc;
	  rvt->Tb = t->Tb; rvt->vangle = t->vangle; rvt->ustart = t->ustart;  rvt->ustop = t->ustop; rvt->R1 = t->R1; rvt->R2 = t->R2;
      o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
	  sSMem += sizeof(RV_Torus)+sizeof(RV_GenShd); sSObj++;
	} else if(p->idnt==CUBE){
      Cube *c = (Cube *)p;
	  vector base;
      RV_TriPara *rvt;
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(-0.5f, -0.5f, -0.5f)*c->TM;
      rvt->edg1 = vector(+0.5f, -0.5f, -0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(-0.5f, +0.5f, -0.5f)*c->TM - rvt->base;
      InitTP(rvt);
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(-0.5f, -0.5f, -0.5f)*c->TM;
      rvt->edg1 = vector(-0.5f, +0.5f, -0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(-0.5f, -0.5f, +0.5f)*c->TM - rvt->base;
      InitTP(rvt);
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(-0.5f, -0.5f, -0.5f)*c->TM;
      rvt->edg1 = vector(-0.5f, -0.5f, +0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(+0.5f, -0.5f, -0.5f)*c->TM - rvt->base;
      InitTP(rvt);
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(+0.5f, +0.5f, +0.5f)*c->TM;
      rvt->edg1 = vector(-0.5f, +0.5f, +0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(+0.5f, -0.5f, +0.5f)*c->TM - rvt->base;
      InitTP(rvt);
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(+0.5f, +0.5f, +0.5f)*c->TM;
      rvt->edg1 = vector(+0.5f, -0.5f, +0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(+0.5f, +0.5f, -0.5f)*c->TM - rvt->base;
      InitTP(rvt);
      rvt = new RV_TriPara; rvt->type=1; rvt->sinfo=NULL; rvt->Mat=c->Mat; o = new(PrLst); o->Prim = rvt; o->next=n; n=o;
      rvt->base = vector(+0.5f, +0.5f, +0.5f)*c->TM;
      rvt->edg1 = vector(+0.5f, +0.5f, -0.5f)*c->TM - rvt->base;
      rvt->edg2 = vector(-0.5f, +0.5f, +0.5f)*c->TM - rvt->base;
      InitTP(rvt);
	  sSMem += 6*sizeof(RV_TriPara); sSObj+=6;
	} else if(p->idnt==RING){
      Ring *r = (Ring *)p;
	  if(r->angle){
        RV_ComplexRing *rvr;
	    rvr = new RV_ComplexRing; rvr->Mat=r->Mat; rvr->angle=r->angle;
	    rvr->r_o = sqr(Betrag(vector(   1.0f, 0.0f, 0.0f)*(r->TM)-vector(r->TM.M41, r->TM.M42, r->TM.M43)));
	    rvr->r_i = sqr(Betrag(vector(r->r_i , 0.0f, 0.0f)*(r->TM)-vector(r->TM.M41, r->TM.M42, r->TM.M43)));
	    rvr->center = vector(r->TM.M41, r->TM.M42, r->TM.M43); rvr->Tb=r->Tb;
	    rvr->N      = unit(vector(0.0f, 0.0f, 1.0f)*r->NM);
        if(r->txc){ rvr->sinfo = new RV_GenShd; sSMem += sizeof(RV_GenShd); rvr->sinfo->txc = r->txc; }
        o = new(PrLst); o->Prim = rvr; o->next=n; n=o;
	    sSMem += sizeof(RV_ComplexRing); sSObj++;
	  } else {
        RV_SimpleRing *rvr;
	    rvr = new RV_SimpleRing; rvr->Mat=r->Mat;
	    rvr->r_o = sqr(Betrag(vector(   1.0f, 0.0f, 0.0f)*(r->TM)-vector(r->TM.M41, r->TM.M42, r->TM.M43)));
	    rvr->r_i = sqr(Betrag(vector(r->r_i , 0.0f, 0.0f)*(r->TM)-vector(r->TM.M41, r->TM.M42, r->TM.M43)));
	    rvr->center = vector(r->TM.M41, r->TM.M42, r->TM.M43);
	    rvr->N      = unit(vector(0.0f, 0.0f, 1.0f)*r->NM);
        o = new(PrLst); o->Prim = rvr; o->next=n; n=o;
	    sSMem += sizeof(RV_SimpleRing); sSObj++;
	  }
	} else if(p->idnt==PARABOLOID){
      Paraboloid *c = (Paraboloid *)p;
      RV_Paraboloid *rvp;
	  rvp = new RV_Paraboloid; rvp->Mat=c->Mat; rvp->sinfo = new RV_GenShd; rvp->sinfo->NM = c->NM; rvp->sinfo->txc = c->txc;
	  rvp->Tb = c->Tb; rvp->angle = c->angle; rvp->height = c->height;
      o = new(PrLst); o->Prim = rvp; o->next=n; n=o;
	  sSMem += sizeof(RV_Paraboloid)+sizeof(RV_GenShd); sSObj++;
	}
    p=p->succ;
  }
  return n;
}

BSPplane *VM2RaVi(PrimPtr p)
{
  // convert primitives
  PrLP n = Prim2RV(p);
  // collect lights
  RV_Lights=NULL; PrLP s=n;
  while(s){
    if(s->Prim->Mat->Emission.R > 0.0f){
	  LitPtr l = new RV_Light;
	  l->v     = s->Prim->Mat->Emission.R;
	  l->next  = RV_Lights; RV_Lights=l;
	  l->Prim  = s->Prim;
      if(s->Prim->type == 2){
		l->p = ((RV_SimpleSphere *)s->Prim)->position;
      } else if (s->Prim->type < 2){
		l->p = ((RV_TriPara *)s->Prim)->base + 0.5f*((RV_TriPara *)s->Prim)->edg1 + 0.5f*((RV_TriPara *)s->Prim)->edg2;
	  }
	}
	s=s->next;
  }
  // set up BSP-tree
  return RV_CreateTree(n);
}
