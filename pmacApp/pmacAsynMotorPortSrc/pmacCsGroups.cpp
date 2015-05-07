/*
 * pmacCsGroup.cpp
 *
 *  Created on: 7 May 2015
 *      Author: Giles Knap
 *
 *  This class supports the controller's ability to switch axes in and out of coordinate systems.
 *
 *  It defines a collection of groups which each list axes, and their definition within a Coordinate system
 */

#include "pmacCsGroups.h"
#include "pmacController.h"

pmacCsGroups::pmacCsGroups(pmacController *pController):
	pC_(pController)
{
}

pmacCsGroups::~pmacCsGroups()
{
}

void pmacCsGroups::addGroup(int id, char* name, int axisCount)
{
	// axisCount is not required for this implementation - keeping it in case we need to drop STL
	pmacCsGroup group;
	group.name = name;

	csGroups[id] = group;
}

void pmacCsGroups::addAxisToGroup(int id, int axis, char* axisDef, int coordSysNumber)
{
	pmacCsAxisDef def;
	def.axisDefinition = axisDef;
	def.axisNo = axis;
	def.coordSysNumber = coordSysNumber;

	csGroups[id].axisDefs.push_back(def);
}

void pmacCsGroups::switchToGroup(int id)
{
	char command[PMAC_MAXBUF] = {0};
	char response[PMAC_MAXBUF] = {0};
	int cmdStatus = 0;

	pmacCsAxisDefList *pAxisDefs = & csGroups[id].axisDefs;

	for(size_t i=0; i<pAxisDefs->size(); i++)
	{
		sprintf(command, "&%d #%d->%s", (*pAxisDefs)[i].coordSysNumber, (*pAxisDefs)[i].axisNo, (*pAxisDefs)[i].axisDefinition.c_str());
		cmdStatus = pC_->lowLevelWriteRead(command, response);
	}
}

