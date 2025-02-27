#include"FeatureExtractor.h"


void FeatureExtractor::ExtractLeftClick( RawRecord rec ){
  
  static bool isDown = false;
  static TimeNode tmpLClick;
  static int      x = 0, y = 0;

  if(rec.lClick == 1){

    if(!isDown){
      tmpLClick.start = rec.time;
      tmpLClick.x     = rec.x;
      tmpLClick.y     = rec.y;
      x = rec.x;
      y = rec.y;
    }
	  
    isDown = true;

  }else if(isDown){
 
    isDown = false;

    tmpLClick.length = rec.time - tmpLClick.start;
    tmpLClick.end    = tmpLClick.start + tmpLClick.length;
    tmpLClick.type   = LEFT;

    if(sqrt((rec.x - x)*(rec.x - x) + (rec.y - y)*(rec.y-y)) > 20){
      tmpLClick.type = DND;
    }

    timeLine.push_back(tmpLClick);    

  }

}


void FeatureExtractor::ExtractRightClick( RawRecord rec ){
 
  static bool     isDown   = false;
  static TimeNode tmpClick;

  if(rec.rClick == 1){

    if(!isDown){
      tmpClick.start = rec.time;
      tmpClick.x     = rec.x;
      tmpClick.y     = rec.y;
    }
	  
    isDown = true;

  }else if(isDown){
 
    isDown = false;

    tmpClick.length = rec.time - tmpClick.start;
    tmpClick.type   = RIGHT;
    tmpClick.end    = tmpClick.start + tmpClick.length;

    timeLine.push_back(tmpClick);    

  }

}


void FeatureExtractor::ExtractPause( RawRecord rec ){

  static bool      isPaused = false;
  static TimeNode  tmpPause;
  static RawRecord prevRec;

  if(rec.lClick == 0    && rec.rClick == 0    && 
     rec.x == prevRec.x && rec.y == prevRec.y ){

    if(!isPaused){
      tmpPause.start = rec.time;
      tmpPause.x     = rec.x;
      tmpPause.y     = rec.y;
    }
	  
    isPaused = true;

  }else if(isPaused){
    
    isPaused = false;

    tmpPause.length = rec.time - tmpPause.start;
    tmpPause.type   = PAUSE;
    tmpPause.end    = tmpPause.start + tmpPause.length;

    if(tmpPause.length > 5000)
      timeLine.push_back(tmpPause);    

  }

  prevRec = rec;
}

void FeatureExtractor::FindDoubles(){

  vector<TimeNode>::iterator front, back;

  if(timeLine.size() > 1){

    back  = timeLine.begin();
    front = (timeLine.begin()+1);

    while(front != timeLine.end()){

      if(front->start - (back->start + back->length) < 500 &&
         front->type == LEFT                               &&
         back->type  == LEFT                               ){

        TimeNode tmp;

	tmp.start   = back->start;
	tmp.int1Len = back->length;
	tmp.int2Len = front->start - (back->start + back->length);
	tmp.int3Len = front->length;
        tmp.length  = tmp.int1Len + tmp.int2Len + tmp.int3Len;
        tmp.type    = DOUBLE;
        tmp.x       = front->x;
        tmp.y       = front->y;
        tmp.end     = tmp.length + tmp.start;

        timeLine.erase(front);
	*back = tmp;

      }
      
      back++;
      front++;
    } 
  }
}


