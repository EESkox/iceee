#ifndef NPC_H
#define NPC_H

#include <vector>
#include <list>
#include <map>
#include <string>

typedef std::vector<std::string> STRINGLIST;
typedef std::vector<STRINGLIST> MULTISTRING;

//Holds the actual values of hate against a specific creature instance (usually a player).
struct HateCreatureData
{
	int CDefID;   //CDef of the attacking object
	int damage;
	int hate;
	short level;  //Level of the attacking object
	HateCreatureData()
	{
		CDefID = 0;
		damage = 0;
		hate = 0;
		level = 0;
	}
	static bool SortByHate(const HateCreatureData &left, const HateCreatureData &right)
	{
		//Needs to be sorted highest to lowest.
		return (left.hate > right.hate);
	}
};

//Every creature instance may have one hate profile, which records and processes all the incoming hate
//generated by applicable targets.
class HateProfile
{
public:
	HateProfile();
	~HateProfile();

	std::vector<HateCreatureData> hateList;
	unsigned long tauntReleaseTime;    //If a target is taunted, this is the time when the taunt wears off.
	unsigned long nextRefreshTime;     //Forces a delay between sorting and verifying updated hate profiles.

	static const int REFRESH_DELAY = 1000;

	void SetImmediateRefresh(void);
	void ExtendTauntRelease(int seconds);
	int GetIndexByCreature(int CDefID);
	void Add(int CDefID, short level, int damage, int hate);
	int GetHighestLevel(void);
	void UnHate(int CreatureDefID);
	bool CheckRefreshAndUpdate(void);
};

//The primary container of all hate profiles.  Every instanciated zone (overworld, dungeon) has one of
//these containers.
class HateProfileContainer
{
public:
	HateProfileContainer();
	~HateProfileContainer();
	std::list<HateProfile> profileList;
	HateProfile * GetProfile(void);
	void RemoveProfile(HateProfile* profile);
	void UnHate(int CreatureDefID);
};


struct PetDef
{
	int mCreatureDefID;            //ID of the CreatureDef entry.  Used as a key into the table, but otherwise has no effect on server operations.
	std::string mDisplayName;      //Display name sent to the client pet table.
	int mLevel;                    //Level sent to the client.
	int mCost;                     //Cost sent to the client, and used when actually purchasing pets.
	std::string mDesc;             //Description sent to the client.
	int mItemDefID;                //ItemDef ID of the inventory item that added when purchasing the pet.
	PetDef();
	void Clear(void);
	void CopyFrom(const PetDef& source);
};

class PetDefManager
{
public:
	typedef std::map<int, PetDef> PETDEF_MAP;

	PETDEF_MAP mDefs;

	PetDefManager();
	~PetDefManager();

	void AddEntry(PetDef& data);
	PetDef* GetEntry(int CreatureDefID);
	void LoadFile(const char *filename);
	int GetStandardCount(void);
	void FillQueryResponse(MULTISTRING& output);
};

extern PetDefManager g_PetDefManager;


#endif //#define NPC_H