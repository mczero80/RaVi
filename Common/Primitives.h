// Geometric Primitives for VModel and WinOSi
// ------------------------------------------
// 25.09.1998                   Michael Granz


#define SURF_DOUBLESIDED     4
#define SURF_ROUNDED         8
#define SURF_GROUPED        16
#define SURF_INVISIBLE      32
#define SURF_ONLY_INDIRECT  64
#define SURF_INVERSE       256
#define SURF_COS_Z         512

#include "materials.h"
#include "function.h"

extern ushort OGLP;

enum PrimID { ROOT,LINE,RAY,TRIANGLE,PARALLELOGRAM,CUBE,CYLINDER,RING,SPHERE,PARABOLOID,TORUS,POLYMESH,FUNCTION };

typedef class  Primitive *PrimPtr;
typedef class  Surface   *SurfPtr;

struct llent  { ushort v1,v2; ushort n1,n2; ushort flags; };

struct IntPre {
  vector base,edg1,edg2,N;
  float  deno;
  ushort fcnr;
  ushort fidx;
  ushort vidx;
  byte   algn;
  byte   flag;
};


class Primitive
{
  public:
    PrimPtr pred;  // pointer to predecessor
    PrimPtr succ;  // pointer to successor
    PrimID  idnt;  // primitive identification code
    int     viss;  // visibility

    void Kill();
    Primitive(void) { pred=NULL; succ=NULL; idnt=ROOT; viss=1; }

	virtual vector RndPnt (void)            =0;
    virtual vector PPoint (float u, float v)=0;
    virtual vector PNormal(float u, float v)=0;
    virtual vector VNormal(vector p)=0;
	virtual int    ParseToken(char *a, CFile *file)=0;
	virtual void   CountFV(long int & vt, long int & fc)=0;
	virtual void   WorldExtents(vector & min, vector & max)=0;
    #ifdef VModel
	  virtual void BuildGL(int hl)=0;
    #endif
};

class Line : public Primitive
{
  public:
    vector p1;  // startpoint
    vector p2;  // endpoint
};

class Ray : public Primitive
{
  public:
    vector origin;
    vector direction;
	float  wavelength;

    Ray(void){ idnt = RAY; }
    
	virtual vector RndPnt (void) { return vector(0.0f, 0.0f, 0.0f); }
    virtual vector PPoint (float u, float v) { return vector(0.0f, 0.0f, 0.0f); }
    virtual vector PNormal(float u, float v) { return vector(0.0f, 0.0f, 1.0f); }
    virtual vector VNormal(vector p) { return vector(0.0f, 0.0f, 1.0f); }
	virtual int    ParseToken(char *a, CFile *file) { return 0; }
	virtual void   CountFV(long int & vt, long int & fc) {;}
	virtual void   WorldExtents(vector & min, vector & max) {;};
    #ifdef VModel
	  virtual void BuildGL(int hl) {;}
    #endif
};

struct PMesh {
  ushort  nv,uc,vc; // number of vertices (u,v)
  ushort  nl;       // number of unique edge lines
  llent  *lin;      // pointer to array[nl] of lines
  vector *pts;      // pointer to array[nv] of polygon-vertices
  float  *tx1,*ty1; // pointer to array[nv] of texture coordinates
  vector *snm;      // pointer to array[nf] of surface normals
  vector *vnm;      // pointer to array[nv] of vertex normals
};

class Surface : public Primitive
{
  public:
    MatPtr  Mat;    // object-material
    matrix  TM;     // forward vertex-transformation-matrix
    matrix  Tb;     // inverse vertex-transformation-matrix
    matrix  NM;     // normal-transformation-matrix
    vector *txc;    // optional array of texture coordinates
    char   *name;   // object-name
	PMesh   mesh;   // polygon mesh representation
	ushort  flags;  // flags
    ushort  dflags; // displayflags
    ushort  OGLL;   // OpenGL-Displaylist
    
    Surface(void){
	  name = NULL; flags=0; txc=NULL;
      OGLL = OGLP++; dflags = 0;
      TM.M11=1.0f; TM.M12=0.0f; TM.M13=0.0f;
      TM.M21=0.0f; TM.M22=1.0f; TM.M23=0.0f;
      TM.M31=0.0f; TM.M32=0.0f; TM.M33=1.0f;
      TM.M41=0.0f; TM.M42=0.0f; TM.M43=0.0f;
	  Tb=TM; NM=TM;
	  mesh.nv=0; mesh.nl=0; mesh.lin=NULL; mesh.pts=NULL; mesh.snm=NULL;
	}
    void    CStretch(vector & min, vector & max, vector u, vector v);
	void    SetLine(float x1, float y1, float z1, float x2, float y2, float z2, float n1x, float n1y, float n1z, float n2x, float n2y, float n2z, int edge, int flags, int hl);
	void    CompleteRMesh(int u, int v, int u_closed, int v_closed, int inverse);
	void    ShadeRMesh(int u, int v, int u_closed, int v_closed, int inverse);
    void    MeshPts(float ustart, float ustop, float ustep, ushort uloop, float vstart, float vstop, float vstep, ushort vloop);
	void    DrawWire(int hl);
    void    Transform(matrix H);
	void    Translate(vector p);
	void    Scale    (vector s);
    void    RotateX(float a);
    void    RotateY(float a);
    void    RotateZ(float a);
};