//Calc features get sthe information from only the timeLine vector.
//Try and phase out the double click pause and sngclick vectors
void FeatureExtractor::CalcFeatures(){

  double temp = 0, temp2 = 0, variance = 0;
  vector<TimeNode>::iterator tIt;


  outFile.open(("./features/" + fileName + ".mrf").c_str());

  outFile << "-----Stats-----" << endl;


  //Calc Features, store them, then outfile them


  int dndNum = 0;
  int rNum = 0;
  int lNum = 0;
  int dcNum = 0;
  //Find num click types
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++){
    switch(tIt->type){

      case LEFT: lNum++;
      break;
      case RIGHT: rNum++;
      break;
      case DND: dndNum++;
      break;
      case DOUBLE: dcNum++;
      break;
      case PAUSE: //Do Nothing
      break;
      default: cout << "INVALID TYPE: " << tIt->type << endl;
      break;
    }

  }

  


  features.numLeftClicks   = lNum;
  features.numRightClicks  = rNum;
  features.numDnDs         = dndNum;
  features.numDoubleClicks = dcNum;


  //Find left and right means
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++){

    if(tIt->type == LEFT)
      temp += tIt->length;
    if(tIt->type == RIGHT)
      temp2 += tIt->length;
  }

  features.leftClickMean  = temp / double(lNum); 
  features.rightClickMean = temp2 / double(rNum);


  double varianceR = 0;
  
  //SngClick variance
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++){

    if(tIt->type == LEFT){
      variance  += pow(tIt->length - features.leftClickMean,2);
    }

    if(tIt->type == RIGHT){
      varianceR += pow(tIt->length - features.rightClickMean,2);
    }
  }

  features.leftClickSTD  =  sqrt(variance  / features.numLeftClicks);
  features.rightClickSTD =  sqrt(varianceR / features.numRightClicks);



  //Double Click Variance
    variance = 0;
    temp = 0;
    double int1Tmp = 0, int2Tmp = 0, int3Tmp = 0;
    double variance1 = 0, variance2 = 0, variance3 = 0;
 
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++){
    if(tIt->type == DOUBLE){
      int1Tmp += tIt->int1Len;
      int2Tmp += tIt->int2Len;
      int3Tmp += tIt->int3Len;
      temp += tIt->int1Len
	      + tIt->int2Len
  	      + tIt->int3Len;
    }
  }
    
	  features.interOneMean    = int1Tmp / features.numDoubleClicks;
	  features.interTwoMean    = int2Tmp / features.numDoubleClicks;
          features.interThreeMean  = int3Tmp / features.numDoubleClicks;
          features.doubleClickMean = temp    / features.numDoubleClicks; 


  //Double click variance
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++){
    if(tIt->type == DOUBLE){
      variance1 += pow(tIt->int1Len - features.interOneMean  , 2);
      variance2 += pow(tIt->int2Len - features.interTwoMean  , 2);
      variance3 += pow(tIt->int3Len - features.interThreeMean, 2); 
      variance  += pow((tIt->int1Len
		      + tIt->int2Len
		      + tIt->int3Len) - features.doubleClickMean, 2);
    }
  }

          features.interOneSTD    = sqrt(variance1 / features.numDoubleClicks);
          features.interTwoSTD    = sqrt(variance2 / features.numDoubleClicks);
          features.interThreeSTD  = sqrt(variance3 / features.numDoubleClicks);
          features.doubleClickSTD = sqrt(variance  / features.numDoubleClicks);

  
  double numEvents = lNum + rNum + dcNum + dndNum;

  /////Find Avg of Speed, Accel, Jerk/////
  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
     temp += tIt->speed;

	  features.avgSpeed = temp / numEvents;



  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
      temp += tIt->accel;

	  features.avgAccel = temp / numEvents;


		  
  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
      temp += tIt->jerk;

	  features.avgJerk = temp / numEvents;



  /////Find STD of Speed, Accel, Jerk/////
  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
     temp += pow(tIt->speed - features.avgSpeed, 2);

	  features.stdSpeed = sqrt(temp / numEvents);



  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
     temp += pow(tIt->accel - features.avgAccel, 2);

	  features.stdAccel = sqrt(temp / numEvents);


		  
  temp = 0;
  for(tIt = timeLine.begin(); tIt != timeLine.end(); tIt++)
    if(tIt->type != PAUSE)
     temp += pow(tIt->jerk - features.avgJerk, 2);

	  features.stdJerk = sqrt(temp / numEvents);
 
  
 //Clean up if there arent any of one type of click
 if(features.numLeftClicks == 0){

   features.leftClickMean = 0;
   features.leftClickSTD = 0;

 }

 if(features.numRightClicks == 0){

   features.rightClickMean = 0;
   features.rightClickSTD = 0;

 }

 if(features.numDoubleClicks == 0){

   features.doubleClickMean = 0;
   features.doubleClickSTD = 0;

 }

  //Output the features into the mrf file
 
 outFile << "Num Left Clicks:      " << features.numLeftClicks   << endl
	 << "Left Click avg Len:   " << features.leftClickMean   << endl
	 << "Left Click STD:       " << features.leftClickSTD    << endl << endl
	 << "Num Right Clicks:     " << features.numRightClicks  << endl
	 << "Right Click avg Len:  " << features.rightClickMean  << endl
	 << "Right Click STD:      " << features.rightClickSTD   << endl << endl
	 << "Num Double Clicks:    " << features.numDoubleClicks << endl
	 << "Double Click avg Len: " << features.doubleClickMean << endl
	 << "Double Click STD:     " << features.doubleClickSTD  << endl
	 << "  Interval 1 Mean:    " << features.interOneMean    << endl
	 << "  Interval 2 Mean:    " << features.interTwoMean    << endl
	 << "  Interval 3 Mean:    " << features.interThreeMean  << endl
	 << "  Interval 1 STD:     " << features.interOneSTD     << endl
	 << "  Interval 2 STD:     " << features.interTwoSTD     << endl
	 << "  Interval 3 STD:     " << features.interThreeSTD   << endl << endl
	 << "Average Speed:        " << features.avgSpeed        << endl
	 << "Average Acceleration: " << features.avgAccel        << endl
	 << "Average Jerk:         " << features.avgJerk         << endl 
	 << "STD Speed:            " << features.stdSpeed        << endl
	 << "STD Accel:            " << features.stdAccel        << endl
	 << "STD Jerk:             " << features.stdJerk         << endl << endl
	 << "Num of Drag n Drop:   " << features.numDnDs         << endl << endl;


  //Create new timeline file
  outFile.close();
  outFile.open(("./features/" + fileName + "TIMELINE.txt").c_str());
  outFile.precision(10);
  outFile.setf(ios::fixed);
   
  outFile << features.leftClickMean << " " << features.leftClickSTD << endl
	  << features.rightClickMean<< " " << features.rightClickSTD<< endl
	  << features.doubleClickMean << " " << features.doubleClickSTD << endl
	  << features.interOneMean << " " << features.interOneSTD << endl
	  << features.interTwoMean << " " << features.interTwoSTD << endl
	  << features.interThreeMean << " " << features.interThreeSTD << endl
	  << features.avgSpeed << " " << features.stdSpeed << endl
	  << features.avgAccel << " " << features.stdAccel << endl
	  << features.avgJerk  << " " << features.stdJerk << endl << endl;

  vector<TimeNode>::iterator it = timeLine.begin();

  for(; it != timeLine.end(); it++){
    outFile << it->start << " " << it->type  << " " << it->length << " " << it->speed << " "
	    << it->accel << " " << it->jerk   << " " << it->int1Len << " "
	    << it->int2Len << " " << it->int3Len << endl;
  }

