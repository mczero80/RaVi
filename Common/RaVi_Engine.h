
extern "C" {
  extern unsigned long int CheckSSE(void);
  extern void RV_IntersectBoundingBox0(Thrdata *Thread);
  extern void RV_TraceList0(Thrdata *Thread, BSPplane *tree);
  extern void RV_DispatchIntersectAll0(Thrdata *Thread, RV_PrP p);
  extern void RV_DispatchIntersectAll1(RV_PrP p, vector *Org, vector *Ray, IntPar *Ints);
}


extern vector *RV_CamPtr;
extern vector *RV_RayPtr;
extern IntPar *RV_IntPtr;
extern vector *RV_OrgPtr;
extern vector *RV_DirPtr;
extern float  *RV_ShfPtr;

extern float  *RV_MDP;
