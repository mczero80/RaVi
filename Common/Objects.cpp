// Virtual Modeller V0.5GL  for MS Visual C++ V4.2   09.06.1999  Michael Granz

#include "stdafx.h"
#include "general.h"
#include "Primitives.h"

#define MAXPVERT 64

int PolyMesh::ParseToken(char *a, CFile *file) { return 1; }

void PolyMesh::CalcSurfaceNormals()
{
  mesh.snm = new vector[nf];
  for(int ap=0,i=0; i<nf; i++){
    mesh.snm[i] = (pts[pfv[ap+1]]-pts[pfv[ap]]) * (pts[pfv[ap+2]]-pts[pfv[ap]]);
    if(Betrag(mesh.snm[i]) < (Betrag(pts[pfv[ap+1]]-pts[pfv[ap]]) * Betrag(pts[pfv[ap+2]]-pts[pfv[ap]]) * 0.001f)){
      mesh.snm[i] = vector(0.0F, 1.0F, 0.0F);
    } else {
      mesh.snm[i] = -unit(mesh.snm[i]);
    }
    ap+=(pnv? pnv[i] : 3);
  }
}

void PolyMesh::CalcSmoothingNormals()
{
  int i,j,k;
  if(pmflags & SURF_GROUPED){
    // calculate groupwise smoothed-vertex-normals
    VectorSet VB;
	if(pnv){
      vector No[MAXPVERT]; BOOL match;
      int ap1,ap2,l,m,c=0; for(i=0; i<nf; i++) c+=pnv[i];
	  pfn = new ushort[c];
	  for(ap1=0,i=0,j=0; i<nf; i++){
        for(l=0; l<MAXPVERT; l++) No[l]=mesh.snm[i];
        if(grp[i]){
          for(ap2=0,k=0; k<nf; k++){
            if((grp[k] == grp[i]) && (k != i)){
              for(l=0; l<pnv[i]; l++){
                match=FALSE;
                for(m=0; m<pnv[k]; m++){
                  if(pfv[ap1+l]==pfv[ap2+m]) match=TRUE;
                }
                if(match) No[l]=No[l]+mesh.snm[k];
              }
            }
            ap2+=pnv[k];
          }
        }
        // search/insert normals in buffer and receive new index
        for(l=0; l<pnv[i]; l++){
          pfn[j+l] = VB.Add((Betrag(No[l]) > 0.001)? unit(No[l]) : vector(0.0F, 1.0F, 0.0F), 0.0001f);
        }
        j+=pnv[i]; ap1+=pnv[i];
      }
    } else {
      int acs=0, ncs=999;
      nrm = new vector[nv]; pfn = new ushort[3*nf];
      for(i=0,j=0; i<nf; i++,j+=3){
        if(grp[i] == acs){
          pfn[j] = pfn[j+1] = pfn[j+2] = VB.Add(mesh.snm[i], 0.0001f);
        } else if((grp[i]>acs) && (grp[i]<ncs)) ncs=grp[i];
      }
      while(ncs<999){
        acs=ncs; ncs=999;
        for(i=0; i<nv; i++) nrm[i] = vector(0.0F, 0.0F, 0.0F);
        for(i=0,j=0; i<nf; i++,j+=3){
          if(grp[i] == acs){
            nrm[pfv[j  ]] += mesh.snm[i];
            nrm[pfv[j+1]] += mesh.snm[i];
            nrm[pfv[j+2]] += mesh.snm[i];
          } else if((grp[i]>acs) && (grp[i]<ncs)) ncs=grp[i];
        }
        for(i=0; i<nv; i++) nrm[i] = (Betrag(nrm[i]) > 0.001)? unit(nrm[i]) : vector(0.0F, 1.0F, 0.0F);
        for(i=0,j=0; i<nf; i++,j+=3){
          if(grp[i] == acs){
            pfn[j  ] = VB.Add(nrm[pfv[j  ]], 0.0001f);
            pfn[j+1] = VB.Add(nrm[pfv[j+1]], 0.0001f);
            pfn[j+2] = VB.Add(nrm[pfv[j+2]], 0.0001f);
          }
        }
      }
      delete nrm;
    }
    VB.Move2Array(&nrm);
  } else if(pmflags & SURF_ROUNDED){
    // calculate rounded-vertex-normals
    ushort nopf; nrm = new vector[nv];
    for(i=0; i<nv; i++) nrm[i]=vector(0.0f, 0.0f, 0.0f);
    for(int ap=0,j=0; j<nf; j++){
      nopf=(pnv? pnv[j] : 3);
      for(k=0; k<nopf; k++) nrm[pfv[ap+k]] += mesh.snm[j];
      ap+=nopf;
    }
    for(i=0; i<nv; i++) nrm[i] = (Betrag(nrm[i]) > 0.001)? unit(nrm[i]) : vector(0.0F, 1.0F, 0.0F);
  }
}

