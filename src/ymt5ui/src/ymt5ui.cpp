#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <termios.h>
#include "ymt5ui.h"
using namespace std;
Ymt5ui::LoadingBar::LoadingBar(unsigned int barWidth,
		unsigned int itemNumber)
: barWidth(barWidth), itemNumber(itemNumber), progress(0)
{
}
void Ymt5ui::LoadingBar::draw()
{
	float percent = (float)this->progress / this->itemNumber;
	float progressWidth = percent * (this->barWidth-4);
	unsigned int i = 0;
	cout << " " << (100 * percent) << "%"<< endl;
	cout << " [";
	for(; i < progressWidth; i++)
		cout << "=";
	for(; i < (barWidth-4); i++)
		cout << " ";
	cout << "]" << endl;
}
void Ymt5ui::LoadingBar::setItemNumber(unsigned int itemNumber)
{
	this->itemNumber = itemNumber;
}
void Ymt5ui::LoadingBar::operator++(int increment)
{
	if(this->progress <= this->itemNumber) this->progress++;
}
bool Ymt5ui::LoadingBar::full()
{
	return (this->progress > this->itemNumber);
}
Ymt5ui::TextBuffer::TextBuffer(unsigned int width, unsigned int height)
: width(width), height(height), linesNumber(height-2)
{
	string blankLine = "";
	for(unsigned int i = 0; i < width-6; i++) blankLine += " ";
	for(unsigned int i = 0; i < linesNumber-3; i++)
		this->lines.push_back(blankLine);
}
void Ymt5ui::TextBuffer::draw()
{
	cout << endl << "  ";
	for(unsigned int i = 0; i < width-4; i++) cout << "_";
	cout << endl;
	for(list<string>::iterator it = lines.begin(); it != lines.end(); it++)
		cout << " | " << *it << " |" << endl;
	cout << " |";
	for(unsigned int i = 0; i < width-4; i++) cout << "_";
	cout << "| " << endl;
}
void Ymt5ui::TextBuffer::operator << (string line)
{
	string newString = "";
	if(line.size() >= width - 6)
	{
		newString = "";
	} else
	{
		newString = line;
		for(unsigned int i = line.size(); i < width - 6; i++)
		newString += " ";
	}
	lines.push_back(newString);
	lines.pop_front();
}
Ymt5ui::Ymt5ui(unsigned int terminalWidth, unsigned int terminalHeight,
		string inputFile, string outputDir)
: loadingBar(NULL), textBuffer(NULL),
	terminalWidth(terminalWidth), terminalHeight(terminalHeight),
	inputFilePath(inputFile), outputDirPath(outputDir)
{
	this->textBuffer = new TextBuffer(terminalWidth, terminalHeight*2/3);
	this->loadingBar = new LoadingBar(terminalWidth,
			this->getMt5FilesNumber(this->inputFilePath.c_str()));
}
Ymt5ui::~Ymt5ui()
{
	if(this->loadingBar != NULL) delete this->loadingBar;
	if(this->textBuffer != NULL) delete this->textBuffer;
}
void Ymt5ui::drawTitleBar(string& title)
{
	unsigned int i;
	unsigned int size = this->terminalWidth/2 - title.size()/2;
	for(i = 0; i < size; i++) cout << "=";
	cout << title;
	for(; i < this->terminalWidth-title.size(); i++) cout << "=";
}
unsigned int Ymt5ui::getMt5FilesNumberFromDir(DIR* directory)
{
    dirent* entry;
    while((entry = readdir(directory)) != NULL)
    {
        if(entry->d_type == DT_DIR)
        {
		    *(this->textBuffer) << entry->d_name;
            DIR* subDirectory = opendir(entry->d_name);
            this->getMt5FilesNumberFromDir(subDirectory);
            closedir(subDirectory);
        }
        else 
		    *(this->textBuffer) << entry->d_name;
        delete entry;
    }
}
unsigned int Ymt5ui::getMt5FilesNumber(const char* fileName)
{
	DIR* directory;
	if((directory = opendir(fileName)) != NULL)
	{
        this->getMt5FilesNumber(directory);
		closedir(directory);
	}
}
void Ymt5ui::run()
{
	string title("ymt5ui");
	unsigned int loadingContainerHeight = this->terminalHeight
		- this->textBuffer->height + 1;
	unsigned int middle = loadingContainerHeight/2;
	unsigned int j = 0;
	do
	{
		this->drawTitleBar(title);
		this->textBuffer->draw();
		for(unsigned int i = 0; i < loadingContainerHeight; i++)
			if(i == middle) this->loadingBar->draw();
			else if(i != middle+1) cout << endl;
		(*loadingBar)++;
		*textBuffer << "test";
	}
	while(!loadingBar->full());
}
int main(int argc, char** argv)
{
    unsigned int columns, lines;
    {
        struct ttysize {
            unsigned short  ts_lines;
            unsigned short  ts_cols;
            unsigned short  ts_xxx;
            unsigned short  ts_yyy;
        } ts;
        ioctl(0, TIOCGWINSZ, &ts);
        lines = ts.ts_lines;
        columns = ts.ts_cols;
    }
    if(argc != 3)
    {
        cerr << "Usage: " << argv[0] << " input outputdir" << endl;
        return 1;
    }
	Ymt5ui ui(columns, lines, string(argv[1]), string(argv[2]));
	ui.run();
}
