#include "stdafx.h"
#include "config.h"
#include "winutil.h"
#include "shared.h"
#include "DB/Query.h"
#include "../stdafx.h"
#include "Log.h"

CQuery db;
#define INVENTORY_COUNT			3776
#define VAULT_COUNT				3840

#define VAULT_ITEMS				240
#define INV_ITEMS				236


typedef struct _tagItem
{ 
	BYTE	itemtype;
	BYTE	itemoption;
	BYTE	itemdurability;
	//DWORD	itemserial;
	BYTE	itemserial1;
	BYTE	itemserial2;
	BYTE	itemserial3;
	BYTE	itemserial4;

	BYTE	itemexemod;
	BYTE	itemexereserved;
	BYTE	type2;
	BYTE	harmony;
	BYTE	sock1;
	BYTE	sock2;
	BYTE	sock3;
	BYTE	sock4;
	BYTE	sock5;
} ITEM; 

BYTE GetType(ITEM *item)
{
	return (item->type2 >> 4);
}

typedef struct _tagCharacter
{ 
	ITEM	inventory[INV_ITEMS];
	char	name[11];
	char	account[11];
} CHARACTER, *PCHARACTER;

typedef struct _tagAccount
{ 
	ITEM	warehouse[VAULT_ITEMS];
	char	account[11];
	int		ExtWare;
} ACCOUNT, *PACCOUNT;

typedef struct _tagItemArrayElement
{ 
	ITEM	item;
	DWORD	account_index;
	DWORD	character_index;
	DWORD	item_index;
	BOOL	isvault;
	int		ExtWare;
} ITEMARRAYELEMENT, *PITEMARRAYELEMENT;

void InitDB()
{
	//int aa = sizeof(ITEM);
	if ( db.Connect(g_Config.m_sDbServerAddr, g_Config.m_sDbLogin, g_Config.m_sDbPassword) == TRUE )
	{
		LogAddC(4, "[DB] Connection Successfull!");
	}
	else
	{
		LogAddC(2, "[DB] Error On Connection!");
	}
}

bool IsSpecialSerial(DWORD Serial)
{
	if((Serial >= 0xfffffff0) && (Serial <= 0xffffffff))
		return true;
	
	if(Serial == 0x00000000) 
		return true;

	if(Serial == 0xcccccccc)
		return true;

	return false;
}

DWORD GetSerial(ITEM *item)
{
	WORD ser1 = MAKE_NUMBERW(item->itemserial1,item->itemserial2);
	WORD ser2 = MAKE_NUMBERW(item->itemserial3,item->itemserial4);

	return MAKE_NUMBERDW(ser1,ser2);
}

bool IsEmptyItem(ITEM *item)
{
	short m_Type = item->itemtype +  GetType(item) * 512 ;

	if(IsItem(m_Type) == false)
		return true;

	DWORD itemserial = GetSerial(item);

	//DWORD ReversedSerial = DWORD_Reverse(itemserial);

	//if(IsSpecialSerial(ReversedSerial) == true)
	//	return true;

	if(IsSpecialSerial(itemserial) == true)
		return true;
	
	return false;
}

void ToHexSinComa(char * out, char * data, int c_len)
{
	char cBuff[5001];
	c_len = c_len & 0xff;
	memset(cBuff, 0, 5000);
	for(int i=0;i<c_len;i++)
	{
		wsprintf(cBuff+(i*2),"%02X",data[i]);
//		s=cBuff;
//		t+=s;
	}
	memcpy(out,cBuff,(c_len*3)+2);
}

PACCOUNT paccount;
PCHARACTER pcharacter;
PITEMARRAYELEMENT pitemarray;