/*float PolyMesh::Intersect(vector Org, vector Ray)
{
  Ray=Ray*Tb-vector(Tb.M41, Tb.M42, Tb.M43);
  Org=Org*Tb;

  int i,j,n,ii,ne;
  float m,k,t,tmin=-1.0f;
  vector iP;

  if(!pre){
	float dxy,dyz,dxz; np=0; double de[6];
    for(i=0; i<nf; i++) {
      ne  = pnv? pnv[i] : 3;
	  np += ne-2;
	}
	pre = new IntPre[np]; int pp=0;
    for(ii=0,i=0; i<nf; i++) {
      ne = pnv? pnv[i] : 3;
      for(j=0; j<(ne-2); j++){
		pre[pp].fcnr = i;
		pre[pp].vidx = j;
		pre[pp].fidx = ii;
        pre[pp].base = pts[pfv[ii]];
	    pre[pp].edg1 = pts[pfv[ii+j+1]]-pre[pp].base;
	    pre[pp].edg2 = pts[pfv[ii+j+2]]-pre[pp].base;
        pre[pp].N = unit(pre[pp].edg1 * pre[pp].edg2); // calculate surface normal-vector
        dxy = pre[pp].edg2.y*pre[pp].edg1.x - pre[pp].edg2.x*pre[pp].edg1.y;
        dyz = pre[pp].edg2.z*pre[pp].edg1.y - pre[pp].edg2.y*pre[pp].edg1.z;
        dxz = pre[pp].edg2.z*pre[pp].edg1.x - pre[pp].edg2.x*pre[pp].edg1.z;
        de[0] = fabs(pre[pp].edg1.x *  dxy);
        de[1] = fabs(pre[pp].edg1.y *  dyz);
        de[2] = fabs(pre[pp].edg1.x *  dxz);
        de[3] = fabs(pre[pp].edg1.y * -dxy);
        de[4] = fabs(pre[pp].edg1.z * -dyz);
        de[5] = fabs(pre[pp].edg1.z * -dxz);
        pre[pp].algn=0; for(n=1; n<6; n++) if(de[n]>de[pre[pp].algn]) pre[pp].algn=n;
        switch(pre[pp].algn){
          case 0: pre[pp].deno =  dxy; break;
          case 1: pre[pp].deno =  dyz; break;
          case 2: pre[pp].deno =  dxz; break;
          case 3: pre[pp].deno = -dxy; break;
          case 4: pre[pp].deno = -dyz; break;
          case 5: pre[pp].deno = -dxz; break;
        }
		pp++;
      }
      ii += ne;
    }
  }

  for(i=0; i<np; i++) {
    if((Ray%pre[i].N)==0.0f) t=-1.0f; else {
      t = ((pre[i].base-Org)%pre[i].N)/(Ray%pre[i].N);
      if(t<=0.0f) t=-1.0f; else {
        iP = Org+t*Ray;
        switch(pre[i].algn){
          case 0: m = ((iP.y - pre[i].base.y)   *pre[i].edg1.x - (iP.x - pre[i].base.x)*pre[i].edg1.y) / pre[i].deno;
                  k =  (iP.x - pre[i].base.x - m*pre[i].edg2.x) / pre[i].edg1.x;
                  break;
          case 1: m = ((iP.z - pre[i].base.z)   *pre[i].edg1.y - (iP.y - pre[i].base.y)*pre[i].edg1.z) / pre[i].deno;
                  k =  (iP.y - pre[i].base.y - m*pre[i].edg2.y) / pre[i].edg1.y;
                  break;
          case 2: m = ((iP.z - pre[i].base.z)   *pre[i].edg1.x - (iP.x - pre[i].base.x)*pre[i].edg1.z) / pre[i].deno;
                  k =  (iP.x - pre[i].base.x - m*pre[i].edg2.x) / pre[i].edg1.x;
                  break;
          case 3: m = ((iP.x - pre[i].base.x)   *pre[i].edg1.y - (iP.y - pre[i].base.y)*pre[i].edg1.x) / pre[i].deno;
                  k =  (iP.y - pre[i].base.y - m*pre[i].edg2.y) / pre[i].edg1.y;
                  break;
          case 4: m = ((iP.y - pre[i].base.y)   *pre[i].edg1.z - (iP.z - pre[i].base.z)*pre[i].edg1.y) / pre[i].deno;
                  k =  (iP.z - pre[i].base.z - m*pre[i].edg2.z) / pre[i].edg1.z;
                  break;
          case 5: m = ((iP.x - pre[i].base.x)   *pre[i].edg1.z - (iP.z - pre[i].base.z)*pre[i].edg1.x) / pre[i].deno;
                  k =  (iP.z - pre[i].base.z - m*pre[i].edg2.z) / pre[i].edg1.z;
                  break;
        }
        if(!((k>=0.0f) && (k<=1.0f) && (m>=0.0f) && (m<=1.0f) && ((m+k)<=1.0f))) t=-1.0f;
	  }
    }

    if((t>=0.0f) && ((tmin<0.0f) || (t<tmin))){
	  tmin=t;
      CInt.t=t; CInt.i=pre[i].fcnr; CInt.IP=iP*TM; CInt.N=-pre[i].N*NM; CInt.u=k; CInt.v=m;
	  if(nrm){
		if(pfn){
	      CInt.N = unit(nrm[pfn[pre[i].fidx]] + k*(nrm[pfn[pre[i].fidx+pre[i].vidx+1]] - nrm[pfn[pre[i].fidx]]) + m*(nrm[pfn[pre[i].fidx+pre[i].vidx+2]] - nrm[pfn[pre[i].fidx]]));
		} else {
	      CInt.N = unit(nrm[pfv[pre[i].fidx]] + k*(nrm[pfv[pre[i].fidx+pre[i].vidx+1]] - nrm[pfv[pre[i].fidx]]) + m*(nrm[pfv[pre[i].fidx+pre[i].vidx+2]] - nrm[pfv[pre[i].fidx]]));
		}
	  }
	  if(txc){
	    CInt.x = txc[pft[pre[i].fidx]].x + k*(txc[pft[pre[i].fidx+pre[i].vidx+1]].x - txc[pft[pre[i].fidx]].x) + m*(txc[pft[pre[i].fidx+pre[i].vidx+2]].x - txc[pft[pre[i].fidx]].x);
	    CInt.y = txc[pft[pre[i].fidx]].y + k*(txc[pft[pre[i].fidx+pre[i].vidx+1]].y - txc[pft[pre[i].fidx]].y) + m*(txc[pft[pre[i].fidx+pre[i].vidx+2]].y - txc[pft[pre[i].fidx]].y);
	  } else {
		CInt.x = k;
		CInt.y = m;
	  }
    }
  }
  
  return tmin;
}*/

