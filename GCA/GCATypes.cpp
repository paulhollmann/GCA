#include "stdafx.h"
#include "GCATypes.h"

using namespace std;

gca::EventRoute::EventRoute(string origin, string destination, string route)
{
	this->destination = destination;
	this->origin = origin;
	this->route = routeTrim(route);
}

string gca::EventRoute::routeTrim(string route)
{
	regex r(" DCT ");
	route = std::regex_replace(route, r, " ");

	r = regex("\/[A-Z0-9]{3,10} ");
	route = std::regex_replace(route, r, " ");

	r = regex("  ");
	route = std::regex_replace(route, r, " ");


	return route;
}

unsigned int gca::EventRoute::matchFP(CFlightPlan FlightPlan)
{
	CFlightPlanData fpdata = FlightPlan.GetFlightPlanData();

	if (this->destination.find(string(fpdata.GetDestination())) == string::npos) return FP_NO_EVENT;
	if (this->origin.find(string(fpdata.GetOrigin())) == string::npos) return FP_NO_EVENT;

	string route = routeTrim(fpdata.GetRoute());
	string reference = routeTrim(this->route);

	if (route.find(reference) != string::npos) return FP_VALID_EVENT;

	return FP_INVAID_EVENT;
}

gca::SID::SID(string origin, string name)
{
}

bool gca::SID::matchFP(CFlightPlan FlightPlan)
{
	CFlightPlanData flightPlanData = FlightPlan.GetFlightPlanData();

	return false;
}

gca::Airport::Airport(string name)
{
	this->name = name;
}
