#include <string>
#include <list>
#include <dirent.h>
class Ymt5ui
{
protected:
	unsigned int terminalWidth;
	unsigned int terminalHeight;
	class LoadingBar
	{
	protected:
		unsigned int barWidth;
		unsigned int itemNumber;
		unsigned int progress;
	public:
		LoadingBar(unsigned int barWidth,
				unsigned int itemNumber);
		void draw();
		void setItemNumber(unsigned int itemNumber);
		void operator++(int increment);
		bool full();
	};
	class TextBuffer
	{
	protected:
		unsigned int width;
		unsigned int linesNumber;
		std::list<std::string> lines;
	public:
		unsigned int height;
		TextBuffer(unsigned int width, unsigned int height);
		void draw();
		void operator << (std::string line);
	};
	LoadingBar* loadingBar;
	TextBuffer* textBuffer;
	std::string inputFilePath;
	std::string outputDirPath;
public:
	Ymt5ui(unsigned int terminalWidth, unsigned int terminalHeight,
		std::string inputFile, std::string outputDir);
	~Ymt5ui();
	void drawTitleBar(std::string& title);
	void run();
protected:
	unsigned int getMt5FilesNumber(const char* fileName);
	unsigned int getMt5FilesNumberFromDir(DIR* directory);
};