// Points ----------------------------------------------------------

vector PolyMesh::RndPnt(void) { return vector(0.0f, 0.0f, 0.0f); }
vector PolyMesh::PPoint(float u, float v){ return vector(0.0f, 0.0f, 0.0f); }

// Normals ---------------------------------------------------------

vector PolyMesh::PNormal(float u, float v){ return vector(0.0f, 0.0f, 1.0f); }
vector PolyMesh::VNormal(vector p){ return vector(0.0f, 0.0f, 1.0f); }

// Extents

void Surface::CStretch(vector & min, vector & max, vector u, vector v)
{
           min= max=                  u   *TM ;
  EStretch(min, max, vector(u.x, u.y, v.z)*TM);
  EStretch(min, max, vector(u.x, v.y, u.z)*TM);
  EStretch(min, max, vector(u.x, v.y, v.z)*TM);
  EStretch(min, max, vector(v.x, u.y, u.z)*TM);
  EStretch(min, max, vector(v.x, u.y, v.z)*TM);
  EStretch(min, max, vector(v.x, v.y, u.z)*TM);
  EStretch(min, max,                  v   *TM);
}

void PolyMesh::WorldExtents(vector & min, vector & max)
{
  for(int i=0; i<nv; i++){
    EStretch(min, max, pts[i]*TM);
  }
}

