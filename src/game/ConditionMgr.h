/*
 * This file is part of the OregonCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONDITIONMGR_H
#define CONDITIONMGR_H

#include "LootMgr.h"

class Player;
class Unit;
class WorldObject;
class LootTemplate;
struct Condition;

enum ConditionType
{                                                           // value1           value2         value3
    CONDITION_NONE                  = 0,                    // 0                0              0                  always true
    CONDITION_AURA                  = 1,                    // spell_id         effindex       use target?        true if player (or target, if value3) has aura of spell_id with effect effindex
    CONDITION_ITEM                  = 2,                    // item_id          count          bank               true if has #count of item_ids (if 'bank' is set it searches in bank slots too)
    CONDITION_ITEM_EQUIPPED         = 3,                    // item_id          0              0                  true if has item_id equipped
    CONDITION_ZONEID                = 4,                    // zone_id          0              0                  true if in zone_id
    CONDITION_REPUTATION_RANK       = 5,                    // faction_id       min_rank       0                  true if has min_rank for faction_id
    CONDITION_TEAM                  = 6,                    // player_team      0,             0                  469 - Alliance, 67 - Horde)
    CONDITION_SKILL                 = 7,                    // skill_id         skill_value    0                  true if has skill_value for skill_id
    CONDITION_QUESTREWARDED         = 8,                    // quest_id         0              0                  true if quest_id was rewarded before
    CONDITION_QUESTTAKEN            = 9,                    // quest_id         0,             0                  true while quest active
    CONDITION_DRUNKENSTATE          = 10,                   // DrunkenState     0,             0                  true if player is drunk enough
    CONDITION_WORLD_STATE           = 11,                   // index            value          0                  true if world has the value for the index
    CONDITION_ACTIVE_EVENT          = 12,                   // event_id         0              0                  true if event is active
    CONDITION_INSTANCE_INFO         = 13,                   // entry            data           type               true if the instance info defined by type (enum InstanceInfo) equals data.
    CONDITION_QUEST_NONE            = 14,                   // quest_id         0              0                  true if doesn't have quest saved
    CONDITION_CLASS                 = 15,                   // class            0              0                  true if player's class is equal to class
    CONDITION_RACE                  = 16,                   // race             0              0                  true if player's race is equal to race
    CONDITION_SPELL_SCRIPT_TARGET   = 17,                   // SpellScriptTargetType          TargetEntry         
    CONDITION_TITLE                 = 18,                   // title id         0              0                  true if player has title
    CONDITION_SPAWNMASK             = 19,                   // spawnMask        0              0                  true if in spawnMask
    CONDITION_GENDER                = 20,                   // gender           0              0                  true if player's gender is equal to gender
    CONDITION_UNIT_STATE            = 21,                   // unitState        0              0                  true if unit has unitState
    CONDITION_MAPID                 = 22,                   // map_id           0              0                  true if in map_id
    CONDITION_AREAID                = 23,                   // area_id          0              0                  true if in area_id
    CONDITION_CREATURE_TYPE         = 24,                   // cinfo.type       0              0                  true if creature_template.type = value1
    CONDITION_SPELL                 = 25,                   // spell_id         0              0                  true if player has learned spell
    CONDITION_UNUSED_26             = 26,
    CONDITION_LEVEL                 = 27,                   // level            opt            0                  true if unit's level is equal to param1 (param2 can modify the statement)
    CONDITION_QUEST_COMPLETE        = 28,                   // quest_id         0              0                  true if player has quest_id with all objectives complete, but not yet rewarded
    CONDITION_NEAR_CREATURE         = 29,                   // creature entry   distance       0                  true if there is a creature of entry in range
    CONDITION_NEAR_GAMEOBJECT       = 30,                   // gameobject entry distance       0                  true if there is a gameobject of entry in range
    CONDITION_OBJECT_ENTRY_GUID     = 31,                   // TypeID           entry          guid               true if object is type TypeID and the entry is 0 or matches entry of the object or matches guid of the object
    CONDITION_TYPE_MASK             = 32,                   // TypeMask         0              0                  true if object is type object's TypeMask matches provided TypeMask
    CONDITION_RELATION_TO           = 33,                   // ConditionTarget  RelationType   0                  true if object is in given relation with object specified by ConditionTarget
    CONDITION_REACTION_TO           = 34,                   // ConditionTarget  rankMask       0                  true if object's reaction matches rankMask object specified by ConditionTarget
    CONDITION_DISTANCE_TO           = 35,                   // ConditionTarget  distance       ComparisonType     true if object and ConditionTarget are within distance given by parameters
    CONDITION_ALIVE                 = 36,                   // 0                0              0                  true if unit is alive
    CONDITION_HP_VAL                = 37,                   // hpVal            ComparisonType 0                  true if unit's hp matches given value
    CONDITION_HP_PCT                = 38,                   // hpPct            ComparisonType 0                  true if unit's hp matches given pct
    CONDITION_MAX                   = 39                    // MAX
};

/*! Documentation on implementing a new ConditionSourceType:
    Step 1: Check for the lowest free ID. Look for CONDITION_SOURCE_TYPE_UNUSED_XX in the enum.
            Then define the new source type.

    Step 2: Determine and map the parameters for the new condition type.

    Step 3: Add a case block to ConditionMgr::isSourceTypeValid with the new condition type
            and validate the parameters.

    Step 4: If your condition can be grouped (determined in step 2), add a rule for it in
            ConditionMgr::CanHaveSourceGroupSet, following the example of the existing types.

    Step 5: Define the maximum available condition targets in ConditionMgr::GetMaxAvailableConditionTargets.

    The following steps only apply if your condition can be grouped:

    Step 6: Determine how you are going to store your conditions. You need to add a new storage container
            for it in ConditionMgr class, along with a function like: 
            ConditionList GetConditionsForXXXYourNewSourceTypeXXX(parameters...)

            The above function should be placed in upper level (practical) code that actually 
            checks the conditions.

    Step 7: Implement loading for your source type in ConditionMgr::LoadConditions.

    Step 8: Implement memory cleaning for your source type in ConditionMgr::Clean.
*/
enum ConditionSourceType
{
    CONDITION_SOURCE_TYPE_NONE                           = 0,
    CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE         = 1,
    CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE       = 2,
    CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE          = 3,
    CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE       = 4,
    CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE             = 5,
    CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE             = 6,
    CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE          = 7,
    CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE    = 8,
    CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE      = 9,
    CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE        = 10,
    CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE         = 11,
    CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE            = 12,
    CONDITION_SOURCE_TYPE_SPELL_SCRIPT_TARGET            = 13,
    CONDITION_SOURCE_TYPE_GOSSIP_MENU                    = 14,
    CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION             = 15,
    CONDITION_SOURCE_TYPE_CREATURE_TEMPLATE_VEHICLE      = 16,
    CONDITION_SOURCE_TYPE_SPELL                          = 17,
    CONDITION_SOURCE_TYPE_UNUSED_18                      = 18,
    CONDITION_SOURCE_TYPE_QUEST_ACCEPT                   = 19,
    CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK                = 20,

