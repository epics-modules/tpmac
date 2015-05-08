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

pmacCsGroups::pmacCsGroups(pmacController *pController) :
		pC_(pController)
{
}

pmacCsGroups::~pmacCsGroups()
{
	delete pC_;
}

void pmacCsGroups::addGroup(int id, char* name, int axisCount)
{
	// axisCount is not required for this implementation - keeping it in case we need to drop STL
	pmacCsGroup group;
	group.name = name;

	csGroups[id] = group;
}

void pmacCsGroups::addAxisToGroup(int id, int axis, char* axisDef,
		int coordSysNumber)
{
	pmacCsAxisDef def;
	def.axisDefinition = axisDef;
	def.axisNo = axis;
	def.coordSysNumber = coordSysNumber;

	csGroups[id].axisDefs.push_back(def);
}

asynStatus pmacCsGroups::switchToGroup(int id)
{
	static const char *functionName = "pmacCsGroups::switchToGroup";
	char command[PMAC_MAXBUF];
	char response[PMAC_MAXBUF];
	asynStatus cmdStatus;

	if (csGroups.count(id) != 1)
	{
		cmdStatus = asynError;
		asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Invalid Coordinate System Group Number\n", functionName);
	}
	else
	{
		pmacCsAxisDefList *pAxisDefs = &csGroups[id].axisDefs;

		sprintf(command, "undefine all");
		cmdStatus = pC_->lowLevelWriteRead(command, response);

		if (cmdStatus == asynSuccess)
		{
			for (size_t i = 0; i < pAxisDefs->size(); i++)
			{
				sprintf(command, "&%d #%d->%s", (*pAxisDefs)[i].coordSysNumber,
						(*pAxisDefs)[i].axisNo,
						(*pAxisDefs)[i].axisDefinition.c_str());
				cmdStatus = pC_->lowLevelWriteRead(command, response);
			}
		}
	}
	return cmdStatus;
}

