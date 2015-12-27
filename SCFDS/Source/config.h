#ifndef TITANDUPEFINDER_CONFIG_H
#define TITANDUPEFINDER_CONFIG_H

#include <vector>
#include <string>

using namespace std;

typedef struct CONFIGSTRUCT {
	char	m_sDbLogin[20];
	char	m_sDbPassword[20];
	char	m_sDbName[20];
	char	m_sDbServerAddr[30];

	int		SearchEachXMinutes;
	BYTE	UseExtendedWarehouse;

	BYTE	DeleteItems;
	BYTE	BanCharacter;
	BYTE	BanAccount;
	BYTE	SkipNonUpdatedPSHOP;

	char	ItemNewPath[500];
} *PCONFIGSTRUCT;

extern CONFIGSTRUCT g_Config;

void ConfigReadIniFile(PCONFIGSTRUCT pConfig);

#endif //TITANDUPEFINDER_CONFIG_H