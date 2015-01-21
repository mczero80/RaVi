
#include "stdafx.h"
#include "general.h"
#include "Primitives.h"
#include "VModelFile.h"
//#include "GraphWin.h"
#ifdef VModel
  #include "..\VModel\TextWin.h"
#endif

#ifndef VModel
  extern int       HitBuffers;
  extern int       DiffDepth;
  extern int       SpecLightDepth;
  extern int       SpecViewDepth;
#endif

//#include "gl\glu.h"

extern "C" {
  #include "OpenImage.h"
}

//extern void  DrawObj(PrimPtr p);
extern void  RFTrans(matrix *M, vector *S, angles *R, vector *T);

#define MaxTokenLength 160

int   FEOF;

View::View(void){
  next=NULL; name=NULL;
  Location = vector(10.0f, 10.0f, 10.0f);
  Target   = vector( 0.0f,  0.0f,  0.0f);
  Up       = vector( 0.0f,  0.0f,  1.0f);
  h=0.82842712f; Zoom=1.0f;
}

void View::LookAt(vector v)
{
  Target=v;
}

void VModelFile::Init(void)
{
  MatList=NULL; TexList=NULL; ViewList=NULL;
  
  Background.R=0.0F; Background.G=0.0F; Background.B=0.0F;
  Ambient.R   =0.3F; Ambient.G   =0.3F; Ambient.B   =0.3F;
  
  Object[0]=0; Author[0]=0; Date[0]=0;
  
  Sky = NULL; Base = NULL; Hidden = NULL;
}

void VModelFile::Kill(void)
{
  MatPtr     mp;
  TexMapPtr  tp;
  ViewPtr    vp;

  #ifdef VModel
    Base->Kill(); Hidden->Kill();
  #endif

  while(MatList){
    mp=MatList; MatList = MatList->next;
    delete(mp);
  }
  while(TexList){
    tp=TexList; TexList = TexList->next;
    if(tp->tbits ) QuitImage(tp->tbits);
    delete(tp);
  }
  while(ViewList){
    vp=ViewList; ViewList = ViewList->next;
    if(vp->name) delete(vp->name); delete(vp);
  }
}

void GetToken(char *a, CFile *file)
{
  char c; int i,n=0;
  do{
    while( n==1 && c!=13 && c!=10){
	  n=file->Read(&c,1);
	} i=0;
    do n=file->Read(&c,1); while( n==1 && (c==' ' || c==',' || c==':' || c=='=' || c==';' || c=='\t' || c=='(' || c==')' || c==13 || c==10 ));
    if(n==1){
      if(c=='\"'){
	    n=file->Read(&c,1);
	    while(n==1 && (c!='\"' && c!=13 && c!=10)){
	      if(i<MaxTokenLength) a[i]=c; i++; n=file->Read(&c,1);
	    }
      } else {
	    do{ if(i<MaxTokenLength) a[i]=c; i++;
		  n=file->Read(&c,1);
		}while( n==1 && (c!=' ' && c!=',' && c!='=' && c!=';' && c!=':' && c!='\t' && c!='(' && c!=')' && c!=13 && c!=10));
      }
    }
    a[min(MaxTokenLength-1,i)]=0;
  } while(a[0]=='!');
  FEOF = n==1? 0 : 1;
}

void GetLine(char *a, CFile *file)
{
  char c; int n,i=0;
  do n=file->Read(&c,1); while( n==1 && (c==' ' || c==',' || c==':' || c=='='));
  if (n==1)
    do{ if(i<MaxTokenLength) a[i]=c; i++; n=file->Read(&c,1); }while( n==1 && c!=13 && c!=10);
  a[min(MaxTokenLength-1,i)]=0;
  FEOF = n==1? 0 : 1;
}

void GetMaterial(char *a, CFile *file, Surface *p, MatPtr mp)
{
  GetToken(a,file);
  while(((mp->next)!=NULL)&&(strcmp(mp->name,a)!=0)) mp=mp->next;
  #ifdef VModel
    if(strcmp(mp->name,a)!=0) TextWin.Output(3,"- Material '%s' nicht gefunden!\n",a);
  #endif
  p->Mat=mp;
}

void GetTransformation(char *a, CFile *file, Surface *p)
{
  // default-transformation:
  vector scl=vector(1.0F,1.0F,1.0F);
  angles rot=angles(0.0F,0.0F,0.0F);
  vector pos=vector(0.0F,0.0F,0.0F);
  BOOL   BMx=TRUE; float tmp;
  do{
    if(!stricmp(a,"TRANSFORM")) {
      GetToken(a,file); p->TM.M11=(float)atof(a);
      GetToken(a,file); p->TM.M12=(float)atof(a);
      GetToken(a,file); p->TM.M13=(float)atof(a);
      GetToken(a,file); p->TM.M21=(float)atof(a);
      GetToken(a,file); p->TM.M22=(float)atof(a);
      GetToken(a,file); p->TM.M23=(float)atof(a);
      GetToken(a,file); p->TM.M31=(float)atof(a);
      GetToken(a,file); p->TM.M32=(float)atof(a);
      GetToken(a,file); p->TM.M33=(float)atof(a);
      GetToken(a,file); p->TM.M41=(float)atof(a);
      GetToken(a,file); p->TM.M42=(float)atof(a);
      GetToken(a,file); p->TM.M43=(float)atof(a);
      GetToken(a,file); BMx=FALSE;
    }
    if(!stricmp(a,"SCALE")) {
      GetToken(a,file); scl.x=(float)atof(a);
      GetToken(a,file); scl.y=(float)atof(a);
      GetToken(a,file); scl.z=(float)atof(a);
      GetToken(a,file);
    }
    if(!stricmp(a,"ROTATE")) {
      GetToken(a,file); rot.x=Deg2Rad*(float)atof(a);
      GetToken(a,file); rot.y=Deg2Rad*(float)atof(a);
      GetToken(a,file); rot.z=Deg2Rad*(float)atof(a);
      GetToken(a,file);
    }
    if(!stricmp(a,"TRANSLATE")) {
      GetToken(a,file); pos.x=(float)atof(a);
      GetToken(a,file); pos.y=(float)atof(a);
      GetToken(a,file); pos.z=(float)atof(a);
      GetToken(a,file);
    }
  }while( (!stricmp(a,"TRANSFORM")) || (!stricmp(a,"SCALE")) || (!stricmp(a,"ROTATE")) || (!stricmp(a,"TRANSLATE")) );
  if(!BMx) RFTrans(&(p->TM), &scl, &rot, &pos);
  tmp=scl.y; scl.y= scl.z; scl.z=tmp;
  tmp=rot.y; rot.y=-rot.z; rot.z=tmp;
  tmp=pos.y; pos.y=-pos.z; pos.z=tmp;
  matrix Sc; Sc.InitTransformation(scl.x, scl.y, scl.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  matrix Rx; Rx.InitTransformation(1.0f, 1.0f, 1.0f, rot.x, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  matrix Ry; Ry.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, rot.y, 0.0f, 0.0f, 0.0f, 0.0f);
  matrix Rz; Rz.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, rot.z, 0.0f, 0.0f, 0.0f);
  matrix Ps; Ps.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, pos.x, pos.y, pos.z);
  p->TM = Sc*Rz*Rx*Ry*Ps;
  
  p->Tb = p->TM.Inverse(); // calculate backtransformation matrix
  p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
}

float ReadAngle(char *a)
{
  if(a[strlen(a)-1]=='°'){
	a[strlen(a)-1]=0;
	return Deg2Rad*(float)atof(a);
  } else {
	return         (float)atof(a);
  }
}

