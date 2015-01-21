
#define RV_SingleSided        4
#define RV_Invisible         32
#define RV_OnlyIndirect      64
#define RV_Inverse          256
#define RV_SurfCosZ         512
#define RV_CastNoShadow    1024
#define RV_ReceiveNoShadow 2048
#define RV_Chequered       4096

extern "C" ushort  cframe;
extern     ushort  UseASM;

extern     float     ray_thresh;
extern "C" vector    RV_BBMin;
extern "C" vector    RV_BBMax;

typedef class  RV_Primitive *RV_PrP;
typedef struct PrLst        *PrLP;
typedef struct RV_Light     *LitPtr;

extern short int RV_Quality;
extern short int RV_AntiAlias;
extern short int RV_Caustics, RV_Caustics_Display, RV_Caustics_Interpolation;
extern short int RV_Filter;

extern unsigned short int RV_Threads;

extern int RV_Caustics_Samples;
extern int RV_Caustics_Quality;

extern vector    RV_Target;
extern vector    RV_Up;
extern float     RV_FoV;

extern     Material  RV_Environment;
extern     RV_PrP    RV_LtObj;
extern "C" LitPtr    RV_Lights;

typedef struct QLst *QLP;
typedef struct PHit *PHP;

struct PHit {
  PHP   nxt;
  float x,y;
  float v;
};

struct QLst {
  float val;
  long  cnt;
  QLP   BL,BR,TL,TR;
  PHP   Hits;
};

extern QLst  QLT[];

struct PrLst {
  RV_PrP  Prim;
  PrLP    next;
};

struct RV_Light {
  LitPtr  next;     // 00
  RV_PrP  Prim;     // 04
  vector  p;        // 08
  float   v;        // 14
};

struct RV_GenShd {
  matrix  NM;
  vector *txc;
};

class RV_Primitive {
  public:
    ushort type;      // 00 type of primitive
    ushort flags;     // 02 flags
    int    rfnum;     // 04 reference number of primitve
    MatPtr Mat;       // 08 object-material
    ushort lastframe; // 0C number of last precalculated frame
	ushort lastray;   // 0E number of last tested ray

    RV_Primitive(void) { flags=0; Mat=NULL; lastframe=42; lastray=43; }
};

class RV_SimpleSphere : public RV_Primitive {
  public:
    vector position;  // 10 center-position of sphere
    vector pvec;      // 1C frame-level-precalculated view-vector (camera-center)
    float  r2;        // 28 squared radius of sphere
    float  cterm;     // 2C frame-level-precalculated algebraic term "c"

    RV_SimpleSphere(void){ type=2; r2=1.0f; }
};

class RV_ComplexSphere : public RV_Primitive {
  public:
    matrix Tb;        // back-transformation matrix
	float  vangle;    // opening angle
	float  ustart;    // start value of u-parameter
	float  ustop;     // end   value of u-parameter
    vector position;  // center-position of sphere
    //vector pvec;      // frame-level-precalculated view-vector (camera-center)
    float  r2;        // squared radius of sphere
    //float  cterm;     // frame-level-precalculated algebraic term "c"
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_ComplexSphere(void){
	  type=7; sinfo=NULL; r2=1.0f; vangle=0.0f;
 	  ustart=-1.5707963f;
	  ustop =+1.5707963f;
	}
};

class RV_Ellipsoid : public RV_Primitive {
  public:
    matrix Tb;        // back-transformation matrix
	float  vangle;    // opening angle
	float  ustart;    // start value of u-parameter
	float  ustop;     // end   value of u-parameter
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_Ellipsoid(void){
	  type=9; sinfo=NULL; vangle=0.0f;
 	  ustart=-1.5707963f;
	  ustop =+1.5707963f;
	}
};

struct RV_TrPShd {
  vector *nrm;
  vector *txc;
};

class RV_TriPara : public RV_Primitive {
  public:
    vector base;      // 10 base vertex
    vector edg1;      // 1C 1st edge vector
    vector edg2;      // 28 2nd edge vector
    vector N;         // 34 scene-level-precalculated normal vector
    float  cpos;      // 40 frame-level-precalculated camera position dependent component
    float  deno;      // 44 scene-level-precalculated denominator for parameter check
    RV_TrPShd *sinfo; // 48 shading information (not needed for intersection test)
    ushort algn;      // 4C scene-level-precalculated alignment case for parameter check

	RV_TriPara(void){ sinfo=NULL; }
};

class RV_Cube : public RV_Primitive {
  public:
    vector  base;     // 10 base point
    vector  top;      // 1C top point
    vector  axis1,axis2,axis3; // 28,34,40 direction vectors
    float   or[6];    // 4C frame-level-precalculated orientations

