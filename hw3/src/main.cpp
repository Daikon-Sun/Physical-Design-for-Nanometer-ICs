#include "Placement.h"
#include "Util.h"
#include "GnuplotLivePlotter.h"
#include "arghandler.h"
#include "Legal.h"
#include "ParamPlacement.h"

#include <omp.h>
#include <iostream>
#include <string.h>
#include <time.h>

using namespace std;

bool handleArgument(const int& argc, char* argv[], CParamPlacement& param) {
  int i;
  if(!strcmp( argv[1]+1, "aux" )) {
    param.auxFilename = string(argv[2]);
    i = 3;
  } else {
    cout << "Usage: " << argv[0] << " -aux benchmark.aux" << endl;
    return false;
  }
  while(i < argc) {
    ////////////////////////////////////////////////////////////////////////
    // Public arguments
    ////////////////////////////////////////////////////////////////////////
    if(strlen(argv[i]) <= 1) {
      i++;
      continue;
    }
    else if(!strcmp(argv[i]+1, "noglobal")) param.bRunGlobal = false;
    else if(!strcmp(argv[i]+1, "nolegal")) param.bRunLegal = false;
    else if(!strcmp(argv[i]+1, "nodetail")) param.bRunDetail = false;
    else if(!strcmp(argv[i]+1, "loadpl")) param.plFilename = string(argv[++i]);
    else if(!strcmp(argv[i]+1, "thread" )) param.threadNum = atoi(argv[++i]);
    i++;
  }
  return true;
}
///////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  gArg.Init(argc, argv);
  if(!handleArgument( argc, argv, param )) return -1;

  ::omp_set_num_threads(param.threadNum);

  Placement placement;
  placement.readBookshelfFormat(param.auxFilename , param.plFilename);

  cout << "Benchmark: " << placement.name() << endl;

  //cout << format("Memory usage: %.1f MB", getCurrentMemoryUsage()) << endl;
  //cout << format("Core region: (%.f,%.f)-(%.f,%.f)",
  //               placement.rectangleChip().left(),
  //               placement.rectangleChip().bottom(),
  //               placement.rectangleChip().right(),
  //               placement.rectangleChip().top()) << endl;

  time_t total_time=0;
  ////////////////////////////////////////////////////////////////
  // Legalization
  ////////////////////////////////////////////////////////////////
  time_t legal_time_start = time(NULL);
  time_t total_legal_time = 0;
  placement.renew_row_width();
  if(param.bRunLegal) {
    //cout<<endl<<"////// Legalization ///////"<<endl;
    CLegal legal(placement);
    legal.solve();
    //bool bLegal = legal.solve();

    //if(bLegal) cout<<"legalization success!"<<endl;
    //else cout<<"legalization fail!"<<endl;

    placement.outputGnuplotFigure(placement.name()+".lg.plt");
    placement.outputBookshelfFormat(placement.name()+".lg.pl");
    total_legal_time = time(NULL) - legal_time_start;
    total_time+=total_legal_time;
  }
  //cout<<endl<<endl<<"////////////////////"<<endl;
  //if(!placement.plname().empty())
  //  cout << "Benchmark: " << placement.plname() << endl;
  //else cout << "Benchmark: " << placement.name() << endl;
  return 0;
}
