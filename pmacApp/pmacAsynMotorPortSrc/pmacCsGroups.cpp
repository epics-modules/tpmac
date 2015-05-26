/*
 * pmacCsGroup.cpp
 *
 *  Created on: 7 May 2015
 *      Author: Giles Knap
 *
 *  This class supports the controller's ability to switch axes in and out of coordinate systems.
 *
 *  It defines a collection of groups which each list axes, and their definition within a Coordinate system
 *  It also allows for coordinated deferred moves of real axes in a CS where they are directly mapped to
 *  CS axes. This last feature currently requires the following PROG101 installed on the geobrick.
 *
 *  It also requires that lookahead buffers are defined for the CS in question + the following (n = CS number)
 *  i5n13=10   ; segmentation time (needed for lookahead)
 *  i5n20=50   ; lookahead length (needed to limit max velocity to max set in CS)
 *  i5n50=0    ; DISable kinematics (unless the CS does have a FORWARD/REVERSE kinematic)

 OPEN PROG 101
 CLEAR
 LINEAR
 ABS
 TM(Q70)
 FRAX(A,B,C,U,V,W,X,Y,Z)
 A(Q71)B(Q72)C(Q73)U(Q74)V(Q75)W(Q76)X(Q77)Y(Q78)Z(Q79)
 DWELL0
 CLOSE

 */

#include "pmacCsGroups.h"
#include "pmacController.h"
#include "math.h"

// offset to get i-variables for coordinate systems
#define CSVAR(n) n+50

pmacCsGroups::pmacCsGroups(pmacController *pController) :
		pC_(pController),
		currentGroup(0)
{
	// Note assume all axes are not in a coordinate system initially
	// PINI of the $(P):COORDINATE_SYS_GROUP will set the cs group later

	// set up a map of axis names and the number of the Q variable used
	// to control it (in DLS standard prog10)
	axisNamesToQ['A'] = 71;
	axisNamesToQ['B'] = 72;
	axisNamesToQ['C'] = 73;
	axisNamesToQ['U'] = 74;
	axisNamesToQ['V'] = 75;
	axisNamesToQ['W'] = 76;
	axisNamesToQ['X'] = 77;
	axisNamesToQ['Y'] = 78;
	axisNamesToQ['Z'] = 79;
}

pmacCsGroups::~pmacCsGroups()
{
}

void pmacCsGroups::addGroup(int id, char* name, int axisCount)
{
	// axisCount is not required for this implementation - keeping it
	// in case we need to drop STL
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

	csGroups[id].axisDefs[axis] = def;

	printf("#### addAxisToGroup - axis %d, CS %d, def %s, COUNT %d\n",axis, coordSysNumber, axisDef, (int)csGroups[id].axisDefs.size());
}

int pmacCsGroups::getAxisCoordSys(int axis)
{
	return csGroups[currentGroup].axisDefs[axis].coordSysNumber;
}

