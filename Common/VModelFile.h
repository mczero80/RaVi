
typedef class  View *ViewPtr;

class View {
  private:
    matrix  Rh,Rz;     // Rotationsmatrix
  public:
    ViewPtr next;      // pointer to next view in list
    char   *name;      // pointer to view-name
    vector  Location;  // Observer-Location-Point  (Worldcoordinates)
    angles  Direction; // Viewing-Direction
    vector  Target;    // View-Target-Point        (Worldcoordinates)
    vector  Up;        // vector of 'up'-direction (Worldcoordinates)
	float   h;         // view plane window size
	float   old_h;     // last non-zero h
    float   Zoom;      // zoom factor
    View(void);
    void Locate(vector v) {Location =v;}
    void LookAt(vector v);
    void SetViewAngle(float a) { h = ( a>0.0f? 2.0f*(float)tan(a/2.0f) : 0.0f ); }
};

class VModelFile
{
  public:
    char      Object[80];
    char      Author[60];
    char      Date[40];

    MatPtr    MatList;
    TexMapPtr TexList;
    ViewPtr   ViewList;
    PrimPtr   Base,Hidden;

    RGB_Color Background;             // Background color
    RGB_Color Ambient;                // color (and intensity) of ambient light
	Material  Environment;
	View      CV;                     // current view
    Sphere   *Sky; TexMapPtr Sky_map; // pointer and texture of sky

    void Init(void);
	void Kill(void);
	void Load(char *filename);
  private:
    void ParseSurface(SurfPtr p, CFile *file, char* typ);
};
