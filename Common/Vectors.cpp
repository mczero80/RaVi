// 3D-Vector-Library   für VModel   21.05.1997 Michael Granz

#include "stdafx.h"
#include "vectors.h"

const double OB64 = 1.0/64.0;

inline float sqr(float x){ return x*x; }

void matrix::InitIdentMat()
{
  M11=1.0f; M12=0.0f; M13=0.0f;
  M21=0.0f; M22=1.0f; M23=0.0f;
  M31=0.0f; M32=0.0f; M33=1.0f;
  M41=0.0f; M42=0.0f; M43=0.0f;
}

void matrix::Translate(float x, float y, float z)
{
  M41+=x; M42+=y; M43+=z;
}

void matrix::RotateX(float a)
{
  matrix H;
  H.M11= 1.0f; H.M12=        0.0f  ; H.M13=         0.0f  ;
  H.M21= 0.0f; H.M22= (float)cos(a); H.M23= -(float)sin(a);
  H.M31= 0.0f; H.M32= (float)sin(a); H.M33=  (float)cos(a);
  H.M41= 0.0f; H.M42=        0.0f  ; H.M43=         0.0f  ;
  *this = *this * H;
}

void matrix::RotateY(float a)
{
  matrix H;
  H.M11=  (float)cos(a); H.M12= 0.0f; H.M13= (float)sin(a);
  H.M21=         0.0f  ; H.M22= 1.0f; H.M23=        0.0f  ;
  H.M31= -(float)sin(a); H.M32= 0.0f; H.M33= (float)cos(a);
  H.M41=         0.0f  ; H.M42= 0.0f; H.M43=        0.0f  ;
  *this = *this * H;
}

void matrix::RotateZ(float a)
{
  matrix H;
  H.M11= (float)cos(a); H.M12=  (float)sin(a); H.M13= 0.0f;
  H.M21=-(float)sin(a); H.M22=  (float)cos(a); H.M23= 0.0f;
  H.M31=        0.0f  ; H.M32=         0.0f  ; H.M33= 1.0f;
  H.M41=        0.0f  ; H.M42=         0.0f  ; H.M43= 0.0f;
  *this = *this * H;
}

void matrix::Scale(float x, float y, float z)
{
  M11*=x; M12*=y; M13*=z;
  M21*=x; M22*=y; M23*=z;
  M31*=x; M32*=y; M33*=z;
  M41*=x; M42*=y; M43*=z;
}

void matrix::InitRotMat(const angles & r)
{
  M11=(float)( cos(r.y)*cos(r.z)+sin(r.y)*sin(r.x)*sin(r.z));
  M12=(float)( sin(r.y)*sin(r.x)*cos(r.z)-cos(r.y)*sin(r.z));
  M13=(float)( sin(r.y)*cos(r.x));
  M21=(float)( cos(r.x)*sin(r.z));
  M22=(float)( cos(r.x)*cos(r.z));
  M23=(float)(-sin(r.x));
  M31=(float)( cos(r.y)*sin(r.x)*sin(r.z)-sin(r.y)*cos(r.z));
  M32=(float)( sin(r.y)*sin(r.z)+cos(r.y)*sin(r.x)*cos(r.z));
  M33=(float)( cos(r.y)*cos(r.x));
  M41= 0.0F; M42=0.0F; M43=0.0F;
}

void matrix::InitTransformation(float Sx, float Sy, float Sz, float Rx, float Ry, float Rz, float Tx, float Ty, float Tz)
{
  M11= Sx*(float)((cos(Ry)*cos(Rz)+sin(Ry)*sin(Rx)*sin(Rz)));
  M12= Sx*(float)((sin(Ry)*sin(Rx)*cos(Rz)-cos(Ry)*sin(Rz)));
  M13= Sx*(float)((sin(Ry)*cos(Rx)));
  M21= Sy*(float)((cos(Rx)*sin(Rz)));
  M22= Sy*(float)((cos(Rx)*cos(Rz)));
  M23=-Sy*(float)((sin(Rx)));
  M31= Sz*(float)((cos(Ry)*sin(Rx)*sin(Rz)-sin(Ry)*cos(Rz)));
  M32= Sz*(float)((sin(Ry)*sin(Rz)+cos(Ry)*sin(Rx)*cos(Rz)));
  M33= Sz*(float)((cos(Ry)*cos(Rx)));
  M41= Tx; M42=Ty; M43=Tz;
}


