#include "stdafx.h"
#include "config.h"
#include "WinUtil.h"

CONFIGSTRUCT g_Config;

void ConfigReadIniFile(PCONFIGSTRUCT pConfig)
{
	GetPrivateProfileString("Connection","ServerODBC","MuOnline",g_Config.m_sDbServerAddr,sizeof(g_Config.m_sDbServerAddr),FILEPATH);
	GetPrivateProfileString("Connection","Name","MuOnline",g_Config.m_sDbName,sizeof(g_Config.m_sDbName),FILEPATH);
	GetPrivateProfileString("Connection","Login","sa",g_Config.m_sDbLogin,sizeof(g_Config.m_sDbLogin),FILEPATH);
	GetPrivateProfileString("Connection","Password","654321",g_Config.m_sDbPassword,sizeof(g_Config.m_sDbPassword),FILEPATH);

	g_Config.SkipNonUpdatedPSHOP = GetPrivateProfileInt("Config","SkipNonUpdatedPSHOP",1,FILEPATH);
	int TmpMins = GetPrivateProfileInt("Config","SearchEachXMinutes",60,FILEPATH);
	g_Config.SearchEachXMinutes = TmpMins * 60000;
	g_Config.UseExtendedWarehouse = GetPrivateProfileInt("Config","UseExtendedWarehouse",0,FILEPATH);

	g_Config.DeleteItems = GetPrivateProfileInt("Action","DeleteItems",0,FILEPATH);
	g_Config.BanAccount = GetPrivateProfileInt("Action","BanCharacter",0,FILEPATH);
	g_Config.BanCharacter = GetPrivateProfileInt("Action","BanAccount",0,FILEPATH);

	GetPrivateProfileString("Config","ItemNewPath","D:\\MuServer\\Data\\Lang\\Kor\\Item(new).txt",g_Config.ItemNewPath,sizeof(g_Config.ItemNewPath),FILEPATH);
}