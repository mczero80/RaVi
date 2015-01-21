global _CheckSSE
global _RV_TraceList0
global _RV_DispatchIntersectAll0
global _RV_DispatchIntersectAll1
global _RV_IntersectBoundingBox0

extern _RV_IntersectCylinder0
extern _RV_IntersectSimpleRing0
extern _RV_IntersectTorus0
extern _RV_IntersectComplexSphere0
extern _RV_IntersectParaboloid0
extern _RV_IntersectEllipsoid0
extern _RV_IntersectComplexRing0

extern _RV_IntersectCylinder1
extern _RV_IntersectSimpleRing1
extern _RV_IntersectTorus1
extern _RV_IntersectComplexSphere1
extern _RV_IntersectParaboloid1
extern _RV_IntersectEllipsoid1
extern _RV_IntersectComplexRing1

extern _BBcandidatePlane
extern _BBlastframe
extern _BBquadrant
extern _BBinside
extern _RV_BBMin
extern _RV_BBMax

extern _cframe
extern _RV_Cam

segment _DATA

align 4

fone:         dd 03F800000h ;  1.0000
fsmall:       dd 038d1b717h ;  0.0001
fzero:        dd 000000000h ;  0.0000
fminus1:      dd 0BF800000h ; -1.0000

segment _CODE

_CheckSSE:push   ebp
          mov    ebp,esp
          push   ebx
          ; first check if CPUID-command is available
          pushfd                 ; copy EFlags-register
          pop    eax             ; to eax
          mov    ebx,eax         ; and to ebx
          xor    eax,00200000h   ; change bit 21
          push   eax             ; copy eax
          popfd                  ; to EFlags-register
          pushfd                 ; and back
          pop    eax             ; to eax
          xor    eax,ebx         ; compare with ebx
          jz     .NoSSE
          ; use CPUID to check for SSE
          mov    eax,1
          cpuid
          test   edx,02000000h
          jz     .NoSSE
          mov    eax,1
          jmp    .exit
.NoSSE:   mov    eax,0
.exit:    pop    ebx
          leave
          ret

; eax = Thrdata *Thread
; edx = RV_SimpleSphere *s

IntersectSimpleSphere0:
          ; is precalculation already done for this frame?
          mov     bx,word [_cframe]    ; current frame
          cmp     bx,word [edx+0Ch]    ; = last precalculated frame?
          je      .samefrm             ; then skip precalculation
          mov     word [edx+0Ch],bx    ; else store current frame number and precalculate:
          ; pvec  = Camera - position;
          ; cterm = pvec.x*pvec.x + pvec.y*pvec.y + pvec.z*pvec.z - r2;
          fld     dword [_RV_Cam    ]
          fsub    dword [edx+10h]
          fst     dword [edx+1Ch]
          fmul    st0
          fld     dword [_RV_Cam+04h]
          fsub    dword [edx+14h]
          fst     dword [edx+20h]
          fmul    st0
          fld     dword [_RV_Cam+08h]
          fsub    dword [edx+18h]
          fst     dword [edx+24h]
          fmul    st0
          faddp   st1,st0
          faddp   st1,st0
          fsub    dword [edx+28h]
          fstp    dword [edx+2Ch]
.samefrm: movss   XMM0,[eax    ]  ; Ray.x
          mulss   XMM0,[edx+1Ch]
          movss   XMM1,[eax+04h]  ; Ray.y
          mulss   XMM1,[edx+20h]
          addss   XMM0,XMM1
          movss   XMM2,[eax+08h]  ; Ray.z
          mulss   XMM2,[edx+24h]
          addss   XMM0,XMM2
          movss   XMM1,XMM0
          mulss   XMM0,XMM0
          subss   XMM0,[edx+2Ch]
          comiss  XMM0,[fzero  ]
          jc      .NoHit
          mulss   XMM1,[fminus1]
          sqrtss  XMM0,XMM0
          subss   XMM1,XMM0
          movss   [eax+40h],XMM1
          ret
