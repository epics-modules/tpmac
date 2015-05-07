/*
 * pmacCsGroup.h
 *
 *  Created on: 7 May 2015
 *      Author: hgv27681
 */

#ifndef PMACAPP_PMACASYNMOTORPORTSRC_PMACCSGROUPS_H_
#define PMACAPP_PMACASYNMOTORPORTSRC_PMACCSGROUPS_H_

#define MAX_AXIS_DEF 32

#include <stdio.h>
#include <vector>
#include <string>
#include <map>

class pmacController;

class pmacCsGroups
{
public:
	pmacCsGroups(pmacController *pController);
	virtual ~pmacCsGroups();

	void addGroup(int id, char* name, int axisCount);
	void addAxisToGroup(int id, int axis, char* axisDef, int coordSysNumber);
	void switchToGroup(int id);

private:
	struct pmacCsAxisDef
	{
		std::string axisDefinition;
		int	axisNo;
		int	coordSysNumber;
	};

	typedef std::vector <pmacCsAxisDef> pmacCsAxisDefList;
	struct pmacCsGroup
	{
		std::string	name;
		pmacCsAxisDefList axisDefs;
	};

	typedef std::map <int, pmacCsGroup> pmacCsGroupList;

	pmacCsGroupList	csGroups;
	pmacController *pC_;

	friend class pmacController;
};

#endif /* PMACAPP_PMACASYNMOTORPORTSRC_PMACCSGROUPS_H_ */