void VModelFile::ParseSurface(SurfPtr p, CFile *file, char* typ)
{
  char a[MaxTokenLength];

  p->succ=Base; if(Base) Base->pred=p; Base=p;

  GetToken(a,file);
  if(*a != '{'){
	p->name = strdup(a);
    GetToken(a,file); // read bracket
  }
  do{
    GetToken(a,file);
	if(p->ParseToken(a,file)){
      if(!stricmp(a,"Material")) {
        GetMaterial(a, file, p, MatList);
      } else if(!stricmp(a,"Inverse")){
	    p->flags |= SURF_INVERSE;
      } else if(!stricmp(a,"Distribution")){
	    GetToken(a,file);
	    if(!stricmp(a,"cos_z")){
	      p->flags |= SURF_COS_Z;
        } else {
          #ifdef VModel
		  TextWin.Output(3,"- unknown Distribution type: '%s'!\n",a);
          #endif
		}
      } else if(!stricmp(a,"Visible")){
	    GetToken(a,file);
		if(atoi(a)<0) p->flags |= SURF_INVISIBLE;
		if(atoi(a)<1) p->flags |= SURF_ONLY_INDIRECT;
	  } else if(!stricmp(a,"Transform")) {
		matrix M;
        GetToken(a,file); M.M11=(float)atof(a);
        GetToken(a,file); M.M12=(float)atof(a);
        GetToken(a,file); M.M13=(float)atof(a);
        GetToken(a,file); M.M21=(float)atof(a);
        GetToken(a,file); M.M22=(float)atof(a);
        GetToken(a,file); M.M23=(float)atof(a);
        GetToken(a,file); M.M31=(float)atof(a);
        GetToken(a,file); M.M32=(float)atof(a);
        GetToken(a,file); M.M33=(float)atof(a);
        GetToken(a,file); M.M41=(float)atof(a);
        GetToken(a,file); M.M42=(float)atof(a);
        GetToken(a,file); M.M43=(float)atof(a);
		p->Transform(M);
	  } else if(!stricmp(a,"Translate")) {
		vector t;
	    GetToken(a,file); t.x = (float)atof(a);
	    GetToken(a,file); t.y = (float)atof(a);
	    GetToken(a,file); t.z = (float)atof(a);
		p->Translate(t);
      } else if(!stricmp(a,"Rotate_X")) {
        GetToken(a,file); p->RotateX(ReadAngle(a));
      } else if(!stricmp(a,"Rotate_Y")) {
        GetToken(a,file); p->RotateY(ReadAngle(a));
      } else if(!stricmp(a,"Rotate_Z")) {
        GetToken(a,file); p->RotateZ(ReadAngle(a));
      } else if(!stricmp(a,"Stretch")) {
        GetToken(a,file); float x=(float)atof(a);
		GetToken(a,file); float y=(float)atof(a);
		GetToken(a,file); float z=(float)atof(a);
		p->Scale(vector(x, y, z));
      } else if(!stricmp(a,"Scale")) {
        GetToken(a,file); p->Scale(vector((float)atof(a), (float)atof(a), (float)atof(a)));
      } else if(!stricmp(a,"TexCoords2D")) {
		p->txc=new vector[4];
        GetToken(a,file); p->txc[0].x = (float)atof(a); GetToken(a,file); p->txc[0].y = (float)atof(a); p->txc[0].z = 0.0f;
        GetToken(a,file); p->txc[1].x = (float)atof(a); GetToken(a,file); p->txc[1].y = (float)atof(a); p->txc[1].z = 0.0f;
        GetToken(a,file); p->txc[2].x = (float)atof(a); GetToken(a,file); p->txc[2].y = (float)atof(a); p->txc[2].z = 0.0f;
        GetToken(a,file); p->txc[3].x = (float)atof(a); GetToken(a,file); p->txc[3].y = (float)atof(a); p->txc[3].z = 0.0f;
	  } else {
        #ifdef VModel
          if(*a != '}') TextWin.Output(3,"- unknown %s-Property: '%s'!\n",typ,a);
        #endif
	  }
	}
  } while(!FEOF && (*a != '}'));
  p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
  // Draw object to screen
//!  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish(); WaitTask(NULL);
}

