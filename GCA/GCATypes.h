#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <regex>

#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace gca {
	struct EventRoute
	{
		string destination;
		string origin;
		string route;

		static const unsigned int FP_NO_EVENT = 0;
		static const unsigned int FP_VALID_EVENT = 1;
		static const unsigned int FP_INVAID_EVENT = 2;

		EventRoute(string origin, string destination,  string route);
		string routeTrim(string route);
		unsigned int matchFP(CFlightPlan FlightPlan);
	};

	struct Airport {
		string name;
		int maxassignalt = 0;
		vector<SID> sids;
		Airport(string name);
	};
	struct SID {
		string name;
		int climb = 0;
		SID(string origin, string name);
		bool matchFP(CFlightPlan FlightPlan);
	};
}