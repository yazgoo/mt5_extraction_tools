/*******************************************************************************
 Author 	Olivier Abdesselam (yazgoo)
 Creation Date 	4 9 2009
 License 	BSD
*******************************************************************************/
#include "mt5.h"
#include <iostream>
// uguu
#include <float.h>
#include <math.h>
// uguu
void mt5::parseHeaders()
{
	this->input.read(this->headers, this->headersSize);
	if(headers[0] != 'H' || headers[1] != 'R'
		|| headers[2] != 'C' || headers[3] != 'M')
		throw wrongFileType;
	this->textureEntryPoint = 
		*(reinterpret_cast<unsigned int*>(&(headers[4])));
	this->firstFooterEnd = 
		*(reinterpret_cast<unsigned int*>(&(headers[8])));
    this->input.seekg(this->firstFooterEnd, ios_base::beg);
}

int mt5::parseHeadingBlock()
{
	static unsigned short headingBlock[17];
	if ((this->input.tellg() < this->footers.endOfPreviousFaces) && 
        this->input.read(reinterpret_cast<char*>(headingBlock), 34) != NULL
			&& headingBlock[1] == 0x10)
	{
		this->currentBlockSize = headingBlock[15] - 2;
		this->currentTextureNumber = headingBlock[11];
		if(this->currentTextureNumber >= this->texturesNumber)
			this->texturesNumber = this->currentTextureNumber;
		this->textureChangingNumberAtIndice[this->faces.size()] = 
			this->currentTextureNumber;
		return 1;
	} else return 0;
}

void mt5::parseFaces()
{
	unsigned short data;
	static unsigned short textureMask = 0x3ff;
	short i = 0;
	short coordinatesNumber = 0;
	bool started = false;
	char remainingVertices = 0;
	int headingBlocksNumber = 0;
	while(this->parseHeadingBlock())
	{
		headingBlocksNumber++;
		for(unsigned int byte = 0; byte < this->currentBlockSize; 
				byte+= 2)
		{
			if(this->input.read(reinterpret_cast<char*>(&data), 
						sizeof(short)) == NULL) throw earlyEndOfFile;
			if(!remainingVertices && (data & 0xff00) == 0xff00)
			{
				this->faces.push_back(Face());
				coordinatesNumber = 0;
				i = 0;
				started = true;
				remainingVertices = 0xffff - data + 1; 
			}
			else if (started && remainingVertices != 0)
			{
			switch (i % 3)
			{
				case 0 :
					if((data & 0xff00) == 0xff00) data = 0;
					this->faces.back().push_back(data 
							+ this->facesOffset);
					this->textureCoordinates
						.push_back(vectors2d());
					break;
				case 1 : 
					this->textureCoordinates.back().x =
					 static_cast<float>
					 ((data & textureMask)*1.0f
					  /textureMask);
					coordinatesNumber++;
					break;
				case 2 : 
					this->textureCoordinates.back().y =
					 static_cast<float>
					 ((data & textureMask)*1.0f
					  /textureMask);
					coordinatesNumber++;
					remainingVertices--;
					break;
			}
			i++;
			}
		}
	}
    this->objectChangingAt.push_back(faces.size());
	this->input.seekg(this->footers.endOfPreviousFaces, ios_base::beg);
}

int mt5::parseObjectFooters()
{
        static int j = 0;
	if (this->input.read(reinterpret_cast<char*>(&(this->footers)),
				sizeof(Footers)) != NULL)
	{
		this->input.seekg(this->footers.endOfPreviousFooter + 16,
                ios_base::beg);
		return 1;
	}
	else return 0;
}

void mt5::parseVerticesAndNormals()
{
    vectors3d stats_min = {FLT_MAX, FLT_MAX, FLT_MAX};
    vectors3d stats_max = {FLT_MIN, FLT_MIN, FLT_MIN};
	vectors3d vect;
	long long i;
	static unsigned int sum = 0;
	for(i = 0; i < this->footers.verticesNumber*2; i++)
	{
		this->input.read(reinterpret_cast<char*>(&vect),
				sizeof(vectors3d));
		if((i % 2) == 0)
        {
            if(vect.x > stats_max.x) stats_max.x = vect.x;
            if(vect.y > stats_max.y) stats_max.y = vect.y;
            if(vect.z > stats_max.z) stats_max.z = vect.z;
            if(vect.x < stats_min.x) stats_min.x = vect.x;
            if(vect.y < stats_min.y) stats_min.y = vect.y;
            if(vect.z < stats_min.z) stats_min.z = vect.z;
#ifndef FOOTER_PARSING
                vect.x += this->objectTableItem.x;
                vect.y += this->objectTableItem.y;
                vect.z += this->objectTableItem.z;
#endif
            this->vertices.push_back(vect);
        }
		else this->normals.push_back(vect);
	}
    this->facesOffset += this->footers.verticesNumber;
}