    CONDITION_SOURCE_TYPE_MAX                            = 21  //MAX
};

enum ComparisionType
{
    COMP_TYPE_EQ = 0,
    COMP_TYPE_HIGH,
    COMP_TYPE_LOW,
    COMP_TYPE_HIGH_EQ,
    COMP_TYPE_LOW_EQ,
    COMP_TYPE_MAX
};

enum RelationType
{
    RELATION_SELF = 0,
    RELATION_IN_PARTY,
    RELATION_IN_RAID_OR_PARTY,
    RELATION_OWNED_BY,
    RELATION_PASSENGER_OF, // Unsupported, WotLK.
    RELATION_CREATED_BY,
    RELATION_MAX
};

enum InstanceInfo
{
    INSTANCE_INFO_DATA = 0,
    INSTANCE_INFO_DATA64,
    INSTANCE_INFO_BOSS_STATE
};

enum MaxConditionTargets
{
    MAX_CONDITION_TARGETS = 3
};

struct ConditionSourceInfo
{
    WorldObject* mConditionTargets[MAX_CONDITION_TARGETS];
    Condition* mLastFailedCondition;
    ConditionSourceInfo(WorldObject* target0, WorldObject* target1 = NULL, WorldObject* target2 = NULL)
    {
        mConditionTargets[0] = target0;
        mConditionTargets[1] = target1;
        mConditionTargets[2] = target2;
        mLastFailedCondition = NULL;
    }
};