/*
//Change Serial of Dif Items with same Serial


DWORD GetNewSerial()
{	
	// ----
	db.ExecQuery("EXEC WZ_GetItemSerial");
	db.Fetch();
	// ----
	DWORD Number				= db.GetResult(0);
	// ----
	db.Close();
	return Number;
}

void ChangeSerial(int CurrentPos, int Max)
{	
	bool Need = false;
	short m_Type2 = pitemarray[CurrentPos].item.itemtype +  GetType(&pitemarray[CurrentPos].item) * 512 ;
	BYTE btItemType2=0;
	btItemType2 |= (m_Type2 & 0x1E00 ) >> 5;
	
	if(!IsEmptyItem(&pitemarray[CurrentPos].item))
	{
		for(int i=0;i<Max;i++)
		{
			if(i != CurrentPos)
			{
				if(pitemarray[CurrentPos].item.itemserial == pitemarray[i].item.itemserial)
				{
					short m_Type = pitemarray[i].item.itemtype +  GetType(&pitemarray[i].item) * 512 ;
					if(m_Type2 != m_Type)
					{
						BYTE btItemType=0;
						btItemType |= (m_Type & 0x1E00 ) >> 5;
						if( ((btItemType+2) == pitemarray[i].item.itemexereserved) && ((btItemType2+2) == pitemarray[CurrentPos].item.itemexereserved) )
						{
							//CSHOP
						}else
						{
							if(Need == false)
							{
								LogAddTD("-------------------------------- NeedChange START --------------------------------");
								Need = true;
							}

							//Generate New Serial
							if(pitemarray[i].isvault == TRUE)
							{
								LogAddC(2,"[Vault][%s] Serial [%d] Item [%d][%s][Pos:%d]",paccount[pitemarray[i].account_index].account,pitemarray[i].item.itemserial,m_Type,ItemAttribute[m_Type].Name,pitemarray[i].item_index);
							
								if(g_Config.DeleteItems == 1)
								{
									pitemarray[i].item.itemserial = GetNewSerial();
									paccount[pitemarray[i].account_index].warehouse[i].itemserial = pitemarray[i].item.itemserial;
									char szTemp[10000]={0};
									BYTE dbWarehouse[VAULT_COUNT]	= {0};
									memcpy(dbWarehouse, paccount[pitemarray[i].account_index].warehouse, sizeof(paccount[pitemarray[i].account_index].warehouse));
									
									wsprintf(szTemp, "UPDATE warehouse SET Items=? WHERE AccountID = '%s'", paccount[pitemarray[i].account_index].account);
									db.SetAsBinary(szTemp, dbWarehouse, sizeof(dbWarehouse));
									db.Fetch();
									db.Close();
								}else
									memset(&pitemarray[i].item,0xFF,sizeof(ITEM));
							}else
							{
								LogAddC(2,"[Inventory][%s][%s] Serial [%d] Item [%d][%s][Pos:%d]",pcharacter[pitemarray[i].character_index].account,pcharacter[pitemarray[i].character_index].name,pitemarray[i].item.itemserial,m_Type,ItemAttribute[m_Type].Name,pitemarray[i].item_index);
								if(g_Config.DeleteItems == 1)
								{
									pitemarray[i].item.itemserial = GetNewSerial();
									pcharacter[pitemarray[i].character_index].inventory[i].itemserial = pitemarray[i].item.itemserial;
									char szTemp[10000]={0};
									BYTE dbInventory[INVENTORY_COUNT]	= {0};
									memcpy(dbInventory, pcharacter[pitemarray[i].character_index].inventory, sizeof(pcharacter[pitemarray[i].character_index].inventory));

									wsprintf(szTemp, "UPDATE character SET Inventory=? WHERE Name = '%s' AND AccountID = '%s'", pcharacter[pitemarray[i].character_index].name, pcharacter[pitemarray[i].character_index].account);
									db.SetAsBinary(szTemp, dbInventory, sizeof(dbInventory));
									db.Fetch();
									db.Close();
								}else
									memset(&pitemarray[i].item,0xFF,sizeof(ITEM));
							}
						}
					}
				}
			}
		}
	}
	if(Need == true)
	{
		if(pitemarray[CurrentPos].isvault == TRUE)
		{
			LogAddC(2,"[Vault][%s] Serial [%d] Item [%d][%s][Pos:%d]",paccount[pitemarray[CurrentPos].account_index].account,pitemarray[CurrentPos].item.itemserial,m_Type2,ItemAttribute[m_Type2].Name,pitemarray[CurrentPos].item_index);

			if(g_Config.DeleteItems == 1)
			{
				pitemarray[CurrentPos].item.itemserial = GetNewSerial();
				paccount[pitemarray[CurrentPos].account_index].warehouse[CurrentPos].itemserial = pitemarray[CurrentPos].item.itemserial;
				char szTemp[10000]={0};
				BYTE dbWarehouse[VAULT_COUNT]	= {0};
				memcpy(dbWarehouse, paccount[pitemarray[CurrentPos].account_index].warehouse, sizeof(paccount[pitemarray[CurrentPos].account_index].warehouse));
				
				wsprintf(szTemp, "UPDATE warehouse SET Items=? WHERE AccountID = '%s'", paccount[pitemarray[CurrentPos].account_index].account);
				db.SetAsBinary(szTemp, dbWarehouse, sizeof(dbWarehouse));
				db.Fetch();
				db.Close();
			}else
				memset(&pitemarray[CurrentPos].item,0xFF,sizeof(ITEM));
		}else
		{
			LogAddC(2,"[Inventory][%s][%s] Serial [%d] Item [%d][%s][Pos:%d]",pcharacter[pitemarray[CurrentPos].character_index].account,pcharacter[pitemarray[CurrentPos].character_index].name,pitemarray[CurrentPos].item.itemserial,m_Type2,ItemAttribute[m_Type2].Name,pitemarray[CurrentPos].item_index);
			if(g_Config.DeleteItems == 1)
			{
				pitemarray[CurrentPos].item.itemserial = GetNewSerial();
				pcharacter[pitemarray[CurrentPos].character_index].inventory[CurrentPos].itemserial = pitemarray[CurrentPos].item.itemserial;
				char szTemp[10000]={0};
				BYTE dbInventory[INVENTORY_COUNT]	= {0};
				memcpy(dbInventory, pcharacter[pitemarray[CurrentPos].character_index].inventory, sizeof(pcharacter[pitemarray[CurrentPos].character_index].inventory));

				wsprintf(szTemp, "UPDATE character SET Inventory=? WHERE Name = '%s' AND AccountID = '%s'", pcharacter[pitemarray[CurrentPos].character_index].name, pcharacter[pitemarray[CurrentPos].character_index].account);
				db.SetAsBinary(szTemp, dbInventory, sizeof(dbInventory));
				db.Fetch();
				db.Close();
			}else
				memset(&pitemarray[CurrentPos].item,0xFF,sizeof(ITEM));
		}
		
		
		//Generate New Serial
		//pitemarray[i].item.itemserial = GetNewSerial();
		LogAddTD("-------------------------------- NeedChange END --------------------------------");
	}
}

*/