class Parallelogram : public Surface
{
  public:
    vector  base;     // base-corner of parallelogram
    vector  edg1;     // first edge from base-corner to next corner
    vector  edg2;     // other edge from base-corner to next corner
    vector  N;        // precalculated surface normal
    float   deno;     // precalculated denominator for parameter check
    int     algn;     // precalculated alignment case for parameter check

    Parallelogram(void){ idnt=PARALLELOGRAM; }
    void Init(void);  // precalculate 

	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=4; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
	void    Translate(vector p);
	void    Scale    (vector s);
    void    RotateX(float a);
    void    RotateY(float a);
    void    RotateZ(float a);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

bool IsParallelogram(vector p1, vector p2, vector p3, vector p4);

class Triangle : public Parallelogram
{
  public:
    Triangle(void){ idnt=TRIANGLE; }
	virtual vector RndPnt (void            );
    virtual void   CountFV(long int & vt, long int & fc){ vt+=3; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
};

class PolyMesh : public Surface
{
  public:
    ushort  pmflags;  // global polymesh-flags
    ushort  nv;       // number of vertices
    ushort  nn;       // number of phong-vertex-normals
    ushort  nt;       // number of texture-coordinates
    ushort  nf;       // number of faces
    ushort  nm;       // number of materials
	ushort  np;
    //ushort  n,m;      // number of vertices along n-, m-direction of regular mesh
    ushort *pfv;      // pointer to collection of vertex-indexes
    ushort *pfn;      // pointer to collection of normal-indexes
    ushort *pft;      // pointer to collection of texture-coordinate-indexes
    //float  *hgt;      // pointer to array[nv] of height-values
    vector *pts;      // pointer to array[nv] of polygon-vertices
    vector *nrm;      // pointer to array[nn] of phong-vertex-normals
    MatPtr *mtl;      // pointer to array[nm] of material-pointers
    byte   *pnv;      // pointer to array[nf] of polygon-vertex-counters
    byte   *flg;      // pointer to array[nf] of polygon-flags
    byte   *grp;      // pointer to array[nf] of smoothing groups
    byte   *mat;      // pointer to array[nf] of material-indexes
    IntPre *pre;

    PolyMesh(void){
	  idnt=POLYMESH; pmflags = 0;
      pfv = NULL; pfn = NULL; pft = NULL;
      pts = NULL; nrm = NULL; txc = NULL;
      mtl = NULL; pnv = NULL;
      flg = NULL; grp = NULL;
      mat = NULL; Mat = NULL; pre = NULL;
      nv=0; nn=0; nt=0; nm=0; nf=0; np=0;
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=nv; fc+=nf; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
    void CalcSurfaceNormals();
    void CalcSmoothingNormals();
};


class Cube : public Surface
{
  public:
    Cube(void){ idnt=CUBE; }

	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=8; fc+=6; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Cylinder : public Surface
{
  public:
	float   angle; // opening angle
	float   r_top; // top radius of cone

    Cylinder(void){
	  idnt=CYLINDER; angle=0.0f; r_top=1.0f;
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=2; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Sphere : public Surface
{
  public:
	float  vangle,ustart,ustop;
	float  rad2;

    Sphere(void){
	  idnt=SPHERE; rad2=0.0f; vangle=0.0f;
	  ustart=-1.5707963f;
	  ustop =+1.5707963f;
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=1; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Paraboloid : public Surface
{
  public:
	float  angle;
	float  height;

    Paraboloid(void){
	  idnt=PARABOLOID; height=1.0f; angle=0.0f;
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=1; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Ring : public Surface
{
  public:
	float   angle, r_i; // rotation angle, inner radius (outer radius is 1.0)

    Ring(void){
	  idnt=RING; angle=0.0f; r_i=0.0f;
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=1; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Function : public Surface
{
  private:
    float  IntFac(vector Org, vector Ray, int algn);
    double frec(float t1, float t2, vector Org, vector Ray, int level);
    double ChkE(int top, float t1, float t2, double v1, double v2, vector Org, vector Ray, int level);
  public:
    ExprPtr func;    // function expression tree
	vector  Min,Max; // boundary of function definition
    float   JS;      // maximum stepsize for solver

    Function(void){
	  idnt=FUNCTION;
	  func=NULL;             JS=0.1f ;
	  Min=vector(-1.0f, -1.0f, -1.0f);
	  Max=vector( 1.0f,  1.0f,  1.0f);
	}
	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=4; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};

class Torus : public Surface
{
  public:
	float  vangle,ustart,ustop,R1,R2; 

    Torus(void){
	  idnt=TORUS; vangle=0.0f;
	  ustart=0.0000000f; R1=1.0f;
	  ustop =6.2831853f; R2=0.5f;
	}

	virtual vector RndPnt (void            );
    virtual vector PPoint (float u, float v);
    virtual vector PNormal(float u, float v);
    virtual vector VNormal(vector p);
	virtual int    ParseToken(char *a, CFile *file);
    virtual void   CountFV(long int & vt, long int & fc){ vt+=1; fc+=1; }
	virtual void   WorldExtents(vector & min, vector & max);
    #ifdef VModel
	  virtual void BuildGL(int hl);
    #endif
};


extern PrimPtr Kill1(PrimPtr p);