	RV_Cube(void){ type=4; }
};

class RV_Cylinder : public RV_Primitive {
  public:
    matrix  Tb;
	float   angle;    // opening angle
	float   r_top;    // 1 - top radius of cone
	vector  TCam;     // frame-level-precalculated transformed ray origin
	float   cterm;    // frame-level-precalculated algebraic term "c"
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_Cylinder(void){ type=3; angle=0.0f; r_top=0.0f; }
};

class RV_SimpleRing : public RV_Primitive {
  public:
	vector  center;   // center of ring
	vector  N;        // normal vector
	float   r_o, r_i; // squared outer radius, squared inner radius
	float   cpos;     // frame-level-precalculated camera position dependent component

    RV_SimpleRing(void){ type=5; r_o=1.0f; r_i=0.0f; }
};

class RV_ComplexRing : public RV_Primitive {
  public:
    matrix Tb;        // back-transformation matrix
	float  angle;     // opening angle
	vector  center;   // center of ring
	vector  N;        // normal vector
	float   r_o, r_i; // squared outer radius, squared inner radius
	//float   cpos;   // frame-level-precalculated camera position dependent component
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_ComplexRing(void){
	  type=10; sinfo=NULL; angle=0.0f; r_o=1.0f; r_i=0.0f;
	}
};

class RV_Paraboloid : public RV_Primitive {
  public:
    matrix  Tb;
	float   angle;    // opening angle
	float   height;
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_Paraboloid(void){ type=8; angle=0.0f; height=1.0f; }
};

class RV_Torus : public RV_Primitive {
  public:
    matrix  Tb;
	float   vangle;   // opening angle
	float   ustart;   // start value of u-parameter
	float   ustop;    // end   value of u-parameter
	float   R1,R2;    // radii of torus
	vector  TCam;     // frame-level-precalculated transformed ray origin
    RV_GenShd *sinfo; // shading information (not needed for intersection test)

    RV_Torus(void){ type=6; vangle=0.0f; ustart=0.0f; ustop =6.2831853f; R1=1.0f; R2=0.5f; }
};

struct IntPar {
  vector  IP; float pad1; // 00h intersection point
  vector  N;  float pad2; // 10h surface normal at intersection point
  float   u,v;            // 20h surface parameters at intersection point
  float   x,y;            // 28h texture coordinates at intersection point
  float   t;              // 30h ray parameter
  RV_PrP  p;              // 34h pointer to intersected primitive
  ushort  i;              // 38h face index for cube
};

struct Thrdata {
  vector             CamRay;
  PHit              *PHB;
  IntPar             Cint, Cmin;
  unsigned long int  PHc;
  int                xs,ys;
  int                xe,ye;
  ushort             CRay;
};

struct BSPplane {
  struct BSPplane  *left;    // left  child (values < dval)
  struct BSPplane  *rigt;    // right child (values > dval)
  float             dval;    // dividing value
  int               axis;    // dividing axis
};

extern HANDLE ThrFinished;

extern BSPplane *RV_SceneTree;

extern void RV_Init();

extern void InitTP(RV_TriPara *p);
extern void InitCube(RV_Cube *c, matrix & TM);

extern vector RV_RndPt(RV_PrP p);
extern vector RV_PtNrm(RV_PrP p, vector pt);

extern void GetIPN(vector *Org, vector *Ray, IntPar *I);
extern void GetTXY(                          IntPar *I);

extern void RV_RenderImage(byte *BMPbits, int XRes, int YRes, int border);

extern RGB_Color RayTrace(Thrdata *Thread, RV_PrP OrgObj, void Init(vector Org), MatPtr mat, vector *Org, vector *Ray, float em, float dist, RGB_Color shade(RV_PrP OrgObj, MatPtr mat, vector *Org, vector *Ray, IntPar Cmin, float em, float dist, int level, Thrdata *Thread), int level, int max_raydepth);
extern RGB_Color RayTrace0(Thrdata *Thread);
extern void      RayTrace4(void);

extern float ShadowTest(RV_PrP OrgObj, RV_PrP LtObj, vector *Org, vector *Ray, float dist);

extern RGB_Color ShadeDepth(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
extern RGB_Color ShadeNormals(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
extern RGB_Color ShadeReflection(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
extern RGB_Color ShadeUV(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
extern RGB_Color ShadeSimple(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
extern RGB_Color Shade(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);

extern RGB_Color (*RV_Shader)(vector *Org, vector *Ray, IntPar IntP, MatPtr mat, int level);
