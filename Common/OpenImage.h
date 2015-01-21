typedef unsigned char byte;
typedef byte *byteptr;

byteptr APIENTRY LoadImageFile(char *filename, BITMAPINFO *BMPinfo);
void    APIENTRY QuitImage(byteptr bmpbits);
void    APIENTRY CopyBitMap(BITMAPINFO *BMPinfo, byte *BMPbits);
void    APIENTRY SaveBitMap(char *name, BITMAPINFO *BMPinfo, byte *BMPbits);
