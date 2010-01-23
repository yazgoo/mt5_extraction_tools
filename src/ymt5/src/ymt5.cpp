#include "mt5.h"
#include <iostream>
int main(int argc, char** argv)
{
	bool useTriangleStrips = true;
	if(argc < 6)
	{
		std::cout << "Usage: " << argv[0] << " [--no-strips] "
			<< "mt5Path outputBasePath "
			<< "outputName mtlName tPrefix" 
			<< std::endl
			<< "Parameters:"
			<< std::endl
			<< "  --no-strips\t\t\tto generate a wavefront"
			<< std::endl
			<< "  mt5path\t\t\tpath to the mt5 file to extract"
			<< std::endl
			<< "  outputBasePath\t\tpath to output directory"
			<< std::endl
			<< "  outputName\t\t\twavefront output file name "
			<< "(with extension)"
			<< std::endl
			<< "  mtlName\t\t\tmtl output file name "
			<< "(with extension)"
			<< std::endl
			<< "  tPrefix\t\t\tprefix to the texture file names"
			<< std::endl;
		return 1;
	}
	if(argc > 6)
	{
		useTriangleStrips = false;
		argv++;
	}
	string mt5Path(argv[1]);
	string path(argv[2]);
	string objfName(argv[3]);
	string mtlName(argv[4]);
	string texturesPrefix(argv[5]);
    try
    {
        mt5 myMt5(mt5Path);
        myMt5.saveToObjf(path, objfName, mtlName, 
                texturesPrefix, useTriangleStrips);
    }
    catch(mt5::WrongFileType& e)
    {
        cerr << mt5Path << " : wrong file type" << endl;
    }
    catch(mt5::EarlyEndOfFile& e)
    {
        cerr << mt5Path << " : early end of file" << endl;
    }
	return 0;
}