/* TODO
  //Make new angle files
  outFile.close();
  outFile.open(("./features/" + fileName + "DC.txt").c_str());
  outFile.precision(3);
  outFile.setf(ios::fixed);

  for(it = timeLine.begin(); it != timeLine.end(); it++){
    if(it->type != PAUSE)
      for(int i = 0; i < it->directions.size(); i++)
        outFile << it->directions[i] << endl;

  }

  outFile.close();
  outFile.open(("./features/" + fileName + "AOC.txt").c_str());
  outFile.precision(3);
  outFile.setf(ios::fixed);

  for(it = timeLine.begin(); it != timeLine.end(); it++){\
    if(it->type != PAUSE)
      for(int i = 0; i < it->AOCs.size(); i++)
        outFile << it->AOCs[i] << endl;

  }
*/
}


void FeatureExtractor::CreateARF(){

 arf << "@relation MouseFeatures" << endl << endl << endl
     << "@attribute Type {Left, Right, Double, DragNDrop, Pause}" << endl
     << "@attribute length real" << endl
     << "@attribute speed real" << endl
     << "@attribute accel real" << endl
     << "@attribute jerk real" << endl
     << "@attribute int1Len real" << endl
     << "@attribute int2Len real" << endl
     << "@attribute int3Len real" << endl << endl << endl
     << "@data" << endl;

}


