//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name action_repair.c	-	The repair action. */
//
//	(c) Copyright 1999-2002 by Vladi Shabanski
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--      Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**	Generic unit repair.
**
**	@param unit	Unit, for that the repair animation is played.
**	@param repair	Repair animation.
*/
local void DoActionRepairGeneric(Unit* unit,const Animation* repair)
{
    int flags;

    flags=UnitShowAnimation(unit,repair);

#ifdef WITH_SOUND
    if( (flags&AnimationSound) ) {
	if( GameSounds.Repair.Sound==(void*)-1 ) {
	    PlayUnitSound(unit,VoiceAttacking);
	} else {
	    PlayUnitSound(unit,VoiceRepair);
	}
    }
#endif
}

/**
**	Repair an unit.
*/
local void RepairUnit(Unit* unit, Unit* goal)
{
    Player* player;
    int costs[MaxCosts];
    int i;
    int animlength;
    Animation* anim;
    int hp;
    int lrr;
    char buf[100];

#define GIVES_HP	4
#define COSTS		1

    //
    //  Calculate the repair points
    //          original per 100 hit points only 25 gold 25 wood
    //
    hp = GIVES_HP;

    player = unit->Player;

    if (goal->Orders[0].Action!=UnitActionBuilded||(!goal->Type->BuilderOutside)) {
	//
	//  Calculate the repair costs.
	//
	DebugCheck(!goal->Stats->HitPoints);

	for (i = 1; i < MaxCosts; ++i) {
	    if (goal->Stats->Costs[i]) {
		costs[i] = COSTS;
	    } else {			// Prepare for repair cycles
		costs[i] = 0;
	    }
	}
	lrr = player->LastRepairResource;
	for (i = player->LastRepairResource; i < MaxCosts; ++i) {
	    if (costs[i] && lrr == player->LastRepairResource) {
		lrr = i;
	    }				// Find next higher resource or...
	}
	if (lrr == player->LastRepairResource) {
	    for (i = player->LastRepairResource; i > 0; --i) {
		if (costs[i]) {
		    lrr = i;
		}
	    }				// ...go through the beginning
	}
	player->LastRepairResource = lrr;
	// Thanx for the help, costs, you are reset!
	for (i = 1; i < MaxCosts; ++i) {
	    costs[i] = 0;
	}
	costs[player->LastRepairResource] = COSTS;	// The one we need
	//
	//  Check if enough resources are available
	//
	for (i = 1; i < MaxCosts; ++i) {
	    if (player->Resources[i] < costs[i]) {
		snprintf(buf,100,"We need more %s for repair!",DefaultResourceNames[i]);
		NotifyPlayer(player, NotifyYellow, unit->X, unit->Y,buf);
		if( player->Ai ) {
		    // FIXME: call back to AI?
		    RefsDebugCheck(!goal->Refs);
		    if (!--goal->Refs) {
			ReleaseUnit(goal);
		    }
		    unit->Orders[0].Goal = NULL;
		    unit->Orders[0].Action = UnitActionStill;
		    unit->State = unit->SubAction = 0;
		    if (unit->Selected) {	// update display for new action
			SelectedUnitChanged();
		    }
		}
		// FIXME: We shouldn't animate if no resources are available.
		return;
	    }
	}
	//
	//  Subtract the resources
	//
	PlayerSubCosts(player, costs);
    }
    
    //
    //  Repair the unit
    //
    if (goal->Type->BuilderOutside) {
	//  hp is the current damage taken by the unit.
	hp=(goal->Data.Builded.Progress*goal->Stats->HitPoints)/
		(goal->Type->Stats->Costs[TimeCost]*600)-goal->HP;
	//
	//  Calculate the length of the attack (repair) anim.
	//
	animlength=0;
	for (anim=unit->Type->Animations->Attack;!(anim->Flags&AnimationReset);anim++) {
	    animlength+=anim->Sleep;
	}
	
	DebugLevel3("Repair animation is %d cycles long\n" _C_ animlength);
	// FIXME: implement this below:
	//unit->Data.Builded.Worker->Type->BuilderSpeedFactor;
	goal->Data.Builded.Progress+=100*animlength;
	//  Keep the same level of damage while increasing HP.
	goal->HP=(goal->Data.Builded.Progress*goal->Stats->HitPoints)/
		(goal->Type->Stats->Costs[TimeCost]*600)-hp;
	if (goal->HP>goal->Stats->HitPoints) {
	    goal->HP=goal->Stats->HitPoints;
	}
	//  HandleActionBuilded will deal with most stuff.
    } else {
	goal->HP += hp;
	if (goal->HP > goal->Stats->HitPoints) {
	    goal->HP = goal->Stats->HitPoints;
	}
    }

    if (CheckUnitToBeDrawn(goal)) {
	MustRedraw |= RedrawMinimap;
    }
    if (IsOnlySelected(goal)) {		// Update panel if unit is selected
	MustRedraw |= RedrawInfoPanel;
    }
}

