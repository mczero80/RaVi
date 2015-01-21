
#include "stdafx.h"

#include "general.h"

#include "Materials.h"
#include "RaVi.h"
#include "RaVi_Tree.h"

#ifdef VModel
  #include "..\VModel\TextWin.h"
#endif

vector *PrMax; unsigned long int *maxp;
vector *PrMin; unsigned long int *minp;

int MaxSplitLevel;

void RV_MinMax(RV_PrP p, vector *Min, vector *Max)
{
  if(p->type == 0){
    Min->x = min3( ((RV_TriPara *)p)->base.x, ((RV_TriPara *)p)->base.x + ((RV_TriPara *)p)->edg1.x, ((RV_TriPara *)p)->base.x + ((RV_TriPara *)p)->edg2.x);
    Max->x = max3( ((RV_TriPara *)p)->base.x, ((RV_TriPara *)p)->base.x + ((RV_TriPara *)p)->edg1.x, ((RV_TriPara *)p)->base.x + ((RV_TriPara *)p)->edg2.x);
    Min->y = min3( ((RV_TriPara *)p)->base.y, ((RV_TriPara *)p)->base.y + ((RV_TriPara *)p)->edg1.y, ((RV_TriPara *)p)->base.y + ((RV_TriPara *)p)->edg2.y);
    Max->y = max3( ((RV_TriPara *)p)->base.y, ((RV_TriPara *)p)->base.y + ((RV_TriPara *)p)->edg1.y, ((RV_TriPara *)p)->base.y + ((RV_TriPara *)p)->edg2.y);
    Min->z = min3( ((RV_TriPara *)p)->base.z, ((RV_TriPara *)p)->base.z + ((RV_TriPara *)p)->edg1.z, ((RV_TriPara *)p)->base.z + ((RV_TriPara *)p)->edg2.z);
    Max->z = max3( ((RV_TriPara *)p)->base.z, ((RV_TriPara *)p)->base.z + ((RV_TriPara *)p)->edg1.z, ((RV_TriPara *)p)->base.z + ((RV_TriPara *)p)->edg2.z);
  } else if(p->type == 1){
	*Min= *Max=          ((RV_TriPara *)p)->base;
	EStretch(*Min, *Max, ((RV_TriPara *)p)->base + ((RV_TriPara *)p)->edg1                          );
	EStretch(*Min, *Max, ((RV_TriPara *)p)->base + ((RV_TriPara *)p)->edg1 + ((RV_TriPara *)p)->edg2);
	EStretch(*Min, *Max, ((RV_TriPara *)p)->base                           + ((RV_TriPara *)p)->edg2);
  } else if(p->type == 2){
    *Min = ((RV_SimpleSphere *)p)->position - vector(1.0f, 1.0f, 1.0f)*sqrtf(((RV_SimpleSphere *)p)->r2);
    *Max = ((RV_SimpleSphere *)p)->position + vector(1.0f, 1.0f, 1.0f)*sqrtf(((RV_SimpleSphere *)p)->r2);
  } else if(p->type == 3){
	float rtop=1.0f-((RV_Cylinder *)p)->r_top; matrix TM=((RV_Cylinder *)p)->Tb.Inverse();
	         *Min= *Max= vector(-1.0f,-1.0f,0.0f)*TM;
	EStretch(*Min, *Max, vector( 1.0f,-1.0f,0.0f)*TM);
	EStretch(*Min, *Max, vector( 1.0f, 1.0f,0.0f)*TM);
	EStretch(*Min, *Max, vector(-1.0f, 1.0f,0.0f)*TM);
	EStretch(*Min, *Max, vector(-rtop,-rtop,1.0f)*TM);
	EStretch(*Min, *Max, vector( rtop,-rtop,1.0f)*TM);
	EStretch(*Min, *Max, vector( rtop, rtop,1.0f)*TM);
	EStretch(*Min, *Max, vector(-rtop, rtop,1.0f)*TM);
  } else if(p->type == 4){
	         *Min= *Max= ((RV_Cube *)p)->top ;
	EStretch(*Min, *Max, ((RV_Cube *)p)->top  - ((RV_Cube *)p)->axis1);
	EStretch(*Min, *Max, ((RV_Cube *)p)->top  - ((RV_Cube *)p)->axis2);
	EStretch(*Min, *Max, ((RV_Cube *)p)->top  - ((RV_Cube *)p)->axis3);
	EStretch(*Min, *Max, ((RV_Cube *)p)->base                        );
	EStretch(*Min, *Max, ((RV_Cube *)p)->base + ((RV_Cube *)p)->axis1);
	EStretch(*Min, *Max, ((RV_Cube *)p)->base + ((RV_Cube *)p)->axis2);
	EStretch(*Min, *Max, ((RV_Cube *)p)->base + ((RV_Cube *)p)->axis3);
  } else if(p->type == 5){
	float ro=sqrtf(((RV_SimpleRing *)p)->r_o); matrix TM=BuildMfZ(((RV_SimpleRing *)p)->N);
	         *Min= *Max= vector(-ro,-ro,0.0f)*TM;
	EStretch(*Min, *Max, vector( ro,-ro,0.0f)*TM);
	EStretch(*Min, *Max, vector( ro, ro,0.0f)*TM);
	EStretch(*Min, *Max, vector(-ro, ro,0.0f)*TM);
  } else if(p->type == 6){
	matrix TM=((RV_Torus *)p)->Tb.Inverse(); float r0=((RV_Torus *)p)->R1; float r1=((RV_Torus *)p)->R2;
	         *Min= *Max= vector(-r0-r1,-r0-r1,-r1)*TM;
	EStretch(*Min, *Max, vector( r0+r1,-r0-r1,-r1)*TM);
	EStretch(*Min, *Max, vector( r0+r1, r0+r1,-r1)*TM);
	EStretch(*Min, *Max, vector(-r0-r1, r0+r1,-r1)*TM);
	EStretch(*Min, *Max, vector(-r0-r1,-r0-r1, r1)*TM);
	EStretch(*Min, *Max, vector( r0+r1,-r0-r1, r1)*TM);
	EStretch(*Min, *Max, vector( r0+r1, r0+r1, r1)*TM);
	EStretch(*Min, *Max, vector(-r0-r1, r0+r1, r1)*TM);
  } else if(p->type == 7){
    *Min = ((RV_ComplexSphere *)p)->position - vector(1.0f, 1.0f, 1.0f)*sqrtf(((RV_ComplexSphere *)p)->r2);
    *Max = ((RV_ComplexSphere *)p)->position + vector(1.0f, 1.0f, 1.0f)*sqrtf(((RV_ComplexSphere *)p)->r2);
  } else if(p->type == 8){
	float higt=((RV_Paraboloid *)p)->height; float widt=sqrtf(higt); matrix TM=((RV_Paraboloid *)p)->Tb.Inverse();
	         *Min= *Max= vector(-widt,-widt,0.0f)*TM;
	EStretch(*Min, *Max, vector( widt,-widt,0.0f)*TM);
	EStretch(*Min, *Max, vector( widt, widt,0.0f)*TM);
	EStretch(*Min, *Max, vector(-widt, widt,0.0f)*TM);
	EStretch(*Min, *Max, vector(-widt,-widt,higt)*TM);
	EStretch(*Min, *Max, vector( widt,-widt,higt)*TM);
	EStretch(*Min, *Max, vector( widt, widt,higt)*TM);
	EStretch(*Min, *Max, vector(-widt, widt,higt)*TM);
  } else if(p->type == 9){
	matrix TM=((RV_Ellipsoid *)p)->Tb.Inverse();
	         *Min= *Max= vector(-1.0f,-1.0f,-1.0f)*TM;
	EStretch(*Min, *Max, vector( 1.0f,-1.0f,-1.0f)*TM);
	EStretch(*Min, *Max, vector( 1.0f, 1.0f,-1.0f)*TM);
	EStretch(*Min, *Max, vector(-1.0f, 1.0f,-1.0f)*TM);
	EStretch(*Min, *Max, vector(-1.0f,-1.0f, 1.0f)*TM);
	EStretch(*Min, *Max, vector( 1.0f,-1.0f, 1.0f)*TM);
	EStretch(*Min, *Max, vector( 1.0f, 1.0f, 1.0f)*TM);
	EStretch(*Min, *Max, vector(-1.0f, 1.0f, 1.0f)*TM);
  } else if(p->type == 10){
	float ro=sqrtf(((RV_ComplexRing *)p)->r_o); matrix TM=((RV_ComplexRing *)p)->Tb.Inverse();
	         *Min= *Max= vector(-ro,-ro,0.0f)*TM;
	EStretch(*Min, *Max, vector( ro,-ro,0.0f)*TM);
	EStretch(*Min, *Max, vector( ro, ro,0.0f)*TM);
	EStretch(*Min, *Max, vector(-ro, ro,0.0f)*TM);
  }
}

