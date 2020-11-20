#pragma once
#include <string>
#include <chrono>

#include "EuroScopePlugIn.h"



using namespace std;
using namespace EuroScopePlugIn;

const string MY_PLUGIN_NAME = "GCA";
const string MY_PLUGIN_VERSION = "20w47a (Alpha)";
const string MY_PLUGIN_DEVELOPER = "Paul Hollmann, Paul à Brassard";
const string MY_PLUGIN_COPYRIGHT = "(c)2019-2020";

const int TAG_ITEM_GCA_SIDCLIMB = 200;
const int TAG_FUNC_GCA_SIDCLIMB_MENU = 201;
const int TAG_ITEM_GCA_SIDRWY = 202;
const int TAG_ITEM_GCA_ACTYPE = 203;


class CGermanControllerAssist : public EuroScopePlugIn::CPlugIn
{
private:
	//functions
	string getTagItemSIDCLIMB(CFlightPlan FlightPlan);
	string getTagItemSIDRWY(CFlightPlan FlightPlan);
	string getTagItemACTYPE(CFlightPlan FlightPlan);

	//variables
	bool assign_equipment_code;
	bool show_heavy_engines;
	bool set_temp_alt;


public:
	CGermanControllerAssist();
	virtual ~CGermanControllerAssist();

	//functions
	void OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	bool OnCompileCommand(const char* sCommandLine);

};
