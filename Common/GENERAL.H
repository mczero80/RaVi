// general utilities for VModel and WinOSi
// ---------------------------------------
// 25.09.1998                Michael Granz

#include "vectors.h"

#define Pi      3.1415927F
#define TwoPi   6.2831853F

#define Deg2Rad 1.7453293e-2F
#define Rad2Deg 5.7295780e+1F

typedef unsigned       char  byte;
typedef unsigned short int   ushort;

typedef struct VSEntry  *VSEPtr;
        struct VSEntry { VSEPtr left,right; float code; vector v; int n; };

inline void swap(               int & a,                int & b){                int t=a; a=b; b=t; }
inline void swap(         long  int & a,          long  int & b){          long  int t=a; a=b; b=t; }
inline void swap(unsigned long  int & a, unsigned long  int & b){ unsigned long  int t=a; a=b; b=t; }
inline void swap(         short int & a,          short int & b){          short int t=a; a=b; b=t; }
inline void swap(float              & a, float              & b){ float              t=a; a=b; b=t; }
inline void swap(ushort             & a, ushort             & b){ ushort             t=a; a=b; b=t; }
inline void swap(vector             & a, vector             & b){ vector             t=a; a=b; b=t; }
inline void swap(vector           * & a, vector           * & b){ vector           * t=a; a=b; b=t; }
inline void swap(double             & a, double             & b){ double             t=a; a=b; b=t; }

inline long int sqr(     int x){ return (long)x * (long)x; }
inline long int sqr(long int x){ return       x *       x; }
inline double   sqr(double   x){ return       x *       x; }
inline float    sqr(float    x){ return       x *       x; }

inline double   sgn(double   x){ return (x==0.0)? 0.0 : (x>0.0)? 1.0 : -1.0; }

inline float lim(float min, float val, float max){ return (val<min)? min : (val>max)? max : val; }

inline float blend(float c, float x1, float x2){ return c*x1+(1.0f-c)*x2; }

inline int round(float x){ return (int)(x+0.5f); }

inline float ang(float x, float y)
{
  float a = ((fabs(x)>0.0001)? (float)atan(y/x) : (y<0.0f? 4.7123890f : 1.5707963f)) + ((x<-0.0001f)? 3.1415927f : 0.0f);
  if(a<0.0f) a+=6.2831853f;
  return a;
}

inline double fract(double x){ return        fabs(x - (double)((long)x)); }
inline float  fract(float  x){ return (float)fabs(x - (float )((long)x)); }

inline float max3(float a, float b, float c)
{
  return (a>b)? ((a>c)? a : c) : ((b>c)? b : c);
}

inline float min3(float a, float b, float c)
{
  return (a<b)? ((a<c)? a : c) : ((b<c)? b : c);
}

float bilinear(float x, float y, float v00, float v10, float v01, float v11);

class VectorSet
{
  public:
    VSEPtr  Root;
    int     Index;
    int     Counter;

    VectorSet(void) { Root=NULL; Index=0; Counter=0; }
    int Move2Array(vector **a);
    int Add(vector v, float eps);
};


extern void WaitTask(HWND WinHd);
extern void Delay(DWORD ms);
extern void doubsort(int n, double a[]);

//extern double  seed;
extern double  rnd();
extern void rndinit(long seed);
extern vector  rnddir();
extern vector  rrdir(float bmax, float amin, float amax);
extern vector crrdir(float bmax, float amin, float amax);
extern vector  rndpdir(vector N, float r);
extern vector  rndhdir(vector N);
extern vector crndhdir(vector N);