void VModelFile::Load(char *filename)
{
  char   a[MaxTokenLength];
  char   CView[40]="";
  float  fileversion;
  int    vn;
  CFile  file;
  DWORD  fpos;
  
  if(file.Open(filename, CFile::modeRead | CFile::shareDenyWrite)){
    GetToken(a,&file);
    if(!stricmp(a,"VModel")){
      GetToken(a,&file); GetToken(a,&file); fileversion=(float)atof(a);
	  if(fileversion<0.6f){
	  GetToken(a,&file);
      do{
        if(!strcmp(a,"OBJECT")){
          GetLine(a,&file); strcpy(Object,a);
          #ifdef VModel
		    TextWin.Output(1,"Objekt: %s\n",a);
          #endif
          GetToken(a,&file);
        }
        if(!strcmp(a,"AUTHOR")){
          GetLine(a,&file); strcpy(Author,a);
          #ifdef VModel
		    TextWin.Output(1,"Autor : %s\n",a);
          #endif
          GetToken(a,&file);
        }
        if(!strcmp(a,"DATE")){
          GetLine(a,&file); strcpy(Date,a);
          #ifdef VModel
		    TextWin.Output(1,"Datum : %s\n",a);
          #endif
          GetToken(a,&file);
        }
        if(!strcmp(a,"BACKGROUND")){
          GetToken(a,&file); Background.R = (float)atof(a);
          GetToken(a,&file); Background.G = (float)atof(a);
          GetToken(a,&file); Background.B = (float)atof(a);
          GetToken(a,&file);
        }
        if(!strcmp(a,"AMBIENT")){
          GetToken(a,&file); Ambient.R = (float)atof(a);
          GetToken(a,&file); Ambient.G = (float)atof(a);
          GetToken(a,&file); Ambient.B = (float)atof(a);
          GetToken(a,&file);
        }
        if(!strcmp(a,"SKY")){
          GetToken(a,&file);
		  TexMapPtr t = TexList; while(t && stricmp(a, t->tname)) t=t->next;
		  if(!t){
		    t = new TexMap; t->next=TexList; TexList=t;
		    t->tname=strdup(a); t->tbits=NULL; t->tflags=0;
		  }
		  Sky_map=t;
          Sky = new Sphere;
          Sky->name   = NULL;
		  Sky->Mat    = NULL;
          Sky->flags  = SURF_INVERSE;
          Sky->ustart = 0.0f;

		  Sky->txc=new vector[4];
          Sky->txc[0] = vector(2.0f, 0.0f, 0.0f);
          Sky->txc[1] = vector(0.0f, 0.0f, 0.0f);
          Sky->txc[2] = vector(2.0f, 1.0f, 0.0f);
          Sky->txc[3] = vector(0.0f, 1.0f, 0.0f);

          GetToken(a,&file);
        }
        if(!strcmp(a,"VIEW")){
          ViewPtr V = new View; V->next=ViewList; ViewList=V;
          GetToken(a,&file); // View-Name
          V->name = new char[strlen(a)+1]; strcpy(V->name,a);
          GetToken(a,&file);

          // default-view:
          V->Zoom = 1.0F;
          V->Locate(vector(0.0F,0.0F,10.0F));
          V->LookAt(vector(0.0F,0.0F, 0.0F)); V->Up = vector(0.0f, 1.0f, 0.0f);
          V->SetViewAngle (0.5F);
          //V->Tilt(0.0F);

          do{
            if(!strcmp(a,"LOCATION")){
              float px,py,pz;
              GetToken(a,&file); px=(float)atof(a);
              GetToken(a,&file); py=(float)atof(a);
              GetToken(a,&file); pz=(float)atof(a);
              V->Locate(vector(px,-pz,py));
              GetToken(a,&file);
            }
            if(!strcmp(a,"TARGET")) {
              float px,py,pz;
              GetToken(a,&file); px=(float)atof(a);
              GetToken(a,&file); py=(float)atof(a);
              GetToken(a,&file); pz=(float)atof(a);
              V->LookAt(vector(px,-pz,py));
              GetToken(a,&file);
            }
            if(!strcmp(a,"UP")) {
              float px,py,pz;
              GetToken(a,&file); px=(float)atof(a);
              GetToken(a,&file); py=(float)atof(a);
              GetToken(a,&file); pz=(float)atof(a);
              V->Up=vector(px,-pz,py);
              GetToken(a,&file);
            }
            if(!strcmp(a,"VIEW-ANGLE")) {
              GetToken(a,&file); V->SetViewAngle(Deg2Rad*(float)atof(a));
              GetToken(a,&file);
            }
            if(!strcmp(a,"TILT-ANGLE")) {
              GetToken(a,&file); //V->Tilt(Deg2Rad*(float)atof(a));
              GetToken(a,&file);
            }
            if(!strcmp(a,"ZOOM")) {
              GetToken(a,&file); //V->Zoom=(float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"PAN")) {
              GetToken(a,&file);
              GetToken(a,&file);
              GetToken(a,&file);
            }
          }while( (!strcmp(a,"LOCATION")) || (!strcmp(a,"TARGET")) || (!strcmp(a,"UP")) || (!strcmp(a,"VIEW-ANGLE")) || (!strcmp(a,"TILT-ANGLE")) || (!strcmp(a,"ZOOM")) || (!strcmp(a,"PAN")) );
          // up-vector has not to be given exact
		  vector N = unit(V->Target - V->Location); V->Up = unit((V->Up)-((V->Up)%N)*N);
          if(!strcmp(V->name,"*")){
            delete(V->name); V->name = NULL;
            CV = *V;
            ViewList=V->next; delete(V);
//!            VMDoc->WMP = V->Target; VMDoc->WRD = 2.0f*Betrag(V->Target - V->Location);
//!            GraphWin.InvalidateRect(NULL,FALSE); WaitTask(NULL);
          }
        }
        if(!strcmp(a,"MATERIAL")){
          MatPtr mat=new Material; mat->type=Shader_Phong;
          GetToken(a,&file); mat->name = strdup(a);

          GetToken(a,&file);
          do{
            if(!strcmp(a,"DIFFUSE")) {
              GetToken(a,&file); float Kd = (float)atof(a);
              GetToken(a,&file);
              if(!strcmp(a,"COLOR")) {
                GetToken(a,&file); mat->Reflection.R = Kd*(float)atof(a);
                GetToken(a,&file); mat->Reflection.G = Kd*(float)atof(a);
                GetToken(a,&file); mat->Reflection.B = Kd*(float)atof(a);
                GetToken(a,&file);
              }
            }
            if(!strcmp(a,"LUMINANCE")) {
              GetToken(a,&file); float Kl = (float)atof(a);
              GetToken(a,&file);
              if(!strcmp(a,"COLOR")) {
                GetToken(a,&file); mat->Emission.R = Kl*(float)atof(a);
                GetToken(a,&file); mat->Emission.G = Kl*(float)atof(a);
                GetToken(a,&file); mat->Emission.B = Kl*(float)atof(a);
                GetToken(a,&file);
              }
            }
            if(!strcmp(a,"SPECULAR")) {
              GetToken(a,&file); mat->Specularity = (float)atof(a);
              GetToken(a,&file);
              if(!strcmp(a,"COLOR")) {
                GetToken(a,&file); //mat->Os.R = (float)atof(a);
                GetToken(a,&file); //mat->Os.G = (float)atof(a);
                GetToken(a,&file); //mat->Os.B = (float)atof(a);
                GetToken(a,&file);
              }
              if(!strcmp(a,"EXP")) {
                GetToken(a,&file); mat->PBExp = (float)atof(a);
                GetToken(a,&file);
              }
            }
            if(!strcmp(a,"TRANSPARENCY")) {
              GetToken(a,&file); float Kt = (float)atof(a);
              GetToken(a,&file);
              if(!strcmp(a,"COLOR")) {
                GetToken(a,&file); mat->Transmission.R = Kt*(float)atof(a);
                GetToken(a,&file); mat->Transmission.G = Kt*(float)atof(a);
                GetToken(a,&file); mat->Transmission.B = Kt*(float)atof(a);
                GetToken(a,&file);
              }
              if(!strcmp(a,"IOR")) {
                GetToken(a,&file); mat->IOR_base = (float)atof(a);
                GetToken(a,&file);
              }
            }
            if(!strcmp(a,"REFLECTIVITY")) {
              GetToken(a,&file); //mat->Kr = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"ROUGHNESS")) {
              GetToken(a,&file); //mat->R  = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"TEXTURE")) {
			  GetToken(a,&file);
			  TexMapPtr t = TexList; while(t && stricmp(a, t->tname)) t=t->next;
			  if(!t){
			    t = new TexMap; t->next=TexList; TexList=t;
			    t->tname=strdup(a); t->tbits=NULL; t->tflags=0;
			  }
			  mat->Texture=t;
			  GetToken(a,&file);
            }
          }while( (!strcmp(a,"DIFFUSE")) || (!strcmp(a,"SPECULAR")) || (!strcmp(a,"TRANSPARENCY")) || (!strcmp(a,"LUMINANCE")) || (!strcmp(a,"REFLECTIVITY")) || (!strcmp(a,"ROUGHNESS")) || (!strcmp(a,"TEXTURE")) );
          // append new material to list
          if(MatList==NULL) MatList=mat;
          else { MatPtr p=MatList;
                 while((p->next)!=NULL) p=p->next;
                 p->next = mat;
               }
        }
        if (!strcmp(a,"CUBE")){
          Cube *p = new Cube; p->succ=Base; if(Base) Base->pred=p; Base=p;

          GetMaterial(a,&file,p,MatList);
          GetToken(a,&file); // Object-Name
          p->name = new char[strlen(a)+1]; strcpy(p->name,a);
          GetToken(a,&file);
          GetTransformation(a,&file,p);
          // Draw object to screen
//!		  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish();
        }
        if (!strcmp(a,"CYLINDER")){
          Cylinder *p = new Cylinder; p->succ=Base; if(Base) Base->pred=p; Base=p;

          GetMaterial(a,&file,p,MatList);
          GetToken(a,&file); // Object-Name
          p->name = new char[strlen(a)+1]; strcpy(p->name,a);
          GetToken(a,&file);
          if(!strcmp(a,"R_TOP")){
            GetToken(a,&file); p->r_top = (float)atof(a);
            GetToken(a,&file);
          }
          if(!strcmp(a,"ANGLE")){
            GetToken(a,&file); p->angle = (float)atof(a);
            GetToken(a,&file);
          }
          if(!strcmp(a,"TCOORDS2D")){
			p->txc=new vector[4];
            GetToken(a,&file); p->txc[0].x = (float)atof(a); GetToken(a,&file); p->txc[0].y = (float)atof(a); p->txc[0].z = 0.0f;
            GetToken(a,&file); p->txc[1].x = (float)atof(a); GetToken(a,&file); p->txc[1].y = (float)atof(a); p->txc[1].z = 0.0f;
            GetToken(a,&file); p->txc[2].x = (float)atof(a); GetToken(a,&file); p->txc[2].y = (float)atof(a); p->txc[2].z = 0.0f;
            GetToken(a,&file); p->txc[3].x = (float)atof(a); GetToken(a,&file); p->txc[3].y = (float)atof(a); p->txc[3].z = 0.0f;
            GetToken(a,&file);
          }
          GetTransformation(a,&file,p);
		  if(p->angle){
            matrix t; t.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, p->angle, 0.0f, 0.0f, 0.0f);
            p->TM = t*(p->TM);
            p->Tb = p->TM.Inverse(); // calculate backtransformation matrix
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
		  }
          // Draw object to screen
//!		  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish();
        }
        if (!strcmp(a,"SPHERE")){
          Sphere *p = new Sphere; p->succ=Base; if(Base) Base->pred=p; Base=p;

          GetMaterial(a,&file,p,MatList);
          GetToken(a,&file); // Object-Name
          p->name = new char[strlen(a)+1]; strcpy(p->name,a);
          GetToken(a,&file);
          if(!strcmp(a,"DOUBLESIDED")){ p->flags |= SURF_DOUBLESIDED; GetToken(a,&file); }
          if(!strcmp(a,"INVERSE")){ p->flags |= SURF_INVERSE; GetToken(a,&file); }
          do{
            if(!strcmp(a,"ANGLE")){
              GetToken(a,&file); p->vangle = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"START")){
              GetToken(a,&file); p->ustart = (float)atof(a);
			  if(!(p->vangle))   p->vangle = TwoPi;
              GetToken(a,&file);
            }
            if(!strcmp(a,"STOP" )){
              GetToken(a,&file); p->ustop  = (float)atof(a);
			  if(!(p->vangle))   p->vangle = TwoPi;
              GetToken(a,&file);
            }
            if(!strcmp(a,"TCOORDS2D")){
			  p->txc=new vector[4];
              GetToken(a,&file); p->txc[0].x = (float)atof(a); GetToken(a,&file); p->txc[0].y = (float)atof(a); p->txc[0].z = 0.0f;
              GetToken(a,&file); p->txc[1].x = (float)atof(a); GetToken(a,&file); p->txc[1].y = (float)atof(a); p->txc[1].z = 0.0f;
              GetToken(a,&file); p->txc[2].x = (float)atof(a); GetToken(a,&file); p->txc[2].y = (float)atof(a); p->txc[2].z = 0.0f;
              GetToken(a,&file); p->txc[3].x = (float)atof(a); GetToken(a,&file); p->txc[3].y = (float)atof(a); p->txc[3].z = 0.0f;
              GetToken(a,&file);
            }
          }while( (!strcmp(a,"ANGLE")) || (!strcmp(a,"START")) || (!strcmp(a,"STOP")) || (!strcmp(a,"TCOORDS2D")) );
          GetTransformation(a,&file,p);
		  if(p->vangle){
            matrix t; t.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, p->vangle, 0.0f, 0.0f, 0.0f);
            p->TM = t*(p->TM);
            p->Tb = p->TM.Inverse(); // calculate backtransformation matrix
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
		  }
          // Draw object to screen
//!		  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish();
        }
        if (!strcmp(a,"TORUS")){
          Torus *p = new Torus; p->succ=Base; if(Base) Base->pred=p; Base=p;

          GetMaterial(a,&file,p,MatList);
          GetToken(a,&file); // Object-Name
          p->name = new char[strlen(a)+1]; strcpy(p->name,a);
          GetToken(a,&file);
          do{
            if(!strcmp(a,"DOUBLESIDED")){ p->flags |= SURF_DOUBLESIDED; GetToken(a,&file); }
            if(!strcmp(a,"INVERSE")){ p->flags |= SURF_INVERSE; GetToken(a,&file); }
            if(!strcmp(a,"ANGLE")){
              GetToken(a,&file); p->vangle = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"START")){
              GetToken(a,&file); p->ustart = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"STOP" )){
              GetToken(a,&file); p->ustop  = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"R1")){
              GetToken(a,&file); p->R1     = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"R2" )){
              GetToken(a,&file); p->R2     = (float)atof(a);
              GetToken(a,&file);
            }
            if(!strcmp(a,"TCOORDS2D")){
			  p->txc=new vector[4];
              GetToken(a,&file); p->txc[2].x = (float)atof(a); GetToken(a,&file); p->txc[2].y = (float)atof(a); p->txc[2].z = 0.0f;
              GetToken(a,&file); p->txc[0].x = (float)atof(a); GetToken(a,&file); p->txc[0].y = (float)atof(a); p->txc[0].z = 0.0f;
              GetToken(a,&file); p->txc[3].x = (float)atof(a); GetToken(a,&file); p->txc[3].y = (float)atof(a); p->txc[3].z = 0.0f;
              GetToken(a,&file); p->txc[1].x = (float)atof(a); GetToken(a,&file); p->txc[1].y = (float)atof(a); p->txc[1].z = 0.0f;
              GetToken(a,&file);
            }
          }while( (!strcmp(a,"ANGLE")) || (!strcmp(a,"START")) || (!strcmp(a,"STOP")) || (!strcmp(a,"R1")) || (!strcmp(a,"R2")) || (!strcmp(a,"TCOORDS2D")) || (!strcmp(a,"DOUBLESIDED"))  || (!strcmp(a,"INVERSE")) );
          GetTransformation(a,&file,p);
		  if(p->vangle){
            matrix t; t.InitTransformation(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, p->vangle, 0.0f, 0.0f, 0.0f);
            p->TM = t*(p->TM);
            p->Tb = p->TM.Inverse(); // calculate backtransformation matrix
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
		  }
          // Draw object to screen
