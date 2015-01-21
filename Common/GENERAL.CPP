// general utilities for VModel and WinOSi
// ---------------------------------------
// 30.03.2001                Michael Granz
//
// math optimizations in rnddir-functions by Dmitry Omelchenko aka tigra

#include "stdafx.h"
#include "general.h"

//double seed;
//double rnd(){ return (double)(seed=fract(pow(seed + 3.1415927 + (double)(rand()&0x7FFF)/(double)0x7FFF, 5.04))); }

/*long int rngh,rng;

double rnd(){
  double temp;
  __asm {
    mov   edx,rngh
    shr   edx,1
    mov   eax,rng
    mov   ecx,eax
    rcr   ecx,1
    rcl   edx,1
    mov   rngh,edx
    shl   eax,12
    xor   ecx,eax
    mov   eax,ecx
    shr   eax,20
    xor   ecx,eax
    mov   rng,ecx
    //fast conversion to float
    mov   edx,ecx
    shr   ecx,12
    or    ecx, 3FF00000H
    shl   edx, 20
    mov   dword ptr [temp+4], ecx
    mov   dword ptr [temp  ], edx
    fld1
    fld   qword ptr [temp  ]
    fsubr
	fstp  qword ptr [temp  ]
  }
  return temp;
}*/

// 63 bit random number generator RANROT type W algorithm
//
// © 1999, 2004 Agner Fog. 
// GNU General Public License www.gnu.org/copyleft/gpl.html

#define KK 17

long p1,p2;         // pointer in circular buffer
long randbuf[2*KK]; // circular buffer

double rnd(){
  double temp;
  __asm {
        PUSH    EBX
		// 64 bit random number -> EDX:EAX
        MOV     EBX, [p1]              // ring buffer pointers
        MOV     ECX, [p2]              // ring buffer pointer
        MOV     EDX, [randbuf][EBX  ]
        MOV     EAX, [randbuf][EBX+4]
        ROL     EDX, 19                // rotate bits
        ROL     EAX, 27
        ADD     EDX, [randbuf][ECX  ]  // add two dwords
        ADD     EAX, [randbuf][ECX+4]
        MOV     [randbuf][EBX  ], EAX  // save in swapped order
        MOV     [randbuf][EBX+4], EDX
        SUB     EBX, 8                 // decrement p1
        JNC     SHORT R30
        MOV     EBX, (KK-1)*8          // wrap around p1
R30:    SUB     ECX, 8                 // decrement p2
        JNC     SHORT R40
        MOV     ECX, (KK-1)*8          // wrap around p2
R40:    MOV     [p1], EBX              // save updated pointers
        MOV     [p2], ECX
        POP     EBX
        // fast convert to float
        SHR     EAX, 12
        OR      EAX, 3FF00000H
        SHL     EDX, 20
        MOV     DWORD PTR [temp+4],EAX
        MOV     DWORD PTR [temp  ],EDX
        FLD1
        FLD     QWORD PTR [temp  ]
        FSUBR
	    FSTP    QWORD PTR [temp  ]
  }
  return temp;
}

void rndinit(long seed){
  __asm {
        MOV     EAX, [seed]
        XOR     ECX, ECX
        // make random numbers and put them into buffer
R80:    IMUL    EAX, 2891336453
        INC     EAX
        MOV     [randbuf][ECX*4], EAX
        INC     ECX
        CMP     ECX, KK*2
        JB      R80
        // initialize buffer pointers
		MOV     [p1], 0
        MOV     [p2], 80
  }
  // prepare first random numbers
  double temp; for(int i=0; i<32; i++) temp=rnd();
}


// random unit direction vector
vector rnddir()
{
  float cos_a = 2.0f*(float)rnd()-1.0f;
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = TwoPi*(float)rnd();
  return vector(sin_a*cosf(b), sin_a*sinf(b), cos_a);
}

// random unit direction (constrained to sphere sector)
vector rrdir(float bmax, float amin, float amax)
{
  float cos_a = (sinf(amax)-sinf(amin))*(float)rnd()+sinf(amin);
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = bmax*(float)rnd();
  return vector(sin_a*cosf(b), sin_a*sinf(b), cos_a);
}