.NoHit:   mov     dword [eax+40h], 0BF800000h
          ret

; eax: +14h *CInt
; ebx: +10h *Ray
; ecx: +0Ch *Org
; edx: +08h *TriPara

IntersectTriPara1:
          mov     edx,dword [ebp+08h]
          mov     ebx,dword [ebp+10h]
          ; calculate vector-dot-product ray * normal of plane
          movss  XMM0,[ebx    ] ;  Ray.x
          mulss  XMM0,[edx+34h] ; *N.x
          movss  XMM1,[ebx+04h] ;  Ray.y
          mulss  XMM1,[edx+38h] ; *N.y
          addss  XMM0,XMM1      ; +
          movss  XMM2,[ebx+08h] ;  Ray.z
          mulss  XMM2,[edx+3Ch] ; *N.z
          addss  XMM0,XMM2      ; +

          mov    eax,dword [ebp+14h]
          sub    eax,10h

          ; is surface doublesided?
          test   word [edx+02h],0004h
          jne    .cont1
          ; then only exit if ray is parallel to plane
          comiss XMM0,[fzero  ]
          je     near outside1
          jmp    .cont2
.cont1:   ; else exit if ray hits backface (backface culling)
          comiss XMM0,[fzero  ]
          jnc    near outside1

.cont2:   ; cpos = (p->base - Org)%(p->N)
          mov    ecx,dword [ebp+0Ch]
          movss  XMM1,[edx+10h]
          subss  XMM1,[ecx    ]
          mulss  XMM1,[edx+34h]
          movss  XMM2,[edx+14h]
          subss  XMM2,[ecx+04h]
          mulss  XMM2,[edx+38h]
          addss  XMM1,XMM2
          movss  XMM3,[edx+18h]
          subss  XMM3,[ecx+08h]
          mulss  XMM3,[edx+3Ch]
          addss  XMM1,XMM3
         
          ; intersection parameter t = cpos / (Ray*N)
          divss  XMM1,XMM0
          movss  [eax+40h],XMM1

          ; exit if t is negative (intersection is behind camera)
          comiss XMM1,[fzero  ]
          jc     near outside1

          ; intersection point IP = Org + t * Dir;
          movss  XMM0,[ebx    ]
          mulss  XMM0,XMM1
          addss  XMM0,[ecx    ]
          movss  [eax+10h],XMM0
          movss  XMM2,[ebx+04h]
          mulss  XMM2,XMM1
          addss  XMM2,[ecx+04h]
          movss  [eax+14h],XMM2
          movss  XMM3,[ebx+08h]
          mulss  XMM3,XMM1
          addss  XMM3,[ecx+08h]
          movss  [eax+18h],XMM3

          jmp    near TriPara01

IntersectTriPara0:
          ; calculate vector-dot-product ray * normal of plane
          movss   XMM0,[eax    ] ;  Ray.x
          mulss   XMM0,[edx+34h] ; *N.x
          movss   XMM1,[eax+04h] ;  Ray.y
          mulss   XMM1,[edx+38h] ; *N.y
          addss   XMM0,XMM1      ; +
          movss   XMM2,[eax+08h] ;  Ray.z
          mulss   XMM2,[edx+3Ch] ; *N.z
          addss   XMM0,XMM2      ; +

          ; is surface doublesided?
          test   word [edx+02h],0004h
          jne    .cont1
          ; then only exit if ray is parallel to plane
          comiss XMM0,[fzero  ]
          je     near outside1
          jmp    .cont2
.cont1:   ; else exit if ray hits backface (backface culling)
          comiss XMM0,[fzero  ]
          jnc    near outside1