//!		  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish();
        }
        if (!strcmp(a,"POLYMESH") || !strcmp(a,"TRIMESH") ){
		  int tr=(!strcmp(a,"TRIMESH"))? 1 : 0;
          PolyMesh *p = new PolyMesh; p->succ=Base; if(Base) Base->pred=p; Base=p;

          ushort i,j; int ss;

          GetToken(a,&file);
          if(!strcmp(a,"DOUBLESIDED")) p->pmflags=SURF_DOUBLESIDED; else p->pmflags=0;
          GetToken(a,&file);
          if(!strcmp(a,"ROUNDED")) p->pmflags+=SURF_ROUNDED;
          if(!strcmp(a,"GROUPED")) p->pmflags+=SURF_GROUPED;
          if(strcmp(a,"PHONG")) p->nn=0;

          if(fileversion<0.505) GetMaterial(a,&file,p,MatList);
          GetToken(a,&file); // Object-Name
          p->name = new char[strlen(a)+1]; strcpy(p->name,a);

		  GetToken(a,&file);
          do{
            if(!strcmp(a,"MATERIAL")){
              GetMaterial(a,&file,p,MatList);
              GetToken(a,&file);
            }
            if(!strcmp(a,"MATERIALS")){
              GetToken(a,&file); p->nm=atoi(a);
              p->mtl=new(MatPtr[p->nm]);
              for(i=0; i<(p->nm); i++){
                GetToken(a,&file); MatPtr mp=MatList;
                while(((mp->next)!=NULL)&&(strcmp(mp->name,a)!=0)) mp=mp->next;
                #ifdef VModel
                  if(strcmp(mp->name,a)!=0) TextWin.Output(3,"- Material '%s' nicht gefunden!\n",a);
                #endif
				p->mtl[i]=mp;
              }
              p->Mat = p->mtl[0];
			  GetToken(a,&file);
            }
            if(!strcmp(a,"POINTS")){
              GetToken(a,&file); p->nv=atoi(a);
              p->pts=new(vector[p->nv]);
              for(i=0; i<(p->nv); i++){
                GetToken(a,&file); (p->pts[i]).x= (float)atof(a);
                GetToken(a,&file); (p->pts[i]).z= (float)atof(a);
                GetToken(a,&file); (p->pts[i]).y=-(float)atof(a);
              }
              GetToken(a,&file);
            }
            if(!strcmp(a,"TCOORDS")){
              GetToken(a,&file); p->nt=atoi(a);
              p->txc=new(vector[p->nt]);
              for(i=0; i<(p->nt); i++){
                GetToken(a,&file); (p->txc[i]).x=(float)atof(a);
                GetToken(a,&file); (p->txc[i]).y=(float)atof(a);
                GetToken(a,&file); (p->txc[i]).z=(float)atof(a);
              }
              GetToken(a,&file);
            }
            if(!strcmp(a,"NORMALS")) {
              GetToken(a,&file); p->nn=atoi(a);
              p->nrm=new(vector[p->nn]);
              for(i=0;i<((p->nn));i++){
                GetToken(a,&file); (p->nrm[i]).x= (float)atof(a);
                GetToken(a,&file); (p->nrm[i]).z= (float)atof(a);
                GetToken(a,&file); (p->nrm[i]).y=-(float)atof(a);
                p->nrm[i]=unit(p->nrm[i]);
              }
              GetToken(a,&file);
            }
			if(!strcmp(a,"FACES")) {
              GetToken(a,&file); p->nf=atoi(a);
			  if(tr){
                p->pfv=new(ushort[3*(p->nf)]);
                if((p->pmflags) & SURF_GROUPED) p->grp=new(byte[p->nf]);
                if(p->nn) p->pfn=new(ushort[3*(p->nf)]);
			    if(p->nt) p->pft=new(ushort[3*(p->nf)]);
                if(p->nm) p->mat=new(byte[p->nf]);
                if(fileversion<0.505){
			      for(i=0;i<(p->nf);i++){
                    GetToken(a,&file); (p->pfv[3*i  ])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+1])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+2])=atoi(a);
                    if(p->nn){
                      GetToken(a,&file); (p->pfn[3*i  ])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+1])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+2])=atoi(a);
                    }
                    if((p->pmflags) & SURF_GROUPED){
                      GetToken(a,&file); p->grp[i] = atoi(a);
                    }
                  }
			    } else {
			      for(i=0;i<(p->nf);i++){
				    if (p->nm)                     { GetToken(a,&file); p->mat[i] = atoi(a); }
                    if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); p->grp[i] = atoi(a); }
                    GetToken(a,&file); (p->pfv[3*i  ])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+1])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+2])=atoi(a);
                    if(p->nn){
                      GetToken(a,&file); (p->pfn[3*i  ])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+1])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+2])=atoi(a);
                    }
                    if(p->nt){
                      GetToken(a,&file); (p->pft[3*i  ])=atoi(a);
                      GetToken(a,&file); (p->pft[3*i+1])=atoi(a);
                      GetToken(a,&file); (p->pft[3*i+2])=atoi(a);
                    }
                  }
			    }
			  } else {
                // count total number of vertices for array size
                fpos=file.GetPosition();
                for(ss=0,i=0; i<((p->nf)); i++){
				  if (p->nm)                     { GetToken(a,&file); }
                  if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); }
                  GetToken(a,&file); vn=atoi(a); ss += vn;
                  for(j=0; j<vn; j++) GetToken(a,&file);
                  if(p->nn) for(j=0; j<vn; j++) GetToken(a,&file);
                  if(p->nt) for(j=0; j<vn; j++) GetToken(a,&file);
                }
                // read faces-chunk again to fill array
                file.Seek(fpos, CFile::begin);
                          p->pfv=new(ushort[ss]);
			    if(p->nn) p->pfn=new(ushort[ss]);
			    if(p->nt) p->pft=new(ushort[ss]);
                                                p->pnv=new(byte[p->nf]);
			    if (p->nm)                      p->mat=new(byte[p->nf]);
                if((p->pmflags) & SURF_GROUPED) p->grp=new(byte[p->nf]);
                if(fileversion<0.505){
                  for(ss=0,i=0; i<(p->nf); i++){
                    GetToken(a,&file); vn=atoi(a); p->pnv[i]=vn;
                    if(p->pmflags & SURF_GROUPED){
                      GetToken(a,&file); p->grp[i]=atoi(a);
                    }
                              for(j=0; j<vn; j++){ GetToken(a,&file); p->pfv[ss+j]=atoi(a); }
				    if(p->nn) for(j=0; j<vn; j++){ GetToken(a,&file); p->pfn[ss+j]=atoi(a); }
				    if(p->nt) for(j=0; j<vn; j++){ GetToken(a,&file); p->pft[ss+j]=atoi(a); }
				    ss+=vn;
                  }
			    } else {
                  for(ss=0,i=0; i<(p->nf); i++){
				    if (p->nm)                     { GetToken(a,&file); p->mat[i] = atoi(a); }
                    if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); p->grp[i] = atoi(a); }
                    GetToken(a,&file); vn=atoi(a); p->pnv[i]=vn;
                              for(j=0; j<vn; j++){ GetToken(a,&file); p->pfv[ss+j]=atoi(a); }
				    if(p->nn) for(j=0; j<vn; j++){ GetToken(a,&file); p->pfn[ss+j]=atoi(a); }
				    if(p->nt) for(j=0; j<vn; j++){ GetToken(a,&file); p->pft[ss+j]=atoi(a); }
				    ss+=vn;
                  }
                }
			  }
			  GetToken(a,&file);
            }
          }while( (!strcmp(a,"POINTS")) || (!strcmp(a,"NORMALS")) || (!strcmp(a,"TCOORDS")) || (!strcmp(a,"FACES")) );
          GetTransformation(a,&file,p);
          // Draw object to screen