// Transformation ---------------------------------------------------------

void Surface::Translate(vector p)
{
  matrix H;
  H.M11=  1.0f; H.M12=  0.0f; H.M13=  0.0f;
  H.M21=  0.0f; H.M22=  1.0f; H.M23=  0.0f;
  H.M31=  0.0f; H.M32=  0.0f; H.M33=  1.0f;
  H.M41=  p.x ; H.M42=  p.y ; H.M43=  p.z ; TM = TM * H;
  H.M41= -p.x ; H.M42= -p.y ; H.M43= -p.z ; Tb = H * Tb;
}

void Surface::RotateX(float a)
{
  matrix H;
  H.M11= 1.0f; H.M12=        0.0f  ; H.M13=         0.0f  ;
  H.M21= 0.0f; H.M22= (float)cos(a); H.M23= -(float)sin(a);
  H.M31= 0.0f; H.M32= (float)sin(a); H.M33=  (float)cos(a);
  H.M41= 0.0f; H.M42=        0.0f  ; H.M43=         0.0f  ;
  TM = TM * H;
  H.M22= (float)cos(-a); H.M23= -(float)sin(-a);
  H.M32= (float)sin(-a); H.M33=  (float)cos(-a);
  Tb = H * Tb;
}

void Surface::RotateY(float a)
{
  matrix H;
  H.M11=  (float)cos(a); H.M12= 0.0f; H.M13= (float)sin(a);
  H.M21=         0.0f  ; H.M22= 1.0f; H.M23=        0.0f  ;
  H.M31= -(float)sin(a); H.M32= 0.0f; H.M33= (float)cos(a);
  H.M41=         0.0f  ; H.M42= 0.0f; H.M43=        0.0f  ;
  TM = TM * H;
  H.M11=  (float)cos(-a); H.M13= (float)sin(-a);
  H.M31= -(float)sin(-a); H.M33= (float)cos(-a);
  Tb = H * Tb;
}

void Surface::RotateZ(float a)
{
  matrix H;
  H.M11= (float)cos(a); H.M12=  (float)sin(a); H.M13= 0.0f;
  H.M21=-(float)sin(a); H.M22=  (float)cos(a); H.M23= 0.0f;
  H.M31=        0.0f  ; H.M32=         0.0f  ; H.M33= 1.0f;
  H.M41=        0.0f  ; H.M42=         0.0f  ; H.M43= 0.0f;
  TM = TM * H;
  H.M21= (float)sin(a); H.M12= -(float)sin(a);
  Tb = H * Tb;
}

void Surface::Scale(vector s)
{
  matrix H;
  H.M11= s.x ; H.M12= 0.0f; H.M13= 0.0f;
  H.M21= 0.0f; H.M22= s.y ; H.M23= 0.0f;
  H.M31= 0.0f; H.M32= 0.0f; H.M33= s.z ;
  H.M41= 0.0f; H.M42= 0.0f; H.M43= 0.0f;
  TM = TM * H;
  H.M11= 1.0f/s.x; H.M22= 1.0f/s.y; H.M33= 1.0f/s.z;
  Tb = H * Tb;
}

void Surface::Transform(matrix H)
{
  TM = TM * H;
  Tb = TM.Inverse(); // calculate backtransformation matrix
}
