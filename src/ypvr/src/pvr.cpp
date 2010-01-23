#include "pvr.h"
#include <iostream>
#include <string.h>
Pvr::Pvr(string path)
: path(path), bmpData(NULL), data(NULL), twiddletab(NULL)
{
	this->initTwiddletab();
	this->open();
	SHIFTTBL s0=
	{ /* ARGB1555 */
		7,0xf8,
		2,0xf8,
		3,0xf8
	};
	SHIFTTBL s1=
	{
		/* RGB565 */
		8,0xf8,
		3,0xfc,
		3,0xf8
	};
	SHIFTTBL s2=
	{
		/* ARGB4444 */
		4,0xf0,
		0,0xf0,
		4,0xf0
	};
	shifttbl[0] = s0;
	shifttbl[1] = s1;
	shifttbl[2] = s2;
}
Pvr::~Pvr()
{
	delete [] twiddletab;
	if(this->data) delete [] this->data;
	if(this->bmpData) delete [] this->bmpData;
}
void Pvr::initTwiddletab()
{
	if(twiddletab != NULL) return;
	twiddletab = new int[1024];
	int x;
	for(x=0;x<32;x++)
		twiddletab[x] = 
			(x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4);
	for(;x<1024;x++)
		twiddletab[x] = twiddletab[x&31] | (twiddletab[(x>>5)&31]<<10);
}
bool Pvr::goToHeader(fstream& file)
{
	char byte;
	unsigned int letter = 0;
	char GBIX[5] = "GBIX";
	void* read;
	while((read = file.read(&byte, 1)) != NULL)
	{
		if(byte == GBIX[letter])
		{
			if (letter == 3) break;
			letter++;
		} else letter = 0;
	}
	if(read == NULL) return false;
	long seekSize;
	file.read(reinterpret_cast<char*>(&seekSize), sizeof(long));
	char seeked[seekSize];
	file.read(seeked, seekSize);
	return true;
}
bool Pvr::parseHeader(fstream& file)
{
	file.read(reinterpret_cast<char*>(&header), sizeof(Header));
	if(memcmp(header.id,"PVRT",4) != 0) throw OnlyPVRTSupported();
	static int depthTable[]={16,16,16,16,8,8,4,8};
	this->colorDepth = depthTable[header.type[0]];
	this->width = header.width;
	this->height = header.height;
	return true;
}
bool Pvr::setData(fstream& file)
{
	if(this->data != NULL) delete this->data;
	this->data = new char[this->header.length];
	file.read(this->data, this->header.length);
	return true;
}
bool Pvr::getNext()
{
	if (!(this->goToHeader(file) && this->parseHeader(file)))
		return false;
	else return this->setData(file);
}
bool Pvr::open()
{
	file.open(this->path.c_str(), ios_base::in | std::ios::binary);
	if(file.is_open()) return this->getNext();
	throw FileDoesNotExists();
}
void Pvr::setBmpHeader()
{
	int bitNumber, imageSize;
	switch(header.type[0])
	{
		case 0x00: /* ARGB155 */
		case 0x01: /* RGB565 */
		case 0x02: /* ARGB4444 */
		case 0x03: /* YUV422 */
			bitNumber = 24; break;
		case 0x04: /* BUMP */
		case 0x05: /* 4BPP */
			bitNumber = 4; break;
		case 0x06: /* 8BPP */
			bitNumber = 8; break;
	}
	imageSize = ((header.width*bitNumber/8+3)&~3) * header.height;
	this->bitmapHeader.size = sizeof(BitmapHeader) + imageSize - 12;
	this->bitmapHeader.reserved1 = 0;
	this->bitmapHeader.reserved2 = 0;
	this->bitmapHeader.dataOffset = 54;
	this->bitmapHeader.headerSize = 40;
	this->bitmapHeader.width = header.width;
	this->bitmapHeader.height = header.height;
	this->bitmapHeader.colorPlaneNumber = 1;
	this->bitmapHeader.bitsPerPixelNumber = bitNumber;
	this->bitmapHeader.compression = 0;
	this->bitmapHeader.rawDataSize = imageSize;
	this->bitmapHeader.horizontalResolution = 0;
	this->bitmapHeader.verticalResolution = 0;
	this->bitmapHeader.colorNumber = 0;
	this->bitmapHeader.importantColors = 0;
}
void Pvr::setBmpDataPal4(char* in0)
{
	int x,y,wbyte;
	wbyte = (width/2+3)&~3;
	for(y=0;y<height;y+=2) {
		char *p = this->bmpData+(height-y-1)*wbyte;
		unsigned char *in=reinterpret_cast<unsigned char*>(in0);
		for(x=0;x<width;x+=2) {
			int c0 = in[(twiddletab[y]>>1)|twiddletab[x  ]];
			int c1 = in[(twiddletab[y]>>1)|twiddletab[x+1]];
			p[x/2      ] = c0&0x0f | c1<<4;
			p[x/2-wbyte] = c0>>4   | c1&0xf0;
		}
	}
}
void Pvr::setBmpDataPal8(char* in0)
{
	int x,y,wbyte;
	wbyte = (width+3)&~3;
	for(y=0;y<height;y++) {
		char *p = this->bmpData+(height-y-1)*wbyte;
		unsigned char *in=reinterpret_cast<unsigned char*>(in0);
		for(x=0;x<width;x++) {
			p[x] = in[twiddletab[y]|(twiddletab[x]<<1)];
		}
	}
}
void Pvr::setBmpDataOther(char* in0)
{
	int x,y,wbyte;
	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y++) {
		unsigned char *p = reinterpret_cast<unsigned char*>(this->bmpData+(height-y-1)*wbyte);
		unsigned short *in=reinterpret_cast<unsigned short*>(in0);
		for(x=0;x<width;x++) {
			int c = in[twiddletab[y]|(twiddletab[x]<<1)];
			p[R] = p[G] = p[B] = (c >> 8);
			p+=3;
		}
	}
}
void Pvr::setBmpDataARGB1555(char* in0)
{
	const SHIFTTBL *s;
	s = &shifttbl[header.type[0]];
	int x,y,wbyte;
	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y++) {
		unsigned char *p = reinterpret_cast<unsigned char*>(this->bmpData+(height-y-1)*wbyte);
		unsigned short *in=reinterpret_cast<unsigned short*>(in0);
		for(x=0;x<width;x++) {
			int c = in[twiddletab[y]|(twiddletab[x]<<1)];
			p[R] = (c >> s->rshift) & s->rmask;
			p[G] = (c >> s->gshift) & s->gmask;
			p[B] = (c << s->bshift) & s->bmask;
			p+=3;
		}
	}
}
void Pvr::setBmpDataRGB565(char* in0)
{
	const SHIFTTBL *s;
	s = &shifttbl[header.type[0]];
	int x,y,wbyte;
	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y++) {
		unsigned char *p = reinterpret_cast<unsigned char*>(this->bmpData+(height-y-1)*wbyte);
		unsigned short *in=reinterpret_cast<unsigned short*>(in0);
		for(x=0;x<width;x++) {
			int c = in[twiddletab[y]|(twiddletab[x]<<1)];
			p[R] = (c >> s->rshift) & s->rmask;
			p[G] = (c >> s->gshift) & s->gmask;
			p[B] = (c << s->bshift) & s->bmask;
			p+=3;
		}
	}
}
void Pvr::setBmpDataTwiddled(char* in)
{
	std::cout << (int) header.type[0] << std::endl;
	switch(header.type[0])
	{
	case ARGB1555:
		this->setBmpDataARGB1555(in);
		break;
	case RGB565:
		this->setBmpDataRGB565(in);
		break;
	case PAL4:
		this->setBmpDataPal4(in);
		break;
	case PAL8:
		this->setBmpDataPal8(in);
		break;
	default:
#if 0
		this->setBmpDataOther(in);
#endif
		this->setBmpDataRGB565(in);

	}
}
void Pvr::setBmpDataVq(char* in0, char* in1)
{
	unsigned char vqtab[3*4*256];
	unsigned char *in;

	int x,y,wbyte;
	const SHIFTTBL *s;

	unsigned short *in_w = reinterpret_cast<unsigned short*>(in1);
	char *p = reinterpret_cast<char*>(vqtab);

	s = &shifttbl[this->header.type[0]];
	for(x=0;x<256*4;x++) {
		int c = in_w[x];
		p[R] = (c >> s->rshift) & s->rmask;
		p[G] = (c >> s->gshift) & s->gmask;
		p[B] = (c << s->bshift) & s->bmask;
		p+=3;
	}

	in = (unsigned char*)in0;
	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y+=2) {
		char *p = this->bmpData+(height-y-1)*wbyte;
		for(x=0;x<width;x+=2) {
			int c = in[twiddletab[y/2]|(twiddletab[x/2]<<1)];
			unsigned char *vq = &vqtab[c*12];

			p[0] = vq[0];
			p[1] = vq[1];
			p[2] = vq[2];

			p[3] = vq[6];
			p[4] = vq[7];
			p[5] = vq[8];

			p[0-wbyte] = vq[3];
			p[1-wbyte] = vq[4];
			p[2-wbyte] = vq[5];

			p[3-wbyte] = vq[9];
			p[4-wbyte] = vq[10];
			p[5-wbyte] = vq[11];

			p+=6;
		}
	}
}
void Pvr::setBmpDataRectangle(char* in0)
{
	int x,y,wbyte;
	const SHIFTTBL *s;
	if (this->header.type[0]<=2)
		s = &shifttbl[this->header.type[0]];

	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y++) {
		char *p = this->bmpData+(height-y-1)*wbyte;
		unsigned short *in = reinterpret_cast<unsigned short*>(in0);
		for(x=0;x<width;x++) {
			int c = in[x];
			p[R] = (c >> s->rshift) & s->rmask;
			p[G] = (c >> s->gshift) & s->gmask;
			p[B] = (c << s->bshift) & s->bmask;
			p+=3;
		}
	}
}
void Pvr::setBmpDataSmallVq(char* in0, char* in1)
{
	unsigned char vqtab[3*4*16];
	unsigned char *in;

	int x,y,wbyte;
	const SHIFTTBL *s;

	unsigned short *in_w = reinterpret_cast<unsigned short*>(in1);
	char *p = reinterpret_cast<char*>(vqtab);

	s = &shifttbl[this->header.type[0]];
	for(x=0;x<16*4;x++) {
		int c = in_w[x];
		p[R] = (c >> s->rshift) & s->rmask;
		p[G] = (c >> s->gshift) & s->gmask;
		p[B] = (c << s->bshift) & s->bmask;
		p+=3;
	}


	in = (unsigned char*)in0;
	wbyte = (width*3+3)&~3;
	for(y=0;y<height;y+=4) {
		char *p = this->bmpData+(height-y-1)*wbyte;
		for(x=0;x<width;x+=2) {
			int c = in[(twiddletab[y/2]>>1)|twiddletab[x/2]];
			unsigned char *vq;

			vq = &vqtab[(c&15)*12];

			p[0] = vq[0];
			p[1] = vq[1];
			p[2] = vq[2];

			p[3] = vq[6];
			p[4] = vq[7];
			p[5] = vq[8];

			p[0-wbyte] = vq[3];
			p[1-wbyte] = vq[4];
			p[2-wbyte] = vq[5];

			p[3-wbyte] = vq[9];
			p[4-wbyte] = vq[10];
			p[5-wbyte] = vq[11];

			vq = &vqtab[(c>>4)*12];
			p-=wbyte*2;

			p[0] = vq[0];
			p[1] = vq[1];
			p[2] = vq[2];

			p[3] = vq[6];
			p[4] = vq[7];
			p[5] = vq[8];

			p[0-wbyte] = vq[3];
			p[1-wbyte] = vq[4];
			p[2-wbyte] = vq[5];

			p[3-wbyte] = vq[9];
			p[4-wbyte] = vq[10];
			p[5-wbyte] = vq[11];

			p+=wbyte*2;
			p+=6;
		}
	}
}
void Pvr::setBmpData()
{
	if(this->bmpData != NULL) delete this->bmpData;
	this->bmpData = new char[this->bitmapHeader.rawDataSize];
	switch(header.type[1]){
	case 0x01: /* twiddled */
		this->setBmpDataTwiddled(this->data);
		break;
	case 0x02: /* twiddled & mipmap */
		this->setBmpDataTwiddled(this->data+calc_n(header.width)*2);
		break;
	case 0x03: /* VQ */
		this->setBmpDataVq(this->data+2048, this->data);
		break;
	case 0x04: /* VQ & MIPMAP */
		this->setBmpDataVq(this->data+2048+calc_n(header.width/2),
				this->data);
		break;
	case 0x05: /* twiddled 4bit? */
		this->setBmpDataTwiddled(this->data);
		break;
	case 0x06: /* twiddled & mipmap 4bit? */
		this->setBmpDataTwiddled(this->data+calc_n(header.width)*2);
		break;
	case 0x07: /* twiddled 8bit? */
		this->setBmpDataTwiddled(this->data);
		break;
	case 0x08: /* twiddled & mipmap 8bit? */
		this->setBmpDataTwiddled(this->data+calc_n(header.width));
		break;
	case 0x09: /* RECTANGLE */
		this->setBmpDataRectangle(this->data);
		break;
	case 0x10: /* SMALL_VQ */
		this->setBmpDataSmallVq(this->data+128, this->data);
		break;
	case 0x11: /* SMALL_VQ & MIPMAP */
		this->setBmpDataSmallVq(this->data+128+calc_n(header.width/2)/2,
				this->data);
		break;
	default:
		this->setBmpDataTwiddled(this->data);
		std::cout << "mode not supported :" 
			<< (int)header.type[1] << std::endl;
		//throw ModeNotListed();
		break;
	}
}
int Pvr::calc_n(int n)
{
	int sum = 1;
	while(n) {
		n>>=1;
		sum +=n*n;
	}
	return sum;
}
void Pvr::saveToBmp(string bmpPath)
{
	this->setBmpHeader();
	this->setBmpData();
	ofstream file;
	file.open(bmpPath.c_str(), ios::out | ios::binary);
	unsigned short magicNumber = 0x4d42;
	
	file.write(reinterpret_cast<char*>(&(magicNumber)),2);
	file.write(reinterpret_cast<char*>(&(this->bitmapHeader)),
			sizeof(BitmapHeader));
	file.write(this->bmpData, this->bitmapHeader.rawDataSize);
	file.close();
}