matrix matrix::Inverse() // matrix-inversion
{
  int   i,j,k,m;
  float M[4][8];
  float p,s;

  M[0][0]=M11; M[0][1]=M12; M[0][2]=M13; M[0][3]=0.0F; M[0][4]=1.0F; M[0][5]=0.0F; M[0][6]=0.0F; M[0][7]=0.0F;
  M[1][0]=M21; M[1][1]=M22; M[1][2]=M23; M[1][3]=0.0F; M[1][4]=0.0F; M[1][5]=1.0F; M[1][6]=0.0F; M[1][7]=0.0F;
  M[2][0]=M31; M[2][1]=M32; M[2][2]=M33; M[2][3]=0.0F; M[2][4]=0.0F; M[2][5]=0.0F; M[2][6]=1.0F; M[2][7]=0.0F;
  M[3][0]=M41; M[3][1]=M42; M[3][2]=M43; M[3][3]=1.0F; M[3][4]=0.0F; M[3][5]=0.0F; M[3][6]=0.0F; M[3][7]=1.0F;

  for(k=0;k<4;k++){
    if(k!=3){
      p=(float)fabs(M[k][k]); m=k;
      for(i=k+1;i<4;i++){
        if((float)fabs(M[i][k])>p){ p=(float)fabs(M[i][k]); m=i; }
      }
      for(j=k;j<8;j++){
        s=M[k][j]; M[k][j]=M[m][j]; M[m][j]=s;
      }
    }
    //if(fabs(M[k][k])<1E-8){ printf("Matrix ist singulär\n"); exit(1); }
    s=1/M[k][k];
    for(j=k;j<8;j++) M[k][j]*=s;
    for(i=0;i<4;i++){
      if(i!=k){
        s=-M[i][k];
        for(j=k;j<8;j++) M[i][j]+=s*M[k][j];
      }
    }
  }

  return matrix(M[0][4], M[0][5], M[0][6],
                M[1][4], M[1][5], M[1][6],
                M[2][4], M[2][5], M[2][6],
                M[3][4], M[3][5], M[3][6]);
}


matrix matrix::NTrans() // transformation matrix for normal vectors
{
  int   i,j,k,m;
  float M[4][8];
  float p,s;

  M[0][0]=M11;  M[0][1]=M12;  M[0][2]=M13;  M[0][3]=0.0F; M[0][4]=1.0F; M[0][5]=0.0F; M[0][6]=0.0F; M[0][7]=0.0F;
  M[1][0]=M21;  M[1][1]=M22;  M[1][2]=M23;  M[1][3]=0.0F; M[1][4]=0.0F; M[1][5]=1.0F; M[1][6]=0.0F; M[1][7]=0.0F;
  M[2][0]=M31;  M[2][1]=M32;  M[2][2]=M33;  M[2][3]=0.0F; M[2][4]=0.0F; M[2][5]=0.0F; M[2][6]=1.0F; M[2][7]=0.0F;
  M[3][0]=0.0f; M[3][1]=0.0f; M[3][2]=0.0f; M[3][3]=1.0F; M[3][4]=0.0F; M[3][5]=0.0F; M[3][6]=0.0F; M[3][7]=1.0F;

  for(k=0;k<4;k++){
    if(k!=3){
      p=(float)fabs(M[k][k]); m=k;
      for(i=k+1;i<4;i++){
        if((float)fabs(M[i][k])>p){ p=(float)fabs(M[i][k]); m=i; }
      }
      for(j=k;j<8;j++){
        s=M[k][j]; M[k][j]=M[m][j]; M[m][j]=s;
      }
    }
    s=1/M[k][k];
    for(j=k;j<8;j++) M[k][j]*=s;
    for(i=0;i<4;i++){
      if(i!=k){
        s=-M[i][k];
        for(j=k;j<8;j++) M[i][j]+=s*M[k][j];
      }
    }
  }

  return matrix(M[0][4], M[1][4], M[2][4],
                M[0][5], M[1][5], M[2][5],
                M[0][6], M[1][6], M[2][6],
                  0.0f ,   0.0f ,   0.0f );
}