void SearchDupe(int CurrentPos, int Max)
{
	bool isDupe = false;
	char tmp[100]={0};
	short m_Type2 = pitemarray[CurrentPos].item.itemtype +  GetType(&pitemarray[CurrentPos].item) * 512 ;
	BYTE btItemType2=0;
	btItemType2 |= (m_Type2 & 0x1E00 ) >> 5;

	DWORD itemserial1 = GetSerial(&pitemarray[CurrentPos].item);

	if(!IsEmptyItem(&pitemarray[CurrentPos].item))
	{
		for(int i=0;i<Max;i++)
		{
			DWORD itemserial2 = GetSerial(&pitemarray[i].item);

			if(i != CurrentPos)
			{
				if(itemserial1 == itemserial2)
				{						
					BYTE btItemType=0;
					short m_Type = pitemarray[i].item.itemtype +  GetType(&pitemarray[i].item) * 512 ;
					if(m_Type2 == m_Type)
					{
						btItemType |= (m_Type & 0x1E00 ) >> 5;
						if( ((btItemType+2) == pitemarray[i].item.itemexereserved) && ((btItemType2+2) == pitemarray[CurrentPos].item.itemexereserved) )
						{
							//CSHOP
						}else
						{
							//Dupe!!
							if(pitemarray[i].isvault == TRUE)
							{
								if(isDupe == false)
								{
									LogAddTD("-------------------------------- DUPE START --------------------------------");
									isDupe = true;
								}
								ToHexSinComa(tmp,(char *)&pitemarray[i].item,sizeof(pitemarray[i].item));
								LogAddC(2,"[Vault][%d][%s] Serial [%d] Item [%d][%s][%s][Pos:%d]",pitemarray[i].ExtWare,paccount[pitemarray[i].account_index].account,itemserial2,m_Type,ItemAttribute[m_Type].Name,tmp,pitemarray[i].item_index);
								//wsprintf(buff,"[%s] Serial[%d] Item[%d] vs ",paccount[pitemarray[i].account_index].account,pitemarray[i].item.itemserial,m_Type);
								memset(&paccount[pitemarray[i].account_index].warehouse[pitemarray[i].item_index], 0xFF, sizeof(ITEM));
								memset(&pitemarray[i].item,0xFF,sizeof(ITEM));

								if(g_Config.DeleteItems == 1)
								{
									char szTemp[10000]={0};
									BYTE dbWarehouse[VAULT_COUNT]	= {0};
									memcpy(dbWarehouse, paccount[pitemarray[i].account_index].warehouse, sizeof(paccount[pitemarray[i].account_index].warehouse));
									
									if( pitemarray[i].ExtWare == 0)
										wsprintf(szTemp, "UPDATE warehouse SET Items=? WHERE AccountID = '%s'", paccount[pitemarray[i].account_index].account);
									else
										wsprintf(szTemp, "UPDATE ExtendedWarehouse SET Items=? WHERE AccountID = '%s' AND SCFExtWare=%d", paccount[pitemarray[i].account_index].account,pitemarray[i].ExtWare);

									db.SetAsBinary(szTemp, dbWarehouse, sizeof(dbWarehouse));
									db.Fetch();
									db.Close();
								}
								if(g_Config.BanAccount == 1)
								{
									db.ExecQuery("UPDATE MEMB_INFO SET bloc_code = 1 WHERE memb___id = '%s'",paccount[pitemarray[i].account_index].account);
									db.Fetch();
									db.Close();
								}
							}else
							{
								if(isDupe == false)
								{
									LogAddTD("-------------------------------- DUPE START --------------------------------");
									isDupe = true;
								}
								ToHexSinComa(tmp,(char *)&pitemarray[i].item,sizeof(pitemarray[i].item));
								LogAddC(2,"[Inventory][%s][%s] Serial [%d] Item [%d][%s][%s][Pos:%d]",pcharacter[pitemarray[i].character_index].account,pcharacter[pitemarray[i].character_index].name,itemserial2,m_Type,ItemAttribute[m_Type].Name,tmp,pitemarray[i].item_index);
								//wsprintf(buff,"DUPE[%s][%s] Serial[%d] Item[%d] vs ",pcharacter[pitemarray[i].character_index].account,pcharacter[pitemarray[i].character_index].name,pitemarray[i].item.itemserial,m_Type);
								memset(&pcharacter[pitemarray[i].character_index].inventory[pitemarray[i].item_index], 0xFF, sizeof(ITEM));
								memset(&pitemarray[i].item,0xFF,sizeof(ITEM));

								if(g_Config.DeleteItems == 1)
								{
									char szTemp[10000]={0};
									BYTE dbInventory[INVENTORY_COUNT]	= {0};
									memcpy(dbInventory, pcharacter[pitemarray[i].character_index].inventory, sizeof(pcharacter[pitemarray[i].character_index].inventory));

									wsprintf(szTemp, "UPDATE character SET Inventory=? WHERE Name = '%s' AND AccountID = '%s'", pcharacter[pitemarray[i].character_index].name, pcharacter[pitemarray[i].character_index].account);
									db.SetAsBinary(szTemp, dbInventory, sizeof(dbInventory));
									db.Fetch();
									db.Close();
								}
								if(g_Config.BanAccount == 1)
								{
									db.ExecQuery("UPDATE MEMB_INFO SET bloc_code = 1 WHERE memb___id = '%s'",paccount[pitemarray[i].account_index].account);
									db.Fetch();
									db.Close();
								}
								if(g_Config.BanCharacter == 1)
								{
									db.ExecQuery("UPDATE Character SET CtlCode = 1 WHERE Name = '%s'",pcharacter[pitemarray[i].character_index].name);
									db.Fetch();
									db.Close();
								}
							}
						}
					}
				}
			}
		}
		if(isDupe == true)
		{
			if(pitemarray[CurrentPos].isvault == TRUE)
			{
				ToHexSinComa(tmp,(char *)&pitemarray[CurrentPos].item,sizeof(pitemarray[CurrentPos].item));
				LogAddC(2,"[Vault][%d][%s] Serial [%d] Item [%d][%s][%s][Pos:%d]",pitemarray[CurrentPos].ExtWare,paccount[pitemarray[CurrentPos].account_index].account,itemserial1,m_Type2,ItemAttribute[m_Type2].Name,tmp,pitemarray[CurrentPos].item_index);
				//wsprintf(buff2,"%s[%s] Serial[%d] Item[%d]",buff,paccount[pitemarray[j].account_index].account,pitemarray[j].item.itemserial,m_Type);
				memset(&paccount[pitemarray[CurrentPos].account_index].warehouse[pitemarray[CurrentPos].item_index], 0xFF, sizeof(ITEM));
				memset(&pitemarray[CurrentPos].item,0xFF,sizeof(ITEM));

				if(g_Config.DeleteItems == 1)
				{
					char szTemp[10000]={0};
					BYTE dbWarehouse[VAULT_COUNT]	= {0};
					memcpy(dbWarehouse, paccount[pitemarray[CurrentPos].account_index].warehouse, sizeof(paccount[pitemarray[CurrentPos].account_index].warehouse));
					
					if( pitemarray[CurrentPos].ExtWare == 0)
						wsprintf(szTemp, "UPDATE warehouse SET Items=? WHERE AccountID = '%s'", paccount[pitemarray[CurrentPos].account_index].account);
					else
						wsprintf(szTemp, "UPDATE ExtendedWarehouse SET Items=? WHERE AccountID = '%s' AND SCFExtWare=%d", paccount[pitemarray[CurrentPos].account_index].account,pitemarray[CurrentPos].ExtWare );

					
					db.SetAsBinary(szTemp, dbWarehouse, sizeof(dbWarehouse));
					db.Fetch();
					db.Close();
				}
				if(g_Config.BanAccount == 1)
				{
					db.ExecQuery("UPDATE MEMB_INFO SET bloc_code = 1 WHERE memb___id = '%s'",paccount[pitemarray[CurrentPos].account_index].account);
					db.Fetch();
					db.Close();
				}
			}else
			{
				ToHexSinComa(tmp,(char *)&pitemarray[CurrentPos].item,sizeof(pitemarray[CurrentPos].item));
				LogAddC(2,"[Inventory][%s][%s] Serial [%d] Item [%d][%s][%s][Pos:%d]",pcharacter[pitemarray[CurrentPos].character_index].account,pcharacter[pitemarray[CurrentPos].character_index].name,itemserial1,m_Type2,ItemAttribute[m_Type2].Name,tmp,pitemarray[CurrentPos].item_index);
				//wsprintf(buff2,"%s[%s][%s] Serial[%d] Item[%d]",buff,pcharacter[pitemarray[j].character_index].account,pcharacter[pitemarray[j].character_index].name,pitemarray[j].item.itemserial,m_Type);
				memset(&pcharacter[pitemarray[CurrentPos].character_index].inventory[pitemarray[CurrentPos].item_index], 0xFF, sizeof(ITEM));
				memset(&pitemarray[CurrentPos].item,0xFF,sizeof(ITEM));

				if(g_Config.DeleteItems == 1)
				{
					char szTemp[10000]={0};
					BYTE dbInventory[INVENTORY_COUNT]	= {0};
					memcpy(dbInventory, pcharacter[pitemarray[CurrentPos].character_index].inventory, sizeof(pcharacter[pitemarray[CurrentPos].character_index].inventory));

					wsprintf(szTemp, "UPDATE character SET Inventory=? WHERE Name = '%s' AND AccountID = '%s'", pcharacter[pitemarray[CurrentPos].character_index].name, pcharacter[pitemarray[CurrentPos].character_index].account);
					db.SetAsBinary(szTemp, dbInventory, sizeof(dbInventory));
					db.Fetch();
					db.Close();
				}
				if(g_Config.BanAccount == 1)
				{
					db.ExecQuery("UPDATE MEMB_INFO SET bloc_code = 1 WHERE memb___id = '%s'",paccount[pitemarray[CurrentPos].account_index].account);
					db.Fetch();
					db.Close();
				}
				if(g_Config.BanCharacter == 1)
				{
					db.ExecQuery("UPDATE Character SET CtlCode = 1 WHERE Name = '%s'",pcharacter[pitemarray[CurrentPos].character_index].name);
					db.Fetch();
					db.Close();
				}
			}
			LogAddTD("--------------------------------- DUPE END ---------------------------------");
		}
	}
}

