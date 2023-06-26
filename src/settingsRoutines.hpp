#include <Arduino.h>
char stationName[50]; //must fit "Kamenný Újezd u Českých Budějovic zastávka"
#define TABLE_LINES 6

struct tableLine {
  bool isNote;                  //if allowed, some lines are actually notes 
  String note;
  String trainType = "\0  ";    //allocates 3 chars (4 B) for the train type
  String trainNumber = "\0     "; //allocates 6 chars (7 B) for the train number 
  String trainLine = "\0   ";   //allocates 4 chars (5 B) for the train line
  String trainCompany = "\0   "; //allocates 4 chars (5 B) for the train company
  String trainName; 
  String schTime = "\0    ";    //allocates 5 chars (6 B) for scheduled time
  String actTime = "\0    ";    //allocates 5 chars (6 B) for actual time 
  int schTimeMin;
  int actTimeMin;
  int delayMin;
  String delayString;           //could be NEJEDE/BUS/- for all we know lol
  String terminus;
  String via[6];                //there never seem to be more than 6 via stations on the website
  int viaNum;
  int viaState;
  String platform = "\0     ";
  String track = "\0     ";
};

struct tableLine table[TABLE_LINES];