/**
**	Animate unit repair
**
**	@param unit	Unit, for that the repair animation is played.
*/
local int AnimateActionRepair(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugCheck( !unit->Type->Animations->Attack );
	// FIXME: A seperate repair animation would be nice?
	DoActionRepairGeneric(unit,unit->Type->Animations->Attack);
    }

    return 0;
}

/**
**	Unit repairs
**
**	@param unit	Unit, for that the attack is handled.
*/
global void HandleActionRepair(Unit* unit)
{
    Unit* goal;
    int err;

    switch( unit->SubAction ) {
	case 0:
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH
	//
	//	Move near to target.
	//
	case 1:
	    // FIXME: RESET FIRST!! Why? We move first and than check if
	    // something is in sight.
	    err=DoActionMove(unit);
	    if( unit->Reset ) {
		//
		//	No goal: if meeting damaged building repair it.
		//
		goal=unit->Orders[0].Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
		    } else if( !goal->HP
				|| goal->Orders[0].Action==UnitActionDie
				|| goal->HP > goal->Stats->HitPoints ) {
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			--goal->Refs;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=goal=NULL;
			// FIXME: should I clear this here?
			NewResetPath(unit);
		    }
		} else if ( unit->Player->AiEnabled ) {
		    // Ai players workers should stop if target is killed
		    err=-1;
		}

		//
		//	Have reached target? FIXME: could use return value
		//
		if(goal&&MapDistanceToUnit(unit->X,unit->Y,goal)<=unit->Type->RepairRange
			&&goal->HP<goal->Type->Stats->HitPoints) {
		    unit->State=0;
		    unit->SubAction=2;
		    unit->Reset=1;
		    UnitHeadingFromDeltaXY(unit,
			goal->X+(goal->Type->TileWidth-1)/2-unit->X,
			goal->Y+(goal->Type->TileHeight-1)/2-unit->Y);
		    // FIXME: only if heading changes
		    CheckUnitToBeDrawn(unit);
		} else if( err<0 ) {
		    if( goal ) {		// release reference
			RefsDebugCheck( !goal->Refs );
			goal->Refs--;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=NoUnitP;
		    }
		    unit->Orders[0].Action=UnitActionStill;
		    unit->State=unit->SubAction=0;
		    if( unit->Selected ) {	// update display for new action
			SelectedUnitChanged();
		    }
		    return;
		}

		// FIXME: Should be it already?
		DebugCheck( unit->Orders[0].Action!=UnitActionRepair );
	    }
	    break;

	//
	//	Repair the target.
	//
	case 2:
	    AnimateActionRepair(unit);
	    if( unit->Reset ) {
		goal=unit->Orders[0].Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
		    } else if( !goal->HP
				|| goal->Orders[0].Action==UnitActionDie ) {
			// FIXME: should I clear this here?
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
		    }
		}
		if( goal ) {
		    RepairUnit(unit,goal);
		    goal=unit->Orders[0].Goal;
		}

		//
		//	Target is fine, choose new one.
		//
		if( !goal || goal->HP >= goal->Stats->HitPoints ) {
		    if( goal ) {		// release reference
			RefsDebugCheck( !goal->Refs );
			goal->Refs--;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=NULL;
		    }
                    unit->Orders[0].Action=UnitActionStill;
		    unit->SubAction=unit->State=0;
		    if( unit->Selected ) {	// update display for new action
			SelectedUnitChanged();
		    }
                    return;
		}

		// FIXME: automatic repair
	    }
	    break;
    }
}

//@}
