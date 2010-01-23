#include "pvr.h"
#include <sstream>
#include <iostream>
using namespace std;
int main(int argc, char** argv)
{
	if(argc < 3) 
	{
		cout << "usage : " << argv[0] << " mt5Path outPrefix" << endl;
		return 1;
	}
	Pvr pvr(argv[1]);
	unsigned int i = 0;
	do
	{
		ostringstream s;
		s << argv[2] << i << ".bmp";
		cout << "extracting texture #" << i 
			<< " to " << s.str() << endl;
		try
		{
		pvr.saveToBmp(s.str());
		} catch(...)
		{
			cout << "extraction failed for texture #" << i << endl;
		}
		i++;
	}
	while (pvr.getNext());
}