asynStatus pmacCsGroups::switchToGroup(int id)
{
	static const char *functionName = "pmacCsGroups::switchToGroup";
	char command[PMAC_MAXBUF] = {0};
	char response[PMAC_MAXBUF]= {0};
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
			currentGroup = id;
			for (size_t i = 0; i < pAxisDefs->size() &&	cmdStatus == asynSuccess; i++)
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

asynStatus pmacCsGroups::abortMotion(int axis)
{
	char command[PMAC_MAXBUF] = {0};
	char response[PMAC_MAXBUF] = {0};
	int coordSysNum = getAxisCoordSys(axis);
	asynStatus status = asynSuccess;

	if(coordSysNum != 0)
	{
		sprintf(command, "&%dA", coordSysNum);
		printf("#### abort move = %s\n", command);

		//Execute the deferred move
		status = pC_->lowLevelWriteRead(command, response);
		printf("#### abort response = %s\n", response);
	}

	return status;
}

asynStatus pmacCsGroups::processDeferredCoordMoves(void)
{
	asynStatus status = asynSuccess;
	pmacAxis *pAxis = NULL;
	static const char *functionName = "pmacCsGroups::processDeferredCoordMoves";
	int coordSysNumber = 0;
	pmacCsaxisNamesToQ moveList;
	float accel = 100, veloc = 100; // TODO hard coded for the moment
	float maxTimeToMove = 0;

	asynPrint(pC_->pasynUserSelf, ASYN_TRACE_FLOW, "%s\n", functionName);
	printf("#### verifying defer move from axisdef list of %d axes\n", (int)csGroups[currentGroup].axisDefs.size());

	// Scan all axes and verify that a valid coordinated deferred move has been made.
	// All real axes with a deferred move request must be in the same coordinate system
	// and have a one to one mapping to a coordinate system axis
	for (int axis = 0; axis < pC_->numAxes_ && status == asynSuccess; axis++)
	{
		pAxis = pC_->getAxis(axis);
		if (pAxis != NULL)
		{
			if (pAxis->deferredMove_)
			{
				if(coordSysNumber == 0)
				{
					abortMotion(axis);
					coordSysNumber = getAxisCoordSys(axis);
					if(coordSysNumber == 0)
					{
						status = asynError;
						asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
								"%s Error: deferred coordinated move on real axis %d not in a coordinate system\n",
								functionName, axis);
					}
				}
				else
				{
					if(getAxisCoordSys(axis) != coordSysNumber)
					{
						status = asynError;
						asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
								"%s Error: deferred coordinated move on multiple coordinate systems %d and %d\n",
								functionName, coordSysNumber, getAxisCoordSys(axis));
					}
				}

				std::string axisDef = csGroups[currentGroup].axisDefs[axis].axisDefinition;
				if(axisDef.length() != 1 || axisNamesToQ.count(axisDef[0]) != 1)
				{
					status = asynError;
					asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
							"%s Error: illegal deferred coordinated move on real axis %d defined as %s in CS %d\n",
							functionName, axis, axisDef.c_str(), coordSysNumber);
				}
				else
				{
					moveList[axisDef[0]] = axis;
					float time = pAxis->deferredTime_;
					maxTimeToMove = (time > maxTimeToMove) ? time : maxTimeToMove;
				}
			}
		}
	}

	// we now have a list of axes that need to move - build the pmac command to do so
	if(status == asynSuccess && moveList.size() > 0)
	{
		char command[PMAC_MAXBUF] = {0};
		char response[PMAC_MAXBUF] = {0};
		char moveStr[PMAC_MAXBUF] = {0};

		pmacCsaxisNamesToQ::iterator it;
		for (it = moveList.begin(); it != moveList.end() && status == asynSuccess; it++)
		{
			char axisDef = it->first;
			int axisNum = it->second;

			pAxis = pC_->getAxis(axisNum);

			sprintf(moveStr, "%s Q%d=%f", moveStr, axisNamesToQ[axisDef], pAxis->deferredPosition_);
		}

		// add in the axes in this CS that have not had a deferred move - send their current position
		for (int axis = 0; axis < pC_->numAxes_ && status == asynSuccess; axis++)
		{
			if(getAxisCoordSys(axis) == coordSysNumber)
			{
				pAxis = pC_->getAxis(axis);
				if (pAxis != NULL && !pAxis->deferredMove_)
				{
					std::string axisDef = csGroups[currentGroup].axisDefs[axis].axisDefinition;
					sprintf(moveStr, "%s Q%d=%f", moveStr, axisNamesToQ[axisDef[0]], pAxis->previous_position_);
				}
			}
		}

		sprintf(command, "I%d87=%f I%d89=%f ", CSVAR(coordSysNumber), accel, CSVAR(coordSysNumber), veloc);
		sprintf(command, "%s &%d Q70=%f %s B101R", command, coordSysNumber, maxTimeToMove, moveStr);

		//Execute the deferred move
		printf("#### deferred move = %s\n", command);
		if (pC_->lowLevelWriteRead(command, response) != asynSuccess)
		{
			asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
					"%s ERROR Sending Deferred Move Command.\n", functionName);
			pC_->setIntegerParam(pC_->PMAC_C_CommsError_, pC_->PMAC_ERROR_);
			status = asynError;
		}
		else
		{
			pC_->setIntegerParam(pC_->PMAC_C_CommsError_, pC_->PMAC_OK_);
			status = asynSuccess;
		}
	}

	//Clear deferred move flag for the axes involved (do this even if there was an error)
	for (int axis = 0; axis < pC_->numAxes_; axis++)
	{
		pAxis = pC_->getAxis(axis);
		if (pAxis != NULL)
		{
			if (pAxis->deferredMove_)
			{
				pAxis->deferredMove_ = 0;
				// TODO this is temporary
				pAxis->deferredPosition_ = 0;
			}
		}
	}
	return status;
}
