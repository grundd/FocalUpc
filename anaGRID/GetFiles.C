// header files
// c++ headers
#include <iostream>

// root headers
#include "TGrid.h"
#include "TGridResult.h"
#include "TFileMerger.h"

// main program
void GetFiles()
{
    // connect to the GRID
    TGrid::Connect("alien://");

    TGridResult* result = NULL;
    TFileMerger m;

    result = gGrid->Query("/alice/cern.ch/user/d/dgrund/FOCAL_simulations/output","analysisJpsi.root");
    m.OutputFile("analysisJpsi_merged.root");

    Int_t i = 0;
    // Loop over the TGridResult entries and add them to the TFileMerger
    while(result->GetKey(i,"turl")) {
    cout << endl << endl;
    cout << " adding " << result->GetKey(i,"turl") << endl;
    m.AddFile(result->GetKey(i,"turl"));
    i++;
    }

    cout << endl << endl;
    cout << endl << endl;
    cout << endl << endl;

    cout << " ********** MERGING ************ " << endl << endl << endl;
    // Merge
    m.Merge();

    cout << " ********** DONE ************ " << endl << endl << endl;
}

	
	
	