struct Condition
{
    ConditionSourceType     SourceType;        //SourceTypeOrReferenceId
    uint32                  SourceGroup;
    int32                   SourceEntry;
    uint32                  ElseGroup;
    ConditionType           ConditionType;     //ConditionTypeOrReference
    uint32                  ConditionValue1;
    uint32                  ConditionValue2;
    uint32                  ConditionValue3;
    uint32                  ErrorType;
    uint32                  ErrorTextId;
    uint32                  ReferenceId;
    uint32                  ScriptId;
    uint8                   ConditionTarget;
    bool                    NegativeCondition;

    Condition()
    {
        SourceType         = CONDITION_SOURCE_TYPE_NONE;
        SourceGroup        = 0;
        SourceEntry        = 0;
        ElseGroup          = 0;
        ConditionType      = CONDITION_NONE;
        ConditionTarget    = 0;
        ConditionValue1    = 0;
        ConditionValue2    = 0;
        ConditionValue3    = 0;
        ReferenceId        = 0;
        ErrorType          = 0;
        ErrorTextId        = 0;
        ScriptId           = 0;
        NegativeCondition  = false;
    }

    bool Meets(ConditionSourceInfo& sourceInfo);
    bool isLoaded() { return ConditionType > CONDITION_NONE || ReferenceId; }
    uint32 GetMaxAvailableConditionTargets();
};

typedef std::list<Condition*> ConditionList;
typedef std::map<uint32, ConditionList> ConditionTypeContainer;
typedef std::map<ConditionSourceType, ConditionTypeContainer> ConditionContainer;

typedef std::map<uint32, ConditionList> ConditionReferenceContainer;//only used for references

class ConditionMgr
{
    public:
        ConditionMgr();
        ~ConditionMgr();

        void LoadConditions(bool isReload = false);
        bool isConditionTypeValid(Condition* cond);
        ConditionList GetConditionReferences(uint32 refId);

        bool IsObjectMeetToConditions(WorldObject* object, ConditionList const& conditions);
        bool IsObjectMeetToConditions(WorldObject* object1, WorldObject* object2, ConditionList const& conditions);
        bool IsObjectMeetToConditions(ConditionSourceInfo& sourceInfo, ConditionList const& conditions);
        ConditionList GetConditionsForNotGroupedEntry(ConditionSourceType sourceType, uint32 entry);

    protected:
        ConditionContainer                ConditionStore;
        ConditionReferenceContainer       ConditionReferenceStore;

    private:
        bool isSourceTypeValid(Condition* cond);
        bool addToLootTemplate(Condition* cond, LootTemplate* loot);
        bool addToGossipMenus(Condition* cond);
        bool addToGossipMenuItems(Condition* cond);
        bool IsObjectMeetToConditionList(ConditionSourceInfo& sourceInfo, ConditionList const& conditions);

        bool isGroupable(ConditionSourceType sourceType) const
        {
            return (sourceType == CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU ||
                    sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION);
        }

        void Clean(); // free up resources
        std::list<Condition*> AllocatedMemoryStore; // some garbage collection :)
};

template <class T> bool CompareValues(ComparisionType type,  T val1, T val2)
{
    switch (type)
    {
        case COMP_TYPE_EQ:
            return val1 == val2;
        case COMP_TYPE_HIGH:
            return val1 > val2;
        case COMP_TYPE_LOW:
            return val1 < val2;
        case COMP_TYPE_HIGH_EQ:
            return val1 >= val2;
        case COMP_TYPE_LOW_EQ:
            return val1 <= val2;
    }
    // incorrect parameter
    ASSERT(false);
    return false;
}

#define sConditionMgr Oregon::Singleton<ConditionMgr>::Instance()

#endif