void FeatureExtractor::AddRecARF(TimeNode tn){


  switch(tn.type){
    case LEFT:   arf << "Left,";      break;
    case RIGHT:  arf << "Right,";     break;
    case DOUBLE: arf << "Double,";    break;
  }

  if(tn.type == LEFT || tn.type == RIGHT || tn.type == DOUBLE){

    arf << tn.length << "," << tn.speed << "," << tn.accel << "," << tn.jerk << ",";

      if(tn.type == DOUBLE){

        arf << tn.int1Len << "," << tn.int2Len << "," << tn.int3Len << "," << fileName << endl;

      }else{

        arf << "?,?,?," << fileName << endl;

    }
  }
}


void FeatureExtractor::ExtractSpeed( RawRecord rec ){

  static vector<TimeNode>::iterator tIt = timeLine.begin();
  static double  startTime = 0;
  static double  dist;
  static int     prevX = rec.x, prevY = rec.y;
  static int     wait = 0;


  if(rec.time >= wait && tIt != timeLine.end()){
	  
    dist += sqrt(pow((rec.x - prevX), 2) + pow((rec.y-prevY), 2));
    if(rec.time == tIt->start){

      if(tIt->type != PAUSE){
        speeds.push_back(dist/(rec.time-startTime));
	accels.push_back(speeds.back() / (rec.time - startTime));
        jerks.push_back(accels.back() / (rec.time - startTime));

        tIt->speed = speeds.back();
	tIt->accel = accels.back();
	tIt->jerk  = jerks.back();
      }

      wait = tIt->end;
      dist = 0;
      startTime = rec.time;

      tIt++;
    }
  }



  static int iter = 0;
  static RawRecord current = rec;

  if(iter = 1)
    static RawRecord second;

  if(iter = 2)
    static RawRecord first;
 
  if(iter != 2){
    iter++;
  }else{
    iter++;
    
  }


  prevX = rec.x;
  prevY = rec.y;
  
}


void FeatureExtractor::ExtractAngle(RawRecord rec){

  static vector<TimeNode>::iterator tIt = timeLine.begin();
  static int     aX, bX, cX, aY, bY, cY;
  static int     wait = 0;
  static int     iter = 0;
  static vector<double>  dir, aoc, cd;


  if(rec.time >= wait && tIt != timeLine.end() && iter >= 3){


    if(aX != bX && bX != cX && cX != aX &&
       aY != bY && bY != cY && cY != aY  ){

       dir.push_back(asin(        abs(bY-aY) 
		                       / 
		   sqrt( pow(aX - bX, 2) + pow(aY-bY,2) )));

       double hypo = sqrt(pow(cX-aX,2) + pow(cY-aY,2));
       double leg1 = sqrt(pow(bX-aX,2) + pow(bY-aY,2));
       double leg2 = sqrt(pow(cX-bX,2) + pow(cY-bY,2));

       aoc.push_back( asin((.5 * hypo) / leg1) + 
		      asin((.5 * hypo) / leg2) );

       cd.push_back(  sqrt(pow((.5 * hypo), 2) - pow(leg1,2)) / 
		      hypo                                    );    

    }


    if(rec.time == tIt->start){

      
      if(tIt->type != PAUSE){
        
        tIt->directions = dir;
	tIt->AOCs       = aoc;
	tIt->curveDists = cd;

	
      }

      dir.clear();
      aoc.clear();
      cd.clear();

      wait = tIt->end;

      tIt++;
    }
  }


  
  iter++;
  
  aY = bY;
  aX = bX;
  bX = cX;
  bY = cY;
  cX = rec.x;
  cY = rec.y;
  

}