void Start(void * lpParam)
{
	while(true)
	{
		int extwarecount = 0;
		int usercount = 0;
		int charcount = 0;
		int TotalWares = 0;

		BOOL bRet = FALSE;
		LogAddC(3,"Starting search...");
		LogAddTD("Getting Character Count...");

		if(g_Config.SkipNonUpdatedPSHOP == 0)
			bRet = db.ExecQuery("select count(*) as charcount from character");
		else
			bRet = db.ExecQuery("select count(*) as charcount from character WHERE SCFUpdatedPShop = 1");

		if (bRet == TRUE )
		{
			if(db.Fetch() != SQL_NO_DATA)
			{
				charcount = db.GetAsInteger("charcount");
			}
		}
		db.Close();

		LogAddTD("Character Count: %d !!",charcount);

		pcharacter = new CHARACTER [charcount];
		memset(pcharacter, 0x00, sizeof(CHARACTER)*charcount);

		LogAddTD("Getting Inventories...");
		if(g_Config.SkipNonUpdatedPSHOP == 0)
			bRet = db.ExecQuery("select AccountID, Name from character");
		else
			bRet = db.ExecQuery("select AccountID, Name from character WHERE SCFUpdatedPShop = 1");

		if (bRet == TRUE )
		{
			int i=0;
			if( db.Fetch() != SQL_NO_DATA)
			{
				do
				{
					db.GetAsString("AccountID", pcharacter[i].account);
					db.GetAsString("Name", pcharacter[i].name);
					i++;
					if(i>=charcount)
						break;
				}while(db.Fetch() != SQL_NO_DATA);
			}
		}
		db.Close();

		for(int i=0;i<charcount;i++)
		{
			char szTemp[1024]={0};
			BYTE InvTmp[INVENTORY_COUNT]={0};
			wsprintf(szTemp, "select Inventory from character WHERE Name='%s'", pcharacter[i].name);
			db.GetAsBinary(szTemp, InvTmp);
			memcpy(pcharacter[i].inventory,InvTmp,sizeof(pcharacter[i].inventory));
			db.Close();
		}
		LogAddTD("Inventories Loaded!!");
		
		LogAddTD("Getting Warehouse Count...");
		bRet = db.ExecQuery("select count(*) as usercount from warehouse");
		if (bRet == TRUE )
		{
			if(db.Fetch() != SQL_NO_DATA)
			{
				usercount = db.GetAsInteger("usercount");
			}
		}
		db.Close();
		LogAddTD("Warehouse Count: %d !!",usercount);
		
		TotalWares = usercount;

		if(g_Config.UseExtendedWarehouse == 1)
		{
			LogAddTD("Getting ExtendedWarehouse Count...");
			bRet = db.ExecQuery("select count(*) as extcount from ExtendedWarehouse");
			if (bRet == TRUE )
			{
				if(db.Fetch() != SQL_NO_DATA)
				{
					extwarecount = db.GetAsInteger("extcount");
				}
			}
			db.Close();
			LogAddTD("ExtendedWarehouse Count: %d !!",extwarecount);
			

			TotalWares += extwarecount;
		}

		paccount = new ACCOUNT [usercount+extwarecount];
		memset(paccount, 0x00, sizeof(ACCOUNT)*usercount);
		
		bRet = db.ExecQuery("select AccountID from warehouse");
		if (bRet == TRUE )
		{
			int i=0;
			if( db.Fetch() != SQL_NO_DATA)
			{
				do
				{
					db.GetAsString("AccountID", paccount[i].account);
					i++;
					if(i>=usercount)
						break;
				}while(db.Fetch() != SQL_NO_DATA);
			}
		}
		db.Close();
		
		if(g_Config.UseExtendedWarehouse == 1)
		{
			bRet = db.ExecQuery("select AccountID,SCFExtWare from ExtendedWarehouse");
			if (bRet == TRUE )
			{
				int i=usercount;
				if( db.Fetch() != SQL_NO_DATA)
				{
					do
					{
						db.GetAsString("AccountID", paccount[i].account);
						paccount[i].ExtWare = db.GetAsInteger("SCFExtWare");
						i++;
						if(i>=TotalWares)
							break;
					}while(db.Fetch() != SQL_NO_DATA);
				}
			}
			db.Close();
		}

		for(int i=0;i<usercount;i++)
		{
			char szTemp[1024]={0};
			BYTE InvTmp[VAULT_COUNT]={0};
			wsprintf(szTemp, "select Items from warehouse WHERE AccountID='%s'", paccount[i].account);
			db.GetAsBinary(szTemp, InvTmp);
			memcpy(paccount[i].warehouse,InvTmp,sizeof(paccount[i].warehouse));
			db.Close();
		}
		LogAddTD("Warehouses Loaded!!");

		if(g_Config.UseExtendedWarehouse == 1)
		{
			for(int i=usercount;i<TotalWares;i++)
			{
				char szTemp[1024]={0};
				BYTE InvTmp[VAULT_COUNT]={0};
				wsprintf(szTemp, "select Items from ExtendedWarehouse WHERE AccountID='%s' AND SCFExtWare=%d", paccount[i].account,paccount[i].ExtWare);
				db.GetAsBinary(szTemp, InvTmp);
				memcpy(paccount[i].warehouse,InvTmp,sizeof(paccount[i].warehouse));
				db.Close();
			}
			LogAddTD("ExtendedWarehouse Loaded!!");
		}

		int iSize=(charcount*INV_ITEMS) + (usercount*VAULT_ITEMS);
		if(g_Config.UseExtendedWarehouse == 1)
		{
			iSize += (extwarecount*VAULT_ITEMS);
		}
		pitemarray = new ITEMARRAYELEMENT [iSize];
		memset(pitemarray, 0x00, sizeof(ITEMARRAYELEMENT)*iSize);

		unsigned int arrayindex=0;

		for(int user=0;user<usercount;user++)
		{
			for(int item=0;item<VAULT_ITEMS;item++) 
			{
				if(!IsEmptyItem(&paccount[user].warehouse[item]))
				{
					pitemarray[arrayindex].account_index = user;
					pitemarray[arrayindex].item_index = item;
					pitemarray[arrayindex].isvault = TRUE;
					pitemarray[arrayindex].ExtWare = 0;
					memcpy(&pitemarray[arrayindex].item, &paccount[user].warehouse[item], sizeof(ITEM));
					arrayindex++;
				}
			}
		}

		if(g_Config.UseExtendedWarehouse == 1)
		{
			for(int user=usercount;user<TotalWares;user++)
			{
				for(int item=0;item<VAULT_ITEMS;item++) 
				{
					if(!IsEmptyItem(&paccount[user].warehouse[item]))
					{
						pitemarray[arrayindex].account_index = user;
						pitemarray[arrayindex].item_index = item;
						pitemarray[arrayindex].isvault = TRUE;
						pitemarray[arrayindex].ExtWare = paccount[user].ExtWare;
						memcpy(&pitemarray[arrayindex].item, &paccount[user].warehouse[item], sizeof(ITEM));
						arrayindex++;
					}
				}
			}
		}

		for(int character=0;character<charcount;character++)
		{
			for(int item=0;item<INV_ITEMS;item++) 
			{
				if(!IsEmptyItem(&pcharacter[character].inventory[item]))
				{
					pitemarray[arrayindex].character_index = character;
					pitemarray[arrayindex].item_index = item;
					pitemarray[arrayindex].isvault = FALSE;
					pitemarray[arrayindex].ExtWare = 0;
					memcpy(&pitemarray[arrayindex].item, &pcharacter[character].inventory[item], sizeof(ITEM));
					arrayindex++;
				}
			}
		}

		for(int i=0;i<arrayindex;i++)
		{			
			SearchDupe(i,arrayindex);
		}

		LogAddTD("Finishing search...");
		LogAddC(3,"Search finished... Will continue in %d minutes",g_Config.SearchEachXMinutes/60000);
		Sleep(g_Config.SearchEachXMinutes);
	}
}

void Run()
{
	_beginthread( Start, 0, NULL  );
}