// build transformation matrix from target Z-vector

matrix BuildMfZ(vector Z)
{
  vector X = XfromZ(Z); vector Y = unit(Z * X); matrix T;
  T.M11=X.x ; T.M12=X.y ; T.M13=X.z ;
  T.M21=Y.x ; T.M22=Y.y ; T.M23=Y.z ;
  T.M31=Z.x ; T.M32=Z.y ; T.M33=Z.z ;
  T.M41=0.0f; T.M42=0.0f; T.M43=0.0f;
  return T;
}

matrix InitAxisRotN(float a, float b, float c, float phi)
{
  float v; if(a<0.0f) phi=-phi;
  
  matrix Rp  = matrix(cosf(phi), -sinf(phi), 0.0f,
                      sinf(phi),  cosf(phi), 0.0f,
                        0.0f,      0.0f,     1.0f,
                        0.0f,      0.0f,     0.0f  );

  if((v = sqrtf(b*b+c*c))<0.0001f) return Rp;

  matrix R1  = matrix(1.0f, 0.0f, 0.0f,
	                  0.0f,  c/v, b/v,
			 		  0.0f, -b/v, c/v,
			 		  0.0f, 0.0f, 0.0f  );
  
  matrix R2  = matrix(v,    0.0f, a,
	                  0.0f, 1.0f, 0.0f,
			 		 -a,    0.0f, v,
			 		  0.0f, 0.0f, 0.0f  );

  matrix R2_ = R2.Inverse();

  matrix R1_ = R1.Inverse();
  
  return R1*R2*Rp*R2_*R1_;
}

matrix InitAxisRot(float x, float y, float z, float a, float b, float c, float phi)
{
  float v; matrix Rp;
  
  matrix T   = matrix(1.0f, 0.0f, 0.0f,
	                  0.0f, 1.0f, 0.0f,
			 		  0.0f, 0.0f, 1.0f,
			 		   -x ,  -y ,  -z   );

  matrix T_  = matrix(1.0f, 0.0f, 0.0f,
	                  0.0f, 1.0f, 0.0f,
			    	  0.0f, 0.0f, 1.0f,
					  x   , y   , z     );
  
  if((v = sqrtf(b*b+c*c))<0.0001f){
    if(a<0.0f) phi=-phi;
         Rp  = matrix(1.0f,  0.0f,      0.0f,
                      0.0f,  cosf(phi),-sinf(phi),
                      0.0f,  sinf(phi), cosf(phi),
			 		  0.0f,  0.0f,      0.0f      );
    return T*Rp*T_;
  }

         Rp  = matrix(cosf(phi), -sinf(phi), 0.0f,
                      sinf(phi),  cosf(phi), 0.0f,
			 		  0.0f,       0.0f,      1.0f,
			 		  0.0f,       0.0f,      0.0f  );

  matrix R1  = matrix(1.0f, 0.0f, 0.0f,
	                  0.0f,  c/v, b/v,
			 		  0.0f, -b/v, c/v,
			 		  0.0f, 0.0f, 0.0f  );
  
  matrix R2  = matrix(v,    0.0f, a,
	                  0.0f, 1.0f, 0.0f,
			 		 -a,    0.0f, v,
			 		  0.0f, 0.0f, 0.0f  );

  matrix R2_ = R2.Inverse();

  matrix R1_ = R1.Inverse();
  
  return T*R1*R2*Rp*R2_*R1_*T_;
}

