#ifndef BOOKSHELFPARSER_H
#define BOOKSHELFPARSER_H

#include <vector>
#include <string>
using namespace std;

//#include <tr1/unordered_map>
//using std::tr1::unordered_map;

#include <map>

#include "Placement/Placement.h"

class BookshelfParser
{
public:
    BookshelfParser(Placement &placement);
    bool readAuxFile(string filePathName);

private:
    bool readNodesFile(const string& filePathName);
    bool readNetsFile (const string& filePathName);
    bool readSclFile  (const string& filePathName);
    bool readPlFile   (const string& filePathName);

    // placement data
    Placement& _placement;

    // mapping nodeName to moduleId
    //unordered_map<string, unsigned> _nodeNameToModuleId;
    map<string, unsigned> _nodeNameToModuleId;
};

#endif // BOOKSHELFPARSER_H
