
#define Shader_OSi   0
#define Shader_Phong 1

typedef struct TexMap    *TexMapPtr;
typedef class  Material  *MatPtr;

class RGB_Color
{
  public:
    float R,G,B;
    RGB_Color(){}
    RGB_Color(float red, float green, float blue) { R=red; G=green; B=blue; }
    friend RGB_Color operator +  (const RGB_Color & c1, const RGB_Color & c2)
      { return RGB_Color(c1.R+c2.R, c1.G+c2.G, c1.B+c2.B); }
    friend RGB_Color operator *  (const double & f, const RGB_Color & c)
      { return RGB_Color((float)(f*c.R), (float)(f*c.G), (float)(f*c.B)); }
    void       operator += (const RGB_Color & c)
      { R+=c.R; G+=c.G; B+=c.B; }
    void       operator *= (const float & f)
      { R*=f; G*=f; B*=f; }
};

inline unsigned long int Clamp2ABGR(RGB_Color Col)
{
  return (((unsigned long int)(Col.R>1.0f? 255 : 255.0f*Col.R))<<16)|
  	     (((unsigned long int)(Col.G>1.0f? 255 : 255.0f*Col.G))<< 8)|
  	     (((unsigned long int)(Col.B>1.0f? 255 : 255.0f*Col.B))    );
}


struct TexMap {
  TexMapPtr    next;      // pointer to next texture in list
  char        *tname;     // filename of texture-map
  BITMAPINFO   tinfo;     // info-block of texture bitmap
  byte        *tbits;     // pointer to texture bitmap
  byte        *glbits;    // pointer to bitmap in OpenGL-Format
  unsigned int glname;    // OpenGL texture bind name
  ushort       glwidth;   // width of OpenGL-Bitmap
  ushort       glheight;  // height of OpenGL-Bitmap
  ushort       tflags;    // texture status
};


class Material
{
  public:
    MatPtr      next;     // pointer to next material in list;
    int         type;     // shader type: Phong or Optical_Simulation
    TexMapPtr   Texture;  // pointer to texture bitmap
    TexMapPtr   ReflMap;  // pointer to environment reflection map
    char       *name;     // reference name of material

    // self-luminance value and color
    RGB_Color   Emission;

    // reflection coefficient and color,
    // ratio of specular/diffuse reflection,
    // specular Phong blending exponent
    RGB_Color   Reflection;   float Specularity, PBExp;

    // spectral light absorption,
    // according to reference length
    RGB_Color   Absorption;   float Ref_Length;
	
    // spectral light transmission values,
    // index of refraction as function of wavelength
    RGB_Color   Transmission; float IOR_base, IOR_offset, IOR_sub;

    // constructor for default material
	Material(void) { next=NULL; type=Shader_OSi; Texture=NULL; ReflMap=NULL; name = "<default>";
	                 Emission.R     = 0.0f; Emission.G     = -1.0f; Emission.B     = -1.0f; 
	                 Reflection.R   = 0.8f; Reflection.G   =  0.7f; Reflection.B   =  0.6f;
					 Absorption.R   = 0.0f; Absorption.G   = -1.0f; Absorption.B   = -1.0f;
	                 Transmission.R = 0.0f; Transmission.G = -1.0f; Transmission.B = -1.0f; 
				     Specularity    = 0.0f; PBExp          =  0.0f; Ref_Length     =  1.0f;
				     IOR_base       = 1.0f; IOR_offset     = -1.0f; IOR_sub        =  0.0f;
	               }

	float IOR (float w);
    float Diff(float w, float x , float y);
    float Spec(float w, float n1, float n2, double ce1);
    float Trns(float w, float n1, float n2, double ce1);
	float Thru(float w, float d);
};

float wlcoff(float cR, float cG, float cB, float v);
float R_fnf(float n1, float n2, double ce1);