void mt5::parse()
{
    this->facesOffset = 0;
	this->parseHeaders();
#ifdef FOOTER_PARSING
    this->input.seekg(-32, ios_base::cur);
	while(this->parseObjectFooters())
	{
        /*static unsigned int index = 0;
        cout << index++ 
            << " " << hex << this->input.tellg() << endl;*/
		this->parseFaces();
		this->parseVerticesAndNormals();
        this->input.seekg((int)this->footers.endOfPreviousFooter - 32
                , ios_base::beg);
	    if(this->footers.endOfPreviousFooter <= 28) break;
	}
#endif
#ifndef FOOTER_PARSING
    do 
    {
        this->input.read(reinterpret_cast<char*>(&this->objectTableItem), 
                sizeof(this->objectTableItem));
        this->nextObjectTableItemStart = (unsigned int)this->input.tellg();
        if(this->objectTableItem.footerStart != 0)
        {
            this->input.seekg((int)this->objectTableItem.footerStart,
                    ios_base::beg);
            this->parseObjectFooters();
            this->parseFaces();
            this->parseVerticesAndNormals();
            this->input.seekg(this->nextObjectTableItemStart,
                    ios_base::beg);
            /*
            static unsigned int index = 0;
            cout << index++ 
                << " " << hex << this->objectTableItem.footerStart << endl;*/
        }
    }
    while(this->nextObjectTableItemStart < this->textureEntryPoint);
#endif
}

mt5::mt5(string& filePath) : texturesNumber(0), 
	input(filePath.c_str(), std::ios::binary), filePath(filePath)
{
	this->parse();
}

void mt5::saveToObjf(string& objfPath, string& mtlName, 
			bool useTriangleStrips)
{
	unsigned int k;
	ofstream output(objfPath.c_str());
	output << "# this file is a wavefront (obj)"
	<< endl << "# file generated by yazgoo basic mt5 disasembler"
	<< endl << "mtllib " << mtlName << endl;
	k = 0;
	for(list<vectors3d>::iterator it = vertices.begin() ; 
			it != vertices.end();
			it++)
	{
		output << "v " << it->x << " " << it->y << " " << it->z << endl;
		k++;
	}
	k = 0;
	for(list<vectors3d>::iterator it = normals.begin() ;
			it != normals.end();
			it++)
	{
		output << "vn "<< it->x << " " << it->y << " " << it->z << endl;
		k++;
	}
	unsigned short faceIndice = 0;
	for(list<vectors2d>::iterator it = textureCoordinates.begin() ;
			it != textureCoordinates.end(); it++)
		output << "vt " << (it->x)
			<< " " <<  (1.f - it->y) << endl;
	unsigned int l = 0;
	unsigned int i = 0;
	list<unsigned int>::iterator objectIndice = objectChangingAt.begin();
	for(list<Face>::iterator it = faces.begin() ;
			it != faces.end(); it++)
	{
		if(i == 0 || i == *objectIndice)
		{
			output << "o object_" << i << endl;
			if(i != 0) objectIndice++;
		}
		if(textureChangingNumberAtIndice.find(i) 
				!= textureChangingNumberAtIndice.end())
			output << "g group_" << i << endl
				<< "usemtl texture_" << 
				textureChangingNumberAtIndice[i] << endl;
		if(useTriangleStrips)
		{
			output << "t";
			for(Face::iterator it2 = it->begin() ;
					it2 != it->end(); it2++)
					output << " " << (*it2 + 1);
			output << endl;
		}
		else
		{
			if(it->size() > 2)
			{
			for(int j = 0; j < (it->size() - 2); j++)
			{
				Face& face = (*it);
				output << "f ";
				int a = face[j] + 1;
				int b = face[j+1] + 1;
				int c = face[j+2] + 1;
				if((j % 2) == 0)
					output 
					<< a << "/" << l+j+1 << "/" << a << " "
					<< b << "/" << l+j+2 << "/" << b << " "
					<< c << "/" << l+j+3 << "/" << c;
				else
					output 
					<< a << "/" << l+j+1 << "/" << a << " "
					<< c << "/" << l+j+3 << "/" << b<< " "
					<< b << "/" << l+j+2 << "/" << b;
				output << endl;
			}
			l += it->size();
			}
		}
		i++;
	}
	output.close();
}

void mt5::saveToMtl(string& mtlPath, string& texturesPrefix)
{
	ofstream mtl(mtlPath.c_str());
	for(int i = 0; i <= this->texturesNumber; i++)
		mtl << "newmtl texture_" << i << endl
			<< "Ka 0 0 0" << endl
			<< "Kd 1 1 1" << endl
			<< "Ks 0 0 0" << endl
			<< "map_Kd " << texturesPrefix << i << ".bmp" << endl;
	mtl.close();
}

void mt5::saveToObjf(string& path, string& objName, string& mtlName, 
		string& texturesPrefix, bool useTriangleStrips)
{
	string objfPath = path + "/" + objName;
	string mtlPath = path + "/" + mtlName;
	this->saveToObjf(objfPath, mtlName, useTriangleStrips);
	this->saveToMtl(mtlPath, texturesPrefix);
}

mt5::~mt5()
{
	this->input.close();
}

