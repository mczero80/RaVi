// 3D-Vector-Library    für VModel   18.04.1997 Michael Granz

#include <math.h>


inline double fmax(double a, double b){ return a>b? a : b; }

inline int equal(float a, float b, float e)
{
  return ( fabs(a-b) < (fmax(fabs(a),fabs(b)) * e) );
}

inline double acs(double x){ return (x>=1.0)? 0.0 : acos(x); }


class angles
{
  public:
    float x,y,z;
    angles(){}
    angles(float alpha, float beta, float gamma) { x=alpha; y=beta; z=gamma; }
    friend  int       operator == (const angles & a, const angles & b)
      { return a.x==b.x && a.y==b.y && a.z==b.z; }
    friend  int       operator != (const angles & a, const angles & b)
      { return a.x!=b.x || a.y!=b.y || a.z!=b.z; }
    friend  angles    operator -  (const angles & a)
      { return angles(-a.x, -a.y, -a.z); }
};


class matrix
{
  public:
    float M11,M12,M13;
    float M21,M22,M23;
    float M31,M32,M33;
    float M41,M42,M43;
    matrix(){}
    matrix(float a11, float a12, float a13, float a21, float a22, float a23, float a31, float a32, float a33, float a41, float a42, float a43 ){
      M11=a11; M12=a12; M13=a13;
      M21=a21; M22=a22; M23=a23;
      M31=a31; M32=a32; M33=a33;
      M41=a41; M42=a42; M43=a43;
    }
    friend matrix operator * (const matrix & A, const matrix & B){
      return matrix(A.M11*B.M11 + A.M12*B.M21 + A.M13*B.M31,
                    A.M11*B.M12 + A.M12*B.M22 + A.M13*B.M32,
                    A.M11*B.M13 + A.M12*B.M23 + A.M13*B.M33,

                    A.M21*B.M11 + A.M22*B.M21 + A.M23*B.M31,
                    A.M21*B.M12 + A.M22*B.M22 + A.M23*B.M32,
                    A.M21*B.M13 + A.M22*B.M23 + A.M23*B.M33,

                    A.M31*B.M11 + A.M32*B.M21 + A.M33*B.M31,
                    A.M31*B.M12 + A.M32*B.M22 + A.M33*B.M32,
                    A.M31*B.M13 + A.M32*B.M23 + A.M33*B.M33,

                    A.M41*B.M11 + A.M42*B.M21 + A.M43*B.M31 + B.M41,
                    A.M41*B.M12 + A.M42*B.M22 + A.M43*B.M32 + B.M42,
                    A.M41*B.M13 + A.M42*B.M23 + A.M43*B.M33 + B.M43);
    }
    void InitIdentMat();
    void Translate(float x, float y, float z);
    void RotateX(float a);
    void RotateY(float a);
    void RotateZ(float a);
    void Scale(float x, float y, float z);
    void InitRotMat(const angles & r);
    void InitTransformation(float Sx, float Sy, float Sz, float Rx, float Ry, float Rz, float Tx, float Ty, float Tz);
    matrix Inverse();
    matrix NTrans();
};


class vector
{
  public:
    float x,y,z;
    vector(){}
    vector(float vx, float vy, float vz) { x=vx; y=vy; z=vz; }

    friend  int       operator == (const vector & u, const vector & v)
      { return u.x==v.x && u.y==v.y && u.z==v.z; }
    friend  int       operator != (const vector & u, const vector & v)
      { return u.x!=v.x || u.y!=v.y || u.z!=v.z; }
    friend  vector    operator +  (const vector & u, const vector & v)
      { return vector(u.x+v.x, u.y+v.y, u.z+v.z); }
    friend  vector    operator -  (const vector & u, const vector & v)
      { return vector(u.x-v.x, u.y-v.y, u.z-v.z); }
    friend  vector    operator -  (const vector & v)
      { return vector(-v.x, -v.y, -v.z); }
    friend  vector    operator *  (const vector & u, const vector & v)
      { return vector(u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x); }
    friend  vector    operator *  (const vector & v, const matrix & A)
      { return vector(v.x*A.M11+v.y*A.M21+v.z*A.M31+A.M41,
                      v.x*A.M12+v.y*A.M22+v.z*A.M32+A.M42,
                      v.x*A.M13+v.y*A.M23+v.z*A.M33+A.M43); }
    friend  vector    operator *  (const float  & c, const vector & v)
      { return vector(c*v.x, c*v.y, c*v.z); }
    friend  vector    operator *  (const vector & v, const float  & c)
      { return vector(v.x*c, v.y*c, v.z*c); }
    friend  float     operator %  (const vector & u, const vector & v)
      { return (u.x*v.x + u.y*v.y + u.z*v.z); }
    friend  vector    operator /  (const vector & v, const float  & c)
      { return vector(v.x/c, v.y/c, v.z/c); }
    /*friend  ostream & operator << (ostream & s     , const vector & v)
      { s << "(" << v.x << "," << v.y << "," << v.z << ")"; return s; }*/
            void      operator += (const vector & v)
      { x+=v.x; y+=v.y; z+=v.z; }
            void      operator -= (const vector & v)
      { x-=v.x; y-=v.y; z-=v.z; }
            void      operator *= (const float  & c)
      { x*=c; y*=c; z*=c; }
            void      operator /= (const float  & c)
      { x/=c; y/=c; z/=c; }
    float   Betrag()  { return (float)(sqrt(x*x+y*y+z*z)); }
    vector  unit()    { return (*this/Betrag());    }
};

matrix BuildMfZ(vector Z);

matrix InitAxisRotN(float a, float b, float c, float phi);

matrix InitAxisRot(float x, float y, float z, float a, float b, float c, float phi);

float  distance(vector u, vector v);
float  Betrag(vector v);
vector refract(vector L, vector N, float n1, float n2);
vector blend (float f, vector u, vector v);
vector unit  (vector v);
vector XfromZ(vector Z);
void   EStretch(vector & min, vector & max, vector v);