.cont2:   ; is precalculation already done for this frame?
          mov    bx,word [ _cframe]    ; current frame
          cmp    bx,word [ edx+0Ch]    ; = last precalculated frame?
          je     .samefrm              ; then skip precalculation
          mov    word [    edx+0Ch],bx ; else store current frame number and precalculate:
          ;p->cpos       = (p->base - Camera)%(p->N);
          movss  XMM1,[    edx+10h]
          subss  XMM1,[_RV_Cam    ]
          mulss  XMM1,[    edx+34h]
          movss  XMM2,[    edx+14h]
          subss  XMM2,[_RV_Cam+04h]
          mulss  XMM2,[    edx+38h]
          addss  XMM1,XMM2
          movss  XMM3,[    edx+18h]
          subss  XMM3,[_RV_Cam+08h]
          mulss  XMM3,[    edx+3Ch]
          addss  XMM1,XMM3
          movss  [edx+40h],XMM1

.samefrm: ; intersection parameter t = cpos / (Ray*N)
          movss  XMM1,[edx+40h]
          divss  XMM1,XMM0
          movss  [eax+40h],XMM1

          ; exit if t is negative (intersection is behind camera)
          comiss XMM1,[fzero  ]
          jc     near outside1

          ; intersection point IP = Camera + t * Ray;
          movss  XMM0,[    eax    ]
          mulss  XMM0,XMM1
          addss  XMM0,[_RV_Cam    ]
          movss  [eax+10h],XMM0
          movss  XMM2,[    eax+04h]
          mulss  XMM2,XMM1
          addss  XMM2,[_RV_Cam+04h]
          movss  [eax+14h],XMM2
          movss  XMM3,[    eax+08h]
          mulss  XMM3,XMM1
          addss  XMM3,[_RV_Cam+08h]
          movss  [eax+18h],XMM3

