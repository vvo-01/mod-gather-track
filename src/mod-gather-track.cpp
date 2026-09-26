/*
 * mod-gather-track
 *
 * Released under the GNU AGPL v3 license, consistent with AzerothCore.
 *
 * Original dual-tracking logic based on mod-dual-tracking by b-wun:
 *   https://github.com/b-wun/mod-dual-tracking
 */

#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "SpellAuraEffects.h"
#include "SharedDefines.h"
#include "DBCStructure.h"
#include "DBCStores.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "Unit.h"

// =====================================================
// SETTINGS
// =====================================================
// Spell IDs and misc values. These are structural data and rarely
// change. Edit here if you need to point the module at different
// spells.

// Dual tracking spells
static constexpr uint32 SPELL_FIND_HERBS    = 2383;
static constexpr uint32 SPELL_FIND_MINERALS = 2580;

// TrackResources MiscValue
static constexpr uint32 TRACK_HERBS    = 2;
static constexpr uint32 TRACK_MINERALS = 3;

// Gathering spell IDs per rank
// Order: Apprentice, Journeyman, Expert, Artisan, Master, Grand Master
static constexpr uint32 GATHER_HERBALISM[] = { 2366, 2368, 3570, 11993, 28695, 50300 };
static constexpr uint32 GATHER_MINING[]    = { 2575, 2576, 3564, 10248, 29354, 50310 };
static constexpr uint32 GATHER_SKINNING[]  = { 8613, 8617, 8618, 10768, 32678, 50305 };

// =====================================================
// HELPERS
// =====================================================
static void ApplyGatheringCastTime(uint32 const* spellIds, size_t count,
                                   uint32 castIndex, char const* groupName)
{
    SpellCastTimesEntry const* castEntry = sSpellCastTimesStore.LookupEntry(castIndex);

    for (size_t i = 0; i < count; ++i)
    {
        uint32 id = spellIds[i];
        SpellInfo* si = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(id));
        if (!si)
        {
            LOG_ERROR("mod-gather-track",
                      "Gathering spell id {} ({}) not found in SpellMgr, skipping.",
                      id, groupName);
            continue;
        }
        si->CastTimeEntry = castEntry;
    }

    LOG_INFO("mod-gather-track",
             "GatheringSpeed: applied CastTime index {} to {} group.",
             castIndex, groupName);
}

// =====================================================
// WORLD SCRIPT
// =====================================================
class mod_gather_track_world : public WorldScript
{
public:
    mod_gather_track_world() : WorldScript("mod_gather_track_world") {}

    void OnStartup() override
    {
        if (sConfigMgr->GetOption<bool>("DualTracking.Enable", true))
            ApplyDualTracking();

        if (sConfigMgr->GetOption<bool>("GatheringSpeed.Enable", true))
            ApplyGatheringSpeed();
    }

private:
    void ApplyDualTracking()
    {
        if (SpellInfo* herbSpell = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(SPELL_FIND_HERBS)))
        {
            herbSpell->Effects[EFFECT_1].Effect        = SPELL_EFFECT_APPLY_AURA;
            herbSpell->Effects[EFFECT_1].TargetA       = TARGET_UNIT_CASTER;
            herbSpell->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_TRACK_RESOURCES;
            herbSpell->Effects[EFFECT_1].MiscValue     = TRACK_MINERALS;

            LOG_INFO("mod-gather-track",
                     "DualTracking: Find Herbs ({}) now also tracks minerals (MiscValue={}).",
                     SPELL_FIND_HERBS, TRACK_MINERALS);
        }
        else
        {
            LOG_ERROR("mod-gather-track",
                      "DualTracking: Find Herbs ({}) not found in SpellMgr.",
                      SPELL_FIND_HERBS);
        }

        if (SpellInfo* mineralSpell = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(SPELL_FIND_MINERALS)))
        {
            mineralSpell->Effects[EFFECT_1].Effect        = SPELL_EFFECT_APPLY_AURA;
            mineralSpell->Effects[EFFECT_1].TargetA       = TARGET_UNIT_CASTER;
            mineralSpell->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_TRACK_RESOURCES;
            mineralSpell->Effects[EFFECT_1].MiscValue     = TRACK_HERBS;

            LOG_INFO("mod-gather-track",
                     "DualTracking: Find Minerals ({}) now also tracks herbs (MiscValue={}).",
                     SPELL_FIND_MINERALS, TRACK_HERBS);
        }
        else
        {
            LOG_ERROR("mod-gather-track",
                      "DualTracking: Find Minerals ({}) not found in SpellMgr.",
                      SPELL_FIND_MINERALS);
        }
    }

    void ApplyGatheringSpeed()
    {
        uint32 herbMiningIndex = sConfigMgr->GetOption<uint32>("GatheringSpeed.HerbalismMiningIndex", 16);
        uint32 skinningIndex   = sConfigMgr->GetOption<uint32>("GatheringSpeed.SkinningIndex", 4);

        ApplyGatheringCastTime(GATHER_HERBALISM, 6, herbMiningIndex, "Herbalism");
        ApplyGatheringCastTime(GATHER_MINING,    6, herbMiningIndex, "Mining");
        ApplyGatheringCastTime(GATHER_SKINNING,  6, skinningIndex,   "Skinning");
    }
};

// =====================================================
// UNIT SCRIPT
// =====================================================
class mod_gather_track_unit : public UnitScript
{
public:
    mod_gather_track_unit() : UnitScript("mod_gather_track_unit") {}

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        if (!sConfigMgr->GetOption<bool>("DualTracking.Enable", true))
            return;

        if (!unit || !unit->IsPlayer() || !aura)
            return;

        Player* player = unit->ToPlayer();
        uint32 spellId = aura->GetId();

        if (spellId == SPELL_FIND_HERBS && !player->HasSkill(SKILL_MINING))
        {
            if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                eff->SetAmount(0);
        }
        else if (spellId == SPELL_FIND_MINERALS && !player->HasSkill(SKILL_HERBALISM))
        {
            if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                eff->SetAmount(0);
        }
    }
};

// =====================================================
// REGISTRATION
// =====================================================
void Addmod_gather_trackScripts()
{
    new mod_gather_track_world();
    new mod_gather_track_unit();
}