void RV_SplitNode(BSPplane *Node, int lastaxis, int level)
{
  vector SplitValue,eps; int j,i=0;
  bool found_x,found_y,found_z;
  int  dp_x, dp_y, dp_z;

  PrLP l,r,p = (PrLP)Node->left; int n=0;
  while(p){	n++; p=p->next; }

  #ifdef VModel
    TextWin.Output(1,"  - %d initial primitives:\n", n);
  #endif
  
  if(n>MaxNdCt){

	// initialize indices for sorting
	p = (PrLP)Node->left;
    while(p){
      maxp[i]=p->Prim->rfnum;
      minp[i]=p->Prim->rfnum;
	  p=p->next; i++;
	}

    // check x-axis
	for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMin[minp[j]].x<PrMin[minp[i]].x) swap(minp[j],minp[i]); } }
    for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMax[maxp[j]].x>PrMax[maxp[i]].x) swap(maxp[j],maxp[i]); } }
    eps.x = 0.001f*(PrMax[maxp[0]].x - PrMin[minp[0]].x);

    for(i=1, found_x=false; (i<n) && !found_x; i++){
      if(PrMin[minp[i]].x >= PrMax[maxp[i]].x){
	    float a = max(PrMax[maxp[i]].x, PrMin[minp[i-1]].x);
	    float b = min(PrMin[minp[i]].x, PrMax[maxp[i-1]].x);
	    SplitValue.x = (a+b)/2; found_x=true;
	  }
	}

    int lc_x=0, hc_x=0; p = (PrLP)Node->left;
    while(p){
	  dp_x=0;
	  if(PrMin[p->Prim->rfnum].x < SplitValue.x-eps.x) { lc_x++; dp_x=1; }
	  if(PrMax[p->Prim->rfnum].x > SplitValue.x+eps.x) { hc_x++; dp_x=1; }
      if(!dp_x){
        if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.x : ((RV_TriPara *)(p->Prim))->N.x) > 0.0f) hc_x++; else lc_x++;
	  }
	  p=p->next;
	}

    if((lc_x==0) || (hc_x==0)) found_x=false;
    int olx=lc_x+hc_x-n; int dfx=abs(lc_x-hc_x);

    #ifdef VModel
      if(found_x) TextWin.Output(1,"  - min.x = %7.2f, max.x = %7.2f, SplitValue = %7.2f: left: %d, right: %d, overlap: %d (%d%%), delta: %d (%d%%)\n", PrMin[minp[0]].x,PrMax[maxp[0]].x,SplitValue.x, lc_x, hc_x, olx, 100*olx/n, dfx, 100*dfx/n); else TextWin.Output(1,"  - min.x = %7.2f, max.x = %7.2f, no useful splitting possible\n", PrMin[minp[0]].x,PrMax[maxp[0]].x);
    #endif

    // check y-axis
    for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMin[minp[j]].y<PrMin[minp[i]].y) swap(minp[j],minp[i]); } }
    for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMax[maxp[j]].y>PrMax[maxp[i]].y) swap(maxp[j],maxp[i]); } }
    eps.y = 0.001f*(PrMax[maxp[0]].y - PrMin[minp[0]].y);

    //TextWin.Output(1,"  - "); for(i=0; i<n; i++){ TextWin.Output(1,"%6.2f ", PrMin[minp[i]].y); } TextWin.Output(1,"\n");
    //TextWin.Output(1,"  - "); for(i=0; i<n; i++){ TextWin.Output(1,"%6.2f ", PrMax[maxp[i]].y); } TextWin.Output(1,"\n");

    for(i=1, found_y=false; (i<n) && !found_y; i++){
      if(PrMin[minp[i]].y >= PrMax[maxp[i]].y){
	    float a = max(PrMax[maxp[i]].y, PrMin[minp[i-1]].y);
	    float b = min(PrMin[minp[i]].y, PrMax[maxp[i-1]].y);
	    SplitValue.y = (a+b)/2; found_y=true;
	  }
	}

    int lc_y=0, hc_y=0; p = (PrLP)Node->left;
    while(p){
	  dp_y=0;
	  if(PrMin[p->Prim->rfnum].y < SplitValue.y-eps.y) { lc_y++; dp_y=1; }
	  if(PrMax[p->Prim->rfnum].y > SplitValue.y+eps.y) { hc_y++; dp_y=1; }
      if(!dp_y){
        if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.y : ((RV_TriPara *)(p->Prim))->N.y) > 0.0f) hc_y++; else lc_y++;
	  }
	  p=p->next;
	}

	if((lc_y==0) || (hc_y==0)) found_y=false;
    int oly=lc_y+hc_y-n; int dfy=abs(lc_y-hc_y);

    #ifdef VModel
      if(found_y) TextWin.Output(1,"  - min.y = %7.2f, max.y = %7.2f, SplitValue = %7.2f: left: %d, right: %d, overlap: %d (%d%%), delta: %d (%d%%)\n", PrMin[minp[0]].y,PrMax[maxp[0]].y,SplitValue.y, lc_y, hc_y, oly, 100*oly/n, dfy, 100*dfy/n); else TextWin.Output(1,"  - min.y = %7.2f, max.y = %7.2f, no useful splitting possible\n", PrMin[minp[0]].y,PrMax[maxp[0]].y);
    #endif

    // check z-axis
    for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMin[minp[j]].z<PrMin[minp[i]].z) swap(minp[j],minp[i]); } }
    for(i=0; i<n-1; i++){ for(j=i; j<n; j++){ if(PrMax[maxp[j]].z>PrMax[maxp[i]].z) swap(maxp[j],maxp[i]); } }
    eps.z = 0.001f*(PrMax[maxp[0]].z - PrMin[minp[0]].z);

    for(i=1, found_z=false; (i<n) && !found_z; i++){
      if(PrMin[minp[i]].z >= PrMax[maxp[i]].z){
	    float a = max(PrMax[maxp[i]].z, PrMin[minp[i-1]].z);
	    float b = min(PrMin[minp[i]].z, PrMax[maxp[i-1]].z);
	    SplitValue.z = (a+b)/2; found_z=true;
	  }
	}

    int lc_z=0, hc_z=0; p = (PrLP)Node->left;
    while(p){
	  dp_z=0;
	  if(PrMin[p->Prim->rfnum].z < SplitValue.z-eps.z) { lc_z++; dp_z=1; }
	  if(PrMax[p->Prim->rfnum].z > SplitValue.z+eps.z) { hc_z++; dp_z=1; }
      if(!dp_z){
		if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.z : ((RV_TriPara *)(p->Prim))->N.z) > 0.0f) hc_z++; else lc_z++;
	  }
	  p=p->next;
	}

    if((lc_z==0) || (hc_z==0)) found_z=false;
    int olz=lc_z+hc_z-n; int dfz=abs(lc_z-hc_z);

    #ifdef VModel
      if(found_z) TextWin.Output(1,"  - min.z = %7.2f, max.z = %7.2f, SplitValue = %7.2f: left: %d, right: %d, overlap: %d (%d%%), delta: %d (%d%%)\n", PrMin[minp[0]].z,PrMax[maxp[0]].z,SplitValue.z, lc_z, hc_z, olz, 100*olz/n, dfz, 100*dfz/n); else TextWin.Output(1,"  - min.z = %7.2f, max.z = %7.2f, no useful splitting possible\n", PrMin[minp[0]].z,PrMax[maxp[0]].z);
    #endif

    // find optimal splitting plane
    if((lc_x<MinNdCt) || (hc_x<MinNdCt) || (lc_x>(n-MinNdCt)) || (hc_x>(n-MinNdCt)) ) found_x=false;
    if((lc_y<MinNdCt) || (hc_y<MinNdCt) || (lc_y>(n-MinNdCt)) || (hc_y>(n-MinNdCt)) ) found_y=false;
    if((lc_z<MinNdCt) || (hc_z<MinNdCt) || (lc_z>(n-MinNdCt)) || (hc_z>(n-MinNdCt)) ) found_z=false;

    switch(lastaxis){
      case 1:
        if(found_z && (((OWeight*olz+dfz) < (OWeight*oly+dfy)) || !found_y)){
	      if(found_x && (((OWeight*olx+dfx) < (OWeight*olz+dfz)) || !found_z)) Node->axis=1; else Node->axis = (found_z? 3 : 0);
		} else {
          if(found_x && (((OWeight*olx+dfx) < (OWeight*olx+dfy)) || !found_y)) Node->axis=1; else Node->axis = (found_y? 2 : 0);
		} break;
      case 2:
        if(found_x && (((OWeight*olx+dfx) < (OWeight*olz+dfz)) || !found_z)){
	      if(found_y && (((OWeight*oly+dfy) < (OWeight*olx+dfx)) || !found_x)) Node->axis=2; else Node->axis = (found_y? 1 : 0);
		} else {
          if(found_y && (((OWeight*oly+dfy) < (OWeight*olz+dfz)) || !found_z)) Node->axis=2; else Node->axis = (found_x? 3 : 0);
		} break;
      case 3:
        if(found_y && (((OWeight*oly+dfy) < (OWeight*olx+dfx)) || !found_x)){
	      if(found_z && (((OWeight*olz+dfz) < (OWeight*oly+dfy)) || !found_y)) Node->axis=3; else Node->axis = (found_y? 2 : 0);
		} else {
          if(found_z && (((OWeight*olz+dfz) < (OWeight*olx+dfx)) || !found_x)) Node->axis=3; else Node->axis = (found_x? 1 : 0);
		} break;
	}
  }

  if(Node->axis){
    switch(Node->axis){
      case 1:
	    Node->dval = SplitValue.x;
        l = NULL; r = NULL; p = (PrLP)Node->left;
        while(p){
		  dp_x=0;
          if(PrMax[p->Prim->rfnum].x > SplitValue.x+eps.x){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o; dp_x=1;
		  }
          if(PrMin[p->Prim->rfnum].x < SplitValue.x-eps.x){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o; dp_x=1;
		  }
          if(!dp_x){
		    if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.x : ((RV_TriPara *)(p->Prim))->N.x) > 0.0f){
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o;
			} else {
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o;
			}
		  }
	      p=p->next;
		}
        #ifdef VModel
          TextWin.Output(1,"  - splitting along x-axis done.\n");
        #endif
	    break;
      case 2:
	    Node->dval = SplitValue.y;
        l = NULL; r = NULL; p = (PrLP)Node->left;
        while(p){
		  dp_y=0;
          if(PrMax[p->Prim->rfnum].y > SplitValue.y+eps.y){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o; dp_y=1;
		  }
          if(PrMin[p->Prim->rfnum].y < SplitValue.y-eps.y){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o; dp_y=1;
		  }
          if(!dp_y){
		    if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.y : ((RV_TriPara *)(p->Prim))->N.y) > 0.0f){
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o;
			} else {
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o;
			}
		  }
	      p=p->next;
		}
        #ifdef VModel
          TextWin.Output(1,"  - splitting along y-axis done.\n");
        #endif
	    break;
      case 3:
	    Node->dval = SplitValue.z;
        l = NULL; r = NULL; p = (PrLP)Node->left;
        while(p){
		  dp_z=0;
          if(PrMax[p->Prim->rfnum].z > SplitValue.z+eps.z){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o; dp_z=1;
		  }
          if(PrMin[p->Prim->rfnum].z < SplitValue.z-eps.z){
            PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o; dp_z=1;
		  }
          if(!dp_z){
		    if( ((p->Prim->type==5)? ((RV_SimpleRing *)(p->Prim))->N.z : ((RV_TriPara *)(p->Prim))->N.z) > 0.0f){
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=r; r=o;
			} else {
              PrLP o = new(PrLst); o->Prim = p->Prim; o->next=l; l=o;
			}
		  }
	      p=p->next;
		}
        #ifdef VModel
          TextWin.Output(1,"  - splitting along z-axis done.\n");
        #endif
	    break;
	}
    level++; if(level > MaxSplitLevel) MaxSplitLevel=level;
    Node->left = new(BSPplane); Node->left->axis=0; Node->left->left=(BSPplane *)l; RV_SplitNode(Node->left, Node->axis, level);
    Node->rigt = new(BSPplane); Node->rigt->axis=0; Node->rigt->left=(BSPplane *)r; RV_SplitNode(Node->rigt, Node->axis, level);
  } else {
    #ifdef VModel
      TextWin.Output(1,"  - no splitting done.\n");
    #endif
  }
}

