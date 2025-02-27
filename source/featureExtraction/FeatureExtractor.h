#include<fstream>
#include<iostream> //for debugging
#include<vector>
#include<cmath>
#include<iomanip>
#include<string>

using namespace std;

class FeatureExtractor{
private:
  std::ofstream outFile;
  std::ofstream arf;
  std::ifstream inFile;
  std::string fileName;

  struct Features{
    //Single Click Features//
    int    numRightClicks;
    int    numLeftClicks;
    double rightClickSTD;
    double leftClickSTD;
    double rightClickMean;
    double leftClickMean;
    
    //Double Click Features//
    int    numDoubleClicks;
    double interOneMean;
    double interTwoMean;
    double interThreeMean;
    double interOneSTD;
    double interTwoSTD;
    double interThreeSTD;
    double doubleClickMean;
    double doubleClickSTD;

    //Drag and Drop Features//
    double dragSpeed;
    int    numDnDs;

    //Movement Features//
    double avgSpeed;
    double avgAccel;
    double avgJerk;
    double stdSpeed;
    double stdAccel;
    double stdJerk;

    //TODO: angle features
  };

  struct RawRecord{
    int  time;
    int  x,y;
    int  lClick;
    int  rClick;
  };

  struct SngClick{
    int  startTime;
    int  length;
    int  dragLen;
    int  x,y;
    bool isLeft;
    bool isDnD;
  };

  struct DblClick{
    int  startTime;
    int  int1Len;
    int  int2Len;
    int  int3Len;
    int  x,y;
  };

  struct Pause{
    int  startTime;
    int  length;
    int  x,y;
  };

  enum EventType{
    LEFT,
    RIGHT,
    DOUBLE,
    DND,
    PAUSE
  };

  struct TimeNode{
    int            start;
    int            int1Len;
    int            int2Len;
    int            int3Len;
    int            end;
    int            length;
    int            x,y;
    double         speed,
	           accel,
	           jerk;
    EventType      type;
    vector<double> directions;
    vector<double> AOCs;
    vector<double> curveDists;
  };
 
  vector<double>    speeds;
  vector<double>    accels;
  vector<double>    jerks;
  vector<TimeNode>  timeLine;
  vector<SngClick>  sngClicks;
  vector<DblClick>  dblClicks;
  vector<Pause>     pauses;
  Features          features;
  RawRecord         currentRec;
  int               lastRecTime;
  double            sngR, dblR, aR, sR, jR;

  ///Helper Functions///

  void ExtractLeftClick( RawRecord rec );

  void ExtractRightClick( RawRecord rec );

  void ExtractPause( RawRecord rec );

  void ExtractSpeed( RawRecord rec );

  void ExtractAngle( RawRecord rec );

  void FindDoubles();

  void CalcFeatures();

  void BuildTimeline();

  void AddRecARF(TimeNode tn);

  void CreateARF();
	  
  void CleanOutliers();

  bool isOutlier(TimeNode tn);

public:
  void ExtractFrom(std::string fName, double snR = 35, double dbR = 200);
  //Reads in a file name, opens that file
  //extracts features in a loop
};
