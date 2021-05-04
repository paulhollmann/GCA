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
	set_temp_alt = FALSE;
	string assign_equipment_code_setting = "";
	string show_heavy_engines_setting = "";
	string set_temp_alt_setting = "";

	if(GetDataFromSettings("assign_equipment_code") != NULL)
		assign_equipment_code_setting = GetDataFromSettings("assign_equipment_code");
	if (GetDataFromSettings("show_heavy_engines") != NULL)
		show_heavy_engines_setting = GetDataFromSettings("show_heavy_engines");
	if (GetDataFromSettings("set_temp_alt") != NULL)
		set_temp_alt_setting = GetDataFromSettings("set_temp_alt");

	if (assign_equipment_code_setting.empty())
	{
		SaveDataToSettings("assign_equipment_code", "assign_equipment_code", "off");
	}
	else if (assign_equipment_code_setting.find("on") != string::npos)
	{
		assign_equipment_code = TRUE;
	}
	if (show_heavy_engines_setting.empty())
	{
		SaveDataToSettings("show_heavy_engines", "show_heavy_engines", "off");
	}
	else if (show_heavy_engines_setting.find("on") != string::npos)
	{
		show_heavy_engines = TRUE;
	}
	if (set_temp_alt_setting.empty())
	{
		SaveDataToSettings("set_temp_alt", "set_temp_alt", "off");
	}
	else if (set_temp_alt_setting.find("on") != string::npos)
	{
		set_temp_alt = TRUE;
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

void CGermanControllerAssist::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	CFlightPlanData flightPlanData = FlightPlan.GetFlightPlanData();
	CFlightPlanControllerAssignedData controllerAssignedData = FlightPlan.GetControllerAssignedData();
	CRadarTargetPositionData radarTargetPositionData = FlightPlan.GetFPTrackPosition();
	CPosition acposition = radarTargetPositionData.GetPosition();
	CController controller = ControllerMyself();
	CPosition myposition = controller.GetPosition();

	
	string callsign = FlightPlan.GetCallsign();
	int clearedFL = controllerAssignedData.GetClearedAltitude();
	int currentAlt = radarTargetPositionData.GetPressureAltitude();
	bool clearenceFlag = FlightPlan.GetClearenceFlag();
	string sidName = flightPlanData.GetSidName();
	string origin = flightPlanData.GetOrigin();
	string destination = flightPlanData.GetDestination();
	char rules = *flightPlanData.GetPlanType();
	
	
	//ASSIGN /L
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
	// SET THE TEMP ALT
	if (
		set_temp_alt &&
		clearedFL == 0 && 
		!clearenceFlag && 
		(origin.find(destination) == string::npos) &&
		myposition.DistanceTo(acposition) < 10
		) {
		if (rules == 'V')
		{
			return;
		}
		if (origin.find("EDDS") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{	
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDS:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDDK") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDK:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDDG") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDG:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDLW") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDLW:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDDH") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDH:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDDL") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDL:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 5000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (origin.find("EDDW") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDW:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 4000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(4000);
			return;
		}
		if (origin.find("EDLN") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDLN:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to 2000ft");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(2000);
			return;
		}
		if (origin.find("EDDC") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDC:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to FL70");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(7000);
			return;
		}
		if (origin.find("EDDP") != string::npos && currentAlt < 1400 && currentAlt != 0)
		{
			string message = callsign;
			message = message.append(" at ");
			message = message.append(std::to_string(currentAlt));
			message = message.append("ft in EDDP:  set from ");
			message = message.append(std::to_string(clearedFL));
			message = message.append(" to FL70");
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(7000);
			return;
		}
		if (origin.find("EDDF") == string::npos || currentAlt > 400 || currentAlt==0)
		{
			return;
		}
		string message = callsign;
		message = message.append(" at ");
		message = message.append(std::to_string(currentAlt));
		message = message.append("ft in EDDF, via ");
		message = message.append(sidName);
		message = message.append(" set from ");
		message = message.append(std::to_string(clearedFL));
		message = message.append(" to ");
		if (sidName.find("OBOKA1M") != string::npos
			|| sidName.find("TOBAK6M") != string::npos
			|| sidName.find("MARUN6M") != string::npos
			|| sidName.find("OBOKA2W") != string::npos
			|| sidName.find("MARUN2W") != string::npos
			|| sidName.find("TOBAK2W") != string::npos
			|| sidName.find("TOBAK4H") != string::npos
			|| sidName.find("OBOKA1H") != string::npos
			|| sidName.find("MARUN4H") != string::npos
			|| sidName.find("OBOKA2K") != string::npos
			|| sidName.find("MARUN2K") != string::npos
			|| sidName.find("TOBAK2K") != string::npos
			)
		{
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.append("FL70").c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(7000);
			return;
		}
		if (sidName.find("OBOKA2G") != string::npos
			|| sidName.find("MARUN9G") != string::npos
			|| sidName.find("TOBAK1G") != string::npos
			|| sidName.find("MARUN6F") != string::npos
			|| sidName.find("TOBAK7F") != string::npos
			|| sidName.find("SULUS1F") != string::npos
			|| sidName.find("CINDY1F") != string::npos
			|| sidName.find("SOBRA6F") != string::npos
			|| sidName.find("ANEKI9F") != string::npos
			|| sidName.find("OBOKA1E") != string::npos
			|| sidName.find("MARUN5E") != string::npos
			|| sidName.find("MARUN9D") != string::npos
			|| sidName.find("TOBAK9D") != string::npos
			|| sidName.find("OBOKA1D") != string::npos

			|| sidName.find("OBOKA4N") != string::npos
			|| sidName.find("MARUN9N") != string::npos
			|| sidName.find("SOBRA7N") != string::npos
			|| sidName.find("TOBAK2N") != string::npos
			|| sidName.find("SOBRA5P") != string::npos
			)
		{
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.append("5000ft").c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(5000);
			return;
		}
		if (sidName.find("SOBRA1L") != string::npos
			|| sidName.find("SOBRA1U") != string::npos
			|| sidName.find("SOBRA1X") != string::npos
			|| sidName.find("SOBRA1Y") != string::npos
			|| sidName.find("ULKIG1L") != string::npos
			|| sidName.find("ANEKI9L") != string::npos
			|| sidName.find("ANEKI1X") != string::npos
			|| sidName.find("ANEKI1Y") != string::npos
			|| sidName.find("CINDY3S") != string::npos
			|| sidName.find("CINDY1X") != string::npos
			|| sidName.find("CINDY1Y") != string::npos
			|| sidName.find("SULUS2S") != string::npos
			|| sidName.find("SULUS1D") != string::npos
			|| sidName.find("SULUS9L") != string::npos
			|| sidName.find("OBOKA1T") != string::npos
			|| sidName.find("MARUN4T") != string::npos
			|| sidName.find("TOBAK6T") != string::npos
			|| sidName.find("OBOKA1R") != string::npos
			|| sidName.find("MARUN3R") != string::npos
			|| sidName.find("TOBAK3R") != string::npos
			|| sidName.find("OBOKA1L") != string::npos
			|| sidName.find("SOBRA6E") != string::npos
			|| sidName.find("ANEKI4E") != string::npos
			|| sidName.find("KOMIB3D") != string::npos
			|| sidName.find("CINDY1A") != string::npos
			|| sidName.find("ANEKI2A") != string::npos
			|| sidName.find("SULUS2A") != string::npos
			|| sidName.find("MARUN7S") != string::npos
			|| sidName.find("TOBAK9S") != string::npos
			|| sidName.find("ULKIG2S") != string::npos
			)
		{
			DisplayUserMessage(GetPlugInName(), "GCA_temp", message.append("4000ft").c_str(), true, false, false, true, false);
			controllerAssignedData.SetClearedAltitude(4000);
			return;
		}
	}
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
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("NOTE this setting will amend the flightplan, pilots will not be able resend a new one, until they logoff").c_str(), true, true, false, true, true);
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("To turn the setting off type '.gca eqcode' again").c_str(), true, true, false, true, false);
			
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
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("Note you have to add the \"Aircraft Type and Category (Advanced)\" item in your departure list").c_str(), true, true, false, true, false);
		}
		else
		{
			SaveDataToSettings("show_heavy_engines", "show_heavy_engines", "off");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("show_heavy_engines = off").c_str(), true, true, false, true, false);
		}
		return TRUE;
	}
	else if (cmd.find(".gca temp") != string::npos)
	{
		set_temp_alt = !set_temp_alt;
		if (set_temp_alt)
		{
			SaveDataToSettings("set_temp_alt", "set_temp_alt", "on");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("set_temp_alt = on").c_str(), true, true, false, true, false);
		}
		else
		{
			SaveDataToSettings("set_temp_alt", "set_temp_alt", "off");
			DisplayUserMessage(GetPlugInName(), "GermanControllerAssist", string("set_temp_alt = off").c_str(), true, true, false, true, false);
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
	if (origin == "EDDK" || origin == "EDDS" || origin == "EDDL" || origin == "EDDH" || origin == "EDDG" || origin == "EDLW")
	{
		return "5000";
	}
	if (origin == "EDDW")
	{
		return "4000";
	}
	if (origin == "EDLN")
	{
		return "2000";
	}
	if (origin == "EDDC" || origin == "EDDP")
	{
		return "7000";
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
		|| sidName == "OBOKA2K"
		|| sidName == "MARUN2K"
		|| sidName == "TOBAK2K"
		|| sidName == "OBOKA2W"
		|| sidName == "MARUN2W"
		|| sidName == "TOBAK2W"
		)
	{

		return "FL70";
	}
	if (sidName == "OBOKA2G"
		|| sidName == "OBOKA1E"
		|| sidName == "OBOKA1D"
		|| sidName == "MARUN9G"
		|| sidName == "MARUN6F"
		|| sidName == "MARUN5E"
		|| sidName == "MARUN9D"
		|| sidName == "TOBAK9G"
		|| sidName == "TOBAK7F"
		|| sidName == "TOBAK9D"
		|| sidName == "SOBRA6F"
		|| sidName == "SOBRA6N"
		|| sidName == "CINDY1F"
		|| sidName == "SULUS1F"
		|| sidName == "OBOKA2N"
		|| sidName == "MARUN7N"
		|| sidName == "TOBAK9N"
		|| sidName == "ANEKI9F"
		|| sidName == "SOBRA5P"
		)
	{
		return "5000";
	}
	if (sidName == "SOBRA1L"
		|| sidName == "SOBRA1U"
		|| sidName == "SOBRA1X"
		|| sidName == "SOBRA1Y"
		|| sidName == "ULKIG1L"
		|| sidName == "ANEKI9L"
		|| sidName == "ANEKI2A"
		|| sidName == "ANEKI1X"
		|| sidName == "ANEKI1Y"
		|| sidName == "CINDY3S"
		|| sidName == "CINDY1X"
		|| sidName == "CINDY1Y"
		|| sidName == "SULUS2S"
		|| sidName == "SULUS1D"
		|| sidName == "SULUS9L"
		|| sidName == "OBOKA1R"
		|| sidName == "MARUN3R"
		|| sidName == "TOBAK3R"
		|| sidName == "OBOKA1T"
		|| sidName == "MARUN4T"
		|| sidName == "TOBAK6T"
		|| sidName == "OBOKA1L"
		|| sidName == "ANEKI4E"
		|| sidName == "SOBRA6E"
		|| sidName == "KOMIB3D"
		|| sidName == "CINDY1A"
		|| sidName == "ANEKI2A"
		|| sidName == "SULUS2A"
		|| sidName == "MARUN7S"
		|| sidName == "ULKIG2S"
		|| sidName == "TOBAK9S"
		)
	{
		return "4000";
	}
	return "!";

}
