#include "stdafx.h"
#include "CGermanControllerAssist.h"



using namespace std;

CGermanControllerAssist::CGermanControllerAssist() : EuroScopePlugIn::CPlugIn(
	EuroScopePlugIn::COMPATIBILITY_CODE,
	MY_PLUGIN_NAME.c_str(),
	MY_PLUGIN_VERSION.c_str(),
	MY_PLUGIN_DEVELOPER.c_str(),
	MY_PLUGIN_COPYRIGHT.c_str())
{
	RegisterTagItemType("SID Initial Climb", TAG_ITEM_GCA_SIDCLIMB);
	RegisterTagItemType("SID and Runway (Departures only)", TAG_ITEM_GCA_SIDRWY);
	RegisterTagItemType("Aircraft Type and Category (Advanced)", TAG_ITEM_GCA_ACTYPE);


	assign_equipment_code = FALSE;
	show_heavy_engines = FALSE;
	if (GetDataFromSettings("assign_equipment_code") == NULL)
	{
		SaveDataToSettings("assign_equipment_code", "assign_equipment_code", "off");
	}
	else if (GetDataFromSettings("assign_equipment_code") == "on")
	{
		assign_equipment_code = TRUE;
	}
	if (GetDataFromSettings("show_heavy_engines") == NULL)
	{
		SaveDataToSettings("show_heavy_engines", "show_heavy_engines", "off");
	}
	else if (GetDataFromSettings("show_heavy_engines") == "on")
	{
		show_heavy_engines = TRUE;
	}

	DisplayUserMessage("Message", "GermanControllerAssist (GCA)", string("Version " + MY_PLUGIN_VERSION + " loaded").c_str(), false, false, false, false, false);
}

CGermanControllerAssist::~CGermanControllerAssist()
{
}

// public es-plugin functions
void CGermanControllerAssist::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	string tagstring;

	switch (ItemCode) {
	case TAG_ITEM_GCA_SIDCLIMB:
		tagstring = getTagItemSIDCLIMB(FlightPlan);
		break;
	case TAG_ITEM_GCA_SIDRWY:
		tagstring = getTagItemSIDRWY(FlightPlan);
		break;
	case TAG_ITEM_GCA_ACTYPE:
		tagstring = getTagItemACTYPE(FlightPlan);
		break;
	default:
		tagstring = "";
		break;
	}
	if (!tagstring.empty())
	{
		strcpy_s(sItemString, 16, tagstring.c_str());
	}
}

void CGermanControllerAssist::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
}

bool CGermanControllerAssist::OnCompileCommand(const char* sCommandLine)
{
	string cmd = string(sCommandLine);
	if (cmd.find(".gca eqcode") != string::npos)
	{
		assign_equipment_code = !assign_equipment_code;
		if (assign_equipment_code)
		{
			SaveDataToSettings("assign_equipment_code", "assign_equipment_code", "on");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("assign_equipment_code = on").c_str(), true, true, false, true, false);
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("Note you have to the \"Aircraft Type and Category (Advanced)\" item in your departure list").c_str(), true, true, false, true, false);
		}
		else
		{
			SaveDataToSettings("assign_equipment_code", "assign_equipment_code", "off");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("assign_equipment_code = off").c_str(), true, true, false, true, false);
		}
		return TRUE;
	}
	else if (cmd.find(".gca engine") != string::npos)
	{
		show_heavy_engines = !show_heavy_engines;
		if (show_heavy_engines)
		{
			SaveDataToSettings("show_heavy_engines", "show_heavy_engines", "on");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("show_heavy_engines = on").c_str(), true, true, false, true, false);
		}
		else
		{
			SaveDataToSettings("show_heavy_engines", "show_heavy_engines", "off");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("show_heavy_engines = off").c_str(), true, true, false, true, false);
		}
		return TRUE;
	}

	return FALSE;
}



// private internal function functions