//!		  glDrawBuffer(GL_FRONT); DrawObj(p); glFinish(); WaitTask(NULL);
        }
      }while(!FEOF && ( (!strcmp(a,"OBJECT")) || (!strcmp(a,"AUTHOR")) || (!strcmp(a,"DATE")) || (!strcmp(a,"BACKGROUND")) || (!strcmp(a,"AMBIENT")) || (!strcmp(a,"VIEW")) || (!strcmp(a,"MATERIAL")) || (!strcmp(a,"CUBE")) || (!strcmp(a,"TRIMESH")) || (!strcmp(a,"POLYMESH")) || (!strcmp(a,"SPHERE")) || (!strcmp(a,"CYLINDER")) || (!strcmp(a,"TORUS")) ) );
      #ifdef VModel
        if(!FEOF) TextWin.Output(3,"- unknown token : '%s' !\n",a);
      #endif
	  } else {
        do{
	      GetToken(a,&file);
          if(!stricmp(a,"Object")){
            GetLine(a,&file); strcpy(Object,a);
            #ifdef VModel
			  TextWin.Output(1,"Object: %s\n",a);
            #endif
		  } else if(!stricmp(a,"Author")){
            GetLine(a,&file); strcpy(Author,a);
            #ifdef VModel
			  TextWin.Output(1,"Author: %s\n",a);
            #endif
		  } else if(!stricmp(a,"Date")){
            GetLine(a,&file); strcpy(Date,a);
            #ifdef VModel
			  TextWin.Output(1,"Date  : %s\n",a);
            #endif
		  } else if(!stricmp(a,"UpDate")){
			GetLine(a,&file);
		  } else if(!stricmp(a,"Settings")){
	        GetToken(a,&file); // read bracket
			do{
	          GetToken(a,&file);
              if(!stricmp(a,"BackGround")){
                GetToken(a,&file); Background.R = (float)atof(a);
                GetToken(a,&file); Background.G = (float)atof(a);
                GetToken(a,&file); Background.B = (float)atof(a);
			  } else if(!stricmp(a,"Ambient")){
                GetToken(a,&file); Ambient.R = (float)atof(a);
                GetToken(a,&file); Ambient.G = (float)atof(a);
                GetToken(a,&file); Ambient.B = (float)atof(a);
              } else if(!stricmp(a,"HitBuffers")){
				GetToken(a,&file);
                #ifndef VModel
				  HitBuffers = atoi(a);
                #endif
              } else if(!stricmp(a,"DiffShots")){
				GetToken(a,&file); // obsolete, always set to 1
              } else if(!stricmp(a,"DiffDepth")){
				GetToken(a,&file);
                #ifndef VModel
				  DiffDepth = atoi(a);
                #endif
              } else if(!stricmp(a,"SpecLightDepth")){
				GetToken(a,&file);
                #ifndef VModel
				  SpecLightDepth = atoi(a);
                #endif
              } else if(!stricmp(a,"SpecViewDepth")){
				GetToken(a,&file);
                #ifndef VModel
				  SpecViewDepth = atoi(a);
                #endif
              } else if(!stricmp(a,"SpecDepth")){
				GetToken(a,&file);
                #ifndef VModel
				  SpecViewDepth = SpecLightDepth = atoi(a);
                #endif
              } else if(!stricmp(a,"GlobalIOR")){
				GetToken(a,&file); Environment.IOR_base = (float)atof(a);
			  } else if(!stricmp(a,"SKY")){
                GetToken(a,&file);
		        TexMapPtr t = TexList; while(t && stricmp(a, t->tname)) t=t->next;
		        if(!t){
		          t = new TexMap; t->next=TexList; TexList=t;
		          t->tname=strdup(a); t->tbits=NULL; t->tflags=0;
				}
		        Sky_map=t; Sky = new Sphere; Sky->flags = SURF_INVERSE; Sky->ustart = 0.0f;

		        Sky->txc    = new vector[4];
                Sky->txc[0] = vector(0.0f, 1.0f, 0.0f);
                Sky->txc[1] = vector(0.0f, 0.0f, 0.0f);
                Sky->txc[2] = vector(2.0f, 1.0f, 0.0f);
                Sky->txc[3] = vector(2.0f, 0.0f, 0.0f);
			  }
			} while(!FEOF && (*a != '}'));
		  } else if(!stricmp(a,"View")){
            ViewPtr V = new View; V->next=ViewList; ViewList=V;
            GetToken(a,&file); // View-Name
			if(*a != '{'){
              V->name = new char[strlen(a)+1]; strcpy(V->name,a);
              GetToken(a,&file);
			} else V->name=NULL;

            // default-view:
            V->Zoom = 1.0F;
            V->Locate(vector(0.0F,0.0F,1000.0F));
            V->LookAt(vector(0.0F,0.0F,   0.0F)); V->Up = vector(0.0f, 1.0f, 0.0f);
            V->SetViewAngle (0.5F);
            //V->Tilt(0.0F);
			do{
	          GetToken(a,&file);
              if((!stricmp(a,"Location")) || (!stricmp(a,"Camera")) || (!stricmp(a,"Observer")) || (!stricmp(a,"Eye"))){
                float px,py,pz;
                GetToken(a,&file); px=(float)atof(a);
                GetToken(a,&file); py=(float)atof(a);
                GetToken(a,&file); pz=(float)atof(a);
                V->Locate(vector(px,py,pz));
			  } else if((!stricmp(a,"Target")) || (!stricmp(a,"LookAt"))) {
                float px,py,pz;
                GetToken(a,&file); px=(float)atof(a);
                GetToken(a,&file); py=(float)atof(a);
                GetToken(a,&file); pz=(float)atof(a);
                V->LookAt(vector(px,py,pz));
			  } else if(!stricmp(a,"Up")) {
                float px,py,pz;
                GetToken(a,&file); px=(float)atof(a);
                GetToken(a,&file); py=(float)atof(a);
                GetToken(a,&file); pz=(float)atof(a);
                V->Up=vector(px,py,pz);
			  } else if((!stricmp(a,"View-Angle")) || (!stricmp(a,"FoVAng"))) {
                GetToken(a,&file); V->SetViewAngle(ReadAngle(a));
			  } else {
                #ifdef VModel
                  if(*a != '}') TextWin.Output(3,"- unknown View-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
            // up-vector has not to be given exact
		    vector N = unit(V->Target - V->Location); V->Up = unit((V->Up)-((V->Up)%N)*N);
            if(!(V->name)){
              CV = *V;
              ViewList=V->next; delete(V);
//!              VMDoc->WMP = V->Target; VMDoc->WRD = 2.0f*Betrag(V->Target - V->Location);
//!              GraphWin.InvalidateRect(NULL,FALSE); WaitTask(NULL);
			}
		  } else if(!stricmp(a,"Material")){
            MatPtr mat=new Material; mat->next=MatList; MatList=mat; mat->PBExp = 0.0f;
            GetToken(a,&file); mat->name = strdup(a);
	        GetToken(a,&file); // read bracket
            do{
	          GetToken(a,&file);
              if(!stricmp(a,"Emission")) {
                GetToken(a,&file);
			    if(!stricmp(a,"RGB")){
			      GetToken(a,&file); mat->Emission.R = (float)atof(a);
			      GetToken(a,&file); mat->Emission.G = (float)atof(a);
			      GetToken(a,&file); mat->Emission.B = (float)atof(a);
				} else {
			      mat->Emission.R = (float)atof(a); mat->Emission.G = -1.0f;
				}
              } else if(!stricmp(a,"Reflection")) {
                GetToken(a,&file);
			    if(!stricmp(a,"Texture")){
			      char texpath[260]; strcpy(texpath,filename);
                  char *p = strrchr(texpath,'\\'); if(!p) p = strrchr(texpath,':'); p = p? p+1 : texpath;
			      GetToken(p,&file);
			      mat->Texture = new TexMap;
				  mat->Texture->tname =strdup(p); mat->Texture->tbits=NULL; mat->Texture->glbits=NULL; mat->Texture->tflags=0;
			      //mat->Texture->tbits=LoadBMP(texpath, &(mat->Texture->tinfo)); mat->Reflection.R = 1.0f;
                  //if(!(mat->Texture->tbits)) TextWin.Output(3,"- couldn't load texture: '%s'!\n",texpath);
				} else {
			      if(!stricmp(a,"RGB")){
			        GetToken(a,&file); mat->Reflection.R = (float)atof(a);
			        GetToken(a,&file); mat->Reflection.G = (float)atof(a);
			        GetToken(a,&file); mat->Reflection.B = (float)atof(a);
				  } else {
			        if(!stricmp(a,"frf") || !stricmp(a,"fresnel")){
			          GetToken(a,&file);
				      mat->Transmission.R = -1.0f; //mat->Reflection.G = (float)atof(a);
					} else {
			          mat->Reflection.R = (float)atof(a); mat->Reflection.G = -1.0f;
					}
				  }
				}
              } else if(!stricmp(a,"Absorption")) {
                GetToken(a,&file);
			    if(!stricmp(a,"RGB")){
			      GetToken(a,&file); mat->Absorption.R = (float)atof(a);
			      GetToken(a,&file); mat->Absorption.G = (float)atof(a);
			      GetToken(a,&file); mat->Absorption.B = (float)atof(a);
				} else {
			      mat->Absorption.R = (float)atof(a); mat->Absorption.G = -1.0f;
				}
                GetToken(a,&file); GetToken(a,&file); mat->Ref_Length = (float)atof(a);
			  } else if(!stricmp(a,"Transmission")) {
                GetToken(a,&file);
			    if(!stricmp(a,"RGB")){
			      GetToken(a,&file); mat->Transmission.R = (float)atof(a);
			      GetToken(a,&file); mat->Transmission.G = (float)atof(a);
			      GetToken(a,&file); mat->Transmission.B = (float)atof(a);
				} else {
			      mat->Transmission.R = (float)atof(a); mat->Transmission.G = -1.0f;
				}
			  } else if(!stricmp(a,"IOR")) {
			    GetToken(a,&file); mat->IOR_base = (float)atof(a);
                GetToken(a,&file);
                if(!stricmp(a,"+")) {
			      GetToken(a,&file); mat->IOR_offset = (float)atof(a);
			      GetToken(a,&file); GetToken(a,&file); GetToken(a,&file);
			      GetToken(a,&file); mat->IOR_sub    = (float)atof(a);
				}
			  } else if(!stricmp(a,"Specularity")) {
			    GetToken(a,&file); mat->Specularity = (float)atof(a);
			  } else if(!stricmp(a,"Environment")) {
			    char texpath[260]; strcpy(texpath,filename);
                char *p = strrchr(texpath,'\\'); if(!p) p = strrchr(texpath,':'); p = p? p+1 : texpath;
			    GetToken(p,&file);
			    mat->ReflMap = new TexMap;
				mat->ReflMap->tname =strdup(p); mat->ReflMap->tbits=NULL; mat->ReflMap->glbits=NULL; mat->ReflMap->tflags=0;
			  } else if(!stricmp(a,"Exponent")) {
			    GetToken(a,&file); mat->PBExp = (float)atof(a);
			  } else if(!stricmp(a,"Phong")) {
			    mat->type = Shader_Phong;
			  } else {
                #ifdef VModel
                  if(*a != '}') TextWin.Output(3,"- unknown Material-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
		  } else if(!stricmp(a,"Sphere")){
            Sphere *p = new Sphere; ParseSurface(p, &file, "Sphere");
		  } else if((!stricmp(a,"Cylinder")) || (!stricmp(a,"Cone"))){
            Cylinder *p = new Cylinder; ParseSurface(p, &file, "Cylinder");
		  } else if((!stricmp(a,"Ring")) || (!stricmp(a,"Disc"))){
            Ring *p = new Ring; ParseSurface(p, &file, "Ring");
		  } else if(!stricmp(a,"Paraboloid")){
            Paraboloid *p = new Paraboloid; ParseSurface(p, &file, "Paraboloid");
		  } else if(!stricmp(a,"Function")){
            Function *p = new Function; ParseSurface(p, &file, "Function");
		  } else if(!stricmp(a,"Torus")){
            Torus *p = new Torus; ParseSurface(p, &file, "Torus");
		  } else if(!stricmp(a,"Cube")){
            Cube *p = new Cube; ParseSurface(p, &file, "Cube");
          } else if(!stricmp(a,"PolyMesh") || !stricmp(a,"TriMesh")){
		    int tr=(!stricmp(a,"TriMesh"))? 1 : 0;
            PolyMesh *p = new PolyMesh; p->succ=Base; if(Base) Base->pred=p; Base=p;
			ushort i,j; int ss;

			GetToken(a,&file);
            if(*a != '{'){
	          p->name = strdup(a);
              GetToken(a,&file); // read bracket
			}
            do{
              GetToken(a,&file);
              if(!stricmp(a,"singlesided")) {
	            p->pmflags &= ~SURF_DOUBLESIDED;
              } else if(!stricmp(a,"doublesided")) {
	            p->pmflags |= SURF_DOUBLESIDED;
              } else if(!stricmp(a,"grouped")) {
	            p->pmflags |= SURF_GROUPED;
              } else if(!stricmp(a,"rounded")) {
	            p->pmflags |= SURF_ROUNDED;
              } else if(!stricmp(a,"flat") || !stricmp(a,"phong")) {
	            p->pmflags &= ~SURF_ROUNDED;
	            p->pmflags &= ~SURF_GROUPED;
			  } else if(!stricmp(a,"Material")) {
	            GetMaterial(a,&file,p,MatList);
			  } else if(!stricmp(a,"Materials")){
                GetToken(a,&file); p->nm=atoi(a);
                p->mtl=new(MatPtr[p->nm]);
                for(i=0; i<(p->nm); i++){
                  GetToken(a,&file); MatPtr mp=MatList;
                  while(((mp->next)!=NULL)&&(stricmp(mp->name,a)!=0)) mp=mp->next;
                  #ifdef VModel
                    if(strcmp(mp->name,a)!=0) TextWin.Output(3,"- Material '%s' nicht gefunden!\n",a);
                  #endif
				  p->mtl[i]=mp;
				}
                p->Mat = p->mtl[0];
			  } else if(!stricmp(a,"Points")){
                GetToken(a,&file); p->nv=atoi(a);
                p->pts=new(vector[p->nv]);
                for(i=0; i<(p->nv); i++){
                  GetToken(a,&file); p->pts[i].x=(float)atof(a);
                  GetToken(a,&file); p->pts[i].y=(float)atof(a);
                  GetToken(a,&file); p->pts[i].z=(float)atof(a);
				}
			  } else if(!stricmp(a,"Normals")){
                GetToken(a,&file); p->nn=atoi(a);
                p->nrm=new(vector[p->nn]);
                for(i=0; i<(p->nn); i++){
                  GetToken(a,&file); p->nrm[i].x=(float)atof(a);
                  GetToken(a,&file); p->nrm[i].y=(float)atof(a);
                  GetToken(a,&file); p->nrm[i].z=(float)atof(a);
                  p->nrm[i]=unit(p->nrm[i]);
				}
			  } else if(!stricmp(a,"TexCoords2D")){
                GetToken(a,&file); p->nt=atoi(a);
                p->txc=new(vector[p->nt]);
                for(i=0; i<(p->nt); i++){
                  GetToken(a,&file); p->txc[i].x=(float)atof(a);
                  GetToken(a,&file); p->txc[i].y=(float)atof(a);
                                     p->txc[i].z=       0.0f   ;
				}
			  } else if(!stricmp(a,"Faces")) {
                GetToken(a,&file); p->nf=atoi(a);
			    if(tr){
                  p->pfv=new(ushort[3*(p->nf)]);
                  if((p->pmflags) & SURF_GROUPED) p->grp=new(byte[p->nf]);
                  if(p->nn) p->pfn=new(ushort[3*(p->nf)]);
			      if(p->nt) p->pft=new(ushort[3*(p->nf)]);
                  if(p->nm) p->mat=new(byte[p->nf]);
			      for(i=0;i<(p->nf);i++){
				    if (p->nm)                     { GetToken(a,&file); p->mat[i] = atoi(a); }
                    if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); p->grp[i] = atoi(a); }
                    GetToken(a,&file); (p->pfv[3*i  ])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+1])=atoi(a);
                    GetToken(a,&file); (p->pfv[3*i+2])=atoi(a);
                    if(p->nn){
                      GetToken(a,&file); (p->pfn[3*i  ])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+1])=atoi(a);
                      GetToken(a,&file); (p->pfn[3*i+2])=atoi(a);
                    }
                    if(p->nt){
                      GetToken(a,&file); (p->pft[3*i  ])=atoi(a);
                      GetToken(a,&file); (p->pft[3*i+1])=atoi(a);
                      GetToken(a,&file); (p->pft[3*i+2])=atoi(a);
                    }
                  }
				} else {
                  // count total number of vertices for array size
                  fpos=file.GetPosition();
                  for(ss=0,i=0; i<((p->nf)); i++){
				    if (p->nm)                     { GetToken(a,&file); }
                    if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); }
                    GetToken(a,&file); vn=atoi(a); ss += vn;
                    for(j=0; j<vn; j++) GetToken(a,&file);
                    if(p->nn) for(j=0; j<vn; j++) GetToken(a,&file);
                    if(p->nt) for(j=0; j<vn; j++) GetToken(a,&file);
				  }
                  // read faces-chunk again to fill array
                  file.Seek(fpos, CFile::begin);
                            p->pfv=new(ushort[ss]);
			      if(p->nn) p->pfn=new(ushort[ss]);
			      if(p->nt) p->pft=new(ushort[ss]);
                                                  p->pnv=new(byte[p->nf]);
                  if( p->nm                     ) p->mat=new(byte[p->nf]);
                  if((p->pmflags) & SURF_GROUPED) p->grp=new(byte[p->nf]);
                  for(ss=0,i=0; i<(p->nf); i++){
				    if (p->nm)                     { GetToken(a,&file); p->mat[i] = atoi(a); }
                    if((p->pmflags) & SURF_GROUPED){ GetToken(a,&file); p->grp[i] = atoi(a); }
                    GetToken(a,&file); vn=atoi(a); p->pnv[i]=vn;
                              for(j=0; j<vn; j++){ GetToken(a,&file); p->pfv[ss+j]=atoi(a); }
				    if(p->nn) for(j=0; j<vn; j++){ GetToken(a,&file); p->pfn[ss+j]=atoi(a); }
				    if(p->nt) for(j=0; j<vn; j++){ GetToken(a,&file); p->pft[ss+j]=atoi(a); }
				    ss+=vn;
                  }
				}
			  } else if(!stricmp(a,"Transform")) {
		        matrix M;
                GetToken(a,&file); M.M11=(float)atof(a);
                GetToken(a,&file); M.M12=(float)atof(a);
                GetToken(a,&file); M.M13=(float)atof(a);
                GetToken(a,&file); M.M21=(float)atof(a);
                GetToken(a,&file); M.M22=(float)atof(a);
                GetToken(a,&file); M.M23=(float)atof(a);
                GetToken(a,&file); M.M31=(float)atof(a);
                GetToken(a,&file); M.M32=(float)atof(a);
                GetToken(a,&file); M.M33=(float)atof(a);
                GetToken(a,&file); M.M41=(float)atof(a);
                GetToken(a,&file); M.M42=(float)atof(a);
                GetToken(a,&file); M.M43=(float)atof(a);
		        p->Transform(M);
              } else if(!stricmp(a,"Translate")) {
		        vector t;
	            GetToken(a,&file); t.x = (float)atof(a);
	            GetToken(a,&file); t.y = (float)atof(a);
	            GetToken(a,&file); t.z = (float)atof(a);
		        p->Translate(t);
              } else if(!stricmp(a,"Scale")) {
                GetToken(a,&file); p->Scale(vector((float)atof(a), (float)atof(a), (float)atof(a)));
              } else if(!stricmp(a,"Stretch")) {
				vector s;
                GetToken(a,&file); s.x = (float)atof(a);
                GetToken(a,&file); s.y = (float)atof(a);
                GetToken(a,&file); s.z = (float)atof(a);
				p->Scale(s);
              } else if(!stricmp(a,"Rotate_X")) {
                GetToken(a,&file); p->RotateX(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Y")) {
                GetToken(a,&file); p->RotateY(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Z")) {
                GetToken(a,&file); p->RotateZ(ReadAngle(a));
			  } else {
                #ifdef VModel
                  if(*a != '}') TextWin.Output(3,"- unknown PolyMesh-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
            // Draw object to screen
//!		    glDrawBuffer(GL_FRONT); DrawObj(p); glFinish(); WaitTask(NULL);
		  } else if(!stricmp(a,"Parallelogram")){
            PolyMesh *p = new PolyMesh; p->succ=Base; if(Base) Base->pred=p; Base=p;
            GetToken(a,&file); p->name = strdup(a);
	        GetToken(a,&file); // read bracket
			vector base,edg1,edg2;
            do{
	          GetToken(a,&file);
              if(!stricmp(a,"Material")) {
	            GetMaterial(a,&file,p,MatList);
			  } else if(!stricmp(a,"Visible")) {
			    GetToken(a,&file);
		        if(atoi(a)<0) p->flags |= SURF_INVISIBLE;
		        if(atoi(a)<1) p->flags |= SURF_ONLY_INDIRECT;
			  } else if(!stricmp(a,"Base")) {
			    GetToken(a,&file); base.x = (float)atof(a);
			    GetToken(a,&file); base.y = (float)atof(a);
			    GetToken(a,&file); base.z = (float)atof(a);
			  } else if(!stricmp(a,"Edge1")) {
			    GetToken(a,&file); edg1.x = (float)atof(a);
			    GetToken(a,&file); edg1.y = (float)atof(a);
			    GetToken(a,&file); edg1.z = (float)atof(a);
			  } else if(!stricmp(a,"Edge2")) {
			    GetToken(a,&file); edg2.x = (float)atof(a);
			    GetToken(a,&file); edg2.y = (float)atof(a);
			    GetToken(a,&file); edg2.z = (float)atof(a);
			  } else if(!stricmp(a,"TexCoords2D")) {
		        p->txc=new vector[4];
                GetToken(a,&file); p->txc[0].x = (float)atof(a); GetToken(a,&file); p->txc[0].y = (float)atof(a); p->txc[0].z = 0.0f;
                GetToken(a,&file); p->txc[1].x = (float)atof(a); GetToken(a,&file); p->txc[1].y = (float)atof(a); p->txc[1].z = 0.0f;
                GetToken(a,&file); p->txc[2].x = (float)atof(a); GetToken(a,&file); p->txc[2].y = (float)atof(a); p->txc[2].z = 0.0f;
                GetToken(a,&file); p->txc[3].x = (float)atof(a); GetToken(a,&file); p->txc[3].y = (float)atof(a); p->txc[3].z = 0.0f;
			  } else if(!stricmp(a,"Translate")) {
				vector t;
			    GetToken(a,&file); t.x = (float)atof(a);
			    GetToken(a,&file); t.y = (float)atof(a);
			    GetToken(a,&file); t.z = (float)atof(a);
				p->Translate(t);
              } else if(!stricmp(a,"Rotate_X")) {
                GetToken(a,&file); p->RotateX(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Y")) {
                GetToken(a,&file); p->RotateY(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Z")) {
                GetToken(a,&file); p->RotateZ(ReadAngle(a));
			  } else {
                #ifdef VModel
                  if(*a != '}') TextWin.Output(3,"- unknown Parallelogram-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
			p->pts=new(vector[p->nv=4]);
            p->pts[0]=base;
            p->pts[1]=base+edg1;
            p->pts[2]=base     +edg2;
            p->pts[3]=base+edg1+edg2;
            p->pnv=new(byte[p->nf=1]); p->pnv[0]=4; p->nt=4;
            p->pfv=new(ushort[4]); p->pfv[0]=0; p->pfv[1]=2; p->pfv[2]=3; p->pfv[3]=1;
			p->pft=new(ushort[4]); p->pft[0]=0; p->pft[1]=2; p->pft[2]=3; p->pft[3]=1;
			if(!(p->txc)) { p->txc=new(vector[4]); p->txc[0]=vector(0.0f, 0.0f, 0.0f); p->txc[1]=vector(1.0f, 0.0f, 0.0f);  p->txc[2]=vector(0.0f, 1.0f, 0.0f);  p->txc[3]=vector(1.0f, 1.0f, 0.0f); }
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
            // Draw object to screen
//!		    glDrawBuffer(GL_FRONT); DrawObj(p); glFinish(); WaitTask(NULL);
		  } else if(!stricmp(a,"Triangle")){
            PolyMesh *p = new PolyMesh; p->succ=Base; if(Base) Base->pred=p; Base=p;
            GetToken(a,&file); p->name = strdup(a);
	        GetToken(a,&file); // read bracket
			vector base,edg1,edg2;
            do{
	          GetToken(a,&file);
              if(!stricmp(a,"Material")) {
	            GetMaterial(a,&file,p,MatList);
			  } else if(!stricmp(a,"Visible")) {
			    GetToken(a,&file);
		        if(atoi(a)<0) p->flags |= SURF_INVISIBLE;
		        if(atoi(a)<1) p->flags |= SURF_ONLY_INDIRECT;
			  } else if(!stricmp(a,"Base")) {
			    GetToken(a,&file); base.x = (float)atof(a);
			    GetToken(a,&file); base.y = (float)atof(a);
			    GetToken(a,&file); base.z = (float)atof(a);
			  } else if(!stricmp(a,"Edge1")) {
			    GetToken(a,&file); edg1.x = (float)atof(a);
			    GetToken(a,&file); edg1.y = (float)atof(a);
			    GetToken(a,&file); edg1.z = (float)atof(a);
			  } else if(!stricmp(a,"Edge2")) {
			    GetToken(a,&file); edg2.x = (float)atof(a);
			    GetToken(a,&file); edg2.y = (float)atof(a);
			    GetToken(a,&file); edg2.z = (float)atof(a);
			  } else if(!stricmp(a,"Translate")) {
				vector t;
			    GetToken(a,&file); t.x = (float)atof(a);
			    GetToken(a,&file); t.y = (float)atof(a);
			    GetToken(a,&file); t.z = (float)atof(a);
				p->Translate(t);
              } else if(!stricmp(a,"Rotate_X")) {
                GetToken(a,&file); p->RotateX(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Y")) {
                GetToken(a,&file); p->RotateY(ReadAngle(a));
              } else if(!stricmp(a,"Rotate_Z")) {
                GetToken(a,&file); p->RotateZ(ReadAngle(a));
			  } else {
                #ifdef VModel
                  if(*a != '}') TextWin.Output(3,"- unknown Triangle-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
			p->pts=new(vector[p->nv=3]);
            p->pts[0]=base;
            p->pts[1]=base+edg1;
            p->pts[2]=base     +edg2;
            p->nf=1; p->nt=4;
            p->pfv=new(ushort[3]); p->pfv[0]=0; p->pfv[1]=2; p->pfv[2]=1;
			p->pft=new(ushort[3]); p->pft[0]=1; p->pft[1]=2; p->pft[2]=0;
			p->txc=new(vector[3]); p->txc[0]=vector(0.0f, 0.0f, 0.0f); p->txc[1]=vector(1.0f, 0.0f, 0.0f);  p->txc[2]=vector(1.0f, 1.0f, 0.0f);
            p->NM = p->TM.NTrans();  // calculate transformation matrix for normals
            // Draw object to screen
//!		    glDrawBuffer(GL_FRONT); DrawObj(p); glFinish(); WaitTask(NULL);
		  } else if(!stricmp(a,"Ray")){
            Ray *p = new Ray; p->succ=Base; if(Base) Base->pred=p; Base=p;
            GetToken(a,&file); // p->name = strdup(a);
	        GetToken(a,&file); // read bracket
            do{
	          GetToken(a,&file);
              if(!stricmp(a,"origin")) {
			    GetToken(a,&file); p->origin.x = (float)atof(a);
			    GetToken(a,&file); p->origin.y = (float)atof(a);
			    GetToken(a,&file); p->origin.z = (float)atof(a);
			  } else if(!stricmp(a,"direction")) {
			    GetToken(a,&file); p->direction.x = (float)atof(a);
			    GetToken(a,&file); p->direction.y = (float)atof(a);
			    GetToken(a,&file); p->direction.z = (float)atof(a);
			  } else if(!stricmp(a,"wavelength")) {
			    GetToken(a,&file); p->wavelength = (float)atof(a);
			  } else {
                #ifdef VModel
				  if(*a != '}') TextWin.Output(3,"- unknown Ray-Property: '%s'!\n",a);
                #endif
			  }
			} while(!FEOF && (*a != '}'));
		  } else {
			if(!FEOF){
              #ifdef VModel
                TextWin.Output(3,"- unknown Chunk: '%s'!\n",a);
              #endif
	          char b[80]; GetToken(b,&file);
			  if(*b='{'){
			    do{
	              GetToken(a,&file);
				} while(!FEOF && (*a != '}'));
			  } else {
                #ifdef VModel
                  TextWin.Output(3,"- invalid file structure!\n",a);
                #endif
			    FEOF=1;
			  }
			}
		  }
		} while(!FEOF);
		// hide invisible objects
		PrimPtr p=Base;
		while(p){
          if((p->idnt != RAY) && (((SurfPtr)p)->flags & SURF_INVISIBLE)){
			PrimPtr next=p->succ;
			if(p->succ) p->succ->pred = p->pred;
			if(p->pred) p->pred->succ = p->succ; else Base=p->succ;
			p->succ=Hidden; p->pred=NULL; if(Hidden) Hidden->pred=p; Hidden=p;
			p=next;
		  } else {
		    p=p->succ;
		  }
		}
	  }
    } else {
      #ifdef VModel
		TextWin.Output(3,"- file is no valid VModel file!\n");
      #endif
	}
	file.Close();
  } else {
    #ifdef VModel
	  TextWin.Output(3,"- file '%s' could not be opened!\n",filename);
    #endif
  }
}