void FeatureExtractor::CleanOutliers(){

  vector<TimeNode>::iterator tIt = timeLine.begin();
 
  for(;tIt != timeLine.end(); tIt++){

    if(isOutlier(*tIt)){
      timeLine.erase(tIt);
      tIt--;
    }
  }
}


bool FeatureExtractor::isOutlier(TimeNode tn){

  double sngLen = 0, dblLen = 0, s = 0, a = 0, j = 0;
  int    sngTot = 0, dblTot = 0, total = 0;
  for(int i = 0; i < timeLine.size(); i++){

    total++;    

    if(tn.type == DOUBLE && timeLine[i].type == DOUBLE){

      if(abs(tn.length - timeLine[i].length) < dblR)
        dblLen++;
      dblTot++;
    }
    
    if(tn.type == LEFT && timeLine[i].type == LEFT){
	    
      if(abs(tn.length - timeLine[i].length) < sngR)
        sngLen++;
      sngTot++;
    }

    if(tn.type == RIGHT && timeLine[i].type == RIGHT){
	    
      if(abs(tn.length - timeLine[i].length) < sngR)
        sngLen++;
      sngTot++;
    } 

    if(abs(tn.speed - timeLine[i].speed) < sR)
      s++;
    if(abs(tn.accel - timeLine[i].accel) < aR)
      a++;
    if(abs(tn.jerk - timeLine[i].jerk) < jR)
      j++;
  }

  if(tn.type == DOUBLE){

    if(dblLen / dblTot < .67)
      return true;

  }else{

    if(sngLen / sngTot < .67)
      return true;
  }


  if(s / total < .67 || a / total < .67 || j / total < .67)
    return true;
  
  return false;
}


//////////////////////////////////////////
//Main Loop///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////





void FeatureExtractor::ExtractFrom(std::string fName, double snR /*= 35*/, double dbR /*= 200*/){
  
  fileName = fName;       //Save name          
  inFile.open(("./raw/" + fileName).c_str());

  static int tlPause  = 0;           //For the pause extraction
                                     //Optimizes a bit so it doesn't double look at records
  static int tlLClick = 0;           //Next two are same thing for left and right clicks
  static int tlRClick = 0;

  //Put them in the global space 
  sngR = snR;
  dblR = dbR;
  sR =
  aR =
  jR = 1;
 
  inFile >> currentRec.time   >> currentRec.x      >> currentRec.y
         >> currentRec.lClick >> currentRec.rClick ;

  ///First Loop to find Features that act as markers for other Extractions///

  while(inFile.good()){

    ExtractLeftClick( currentRec );


    ExtractRightClick( currentRec );


    ExtractPause( currentRec ); 


    lastRecTime = currentRec.time;


    inFile >> currentRec.time   >> currentRec.x      >> currentRec.y
           >> currentRec.lClick >> currentRec.rClick ;

  }



  FindDoubles(); //Goes through timeLine and finds double clicks


  inFile.close();
  inFile.open(("./raw/" + fName).c_str());
  
  //Check for click fragment in beginning
  if(timeLine.begin()->start == 0)
    timeLine.erase(timeLine.begin());

  ///Second Loop///
  inFile >> currentRec.time   >> currentRec.x      >> currentRec.y
         >> currentRec.lClick >> currentRec.rClick ;
  
  while(inFile.good()){

    ExtractSpeed( currentRec );

    ExtractAngle( currentRec );

    inFile >> currentRec.time   >> currentRec.x      >> currentRec.y
         >> currentRec.lClick >> currentRec.rClick ;

  }


 
  CleanOutliers();

  arf.open(("./features/MouseRecords.arff"), ios::app);
  arf.setf(ios_base::fixed);
  arf.precision(10);

  for(int i = 0 ; i < timeLine.size(); i++){
    AddRecARF(timeLine[i]);
  }

  CalcFeatures();


  inFile.close();
  outFile.close();
}