BSPplane *RV_CreateTree(PrLP p)
{
  #ifdef VModel
    TextWin.Output(1,"\n- setting up BSP-tree ...\n");
  #endif
  // initialize flat tree
  BSPplane *BSPtree = new(BSPplane); BSPtree->axis=0; BSPtree->left=(BSPplane *)p;
  // count primitives and store numbers
  int pct = 0; PrLP n=p; while(n){ n->Prim->rfnum=pct++; n=n->next; }
  // create arrays and index pointers for primitive bounds
  PrMax = new vector           [pct]; PrMin = new vector           [pct];
  maxp  = new unsigned long int[pct]; minp  = new unsigned long int[pct];
  // calculate bounds of primitives and scene
  int i=0; n=p; RV_MinMax(n->Prim, &RV_BBMin, &RV_BBMax);
  while(n){
	RV_MinMax(n->Prim, &(PrMin[i]), &(PrMax[i]));
	EStretch(RV_BBMin, RV_BBMax, PrMin[i]);
	EStretch(RV_BBMin, RV_BBMax, PrMax[i]);
	n=n->next; i++;
  }
  // extend scene bounds to avoid numerical errors
  vector eps = 0.01f*(RV_BBMax-RV_BBMin); RV_BBMin=RV_BBMin-eps; RV_BBMax=RV_BBMax+eps;
  // split tree into subtrees (branches)
  MaxSplitLevel=0; RV_SplitNode(BSPtree,3,0);
  // clean up storage for bounds
  if(PrMin) delete PrMin; if(PrMax) delete PrMax;
  if(minp ) delete minp;  if(maxp)  delete maxp ;
  // return pointer to finished BSPtree
  return BSPtree;
}

