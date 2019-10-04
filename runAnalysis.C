// include the header of your analysis task here! for classes already compiled by aliBuild,
// precompiled header files (with extension pcm) are available, so that you do not need to
// specify includes for those. for your own task however, you (probably) have not generated a
// pcm file, so we need to include it explicitly
#include "AliAnalysisTaskAntipdh.h"

void runAnalysis()
{
    // set if you want to run the analysis locally (kTRUE), or on grid (kFALSE)
    Bool_t local = kFALSE;
    // if you run on grid, specify test mode (kTRUE) or full grid model (kFALSE)
    Bool_t gridTest = kTRUE;

	const char* taskname = "17h_highMultiplicityTest_021019";
    
    // since we will compile a class, tell root where to look for headers  
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->ProcessLine(".include $ROOTSYS/include");
    gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
#else
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");
#endif
     
    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("AnalysisTaskExample");
    AliAODInputHandler *aodH = new AliAODInputHandler();
    mgr->SetInputEventHandler(aodH);

	//Lead the macro for PID
	TMacro PIDadd(gSystem->ExpandPathName("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C"));
	//AliAnalysisTaskPIDResponse *taskPID=AddTaskPIDResponse(kFALSE);
	AliAnalysisTaskPIDResponse* PIDresponseTask = reinterpret_cast<AliAnalysisTaskPIDResponse*>(PIDadd.Exec("kFALSE, kTRUE, kTRUE, \"1\""));

	TMacro PhysicsSelection(gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"));
	AliPhysicsSelectionTask* physSelection = reinterpret_cast<AliPhysicsSelectionTask*>(PhysicsSelection.Exec("kFALSE, kTRUE"));

	TMacro MultiplicitySelection(gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C"));
	AliMultSelectionTask* multSelection = reinterpret_cast<AliMultSelectionTask*>(MultiplicitySelection.Exec("kFALSE"));//user mode

    // compile the class and load the add task macro
    // here we have to differentiate between using the just-in-time compiler
    // from root6, or the interpreter of root5
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->LoadMacro("AliAnalysisTaskAntipdh.cxx++g");
    AliAnalysisTaskAntipdh *task = reinterpret_cast<AliAnalysisTaskAntipdh*>(gInterpreter->ExecuteMacro("AddMyTask.C"));
#else
    gROOT->LoadMacro("AliAnalysisTaskAntipdh.cxx++g");
    gROOT->LoadMacro("AddMyTask.C");
    AliAnalysisTaskMyTask *task = AddMyTask();
#endif


    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(local) {
        // if you want to run locally, we need to define some input
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
        chain->Add("AliAOD.root");
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {
        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
        // make sure your source files get copied to grid
        alienHandler->SetAdditionalLibs("AliAnalysisTaskAntipdh.cxx AliAnalysisTaskAntipdh.h");
        alienHandler->SetAnalysisSource("AliAnalysisTaskAntipdh.cxx");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        alienHandler->SetAliPhysicsVersion("vAN-20181028_ROOT6-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir("/alice/data/2017/LHC17h");
        //alienHandler->SetGridDataDir("/alice/data/2016/LHC16q");
        alienHandler->SetDataPattern("*pass1/AOD208/*/AliAOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");//real data
        // runnumber
		// 17h subset for testing
        alienHandler->AddRunNumber(271871);
        alienHandler->AddRunNumber(271870);
        alienHandler->AddRunNumber(271873);
        alienHandler->AddRunNumber(271874);
        alienHandler->AddRunNumber(271880);
        alienHandler->AddRunNumber(271886);
        alienHandler->AddRunNumber(272018);
        alienHandler->AddRunNumber(272020);
        alienHandler->AddRunNumber(272036);
        alienHandler->AddRunNumber(272038);
        alienHandler->AddRunNumber(272039);

        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(40);
        alienHandler->SetExecutable(Form("%s.sh", taskname));
        // specify how many seconds your job may take
        alienHandler->SetTTL(10000);
        alienHandler->SetJDLName(Form("%s.jdl", taskname));

        alienHandler->SetOutputToRunNo(kTRUE);
        alienHandler->SetKeepLogs(kTRUE);
        // merging: run with kTRUE to merge on grid
        // after re-running the jobs in SetRunMode("terminate") 
        // (see below) mode, set SetMergeViaJDL(kFALSE) 
        // to collect final results
        alienHandler->SetMaxMergeStages(2);
        alienHandler->SetMergeViaJDL(kTRUE);

        // define the output folders
        alienHandler->SetGridWorkingDir(Form("%s_WorkingDir", taskname));
        alienHandler->SetGridOutputDir(Form("%s_OutputDir", taskname));

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(gridTest) {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
            mgr->StartAnalysis("grid");
        } else {
            // else launch the full grid analysis
            alienHandler->SetRunMode("full");
            mgr->StartAnalysis("grid");
        }
    }
}