float  distance(vector u, vector v) { return sqrtf(sqr(u.x-v.x)+sqr(u.y-v.y)+sqr(u.z-v.z)); }

float  Betrag(vector v) { return (sqrtf(v.x*v.x+v.y*v.y+v.z*v.z)); }

vector unit(vector v)   { return (v/Betrag(v)); }

vector blend (float f1, vector v1, vector v2)
{
  float f2=1.0f-f1;
  return vector(f1*v1.x+f2*v2.x, f1*v1.y+f2*v2.y, f1*v1.z+f2*v2.z);
}

vector XfromZ(vector Z)
{
  return unit( ((fabs(Z.x)<OB64) && (fabs(Z.y)<OB64))? vector(Z.z, 0.0f, -Z.x) : vector(-Z.y, Z.x, 0.0f) );
}

float PMinDist(vector P1, vector D1, vector P2, vector D2)
{
  float z1 = D1.x * D2.x + D1.y * D2.y + D1.z * D2.z;
  float n1 = D1.x * D1.x + D1.y * D1.y + D1.z * D1.z; float a = z1/n1;
  float n2 = D2.x * D2.x + D2.y * D2.y + D2.z * D2.z; float c = z1/n2;
  float b  = (D1.x*(P2.x-P1.x) + D1.y*(P2.y-P1.y) + D1.z*(P2.z-P1.z)) / n1;
  float d  = (D2.x*(P2.x-P1.x) + D2.y*(P2.y-P1.y) + D2.z*(P2.z-P1.z)) / n2;
  if(fabs(1.0f-c*a) < 1e-9f) return 0.0f;
  return (c*b-d)/(1.0f-c*a);
}

void EStretch(vector & min, vector & max, vector v)
{
  if(v.x < min.x) min.x=v.x; if(v.x > max.x) max.x=v.x;
  if(v.y < min.y) min.y=v.y; if(v.y > max.y) max.y=v.y;
  if(v.z < min.z) min.z=v.z; if(v.z > max.z) max.z=v.z;
}

vector refract(vector L, vector N, float n1, float n2)
{
  float  rf = n1/n2;
  double ai = acs(N % -L);
  double sa = rf*sin(ai);
  if(sa>1.0){
	return -unit(2*(N % L)*N-L);
  } else {
    double at = asin(sa);
    return rf*L-(((float)cos(at)+rf*(L%N))*N);
  }
}

float MDiff(angles a, matrix D)
{
  matrix S; S.InitRotMat(a);
  return (float)(fabs(S.M11-D.M11)+fabs(S.M12-D.M12)+fabs(S.M13-D.M13)+
                 fabs(S.M21-D.M21)+fabs(S.M22-D.M22)+fabs(S.M23-D.M23)+
                 fabs(S.M31-D.M31)+fabs(S.M32-D.M32)+fabs(S.M33-D.M33));
}

float MDiff2(angles a, matrix D)
{
  matrix S1; S1.InitRotMat(angles(0.0f, 0.0f, a.z));
  matrix S2; S2.InitRotMat(angles(a.x, 0.0f, 0.0f));
  matrix S3; S3.InitRotMat(angles(0.0f, a.y, 0.0f));
  matrix S = S1*S2*S3;
  return (float)(fabs(S.M11-D.M11)+fabs(S.M12-D.M12)+fabs(S.M13-D.M13)+
                 fabs(S.M21-D.M21)+fabs(S.M22-D.M22)+fabs(S.M23-D.M23)+
                 fabs(S.M31-D.M31)+fabs(S.M32-D.M32)+fabs(S.M33-D.M33));
}