TriPara01:
          ;switch(p->algn){
          mov    bx, word [edx+4Ch]
.case0:   cmp    bx,0
          jne    .case1
	  ; surface parameter v = ((IP.y - base.y)*edg1.x - (IP.x - base.x)*edg1.y) * (1/denominator)
          subss   XMM0,[edx+10h]    ; IP.x - base.x
          movss   XMM5,XMM0         ; store result for later use
          mulss   XMM0,[edx+20h]    ; *edg1.y
          subss   XMM2,[edx+14h]    ; IP.y - base.y
          mulss   XMM2,[edx+1Ch]    ; *edg1.x
          subss   XMM2,XMM0         ; -
          mulss   XMM2,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM2,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM2    ; store v
          ; surface parameter u =  (IP.x - base.x - v*edg2.x) / edg1.x
          mulss   XMM2,[edx+28h]    ; v*edg2.x
          subss   XMM5,XMM2         ; subtract from stored result (IP.x - base.x)
          divss   XMM5,[edx+1Ch]    ; /edg1.x
          jmp    .endcase           ; break
.case1:   cmp    bx,1
          jne    .case2
	  ; surface parameter v = ((IP.z - base.z)*edg1.y - (IP.y - base.y)*edg1.z) * (1/denominator)
          subss   XMM2,[edx+14h]    ; IP.y - base.y
          movss   XMM5,XMM2         ; store result for later use
          mulss   XMM2,[edx+24h]    ; *edg1.z
          subss   XMM3,[edx+18h]    ; IP.z - base.z
          mulss   XMM3,[edx+20h]    ; *edg1.y
          subss   XMM3,XMM2         ; -
          mulss   XMM3,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM3,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM3    ; store v
          ; surface parameter u =  (IP.y - base.y - v*edg2.y) / edg1.y
          mulss   XMM3,[edx+2Ch]    ; v*edg2.y
          subss   XMM5,XMM3         ; subtract from stored result (IP.y - base.y)
          divss   XMM5,[edx+20h]    ; /edg1.y
          jmp    .endcase           ; break
.case2:   cmp    bx,2
          jne    .case3
	  ; surface parameter v = ((IP.z - base.z)*edg1.x - (IP.x - base.x)*edg1.z) * (1/denominator)
          subss   XMM0,[edx+10h]    ; IP.x - base.x
          movss   XMM5,XMM0         ; store result for later use
          mulss   XMM0,[edx+24h]    ; *edg1.z
          subss   XMM3,[edx+18h]    ; IP.z - base.z
          mulss   XMM3,[edx+1Ch]    ; *edg1.x
          subss   XMM3,XMM0         ; -
          mulss   XMM3,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM3,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM3    ; store v
          ; surface parameter u =  (IP.x - base.x - v*edg2.x) / edg1.x
          mulss   XMM3,[edx+28h]    ; v*edg2.x
          subss   XMM5,XMM3         ; subtract from stored result (IP.x - base.x)
          divss   XMM5,[edx+1Ch]    ; /edg1.x
          jmp    .endcase           ; break
.case3:   cmp    bx,3
          jne    .case4
	  ; surface parameter v = ((IP.x - base.x)*edg1.y - (IP.y - base.y)*edg1.x) * (1/denominator)
          subss   XMM2,[edx+14h]    ; IP.y - base.y
          movss   XMM5,XMM2         ; store result for later use
          mulss   XMM2,[edx+1Ch]    ; *edg1.x
          subss   XMM0,[edx+10h]    ; IP.x - base.x
          mulss   XMM0,[edx+20h]    ; *edg1.y
          subss   XMM0,XMM2         ; -
          mulss   XMM0,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM0,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM0    ; store v
          ; surface parameter u =  (IP.y - base.y - v*edg2.y) / edg1.y
          mulss   XMM0,[edx+2Ch]    ; v*edg2.y
          subss   XMM5,XMM0         ; subtract from stored result (IP.y - base.y)
          divss   XMM5,[edx+20h]    ; /edg1.y
          jmp    .endcase           ; break
.case4:   cmp    bx,4
          jne    .case5
	  ; surface parameter v = ((IP.y - base.y)*edg1.z - (IP.z - base.z)*edg1.y) * (1/denominator)
          subss   XMM3,[edx+18h]    ; IP.z - base.z
          movss   XMM5,XMM3         ; store result for later use
          mulss   XMM3,[edx+20h]    ; *edg1.y
          subss   XMM2,[edx+14h]    ; IP.y - base.y
          mulss   XMM2,[edx+24h]    ; *edg1.z
          subss   XMM2,XMM3         ; -
          mulss   XMM2,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM2,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM2    ; store v
          ; surface parameter u =  (IP.z - base.z - v*edg2.z) / edg1.z
          mulss   XMM2,[edx+30h]    ; v*edg2.z
          subss   XMM5,XMM2         ; subtract from stored result (IP.z - base.z)
          divss   XMM5,[edx+24h]    ; /edg1.z
          jmp    .endcase           ; break
.case5:   cmp    bx,5
          jne    near outside1
	  ; surface parameter v = ((IP.x - base.x)*edg1.z - (IP.z - base.z)*edg1.x) * (1/denominator)
          subss   XMM3,[edx+18h]    ; IP.z - base.z
          movss   XMM5,XMM3         ; store result for later use
          mulss   XMM3,[edx+1Ch]    ; *edg1.x
          subss   XMM0,[edx+10h]    ; IP.x - base.x
          mulss   XMM0,[edx+24h]    ; *edg1.z
          subss   XMM0,XMM3         ; -
          mulss   XMM0,[edx+44h]    ; *(1/denominator)
          ; exit if v<0
          comiss  XMM0,[fzero  ]
          jc      near outside1
          movss   [eax+34h],XMM0    ; store v
          ; surface parameter u =  (IP.z - base.z - v*edg2.z) / edg1.z
          mulss   XMM0,[edx+30h]    ; v*edg2.z
          subss   XMM5,XMM0         ; subtract from stored result (IP.z - base.z)
          divss   XMM5,[edx+24h]    ; /edg1.z

.endcase: comiss  XMM5,[fzero  ]    ; u<0 ?
          jc      outside1          ; then exit with negative result
          movss   [eax+30h],XMM5    ; store u      
          test    word [edx],0FFFFh ; are we a parallelogram ?
          jnz     .para             ; then goto .para else continue .triangl
.triangl: addss   XMM5, [eax+34h]   ; u+v
          comiss  XMM5, [fone   ]   ; >1 ?
          jnc     outside1          ; then exit with negative result
          ret                       ; else return positive
.para:    comiss  XMM5,[fone   ]    ; u>1 ?
          jnc     outside1          ; then exit with negative result
          movss   XMM0,[eax+34h]    ; v
          comiss  XMM0,[fone   ]    ; >1 ?
          jnc     outside1          ; then exit with negative result
          ret                       ; else return positive
outside1: mov     dword [eax+40h], 0BF800000h ; set intersection parameter t = -1 (no desirable intersection)
          ret


DispatchIntersectAll0:
          ; exit if primitive is only visible indirectly
          mov    cx,word [edx+2]
          test   cx,40h ; RV_OnlyIndirect
          jnz    near .exit1
          ; exit if primitive has already been tested against this ray
          mov    cx,word [eax+9Ch] ; Thread->CRay
          cmp    cx,word [edx+0Eh] ; p->lastray
          je     near .exit1
          mov     word [edx+0Eh],cx
          ; dispatch intersector by primitive type
          mov    cx,word [edx]
          test   cx,0FFFEh         ; 0 or 1
          jnz    .cont1
          call   IntersectTriPara0
          jmp    .exit2
.cont1:   cmp    cx,2
          jne    .cont2
          call   IntersectSimpleSphere0
          jmp    .exit2
.cont2:   cmp    cx,3
          jne    .cont3
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectCylinder0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont3:   cmp    cx,5
          jne    .cont4
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectSimpleRing0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont4:   cmp    cx,6
          jne    .cont5
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectTorus0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont5:   cmp    cx,7
          jne    .cont6
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectComplexSphere0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont6:   cmp    cx,8
          jne    .cont7
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectParaboloid0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont7:   cmp    cx,9
          jne    .cont8
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectEllipsoid0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.cont8:   cmp    cx,10
          jne    .exit1
          push   ecx
          push   edx
          push   eax
          call   _RV_IntersectComplexRing0
          pop    eax
          pop    edx
          pop    ecx
          jmp    .exit2
.exit1:   mov    dword [eax+40h], 0BF800000h 
.exit2:   ret

_RV_DispatchIntersectAll0:
          push   ebp
          mov    ebp,esp
          push   esi
          push   edi
          push   ebx
          mov    eax,dword [ebp+08h]
          mov    edx,dword [ebp+0Ch]
          call   DispatchIntersectAll0
          pop    ebx
          pop    edi
          pop    esi
          mov    esp,ebp
          pop    ebp
          ret

_RV_TraceList0:
          push   ebp
          mov    ebp,esp
          push   esi
          push   edi
          push   ebx
          ; PrLP p = (PrLP)tree->left;
          mov    eax,dword [ebp+0Ch]
          mov    ebx,dword [eax]
          mov    eax,dword [ebp+08h]
          ; while(p){
.loop:    cmp    ebx,0
          je     .exit
          push   ebx
          mov    edx,dword [ebx]
          call   DispatchIntersectAll0
          pop    ebx
          ; if(Thread->Cint.t>=0.0f){
          movss  XMM5,[eax+40h]
          comiss XMM5,[fzero  ]
          jc     .next
          ; if((Thread->Cmin.t<0.0f) || (Thread->Cint.t<Thread->Cmin.t)){ Thread->Cint.p = p->Prim; Thread->Cmin = Thread->Cint; }
          movss  XMM6,[eax+7Ch]
          comiss XMM6,[fzero  ]
          jc     .storHit
          comiss XMM5,XMM6
          jnc    .next
.storHit: ;mov    edx,dword [ebx]
          mov    dword [eax+44h],edx
          mov    ecx,0Fh
          lea    esi, [eax+10h]
          lea    edi, [eax+4Ch]
      rep movsd
.next:    mov    ebx,dword [ebx+4]
          jmp    .loop
.exit:    pop    ebx
          pop    edi
          pop    esi
          mov    esp,ebp
          pop    ebp
          ret


_RV_IntersectBoundingBox0:
          push   ebp
          mov    ebp,esp
          push   ebx
          mov    eax,dword [ebp+08h]

          ; is precalculation already done for this frame?
          mov    cx,word [_cframe]       ; current frame
          cmp    cx,word [_BBlastframe]  ; = last precalculated frame?
          je     near .samefrm           ; then skip precalculation
          mov    word [_BBlastframe],cx  ; else store current frame number and precalculate:

          ; Find candidate planes
          mov    word [_BBinside],1
          mov    bx,2
          movss  XMM0,[_RV_Cam]
          comiss XMM0,[_RV_BBMin]
          jnc    .p1
          mov    ecx, dword [_RV_BBMin]
          mov    bx,1
          jmp    .f1
.p1:      comiss XMM0,[_RV_BBMax]
          jc     .p2
          mov    ecx, dword [_RV_BBMax]
          mov    bx,0
.f1:      mov    dword [_BBcandidatePlane],ecx
          mov    word [_BBinside],0
.p2:      mov    word [_BBquadrant],bx

          mov    bx,2
          movss  XMM0,[_RV_Cam+4]
          comiss XMM0,[_RV_BBMin+4]
          jnc    .p3
          mov    ecx, dword [_RV_BBMin+4]
          mov    bx,1
          jmp    .f2
.p3:      comiss XMM0,[_RV_BBMax+4]
          jc     .p4
          mov    ecx, dword [_RV_BBMax+4]
          mov    bx,0
.f2:      mov    dword [_BBcandidatePlane+4],ecx
          mov    word [_BBinside],0
.p4:      mov    word [_BBquadrant+2],bx

          mov    bx,2
          movss  XMM0,[_RV_Cam+8]
          comiss XMM0,[_RV_BBMin+8]
          jnc    .p5
          mov    ecx, dword [_RV_BBMin+8]
          mov    bx,1
          jmp    .f3
.p5:      comiss XMM0,[_RV_BBMax+8]
          jc     .p6
          mov    ecx, dword [_RV_BBMax+8]
          mov    bx,0
.f3:      mov    dword [_BBcandidatePlane+8],ecx
          mov    word [_BBinside],0
.p6:      mov    word [_BBquadrant+4],bx

.samefrm: ; exit if camera is inside bounding box
          test   word [_BBinside], 0FFFFh
          jnz    near .inside
          
          ; calculate distances to candidate planes
          movss  XMM1,[fminus1]
          mov    bx, word [_BBquadrant  ]
          cmp    bx,2
          je     .s1
          movss  XMM1,[_BBcandidatePlane  ]
          subss  XMM1,[_RV_Cam    ]
          divss  XMM1,[    eax    ] ; Ray.x
.s1:      movss  XMM2,[fminus1]
          mov    bx, word [_BBquadrant+2]
          cmp    bx,2
          je     .s2
          movss  XMM2,[_BBcandidatePlane+4]
          subss  XMM2,[_RV_Cam+04h]
          divss  XMM2,[    eax+04h] ; Ray.y
.s2:      movss  XMM3,[fminus1]
          mov    bx, word [_BBquadrant+4]
          cmp    bx,2
          je     .s3
          movss  XMM3,[_BBcandidatePlane+8]
          subss  XMM3,[_RV_Cam+08h]
          divss  XMM3,[    eax+08h] ; Ray.z
.s3:      
          ; get largest of the maxT's for final choice of intersection
          mov    bx,0
          comiss XMM1,XMM2
          jnc    .c1
          mov    bx,1
          movss  XMM1,XMM2
.c1:      comiss XMM1,XMM3
          jnc    .c2
          mov    bx,2
          movss  XMM1,XMM3
.c2:      movss  [eax+40h],XMM1

          ; exit if t is negative (intersection is behind camera)
          comiss XMM1,[fzero  ]
          jc     near .NoHit

          cmp    bx,0
          je     .skip1
          ; intersection point IP.x = Camera.x + t * Ray.x
          movss  XMM0,[    eax    ] ; Ray.x
          mulss  XMM0,XMM1
          addss  XMM0,[_RV_Cam    ]
          movss  [eax+10h],XMM0
          ; exit if point is outside box
          comiss XMM0,[_RV_BBMin  ]
          jc     near .NoHit
          comiss XMM0,[_RV_BBMax  ]
          jnc    near .NoHit
          jmp    .cont1
.skip1:   mov    ecx, dword [_BBcandidatePlane  ]
          mov    [eax+10h],ecx
.cont1:   cmp    bx,1
          je     .skip2
          ; intersection point IP.y = Camera.y + t * Ray.y
          movss  XMM0,[    eax+04h] ; Ray.y
          mulss  XMM0,XMM1
          addss  XMM0,[_RV_Cam+04h]
          movss  [eax+14h],XMM0
          ; exit if point is outside box
          comiss XMM0,[_RV_BBMin+4]
          jc     near .NoHit
          comiss XMM0,[_RV_BBMax+4]
          jnc    near .NoHit
          jmp    .cont2
.skip2:   mov    ecx, dword [_BBcandidatePlane+4]
          mov    [eax+14h],ecx
.cont2:   cmp    bx,2
          je     .skip3
          ; intersection point IP.z = Camera.z + t * Ray.z
          movss  XMM0,[    eax+08h] ; Ray.z
          mulss  XMM0,XMM1
          addss  XMM0,[_RV_Cam+08h]
          movss  [eax+18h],XMM0
          ; exit if point is outside box
          comiss XMM0,[_RV_BBMin+8]
          jc     .NoHit
          comiss XMM0,[_RV_BBMax+8]
          jnc    .NoHit
          jmp    .cont3
.skip3:   mov    ecx, dword [_BBcandidatePlane+8]
          mov    [eax+18h],ecx
.cont3:   pop    ebx
          mov    esp,ebp
          pop    ebp
          ret
.NoHit:   ;exit with negative return value
          mov    dword [eax+40h],0BF800000h
          pop    ebx
          mov    esp,ebp
          pop    ebp
          ret
.inside:  mov    ebx,[_RV_Cam  ]
          mov    [eax+10h],ebx
          mov    ecx,[_RV_Cam+4]
          mov    [eax+14h],ecx
          mov    edx,[_RV_Cam+8]
          mov    [eax+18h],edx
          mov    dword [eax+40h],03F800000h
          pop    ebx
          mov    esp,ebp
          pop    ebp
          ret

; ebx: +14h *CInt
; edx: +10h *Ray
; eax: +0Ch *Org
; ecx: +08h *Sphere

IntersectSimpleSphere1:
          ; XMM 0,1,2 = Org - position;
          ; XMM4 = sqr(XMM0)+sqr(XMM1)+sqr(XMM2) - r2;
          mov     eax,dword [ebp+0Ch]
          mov     ecx,dword [ebp+08h]
          mov     edx,dword [ebp+10h]
          movss   XMM4,[eax    ]
          subss   XMM4,[ecx+10h]
          movss   XMM0,XMM4
          movss   XMM5,[eax+04h]
          subss   XMM5,[ecx+14h]
          movss   XMM1,XMM5
          mulss   XMM4,XMM4
          movss   XMM6,[eax+08h]
          subss   XMM6,[ecx+18h]
          movss   XMM2,XMM6
          mulss   XMM5,XMM5
          mulss   XMM6,XMM6
          addss   XMM4,XMM5
          subss   XMM4,[ecx+28h]
          addss   XMM4,XMM6 
          ; XMM1 = Ray.x * tmpvec1.x + Ray.y * tmpvec1.y + Ray.z * tmpvec1.z
          movss   XMM3,[edx    ]
          mulss   XMM3,XMM0
          movss   XMM5,[edx+04h]
          mulss   XMM5,XMM1
          addss   XMM3,XMM5
          movss   XMM6,[edx+08h]
          mulss   XMM6,XMM2
          addss   XMM3,XMM6
          movss   XMM1,XMM3
          ; XMM3 = XMM1*XMM1 - XMM4
          mulss   XMM3,XMM3
          subss   XMM3,XMM4
          ; return negative if XMM3<0 (Ray missing Sphere)
          mov     ebx,dword [ebp+14h]
          comiss  XMM3,[fzero  ]
          jc      .NoHit
          ; XMM0  = -XMM1 + sqrt(XMM3)
          ; XMM1  = -XMM1 - sqrt(XMM3)
          mulss   XMM1,[fminus1]
          sqrtss  XMM3,XMM3
          movss   XMM0,XMM1
          subss   XMM1,XMM3
          addss   XMM0,XMM3
          ; return smaller XMM, where XMM is not too small
          comiss  XMM0,XMM1
          jnc     .ct
          comiss  XMM0,[fsmall]
          jc      .s1
.s0:      movss   [ebx+30h],XMM0
          ret
.ct:      comiss  XMM1,[fsmall]
          jc      .s0
.s1:      movss   [ebx+30h],XMM1
          ret
.NoHit:   mov     dword [ebx+30h],0BF800000h
          ret


_RV_DispatchIntersectAll1:
          push   ebp
          mov    ebp,esp
          sub    esp,40h
          push   ebx
          push   esi
          push   edi
          ; dispatch intersector by primitive type
          mov    edx,dword [ebp+8]
          mov    cx,word [edx]
          test   cx,0FFFEh         ; 0 or 1
          jnz    .cont1
          call   IntersectTriPara1
          jmp    .exit2
.cont1:   cmp    cx,2
          jne    .cont2
          call   IntersectSimpleSphere1
          jmp    .exit2
.cont2:   cmp    cx,3
          jne    .cont3
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectCylinder1
          add    esp,16
          jmp    .exit2
.cont3:   cmp    cx,5
          jne    .cont4
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectSimpleRing1
          add    esp,16
          jmp    .exit2
.cont4:   cmp    cx,6
          jne    .cont5
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectTorus1
          add    esp,16
          jmp    .exit2
.cont5:   cmp    cx,7
          jne    .cont6
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectComplexSphere1
          add    esp,16
          jmp    .exit2
.cont6:   cmp    cx,8
          jne    .cont7
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectParaboloid1
          add    esp,16
          jmp    .exit2
.cont7:   cmp    cx,9
          jne    .cont8
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectEllipsoid1
          add    esp,16
          jmp    .exit2
.cont8:   cmp    cx,10
          jne    .exit1
          mov    eax,dword [ebp+14h]
          push   eax
          mov    edx,dword [ebp+10h]
          push   edx
          mov    eax,dword [ebp+0Ch]
          push   eax
          mov    ecx,dword [ebp+08h]
          push   ecx
          call   _RV_IntersectComplexRing1
          add    esp,16
          jmp    .exit2
.exit1:   mov    edx,dword [ebp+14h]
          mov    dword [edx+30h], 0BF800000h 
.exit2:   pop    edi
          pop    esi
          pop    ebx
          add    esp,40h
          mov    esp,ebp
          pop    ebp
          ret
