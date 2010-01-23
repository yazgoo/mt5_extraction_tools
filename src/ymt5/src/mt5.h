#ifndef MT5_H
#define MT5_H
#include <utility>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <exception>
using namespace std;
class mt5
{
protected :
	struct Footers
	{
		unsigned int empty;
		unsigned int endOfPreviousFaces;
		unsigned int verticesNumber;
		unsigned int endOfPreviousFooter;
		float f1, f2, f3, f4;
	};
	Footers footers;
	unsigned short currentBlockSize;
	unsigned short currentTextureNumber;
	unsigned short texturesNumber;
	unsigned int textureEntryPoint;
	unsigned int firstFooterEnd;
	unsigned int texturesOffset;
	unsigned int facesOffset;
	ifstream input;
	void parseHeaders();
	int parseHeadingBlock();
	int parseObjectFooters();
	void skipFacesEnd();
	void parseFaces();
	void parseVerticesAndNormals();
	void parse();
	void saveToObjf(string& objfPath, string& mtlName, 
			bool useTriangleStrips);
	void saveToMtl(string& mtlPath, string& texturesPrefix);
public :
	class WrongFileType : public exception {} wrongFileType;
	class EarlyEndOfFile : public exception {} earlyEndOfFile;
	struct vectors3d { float x, y, z; };
	struct vectors2d { float x, y; };
	typedef vector<unsigned short> Face;
	static const unsigned int headersSize = 28;
	char headers[headersSize];
	list<vectors3d> vertices;
	list<vectors3d> normals;
	list<vectors2d> textureCoordinates;
	map<unsigned short, unsigned int> textureChangingNumberAtIndice;
	list<unsigned int> objectChangingAt;
	list<Face> faces;
	string filePath;
	mt5(string& filePath);
	void saveToObjf(string& path, string& objName, string& mtlName, 
		string& texturesPrefix, bool useTriangleStrips=true);
	~mt5();
};
#endif