void RFTrans(matrix *M, vector *S, angles *R, vector *T)
{
  T->x = M->M41; T->y = M->M42; T->z = M->M43;
  matrix SM = *M; SM.M41=0.0f; SM.M42=0.0f; SM.M43=0.0f;
  vector sx=vector(1.0f, 0.0f, 0.0f)*SM;
  vector sy=vector(0.0f, 1.0f, 0.0f)*SM;
  vector sz=vector(0.0f, 0.0f, 1.0f)*SM;
  *S = vector(Betrag(sx), Betrag(sy), Betrag(sz));
  matrix IS; IS.InitTransformation(1.0f/S->x, 1.0f/S->y, 1.0f/S->z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  matrix UM = IS * SM;
  float  dx1,dy1,dz1,dx,dy,dz; int i=1;
  float  x=0.0f, y=0.0f, z=0.0f;
  float  g1, g0=MDiff(angles(x,y,z),UM);

  if(g0>0.001f){
    float s=0.2f; float g=0.000001f;
    while(s>g){
      //TextWin.Output(1,"  iteration %2d, stepsize %f -> mismatch = %f\n", i, s, g0);

      dx1=MDiff(angles(x+0.1f*s,y,z),UM)-MDiff(angles(x-0.1f*s,y,z),UM);
      dy1=MDiff(angles(x,y+0.1f*s,z),UM)-MDiff(angles(x,y-0.1f*s,z),UM);
      dz1=MDiff(angles(x,y,z+0.1f*s),UM)-MDiff(angles(x,y,z-0.1f*s),UM);
      dx=-dx1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      dy=-dy1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      dz=-dz1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      if((g1=MDiff(angles(x+s*dx, y+s*dy, z+s*dz),UM)) < g0){
        x += s*dx;
        y += s*dy;
        z += s*dz;
        g0=g1;
	  } else {
        s*=0.8f;
	  } i++;
	}
  }
  *R=angles(x, y, z);
}

float RFTrans2(matrix *M, vector *S, angles *R, vector *T)
{
  T->x = M->M41; T->y = M->M42; T->z = M->M43;
  matrix SM = *M; SM.M41=0.0f; SM.M42=0.0f; SM.M43=0.0f;
  vector sx=vector(1.0f, 0.0f, 0.0f)*SM;
  vector sy=vector(0.0f, 1.0f, 0.0f)*SM;
  vector sz=vector(0.0f, 0.0f, 1.0f)*SM;
  *S = vector(Betrag(sx), Betrag(sy), Betrag(sz));
  matrix IS; IS.InitTransformation(1.0f/S->x, 1.0f/S->y, 1.0f/S->z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  matrix UM = IS * SM;
  float  dx1,dy1,dz1,dx,dy,dz; int i=1;
  float  x=0.0f, y=0.0f, z=0.0f;
  float  g1, g0=MDiff2(angles(x,y,z),UM);

  if(g0>0.001f){
    float s=0.2f; float g=0.000001f;
    while(s>g){
      //TextWin.Output(1,"  iteration %2d, stepsize %f -> mismatch = %f\n", i, s, g0);

      dx1=MDiff2(angles(x+0.1f*s,y,z),UM)-MDiff2(angles(x-0.1f*s,y,z),UM);
      dy1=MDiff2(angles(x,y+0.1f*s,z),UM)-MDiff2(angles(x,y-0.1f*s,z),UM);
      dz1=MDiff2(angles(x,y,z+0.1f*s),UM)-MDiff2(angles(x,y,z-0.1f*s),UM);
      dx=-dx1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      dy=-dy1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      dz=-dz1/(float)sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
      if((g1=MDiff2(angles(x+s*dx, y+s*dy, z+s*dz),UM)) < g0){
        x += s*dx;
        y += s*dy;
        z += s*dz;
        g0=g1;
	  } else {
        s*=0.8f;
	  } i++;
	}
  }
  *R=angles(x, y, z);
  return g0;
}