// cosine distributed random unit direction (constrained to sphere sector)
vector crrdir(float bmax, float amin, float amax)
{
  float cos_a = sqrtf((sinf(amax)-sinf(amin))*(float)rnd()+sinf(amin));
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = bmax*(float)rnd();
  return vector(sin_a*cosf(b), sin_a*sinf(b), cos_a);
}

// random unit direction on hemisphere around normal vector N
vector rndhdir(vector N)
{
  float cos_a = (float)rnd();
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = TwoPi*(float)rnd();
  vector    u = XfromZ(N);
  return unit(cos_a*N + sin_a*cosf(b)*u - sin_a*sinf(b)*(N*u));
}

// cosine distributed random unit direction on hemisphere around normal vector N
vector crndhdir(vector N)
{
  float cos_a = (float)sqrt(rnd());
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = TwoPi*(float)rnd();
  vector    u = XfromZ(N);
  return unit(cos_a*N + sin_a*cosf(b)*u - sin_a*sinf(b)*(N*u));
}

// random unit direction on hemisphere around normal vector N
// with phong exponent distibution
vector rndpdir(vector N, float r)
{
  float cos_a = (float)pow(rnd(), (double)r);
  float sin_a = sqrtf(1.0f - cos_a * cos_a);
  float     b = TwoPi*(float)rnd();
  vector    u = XfromZ(N);
  return unit(cos_a*N + sin_a*cosf(b)*u - sin_a*sinf(b)*(N*u));
}


float bilinear(float x, float y, float v00, float v10, float v01, float v11)
{
  return blend(y, blend(x, v00, v01), blend(x, v10, v11));
}

// waiting

void WaitTask(HWND WinHd)
{
  MSG  msg;
  BOOL MSG_found;
  do{
    if(MSG_found=PeekMessage(&msg,WinHd,0,0,PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }while(MSG_found);
}

void Delay(DWORD ms)
{
  DWORD te = GetTickCount() + ms;
  do {
    WaitTask(NULL);
  } while(GetTickCount()<te);
}

// vector sets

VSEPtr VS_Add(VSEPtr s, int *ix, int *ct, vector v, float eps)
{
  float c = v.x+v.y+v.z;

  if(!s){
    s=new VSEntry;
    s->right = NULL; s->left = NULL; s->code = c; s->v = v;
    s->n = *ix = (*ct)++;
    return s;
  }
  if(fabs((s->code)-c) < eps){
    if((s->v)==v){
      *ix = s->n;
      return s;
    }
  }
  if((s->code)<c){
    s->right = VS_Add(s->right, ix, ct, v, eps);
  } else {
    s->left  = VS_Add(s->left , ix, ct, v, eps);
  }
  return s;
}

int VectorSet::Add(vector v, float eps)
{
  Root = VS_Add(Root, &Index, &Counter, v, eps);
  return Index;
}

void VS_Collect(VSEPtr s, vector *array)
{
  if(s){
    if(s->right) VS_Collect(s->right, array);
    if(s->left)  VS_Collect(s->left , array);
    array[s->n] = s->v;
    delete(s);
  }
}

int VectorSet::Move2Array(vector **a)
{
  int i; *a = new(vector[Counter]);
  // copy vectors from VectorSet into destination-array and delete Set-Entrys
  VS_Collect(Root,*a);
  i=Counter; Counter=0; Root=NULL; // Re-Initialisation for next usage
  return i;
}

// sorting

void doubsort(int n, double a[])
{
  switch(n){
    case  3: if(a[1]<a[0]) swap(a[1],a[0]);
	         if(a[2]<a[1]) swap(a[2],a[1]);
    case  2: if(a[1]<a[0]) swap(a[1],a[0]);
    case  1: return;
    default: {
	  int i,j;
	  for(i=0; i<n; i++){
		for(j=i+1; j<n; j++){
		  if(a[j]<a[i]) swap(a[j],a[i]);
		}
	  }
	}
  }
}