// private internal tag functions
string CGermanControllerAssist::getTagItemACTYPE(CFlightPlan FlightPlan)
{
	CFlightPlanData flightPlanData = FlightPlan.GetFlightPlanData();
	string ac_info = flightPlanData.GetAircraftInfo();
	if (
		assign_equipment_code &&
		(ac_info.find("/L") == string::npos && (ac_info.length() == 4) || ac_info.find("/") == 1 && (ac_info.length() == 6))
		)
	{
		ac_info.append("/L");
		flightPlanData.SetAircraftInfo(ac_info.c_str());
		flightPlanData.AmendFlightPlan();
	}
	ac_info = flightPlanData.GetAircraftInfo();
	if (ac_info.find("/") == 1
		&& ac_info.length() > 3)
	{
		ac_info.erase(ac_info.begin());
		ac_info.erase(ac_info.begin());
	}
	ac_info = ac_info.substr(0, 4);
	ac_info.append("/");
	if (show_heavy_engines && flightPlanData.GetAircraftWtc() == 'H')
	{
		ac_info.append(to_string(flightPlanData.GetEngineNumber()));
	}
	ac_info.append(string(1, flightPlanData.GetAircraftWtc()));
	return ac_info;
}


string CGermanControllerAssist::getTagItemSIDRWY(CFlightPlan FlightPlan)
{
	CFlightPlanData flightPlanData;
	string origin, destination, sidName, rwy;
	char rules;
	bool clearence;

	flightPlanData = FlightPlan.GetFlightPlanData();
	origin = flightPlanData.GetOrigin();
	destination = flightPlanData.GetDestination();
	sidName = flightPlanData.GetSidName();
	rwy = flightPlanData.GetDepartureRwy();
	clearence = FlightPlan.GetClearenceFlag();
	rules = *flightPlanData.GetPlanType();
	string my_cs = ControllerMyself().GetCallsign();


	//quick and dirty method
	if (rules == 'V')
	{
		return "VFR/" + rwy;
	}

	if (origin == "EDDS" && (my_cs.find(string("EDDS")) != std::string::npos))
	{
		return sidName + "/" + rwy;
	}
	if (origin == "EDDF" && (my_cs.find(string("EDDF")) != std::string::npos))
	{
		return sidName + "/" + rwy;
	}
	return "";
}


string CGermanControllerAssist::getTagItemSIDCLIMB(CFlightPlan FlightPlan)
{
	CFlightPlanData flightPlanData;
	string origin, destination, sidName, rwy;
	char rules;
	bool clearence;

	flightPlanData = FlightPlan.GetFlightPlanData();
	origin = flightPlanData.GetOrigin();
	destination = flightPlanData.GetDestination();
	sidName = flightPlanData.GetSidName();
	rwy = flightPlanData.GetDepartureRwy();
	clearence = FlightPlan.GetClearenceFlag();
	rules = *flightPlanData.GetPlanType();

	// check vfr
	if (rules == 'V')
	{
		return  "";
	}
	//quick and dirty method
	if (origin == "EDDS")
	{
		return "5000";
	}
	if (origin != "EDDF")
	{
		return "";
	}

	if (sidName == "OBOKA1M"
		|| sidName == "MARUN6M"
		|| sidName == "TOBAK6M"
		|| sidName == "OBOKA1H"
		|| sidName == "MARUN4H"
		|| sidName == "TOBAK4H"
		)
	{
		return "FL70";
	}
	if (sidName == "OBOKA1G"
		|| sidName == "OBOKA1E"
		|| sidName == "OBOKA1D"
		|| sidName == "MARUN7G"
		|| sidName == "MARUN5E"
		|| sidName == "MARUN9D"
		|| sidName == "TOBAK8G"
		|| sidName == "TOBAK9D"
		)
	{
		return "5000";
	}
	if (sidName == "SOBRA1L"
		|| sidName == "ULKIG1L"
		|| sidName == "ANEKI9L"
		|| sidName == "CINDY1S"
		|| sidName == "SULUS9S"
		|| sidName == "SULUS1D"
		)
	{
		return "4000";
	}
	return "!";

}
