#ifndef PVR_H
#define PVR_H
#include <string>
#include <fstream>
using namespace std;
class Pvr
{
protected:
	typedef struct {
		unsigned char  id[4];
		unsigned int length;
		unsigned char  type[4];
		short width;
		short height;
	} Header;
	short width;
	short height;
	typedef struct {
		unsigned int size;
		unsigned short reserved1;
		unsigned short reserved2;
		unsigned int dataOffset;
		unsigned int headerSize;
		unsigned int width;
		unsigned int height;
		short colorPlaneNumber;
		short bitsPerPixelNumber;
		int compression;
		int rawDataSize;
		int horizontalResolution;
		int verticalResolution;
		int colorNumber;
		int importantColors;
	} BitmapHeader;
	typedef struct {
		int rshift,rmask;
		int gshift,gmask;
		int bshift,bmask;
	} SHIFTTBL;
	SHIFTTBL shifttbl[4];
	Header header;
	BitmapHeader bitmapHeader;
	enum {ARGB1555,RGB565,ARGB4444,YUV422,BUMP,PAL4,PAL8};
	enum {B,G,R};
	string path;
	bool open();
	bool goToHeader(fstream& file);
	bool parseHeader(fstream& file);
	bool setData(fstream& file);
	static int calc_n(int n);
	void initTwiddletab();
	void setBmpHeader();
	void setBmpData();
	void setBmpDataPal4(char* dataStart);
	void setBmpDataPal8(char* dataStart);
	void setBmpDataARGB1555(char* dataStart);
	void setBmpDataRGB565(char* dataStart);
	void setBmpDataOther(char* dataStart);
	void setBmpDataTwiddled(char* dataStart);
	void setBmpDataVq(char* in0, char* in1);
	void setBmpDataRectangle(char* dataStart);
	void setBmpDataSmallVq(char* in0, char* in1);
	class FileNotGBIX {};
	class OnlyPVRTSupported {};
	class FileDoesNotExists {};
	class ModeNotListed {};
	unsigned char colorDepth;
	char* bmpData;
	char* data;
	int* twiddletab;
	fstream file;
public:
	Pvr(string path);
	~Pvr();
	void saveToBmp(string bmpPath);
	bool getNext();
};

#endif
