#include "settings.h"
#include "hid.h"
#include "input.h"

SettingsContext gSettingsContext = {0};
u8 Damage32 = 0;

s32 Settings_ApplyDamageMultiplier(GlobalContext* globalCtx, s32 changeHealth) {
    // Fairy healing also gets sent to this function and should be ignored
    if (changeHealth >= 0) {
        return changeHealth;
    }
    // If supposed to take damage on OHKO ignore everything else
    if (gSettingsContext.damageMultiplier == DAMAGEMULTIPLIER_OHKO) {
        return -1000;
    }

    s32 modifiedChangeHealth = changeHealth;
    //disregard master quest damage
    if (gSaveContext.masterQuestFlag) {
        modifiedChangeHealth /= 2;
    }

    // MQ damage is applied after this function so changeHealth can be -1
    // In this case modifiedChangeHealth would always be 0 which is wrong
    if (modifiedChangeHealth == 0) {
        modifiedChangeHealth = -(Damage32 >> 2);
        Damage32 ^= 4;
    }

    if (modifiedChangeHealth < 0) {
        switch (gSettingsContext.damageMultiplier) {
            case DAMAGEMULTIPLIER_HALF:
                modifiedChangeHealth /= 2;
                break;
            case DAMAGEMULTIPLIER_DEFAULT:
                break;
            case DAMAGEMULTIPLIER_DOUBLE:
                modifiedChangeHealth *= 2;
                break;
            case DAMAGEMULTIPLIER_QUADRUPLE:
                modifiedChangeHealth *= 4;
                break;
            case DAMAGEMULTIPLIER_OCTUPLE:
                modifiedChangeHealth *= 8;
                break;
            case DAMAGEMULTIPLIER_SEXDECUPLE:
                modifiedChangeHealth *= 16;
                break;
        }

        // Can only be 0 if supposed to be -0.5: alternate -1 and 0
        if (modifiedChangeHealth == 0) {
            modifiedChangeHealth = -(Damage32 & 1);
            Damage32 ^= 1;
        }
    }

    // Double defense seems to round up after halving so values of -1 should instead alternate between -2 and 0 (-1 would also work, but -2 was easier)
    if (gSaveContext.doubleDefense && modifiedChangeHealth == -1) {
        modifiedChangeHealth = -(Damage32 & 2);
        Damage32 ^= 2;
    }

    return modifiedChangeHealth;
}
//With the No Health Refill option on, full health refills from health upgrades and Bombchu Bowling are turned off, and fairies restore 3 hearts
//Otherwise, they grant a full heal, and the default effect applies (full heal from bottle, 8 hearts on contact)
u32 Settings_SetFullHealthRestore(u8 setAmount) {
    if((gSettingsContext.heartDropRefill == HEARTDROPREFILL_NOREFILL) || (gSettingsContext.heartDropRefill == HEARTDROPREFILL_NODROPREFILL)){
        return setAmount;
    } else {
        return 0x140;
    }
}
u32 NoHealFromHealthUpgrades(void) {
    return Settings_SetFullHealthRestore(0);
}
u32 NoHealFromBombchuBowlingPrize(void) {
    return Settings_SetFullHealthRestore(0);
}
u32 FairyReviveHealAmount(void) {
    return Settings_SetFullHealthRestore(0x30);
}
u32 FairyUseHealAmount(void) {
    return Settings_SetFullHealthRestore(0x30);
}

void FairyPickupHealAmount(void) {
    if(gSettingsContext.heartDropRefill == HEARTDROPREFILL_NOREFILL || gSettingsContext.heartDropRefill == HEARTDROPREFILL_NODROPREFILL){
        Health_ChangeBy(gGlobalContext, 0x30);
    } else {
        Health_ChangeBy(gGlobalContext, 0x80);
    }
}

u32 Settings_GetQuickTextOption() {
    return gSettingsContext.quickText;
}

u32 Settings_GetSongReplaysOption() {
    return gSettingsContext.skipSongReplays;
}

u32 Settings_IsTurboText() {
    return (gSettingsContext.quickText >= QUICKTEXT_TURBO && rInputCtx.cur.b);
}

void Settings_SkipSongReplays() {
    // msgModes 18 to 23 are used to manage the song replays. Skipping to mode 23 ends the replay.
    // msgMode 18 starts the playback music. It can't be skipped for scarecrow's song (song "12") because it spawns Pierre.
    if ((gSettingsContext.skipSongReplays == SONGREPLAYS_SKIP_NO_SFX && gGlobalContext->msgMode == 18 && gGlobalContext->unk_2A91[0xEB] != 12) ||
        (gSettingsContext.skipSongReplays != SONGREPLAYS_DONT_SKIP   && gGlobalContext->msgMode == 19)
       ) {
        // In Water Temple, playing ZL cycles through the modes to avoid problems with the dimmed bottom screen at the ZL switches
        if (gGlobalContext->sceneNum == 5 && gGlobalContext->unk_2A91[0xEB] == 8) {
            gGlobalContext->msgMode = 20;
        }
        else {
            gGlobalContext->msgMode = 23;
        }
    }
    else if (gSettingsContext.skipSongReplays != SONGREPLAYS_DONT_SKIP && gGlobalContext->msgMode > 19 && gGlobalContext->msgMode < 23) {
        gGlobalContext->msgMode++;
    }
}

  const char hashIconNames[32][25] = {
    "Deku Stick",
    "Deku Nut",
    "Bow",
    "Slingshot",
    "Fairy Ocarina",
    "Bombchu",
    "Longshot",
    "Boomerang",
    "Lens of Truth",
    "Beans",
    "Megaton Hammer",
    "Bottled Fish",
    "Bottled Milk",
    "Mask of Truth",
    "SOLD OUT",
    "Cucco",
    "Mushroom",
    "Saw",
    "Frog",
    "Master Sword",
    "Mirror Shield",
    "Kokiri Tunic",
    "Hover Boots",
    "Silver Gauntlets",
    "Gold Scale",
    "Shard of Agony",
    "Skull Token",
    "Heart Container",
    "Boss Key",
    "Compass",
    "Map",
    "Big Magic",
  };
