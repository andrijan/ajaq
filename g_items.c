//-----------------------------------------------------------------------------
// g_itmes.c
//
// $Id: g_items.c,v 1.13 2002/12/31 17:07:22 igor_rock Exp $
//
//-----------------------------------------------------------------------------
// $Log: g_items.c,v $
// Revision 1.13  2002/12/31 17:07:22  igor_rock
// - corrected the Add_Ammo function to regard wp_flags
//
// Revision 1.12  2002/03/30 17:20:59  ra
// New cvar use_buggy_bandolier to control behavior of dropping bando and grenades
//
// Revision 1.11  2002/03/28 20:28:56  ra
// Forgot a }
//
// Revision 1.10  2002/03/28 20:24:08  ra
// I overfixed the bug...
//
// Revision 1.9  2002/03/28 20:20:30  ra
// Nasty grenade bug fixed.
//
// Revision 1.8  2002/03/27 15:16:56  freud
// Original 1.52 spawn code implemented for use_newspawns 0.
// Teamplay, when dropping bandolier, your drop the grenades.
// Teamplay, cannot pick up grenades unless wearing bandolier.
//
// Revision 1.7  2001/09/28 13:48:34  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.6  2001/07/18 15:19:11  slicerdw
// Time for weapons and items dissapearing is set to "6" to prevent lag on ctf
//
// Revision 1.5  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.4.2.4  2001/05/26 13:04:34  igor_rock
// added some sound to the precache list for flags
//
// Revision 1.4.2.3  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.4.2.2  2001/05/20 18:54:19  igor_rock
// added original ctf code snippets from zoid. lib compilesand runs but
// doesn't function the right way.
// Jsut committing these to have a base to return to if something wents
// awfully wrong.
//
// Revision 1.4.2.1  2001/05/20 15:17:31  igor_rock
// removed the old ctf code completly
//
// Revision 1.4  2001/05/15 15:49:14  igor_rock
// added itm_flags for deathmatch
//
// Revision 1.3  2001/05/14 21:10:16  igor_rock
// added wp_flags support (and itm_flags skeleton - doesn't disturb in the moment)
//
// Revision 1.2  2001/05/11 16:07:25  mort
// Various CTF bits and pieces...
//
// Revision 1.1.1.1  2001/05/06 17:31:33  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include "g_local.h"


qboolean Pickup_Weapon (edict_t * ent, edict_t * other);
void Use_Weapon (edict_t * ent, gitem_t * inv);
void Drop_Weapon (edict_t * ent, gitem_t * inv);

void Weapon_Blaster (edict_t * ent);
void Weapon_Shotgun (edict_t * ent);
void Weapon_SuperShotgun (edict_t * ent);
void Weapon_Machinegun (edict_t * ent);
void Weapon_Chaingun (edict_t * ent);
void Weapon_HyperBlaster (edict_t * ent);
void Weapon_RocketLauncher (edict_t * ent);
void Weapon_Grenade (edict_t * ent);
void Weapon_GrenadeLauncher (edict_t * ent);
void Weapon_Railgun (edict_t * ent);
void Weapon_BFG (edict_t * ent);

// zucc
void Weapon_MK23 (edict_t * ent);
void Weapon_MP5 (edict_t * ent);
void Weapon_M4 (edict_t * ent);
void Weapon_M3 (edict_t * ent);
void Weapon_HC (edict_t * ent);
void Weapon_Sniper (edict_t * ent);
void Weapon_Dual (edict_t * ent);
void Weapon_Knife (edict_t * ent);
void Weapon_Gas (edict_t * ent);
void Weapon_Proxmine (edict_t * ent);

gitem_armor_t jacketarmor_info = { 25, 50, .30f, .00f, ARMOR_JACKET };
gitem_armor_t combatarmor_info = { 50, 100, .60f, .30f, ARMOR_COMBAT };
gitem_armor_t bodyarmor_info = { 100, 200, .80f, .60f, ARMOR_BODY };

static int jacket_armor_index;
static int combat_armor_index;
static int body_armor_index;
static int power_screen_index;
static int power_shield_index;

#define HEALTH_IGNORE_MAX       1
#define HEALTH_TIMED            2

void Use_Quad (edict_t * ent, gitem_t * item);
static int quad_drop_timeout_hack;

//======================================================================

/*
===============
Use_Jet
===============
*/
void Use_Jet ( edict_t *ent, gitem_t *item )
{
    ValidateSelectedItem ( ent );

    /*jetpack in inventory but no fuel time? must be one of the
      give all/give jetpack cheats, so put fuel in*/
    if ( ent->client->Jet_remaining == 0 )
        ent->client->Jet_remaining = 700;

    if ( Jet_Active(ent) ) 
        ent->client->Jet_framenum = 0; 
    else
        ent->client->Jet_framenum = level.framenum + ent->client->Jet_remaining;

    /*The On/Off Sound taken from the invulnerability*/
    gi.sound( ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 0.8, ATTN_NORM, 0 );

    /*this is the sound played when flying. To here this sound 
      immediately we play it here the first time*/
    gi.sound ( ent, CHAN_AUTO, gi.soundindex("hover/hovidle1.wav"), 0.8, ATTN_NORM, 0 );
}

/*
===============
GetItemByIndex
===============
*/
gitem_t *GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}

gitem_t *FindItemByNum (int num)
{
	int i;
	gitem_t *it;

	it = itemlist+1;
	for (i = 1; i < game.num_items; i++, it++)
	{
		if (it->typeNum == num)
			return it;
	}

	return NULL;
}
/*
===============
FindItemByClassname

===============
*/
gitem_t *FindItemByClassname (char *classname)
{
	int i;
	gitem_t *it;

	it = itemlist+1;
	for (i = 1; i < game.num_items; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp (it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t *FindItem (char *pickup_name)
{
	int i;
	gitem_t *it;

	it = itemlist+1;
	for (i = 1; i < game.num_items; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp (it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

//======================================================================

void
DoRespawn (edict_t * ent)
{
  if (ent->team)
    {
      edict_t *master;
      int count;
      int choice;

      master = ent->teammaster;

      //in ctf, when we are weapons stay, only the master of a team of weapons
      //is spawned
      // if (ctf->value &&
      //    ((int)dmflags->value & DF_WEAPONS_STAY) &&
      //    master->item && (master->item->flags & IT_WEAPON))
      //  ent = master;
      //else {
      for (count = 0, ent = master; ent; ent = ent->chain, count++)
	;

      choice = rand () % count;

      for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
	;
      //}
    }

  ent->svflags &= ~SVF_NOCLIENT;
  ent->solid = SOLID_TRIGGER;
  gi.linkentity (ent);

  // send an effect
  ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t * ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}


//======================================================================

qboolean Pickup_Powerup (edict_t * ent, edict_t * other)
{
	int quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX (ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX (ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
			SetRespawn (ent, ent->item->quantity);
		//if (((int)dmflags->value & DF_INSTANT_ITEMS)
		//	|| ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		//{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		//}
	}

	return true;
}

void AddItem(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX (item)]++;
	ent->client->unique_item_total++;
	if (item->typeNum == LASER_NUM)
	{
		ent->client->have_laser = 1;
		SP_LaserSight (ent, item);	//ent->item->use(other, ent->item);
	}
	else if (item->typeNum == BAND_NUM)
	{

		if (ent->client->pers.max_bullets < 4)
			ent->client->pers.max_bullets = 4;
		if (ent->client->pers.max_shells < 28)
			ent->client->pers.max_shells = 28;
		if (ent->client->pers.max_cells < 2)
			ent->client->pers.max_cells = 2;
		if (ent->client->pers.max_slugs < 40)
			ent->client->pers.max_slugs = 40;
		if (ent->client->pers.max_grenades < 6)
			ent->client->pers.max_grenades = 6;
		if (ent->client->pers.max_rockets < 4)
			ent->client->pers.max_rockets = 4;
		if (ent->client->knife_max < 20)
			ent->client->knife_max = 20;
		if (ent->client->grenade_max < 4)
			ent->client->grenade_max = 4;
		if (ent->client->proxmine_max < 4)
			ent->client->proxmine_max = 4;
		// zucc for ir
		/*if ( ir->value && other->client->resp.ir == 0 )
		{
			other->client->ps.rdflags |= RDF_IRGOGGLES;
		}
		*/
	}
}

qboolean Pickup_ItemPack (edict_t * ent, edict_t * other)
{
	gitem_t *spec;
	int count = 0;
	int	item;

	while(count < 2)
	{
		item = newrand(ITEM_COUNT);
		if (INV_AMMO(ent, tnums[item]) > 0 || !((int)itm_flags->value & items[tnums[item]].flag))
			continue;

		spec = GET_ITEM(tnums[item]);
		AddItem(other, spec);
		count++;
	}

	SetRespawn (ent, item_respawn->value);

	return true;
}

//zucc pickup function for special items
qboolean Pickup_Special (edict_t * ent, edict_t * other)
{
    if (heroes->value) {
        if (other->client->resp.team == 1) //ESJ Heroes don't pick up items.
            return false;
    }

	if (other->client->unique_item_total >= unique_items->value)
		return false;

	AddItem(other, ent->item);

	if(!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && item_respawnmode->value)
		SetRespawn (ent, item_respawn->value);

	return true;
}



void Drop_Special (edict_t * ent, gitem_t * item)
{
	int count;

	ent->client->unique_item_total--;
	/*  if ( Q_stricmp( item->pickup_name, LASER_NAME ) == 0 
		&& ent->client->pers.inventory[ITEM_INDEX(item)] <= 1 )
		{
			ent->client->have_laser = 0;
			item->use(ent, item);
		}
	*/
	if (item->typeNum == BAND_NUM && INV_AMMO(ent, BAND_NUM) <= 1)
	{
		if(teamplay->value && !teamdm->value)
			count = 1;
		else
			count = 2;
		ent->client->pers.max_bullets = count;
		if (INV_AMMO(ent, MK23_ANUM) > count)
			INV_AMMO(ent, MK23_ANUM) = count;

		if(teamplay->value && !teamdm->value) {
			if(ent->client->resp.weapon->typeNum == HC_NUM)
				count = 12;
			else
				count = 7;
		} else {
			count = 14;
		}
		ent->client->pers.max_shells = count;
		if (INV_AMMO(ent, SHELL_ANUM) > count)
			INV_AMMO(ent, SHELL_ANUM) = count;

		ent->client->pers.max_cells = 1;
		if (INV_AMMO(ent, M4_ANUM) > 1)
			INV_AMMO(ent, M4_ANUM) = 1;

		ent->client->grenade_max = 2;
		if (use_buggy_bandolier->value == 0) {
			if ((!teamplay->value || teamdm->value) && INV_AMMO(ent, GRENADE_NUM) > 2)
				INV_AMMO(ent, GRENADE_NUM) = 2;
			else if (teamplay->value) {
				if (ent->client->curr_weap == GRENADE_NUM)
					INV_AMMO(ent, GRENADE_NUM) = 1;
				else
					INV_AMMO(ent, GRENADE_NUM) = 0;
			}
		} else {
			if (INV_AMMO(ent, GRENADE_NUM) > 2)
				INV_AMMO(ent, GRENADE_NUM) = 2;
		}
		ent->client->proxmine_max = 2;
		if (use_buggy_bandolier->value == 0) {
			if ((!teamplay->value || teamdm->value) && INV_AMMO(ent, PROXMINE_NUM) > 2)
				INV_AMMO(ent, PROXMINE_NUM) = 2;
			else if (teamplay->value) {
				if (ent->client->curr_weap == PROXMINE_NUM)
					INV_AMMO(ent, PROXMINE_NUM) = 1;
				else
					INV_AMMO(ent, PROXMINE_NUM) = 0;
			}
		} else {
			if (INV_AMMO(ent, PROXMINE_NUM) > 2)
				INV_AMMO(ent, PROXMINE_NUM) = 2;
		}
		if(teamplay->value && !teamdm->value)
			count = 1;
		else
			count = 2;
		ent->client->pers.max_rockets = count;
		if (INV_AMMO(ent, MP5_ANUM) > count)
			INV_AMMO(ent, MP5_ANUM) = count;

		ent->client->knife_max = 10;
		if (INV_AMMO(ent, KNIFE_NUM) > 10)
			INV_AMMO(ent, KNIFE_NUM) = 10;

		if(teamplay->value && !teamdm->value)
			count = 10;
		else
			count = 20;
		ent->client->pers.max_slugs = count;
		if (INV_AMMO(ent, SNIPER_ANUM) > count)
			INV_AMMO(ent, SNIPER_ANUM) = count;

		if (ent->client->unique_weapon_total > unique_weapons->value && !allweapon->value)
		{
			DropExtraSpecial (ent);
			gi.cprintf (ent, PRINT_HIGH, "One of your guns is dropped with the bandolier.\n");
		}
	}
	Drop_Spec (ent, item);
	ValidateSelectedItem (ent);
	SP_LaserSight (ent, item);

}

// called by the "drop item" command
void DropSpecialItem (edict_t * ent)
{
	// this is the order I'd probably want to drop them in...       
	if (INV_AMMO(ent, BAND_NUM))
	    Drop_Special (ent, GET_ITEM(BAND_NUM));
	else if (INV_AMMO(ent, SLIP_NUM))
		Drop_Special (ent, GET_ITEM(SLIP_NUM));
	else if (INV_AMMO(ent, SIL_NUM))
		Drop_Special (ent, GET_ITEM(SIL_NUM));
	else if (INV_AMMO(ent, LASER_NUM))
		Drop_Special (ent, GET_ITEM(LASER_NUM));
	else if (INV_AMMO(ent, HELM_NUM))
		Drop_Special (ent, GET_ITEM(HELM_NUM));
	else if (INV_AMMO(ent, KEV_NUM))
		Drop_Special (ent, GET_ITEM(KEV_NUM));
}


void Drop_General (edict_t * ent, gitem_t * item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t * ent, edict_t * other)
{
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t * ent, edict_t * other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t * ent, edict_t * other)
{
	gitem_t *item;
	int index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;

	item = FindItem ("Bullets");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem ("Shells");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t * ent, edict_t * other)
{
	gitem_t *item;
	int index;

	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 100)
		other->client->pers.max_grenades = 100;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
	if (other->client->pers.max_slugs < 100)
		other->client->pers.max_slugs = 100;

	item = FindItem ("Bullets");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem ("Shells");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem ("Cells");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
		other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem ("Grenades");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem ("Rockets");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem ("Slugs");
	if (item)
	{
		index = ITEM_INDEX (item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
		other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

void Use_Quad (edict_t * ent, gitem_t * item)
{
	int timeout;

	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound (ent, CHAN_ITEM, gi.soundindex ("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather (edict_t * ent, gitem_t * item)
{
	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//      gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t * ent, gitem_t * item)
{
	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//      gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Invulnerability (edict_t * ent, gitem_t * item)
{
	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound (ent, CHAN_ITEM, gi.soundindex ("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Silencer (edict_t * ent, gitem_t * item)
{
	ent->client->pers.inventory[ITEM_INDEX (item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//      gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t * ent, edict_t * other)
{
	if (coop->value)
	{
		if (strcmp (ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00) >> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX (ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX (ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX (ent->item)] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX (ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t * ent, gitem_t * item, int count)
{
	int index;
	int max = 0;

	if (!ent->client)
		return false;

	switch(item->tag) {
	case AMMO_BULLETS:
		if (((int)wp_flags->value & WPF_MK23) || ((int)wp_flags->value & WPF_DUAL))
			max = ent->client->pers.max_bullets;
		break;
	case AMMO_SHELLS:
		if (((int)wp_flags->value & WPF_M3) || ((int)wp_flags->value & WPF_HC))
			max = ent->client->pers.max_shells;
		break;
	case AMMO_ROCKETS:
		if ((int)wp_flags->value & WPF_MP5)
			max = ent->client->pers.max_rockets;
		break;
	case AMMO_GRENADES:
		if ((int)wp_flags->value & WPF_GRENADE)
			max = ent->client->pers.max_grenades;
		break;
	case AMMO_CELLS:
		if ((int)wp_flags->value & WPF_M4)
			max = ent->client->pers.max_cells;
		break;
	case AMMO_SLUGS:
		if ((int)wp_flags->value & WPF_SNIPER)
			max = ent->client->pers.max_slugs;
		break;
	default:
		return false;
	}

	index = ITEM_INDEX (item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t * ent, edict_t * other)
{
	int oldcount;
	int count;
	qboolean weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if ((weapon) && ((int) dmflags->value & DF_INFINITE_AMMO))
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX (ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item
		&& (!deathmatch->value || other->client->pers.weapon == FindItem("blaster")))
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, ammo_respawn->value);

	return true;
}

void Drop_Ammo (edict_t * ent, gitem_t * item)
{
	edict_t *dropped;
	int index;

	if (ent->client->weaponstate == WEAPON_RELOADING)
	{
		gi.cprintf (ent, PRINT_HIGH, "Cant drop ammo while reloading\n");
		return;
	}

	index = ITEM_INDEX (item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];
	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


//======================================================================

void MegaHealth_think (edict_t * self)
{
	if (self->owner->health > self->owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t * ent, edict_t * other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

	other->health += ent->count;

	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else				// (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t * ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t * ent, edict_t * other)
{
	int old_armor_index;
	gitem_armor_t *oldinfo;
	gitem_armor_t *newinfo;
	int newcount;
	float salvage;
	int salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *) ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}
	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX (ent->item)] =
		newinfo->base_count;
	}
	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else			// (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount =
			salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX (ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount =
			other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

int PowerArmorType (edict_t * ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t * ent, gitem_t * item)
{
	int index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX (FindItem ("cells"));
		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t * ent, edict_t * other)
{
	int quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX (ent->item)];

	other->client->pers.inventory[ITEM_INDEX (ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	return true;
}

void Drop_PowerArmor (edict_t * ent, gitem_t * item)
{
	if ((ent->flags & FL_POWER_ARMOR)
		&& (ent->client->pers.inventory[ITEM_INDEX (item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t * ent, edict_t * other, cplane_t * plane,
	    csurface_t * surf)
{
  qboolean taken;

  if (!other->client)
    return;
  if (other->health < 1)
    return;			// dead people can't pickup
  if (!ent->item->pickup)
    return;			// not a grabbable item?

  taken = ent->item->pickup (ent, other);

  if (taken)
    {
      // flash the screen
      other->client->bonus_alpha = 0.25;

      // show icon and name on status bar
//FIREBLADE (debug code)
      if (!ent->item->icon || strlen (ent->item->icon) == 0)
	{
	  if (ent->item->classname)
	    gi.dprintf ("Warning: null icon filename (classname = %s)\n",
			ent->item->classname);
	  else
	    gi.dprintf ("Warning: null icon filename (no classname)\n");

	}
//FIREBLADE
      other->client->ps.stats[STAT_PICKUP_ICON] =
	gi.imageindex (ent->item->icon);
      other->client->ps.stats[STAT_PICKUP_STRING] =
	CS_ITEMS + ITEM_INDEX (ent->item);
      other->client->pickup_msg_time = level.time + 3.0;

      // change selected item
      if (ent->item->use)
	other->client->pers.selected_item =
	  other->client->ps.stats[STAT_SELECTED_ITEM] =
	  ITEM_INDEX (ent->item);

      else
	gi.sound (other, CHAN_ITEM, gi.soundindex (ent->item->pickup_sound),
		  1, ATTN_NORM, 0);
    }

  if (!(ent->spawnflags & ITEM_TARGETS_USED))
    {
      G_UseTargets (ent, other);
      ent->spawnflags |= ITEM_TARGETS_USED;
    }

  if (!taken)
    return;

  if (!((coop->value) && (ent->item->flags & IT_STAY_COOP))
      || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
    {
      if (ent->flags & FL_RESPAWN)
	ent->flags &= ~FL_RESPAWN;
      else
	G_FreeEdict (ent);
    }
}

//======================================================================

static void drop_temp_touch (edict_t * ent, edict_t * other, cplane_t * plane,
		 csurface_t * surf)
{
  if (other == ent->owner)
    return;

  Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t * ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		//AQ2:TNG - Slicer
		if (ctf->value)
		{
			ent->nextthink = level.time + 6;
			ent->think = G_FreeEdict;
		}
		else
		{
			ent->nextthink = level.time + 119;
			ent->think = G_FreeEdict;
		}
	}
}

edict_t *Drop_Item (edict_t * ent, gitem_t * item)
{
	edict_t *dropped;
	vec3_t forward, right;
	vec3_t offset;

	dropped = G_Spawn ();

	dropped->classname = item->classname;
	dropped->typeNum = item->typeNum;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	// zucc dumb hack to make knife look like it is on the ground
	if (item->typeNum == KNIFE_NUM
	 || item->typeNum == LASER_NUM
	 || item->typeNum == GRENADE_NUM
	 || item->typeNum == PROXMINE_NUM)
	{
		VectorSet (dropped->mins, -15, -15, -1);
		VectorSet (dropped->maxs, 15, 15, 1);
	}
	// spin?
	VectorSet (dropped->avelocity, 0, 600, 0);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;

	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet (offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right,
		dropped->s.origin);
		PRETRACE ();
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
		dropped->s.origin, ent, CONTENTS_SOLID);
		POSTTRACE ();
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t * ent, edict_t * other, edict_t * activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor (edict_t * ent)
{
	trace_t tr;
	vec3_t dest;

	VectorSet(ent->mins, -15, -15, -15);
	VectorSet(ent->maxs, 15, 15, 15);

	if (ent->item)
	{
		if (ent->item->typeNum == KNIFE_NUM
		 || ent->item->typeNum == LASER_NUM
		 || ent->item->typeNum == GRENADE_NUM
		 || ent->item->typeNum == PROXMINE_NUM)
		{
			VectorSet (ent->mins, -15, -15, -1);
			VectorSet (ent->maxs, 15, 15, 1);
		}
	}

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;

	VectorCopy(ent->s.origin, dest);
	dest[2] -= 128;

	PRETRACE ();
	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	POSTTRACE ();
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t * it)
{
	char *s, *start;
	char data[MAX_QPATH];
	int len;
	gitem_t *ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s - start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp (data + len - 3, "md2"))
			gi.modelindex (data);
		else if (!strcmp (data + len - 3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp (data + len - 3, "wav"))
			gi.soundindex (data);
		else if (!strcmp (data + len - 3, "pcx"))
			gi.imageindex (data);
	}
}


/*
============
PrecacheItems

Makes sure the client loads all necessary data on connect to avoid lag.
============
*/
void PrecacheItems ()
{
	PrecacheItem(FindItemByClassname("weapon_Mk23"));
	PrecacheItem(FindItemByClassname("weapon_MP5"));
	PrecacheItem(FindItemByClassname("weapon_M4"));
	PrecacheItem(FindItemByClassname("weapon_M3"));
	PrecacheItem(FindItemByClassname("weapon_HC"));
	PrecacheItem(FindItemByClassname("weapon_Sniper"));
	PrecacheItem(FindItemByClassname("weapon_Dual"));
	PrecacheItem(FindItemByClassname("weapon_Knife"));
	PrecacheItem(FindItemByClassname("weapon_Grenade"));
	PrecacheItem(FindItemByClassname("item_quiet"));
	PrecacheItem(FindItemByClassname("item_band"));
	PrecacheItem(FindItemByClassname("item_lasersight"));
	PrecacheItem(FindItemByClassname("item_slippers"));
	PrecacheItem(FindItemByClassname("item_vest"));
	PrecacheItem(FindItemByClassname("item_helmet"));
	PrecacheItem(FindItemByClassname("item_bandolier"));

	if (ctf->value) {
		PrecacheItem(FindItemByClassname("item_flag_team1"));
		PrecacheItem(FindItemByClassname("item_flag_team2"));
	}
}


/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t * ent, gitem_t * item)
{
	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp (ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf ("%s at %s has invalid spawnflags set\n", ent->classname,
			vtos (ent->s.origin));
		}
	}

	//AQ2:TNG - Igor adding wp_flags/itm_flags

	// Weapons and Ammo
	switch(item->typeNum)
	{
	case MK23_NUM:
	case MK23_ANUM:
		if (!((int)wp_flags->value & WPF_MK23)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case MP5_NUM:
	case MP5_ANUM:
		if (!((int)wp_flags->value & WPF_MP5)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case M4_NUM:
	case M4_ANUM:
		if (!((int)wp_flags->value & WPF_M4)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case M3_NUM:
		if (!((int)wp_flags->value & WPF_M3)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case HC_NUM:
		if (!((int)wp_flags->value & WPF_HC)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case SHELL_ANUM:
		if (!(((int)wp_flags->value & WPF_M3) || ((int)wp_flags->value & WPF_HC))) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case SNIPER_NUM:
	case SNIPER_ANUM:
		if (!((int)wp_flags->value & WPF_SNIPER)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case DUAL_NUM:
		if (!((int)wp_flags->value & WPF_DUAL)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case KNIFE_NUM:
		if (!((int)wp_flags->value & WPF_KNIFE)) {
			G_FreeEdict (ent);
			return;
		}
		break;

	case GRENADE_NUM:
		if (!((int)wp_flags->value & WPF_GRENADE)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case PROXMINE_NUM:
		if (!((int)wp_flags->value & WPF_PROXMINE)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	// Items
	case SIL_NUM:
		if (!((int)itm_flags->value & ITF_SIL)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case SLIP_NUM:
		if (!((int)itm_flags->value & ITF_SLIP)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case BAND_NUM:
		if (!((int)itm_flags->value & ITF_BAND)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case KEV_NUM:
		if (!((int)itm_flags->value & ITF_KEV)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case LASER_NUM:
		if (!((int)itm_flags->value & ITF_LASER)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	case HELM_NUM:
		if (!((int)itm_flags->value & ITF_HELM)) {
			G_FreeEdict (ent);
			return;
		}
		break;
	}

	//AQ2:TNG End adding flags

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ((int) dmflags->value & DF_NO_ARMOR)
		{
			if (item->pickup == Pickup_Armor
			|| item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ((int) dmflags->value & DF_NO_ITEMS)
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		// zucc remove health from the game
		if (1 /*(int)dmflags->value & DF_NO_HEALTH */ )
		{
			if (item->pickup == Pickup_Health
			|| item->pickup == Pickup_Adrenaline
			|| item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ((int) dmflags->value & DF_INFINITE_AMMO)
		{
			if ((item->flags == IT_AMMO)
			|| (strcmp (ent->classname, "weapon_bfg") == 0))
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

	if (coop->value && (strcmp (ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

	//Don't spawn the flags unless enabled
	if (!ctf->value && (item->typeNum == FLAG_T1_NUM || item->typeNum == FLAG_T2_NUM))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;	// items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	ent->typeNum = item->typeNum;
	if (ent->model)
		gi.modelindex (ent->model);

	//flags are server animated and have special handling
	if (ctf->value && (item->typeNum == FLAG_T1_NUM || item->typeNum == FLAG_T2_NUM))
	{
		ent->think = CTFFlagSetup;
	}
}

//======================================================================

gitem_t itemlist[] = {
  {
   NULL}
  ,				// leave index 0 alone

  //
  // ARMOR
  //

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_armor_body",
   Pickup_Armor,
   NULL,
   NULL,
   NULL,
   "misc/ar1_pkup.wav",
   "models/items/armor/body/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_bodyarmor",
/* pickup */ "Body Armor",
/* width */ 3,
   0,
   NULL,
   IT_ARMOR,
   &bodyarmor_info,
   ARMOR_BODY,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_armor_combat",
   Pickup_Armor,
   NULL,
   NULL,
   NULL,
   "misc/ar1_pkup.wav",
   "models/items/armor/combat/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_combatarmor",
/* pickup */ "Combat Armor",
/* width */ 3,
   0,
   NULL,
   IT_ARMOR,
   &combatarmor_info,
   ARMOR_COMBAT,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_armor_jacket",
   Pickup_Armor,
   NULL,
   NULL,
   NULL,
   "misc/ar1_pkup.wav",
   "models/items/armor/jacket/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_jacketarmor",
/* pickup */ "Jacket Armor",
/* width */ 3,
   0,
   NULL,
   IT_ARMOR,
   &jacketarmor_info,
   ARMOR_JACKET,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_armor_shard",
   Pickup_Armor,
   NULL,
   NULL,
   NULL,
   "misc/ar2_pkup.wav",
   "models/items/armor/shard/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_jacketarmor",
/* pickup */ "Armor Shard",
/* width */ 3,
   0,
   NULL,
   IT_ARMOR,
   NULL,
   ARMOR_SHARD,
/* precache */ "",
	NO_NUM
   }
  ,


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_power_screen",
   Pickup_PowerArmor,
   Use_PowerArmor,
   Drop_PowerArmor,
   NULL,
   "misc/ar3_pkup.wav",
   "models/items/armor/screen/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_powerscreen",
/* pickup */ "Power Screen",
/* width */ 0,
   60,
   NULL,
   IT_ARMOR,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_power_shield",
   Pickup_PowerArmor,
   Use_PowerArmor,
   Drop_PowerArmor,
   NULL,
   "misc/ar3_pkup.wav",
   "models/items/armor/shield/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_powershield",
/* pickup */ "Power Shield",
/* width */ 0,
   60,
   NULL,
   IT_ARMOR,
   NULL,
   0,
				/* precache */ "",
				// "misc/power2.wav misc/power1.wav"
	NO_NUM
   }
  ,


  //
  // WEAPONS 
  //

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
  {
   "weapon_blaster",
   NULL,
   Use_Weapon,
   NULL,
   Weapon_Blaster,
   "misc/w_pkup.wav",
   NULL, 0,
   "models/weapons/v_blast/tris.md2",
/* icon */ "w_blaster",
/* pickup */ "Blaster",
   0,
   0,
   NULL,
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,

				/* precache */ "",
	NO_NUM
   }
  ,
/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)^M
always owned, never in the world^M
*/
	{
		"weapon_grapple",
		NULL,
		Use_Weapon,
		NULL,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
		/* icon */              "w_grapple",
		/* pickup */    "Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		NULL,
		0,
		/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav",
		GRAPPLE_NUM
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_shotgun",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Shotgun,
   "misc/w_pkup.wav",
   "models/weapons/g_shotg/tris.md2", EF_ROTATE,
   "models/weapons/v_shotg/tris.md2",
/* icon */ "w_shotgun",
/* pickup */ "Shotgun",
   0,
   1,
   "Shells",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/shotgf1b.wav weapons/shotgr1b.wav"
	NO_NUM
   }
  ,

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_supershotgun",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_SuperShotgun,
   "misc/w_pkup.wav",
   "models/weapons/g_shotg2/tris.md2", EF_ROTATE,
   "models/weapons/v_shotg2/tris.md2",
/* icon */ "w_sshotgun",
/* pickup */ "Super Shotgun",
   0,
   2,
   "Shells",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/sshotf1b.wav"
	NO_NUM
   }
  ,

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_machinegun",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Machinegun,
   "misc/w_pkup.wav",
   "models/weapons/g_machn/tris.md2", EF_ROTATE,
   "models/weapons/v_machn/tris.md2",
/* icon */ "w_machinegun",
/* pickup */ "Machinegun",
   0,
   1,
   "Bullets",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	NO_NUM
   }
  ,

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_chaingun",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Chaingun,
   "misc/w_pkup.wav",
   "models/weapons/g_chain/tris.md2", EF_ROTATE,
   "models/weapons/v_chain/tris.md2",
/* icon */ "w_chaingun",
/* pickup */ "Chaingun",
   0,
   1,
   "Bullets",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	NO_NUM
   }
  ,

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_grenades",
   Pickup_Ammo,
   Use_Weapon,
   Drop_Ammo,
   Weapon_Grenade,
   "misc/am_pkup.wav",
   "models/items/ammo/grenades/medium/tris.md2", 0,
   "models/weapons/v_handgr/tris.md2",
/* icon */ "a_grenades",
/* pickup */ "Grenades",
/* width */ 3,
   5,
   "grenades",
   0,				//IT_AMMO|IT_WEAPON,
   NULL,
   AMMO_GRENADES,
/* precache */
   "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/grenlb1b.wav",
	NO_NUM
   }
  ,

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_grenadelauncher",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_GrenadeLauncher,
   "misc/w_pkup.wav",
   "models/weapons/g_launch/tris.md2", EF_ROTATE,
   "models/weapons/v_launch/tris.md2",
/* icon */ "w_glauncher",
/* pickup */ "Grenade Launcher",
   0,
   1,
   "Grenades",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	NO_NUM
   }
  ,

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_rocketlauncher",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_RocketLauncher,
   "misc/w_pkup.wav",
   "models/weapons/g_rocket/tris.md2", EF_ROTATE,
   "models/weapons/v_rocket/tris.md2",
/* icon */ "w_rlauncher",
/* pickup */ "Rocket Launcher",
   0,
   1,
   "Rockets",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	NO_NUM
   }
  ,

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_hyperblaster",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_HyperBlaster,
   "misc/w_pkup.wav",
   "models/weapons/g_hyperb/tris.md2", EF_ROTATE,
   "models/weapons/v_hyperb/tris.md2",
/* icon */ "w_hyperblaster",
/* pickup */ "HyperBlaster",
   0,
   1,
   "Cells",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	NO_NUM
   }
  ,

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "weapon_railgun",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Railgun,
   "misc/w_pkup.wav",
   "models/weapons/g_rail/tris.md2", EF_ROTATE,
   "models/weapons/v_rail/tris.md2",
/* icon */ "w_railgun",
/* pickup */ "Railgun",
   0,
   1,
   "Slugs",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"weapons/rg_hum.wav"
	NO_NUM
   }
  ,



  /*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
   */
  {
   "weapon_bfg",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_BFG,
   "misc/w_pkup.wav",
   "models/weapons/g_bfg/tris.md2", EF_ROTATE,
   "models/weapons/v_bfg/tris.md2",
/* icon */ "w_bfg",
/* pickup */ "BFG10K",
   0,
   50,
   "Cells",
   0,				//IT_WEAPON|IT_STAY_COOP,
   NULL,
   0,
				/* precache */ "",
				//"sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	NO_NUM
   }
  ,

// zucc - New Weapons
/*
gitem_t

referenced by 'entity_name->item.attribute'

Name              Type              Notes

ammo              char *            type of ammo to use
classname         char *            name when spawning it
count_width       int               number of digits to display by icon
drop              void              function called when entity dropped
flags             int               type of pickup :
                                    IT_WEAPON, IT_AMMO, IT_ARMOR
icon              char *            filename of icon
info              void *            ? unused
pickup            qboolean          function called when entity picked up
pickup_name       char *            displayed onscreen when item picked up
pickup_sound      char *            filename of sound to play when picked up
precaches         char *            string containing all models, sounds etc. needed by this
                                    item
quantity          int               ammo gained by item/ammo used per shot by item
tag               int               ? unused
use               void              function called when entity used
view_model        char *            filename of model when being held
weaponthink       void              unused function
world_model       char *            filename of model when item is sitting on level
world_model_flags int               copied to 'ent->s.effects' (see s.effects for values)



  */
  {
   "weapon_Mk23",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_MK23,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_dual/tris.md2",
   0,
   "models/weapons/v_blast/tris.md2",
   "w_mk23",
   MK23_NAME,
   0,
   1,
   "Pistol Clip",
   IT_WEAPON,
   NULL,
   0,
   "weapons/mk23fire.wav weapons/mk23in.wav weapons/mk23out.wav weapons/mk23slap.wav weapons/mk23slide.wav misc/click.wav weapons/machgf4b.wav weapons/blastf1a.wav",
  MK23_NUM}
  ,


  {
   "weapon_MP5",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_MP5,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_machn/tris.md2",
   0,
   "models/weapons/v_machn/tris.md2",
   "w_mp5",
   MP5_NAME,
   0,
   0,
   "Machinegun Magazine",
   IT_WEAPON,
   NULL,
   0,
   "weapons/mp5fire1.wav weapons/mp5in.wav weapons/mp5out.wav weapons/mp5slap.wav weapons/mp5slide.wav weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf5b.wav",
   MP5_NUM}
  ,
  {
   "weapon_M4",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_M4,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_m4/tris.md2",
   0,
   "models/weapons/v_m4/tris.md2",
   "w_m4",
   M4_NAME,
   0,
   0,
   "M4 Clip",
   IT_WEAPON,
   NULL,
   0,
   "weapons/m4a1fire.wav weapons/m4a1in.wav weapons/m4a1out.wav weapons/m4a1slide.wav weapons/rocklf1a.wav weapons/rocklr1b.wav",
  M4_NUM}
  ,
  {
   "weapon_M3",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_M3,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_shotg/tris.md2",
   0,
   "models/weapons/v_shotg/tris.md2",
   "w_super90",
   M3_NAME,
   0,
   0,
   "12 Gauge Shells",
   IT_WEAPON,
   NULL,
   0,
   "weapons/m3in.wav weapons/shotgr1b.wav weapons/shotgf1b.wav",
  M3_NUM}
  ,
  {
   "weapon_HC",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_HC,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_cannon/tris.md2",
   0,
   "models/weapons/v_cannon/tris.md2",
   "w_cannon",
   HC_NAME,
   0,
   0,
   "12 Gauge Shells",
   IT_WEAPON,
   NULL,
   0,
   "weapons/cannon_fire.wav weapons/sshotf1b.wav weapons/cclose.wav weapons/cin.wav weapons/cout.wav weapons/copen.wav",
  HC_NUM}
  ,
  {
   "weapon_Sniper",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Sniper,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_sniper/tris.md2",
   0,
   "models/weapons/v_sniper/tris.md2",
   "w_sniper",
   SNIPER_NAME,
   0,
   0,
   "AP Sniper Ammo",
   IT_WEAPON,
   NULL,
   0,
   "weapons/ssgbolt.wav weapons/ssgfire.wav weapons/ssgin.wav misc/lensflik.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav",
  SNIPER_NUM}
  ,
  {
   "weapon_Dual",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Dual,
   //"misc/w_pkup.wav",
   NULL,
   "models/weapons/g_dual/tris.md2",
   0,
   "models/weapons/v_dual/tris.md2",
   "w_akimbo",
   DUAL_NAME,
   0,
   0,
   "Pistol Clip",
   IT_WEAPON,
   NULL,
   0,
   "weapons/mk23fire.wav weapons/mk23in.wav weapons/mk23out.wav weapons/mk23slap.wav weapons/mk23slide.wav",
  DUAL_NUM}
  ,
  {
   "weapon_Knife",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Knife,
   NULL,
   "models/objects/knife/tris.md2",
   0,
   "models/weapons/v_knife/tris.md2",
   "w_knife",
   KNIFE_NAME,
   0,
   0,
   NULL,
   IT_WEAPON,
   NULL,
   0,
   "weapons/throw.wav weapons/stab.wav weapons/swish.wav weapons/clank.wav",
  KNIFE_NUM}
  ,
  {
   "weapon_Grenade",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Gas,
   NULL,
   "models/objects/grenade2/tris.md2",
   0,
   "models/weapons/v_handgr/tris.md2",
   "a_m61frag",
   GRENADE_NAME,
   0,
   0,
   NULL,
   IT_WEAPON,
   NULL,
   0,
   "misc/grenade.wav weapons/grenlb1b.wav weapons/hgrent1a.wav",
  GRENADE_NUM}
  ,
  {
   "weapon_Proxmine",
   Pickup_Weapon,
   Use_Weapon,
   Drop_Weapon,
   Weapon_Proxmine,
   NULL,
   "models/items/keys/red_key/tris.md2",
   0,
   "models/weapons/v_flareg/tris.md2",
   "a_prox",
   PROXMINE_NAME,
   0,
   0,
   NULL,
   IT_WEAPON,
   NULL,
   0,
   "misc/grenade.wav weapons/grenlb1b.wav weapons/hgrent1a.wav",
  PROXMINE_NUM}
  ,



  //
  // AMMO ITEMS
  //

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_shells",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   "misc/am_pkup.wav",
   "models/items/ammo/shells/medium/tris.md2", 0,
   NULL,
/* icon */ "a_shells",
/* pickup */ "Shells",
/* width */ 3,
   10,
   NULL,
   0,				//IT_AMMO,
   NULL,
   AMMO_SHELLS,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_bullets",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   "misc/am_pkup.wav",
   "models/items/ammo/bullets/medium/tris.md2", 0,
   NULL,
/* icon */ "a_bullets",
/* pickup */ "Bullets",
/* width */ 3,
   50,
   NULL,
   0,				//IT_AMMO,
   NULL,
   AMMO_BULLETS,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_cells",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   "misc/am_pkup.wav",
   "models/items/ammo/cells/medium/tris.md2", 0,
   NULL,
/* icon */ "a_cells",
/* pickup */ "Cells",
/* width */ 3,
   50,
   NULL,
   0,				//IT_AMMO,
   NULL,
   AMMO_CELLS,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_rockets",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   "misc/am_pkup.wav",
   "models/items/ammo/rockets/medium/tris.md2", 0,
   NULL,
/* icon */ "a_rockets",
/* pickup */ "Rockets",
/* width */ 3,
   5,
   NULL,
   0,				//IT_AMMO,
   NULL,
   AMMO_ROCKETS,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "ammo_slugs",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   "misc/am_pkup.wav",
   "models/items/ammo/slugs/medium/tris.md2", 0,
   NULL,
/* icon */ "a_slugs",
/* pickup */ "Slugs",
/* width */ 3,
   10,
   NULL,
   0,				//IT_AMMO,
   NULL,
   AMMO_SLUGS,
/* precache */ "",
	NO_NUM
   }
  ,

// zucc new ammo
  {
   "ammo_clip",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   //"misc/click.wav",
   NULL,
   "models/items/ammo/clip/tris.md2", 0,
   NULL,
/* icon */ "a_clip",
/* pickup */ "Pistol Clip",
/* width */ 3,
   1,
   NULL,
   IT_AMMO,
   NULL,
   AMMO_BULLETS,
/* precache */ "",
	MK23_ANUM
   }
  ,

  {
   "ammo_mag",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   //"misc/click.wav",
   NULL,
   "models/items/ammo/mag/tris.md2", 0,
   NULL,
/* icon */ "a_mag",
/* pickup */ "Machinegun Magazine",
/* width */ 3,
   1,
   NULL,
   IT_AMMO,
   NULL,
   AMMO_ROCKETS,
/* precache */ "",
	MP5_ANUM
   }
  ,

  {
   "ammo_m4",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   //"misc/click.wav",
   NULL,
   "models/items/ammo/m4/tris.md2", 0,
   NULL,
/* icon */ "a_m4",
/* pickup */ "M4 Clip",
/* width */ 3,
   1,
   NULL,
   IT_AMMO,
   NULL,
   AMMO_CELLS,
/* precache */ "",
	M4_ANUM
   }
  ,
  {
   "ammo_m3",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   //"misc/click.wav",
   NULL,
   "models/items/ammo/shells/medium/tris.md2", 0,
   NULL,
/* icon */ "a_shells",
/* pickup */ "12 Gauge Shells",
/* width */ 3,
   7,
   NULL,
   IT_AMMO,
   NULL,
   AMMO_SHELLS,
/* precache */ "",
	SHELL_ANUM
   }
  ,
  {
   "ammo_sniper",
   Pickup_Ammo,
   NULL,
   Drop_Ammo,
   NULL,
   //"misc/click.wav",
   NULL,
   "models/items/ammo/sniper/tris.md2", 0,
   NULL,
/* icon */ "a_bullets",
/* pickup */ "AP Sniper Ammo",
/* width */ 3,
   10,
   NULL,
   IT_AMMO,
   NULL,
   AMMO_SLUGS,
/* precache */ "",
	SNIPER_ANUM
   }
  ,



  //
  // POWERUP ITEMS
  //

  // zucc the main items
  {
   "item_quiet",
   Pickup_Special,
   NULL,
   Drop_Special,
   NULL,
   "misc/screw.wav",
   "models/items/quiet/tris.md2",
   0,
   NULL,
   /* icon */ "p_silencer",
   /* pickup */ "Silencer",
   /* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   SIL_NUM
   }
  ,

  {
   "item_slippers",
   Pickup_Special,
   NULL,
   Drop_Special,
   NULL,
   "misc/veston.wav",		// sound
   "models/items/slippers/slippers.md2",
   0,
   NULL,
/* icon */ "slippers",
/* pickup */ "Stealth Slippers",
/* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   SLIP_NUM
   }
  ,

  {
   "item_band",
   Pickup_Special,
   NULL,
   Drop_Special,
   NULL,
   "misc/veston.wav",		// sound
   "models/items/band/tris.md2",
   0,
   NULL,
   /* icon */ "p_bandolier",
   /* pickup */ "Bandolier",
   /* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   BAND_NUM
   }
  ,
  {
   "item_vest",
   Pickup_Special,
   NULL,
   Drop_Special,
   NULL,
   "misc/veston.wav",		// sound
   "models/items/armor/jacket/tris.md2",
   0,
   NULL,
   /* icon */ "i_jacketarmor",
   /* pickup */ "Kevlar Vest",
   /* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   KEV_NUM
   }
  ,
  {
   "item_lasersight",
   Pickup_Special,
   NULL,			//SP_LaserSight,
   Drop_Special,
   NULL,
   "misc/lasersight.wav",	// sound
   "models/items/laser/tris.md2",
   0,
   NULL,
   /* icon */ "p_laser",
   /* pickup */ "Lasersight",
   /* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   LASER_NUM
   }
  ,


/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_quad",
   Pickup_Powerup,
   Use_Quad,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/hover/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_quad",
/* pickup */ "Quad Damage",
/* width */ 2,
   60,
   NULL,
   0,
   NULL,
   0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
   }
  ,

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_invulnerability",
   Pickup_Powerup,
   Use_Invulnerability,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/invulner/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_invulnerability",
/* pickup */ "Invulnerability",
/* width */ 2,
   300,
   NULL,
  IT_POWERUP,
   NULL,
   0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav",
	NO_NUM
   }
  ,

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_silencer",
   Pickup_Powerup,
   Use_Silencer,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/silencer/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_silencer",
/* pickup */ "Silencer",
/* width */ 2,
   60,
   NULL,
   0, //IT_POWERUP,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

  {
   "item_helmet",
   Pickup_Special,
   NULL,
   Drop_Special,
   NULL,
   "misc/veston.wav",		// sound
   "models/items/breather/tris.md2",
   0,
   NULL,
   /* icon */ "p_rebreather",
   /* pickup */ "Kevlar Helmet",
   /* width */ 2,
   60,
   NULL,
   IT_ITEM,
   NULL,
   0,
   /* precache */ "",
   HELM_NUM
   }
  ,
/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_breather",
   Pickup_Powerup,
   Use_Breather,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/breather/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_rebreather",
/* pickup */ "Rebreather",
/* width */ 2,
   60,
   NULL,
   0, //IT_STAY_COOP | IT_POWERUP,
   NULL,
   0,
/* precache */ "items/airout.wav",
	NO_NUM
   }
  ,

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_enviro",
   Pickup_Powerup,
   Use_Envirosuit,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/enviro/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_envirosuit",
/* pickup */ "Environment Suit",
/* width */ 2,
   60,
   NULL,
   0,				//IT_STAY_COOP|IT_POWERUP,
   NULL,
   0,
/* precache */ "items/airout.wav",
	NO_NUM
   }
  ,

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
  {
   "item_ancient_head",
   Pickup_AncientHead,
   NULL,
   NULL,
   NULL,
   "items/pkup.wav",
   "models/items/c_head/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_fixme",
/* pickup */ "Ancient Head",
/* width */ 2,
   60,
   NULL,
   0,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
  {
   "item_adrenaline",
   Pickup_Adrenaline,
   NULL,
   NULL,
   NULL,
   "items/pkup.wav",
   "models/items/adrenal/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_adrenaline",
/* pickup */ "Adrenaline",
/* width */ 2,
   60,
   NULL,
   0,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_bandolier",
   Pickup_Bandolier,
   NULL,
   NULL,
   NULL,
   "items/pkup.wav",
   "models/items/band/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "p_bandolier",
/* pickup */ "NogBandolier",
/* width */ 2,
   60,
   NULL,
   0,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
  {
   "item_pack",
   Pickup_ItemPack, //Pickup_Pack,
   NULL,
   NULL,
   NULL,
   "items/pkup.wav",
   "models/items/pack/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_pack",
/* pickup */ "Ammo Pack",
/* width */ 2,
   180,
   NULL,
   IT_ITEM, //0,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

  //
  // KEYS
  //
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
  {
   "key_data_cd",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/data_cd/tris.md2", EF_ROTATE,
   NULL,
   "k_datacd",
   "Data CD",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
  {
   "key_power_cube",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/power/tris.md2", EF_ROTATE,
   NULL,
   "k_powercube",
   "Power Cube",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
  {
   "key_pyramid",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/pyramid/tris.md2", EF_ROTATE,
   NULL,
   "k_pyramid",
   "Pyramid Key",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
  {
   "key_data_spinner",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/spinner/tris.md2", EF_ROTATE,
   NULL,
   "k_dataspin",
   "Data Spinner",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
  {
   "key_pass",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/pass/tris.md2", EF_ROTATE,
   NULL,
   "k_security",
   "Security Pass",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
  {
   "key_blue_key",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/key/tris.md2", EF_ROTATE,
   NULL,
   "k_bluekey",
   "Blue Key",
   2,
   0,
   NULL,
  0, // IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
  {
   "key_red_key",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/red_key/tris.md2", EF_ROTATE,
   NULL,
   "k_redkey",
   "Red Key",
   2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
  {
   "key_commander_head",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/monsters/commandr/head/tris.md2", EF_GIB,
   NULL,
/* icon */ "k_comhead",
/* pickup */ "Commander's Head",
/* width */ 2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
  {
   "key_airstrike_target",
   Pickup_Key,
   NULL,
   Drop_General,
   NULL,
   "items/pkup.wav",
   "models/items/keys/target/tris.md2", EF_ROTATE,
   NULL,
/* icon */ "i_airstrike",
/* pickup */ "Airstrike Marker",
/* width */ 2,
   0,
   NULL,
   0, //IT_STAY_COOP | IT_KEY,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

  {
   NULL,
   Pickup_Health,
   NULL,
   NULL,
   NULL,
   "items/pkup.wav",
   NULL, 0,
   NULL,
/* icon */ "i_health",
/* pickup */ "Health",
/* width */ 3,
   0,
   NULL,
   0,
   NULL,
   0,
/* precache */ "",
	NO_NUM
   }
  ,

  /*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
   */
  {
   "item_flag_team1",
   CTFPickup_Flag,
   NULL,
   CTFDrop_Flag,		//Should this be null if we don't want players to drop it manually?
   NULL,
   "tng/flagtk.wav",
   "models/flags/flag1.md2", EF_FLAG1,
   NULL,
   /* icon */ "i_ctf1",
   /* pickup */ "Red Flag",
   /* width */ 2,
   0,
   NULL,
   IT_FLAG,
   NULL,
   0,
   /* precache */ "tng/flagcap.wav tng/flagret.wav",
	FLAG_T1_NUM
   }
  ,

  /*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
   */
  {
   "item_flag_team2",
   CTFPickup_Flag,
   NULL,
   CTFDrop_Flag,		//Should this be null if we don't want players to drop it manually?
   NULL,
   "tng/flagtk.wav",
   "models/flags/flag2.md2", EF_FLAG2,
   NULL,
   /* icon */ "i_ctf2",
   /* pickup */ "Blue Flag",
   /* width */ 2,
   0,
   NULL,
   IT_FLAG,
   NULL,
   0,
   /* precache */ "tng/flagcap.wav tng/flagret.wav",
   FLAG_T2_NUM
   }
  ,

  // end of list marker
  {NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void
SP_item_health (edict_t * self)
{
  if (1)			//deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
    {
      G_FreeEdict (self);
      return;
    }

  self->model = "models/items/healing/medium/tris.md2";
  self->count = 10;
  SpawnItem (self, FindItem ("Health"));
  gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void
SP_item_health_small (edict_t * self)
{
  if (1)			//deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
    {
      G_FreeEdict (self);
      return;
    }

  self->model = "models/items/healing/stimpack/tris.md2";
  self->count = 2;
  SpawnItem (self, FindItem ("Health"));
  self->style = HEALTH_IGNORE_MAX;
  gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void
SP_item_health_large (edict_t * self)
{
  if (1)			//deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
    {
      G_FreeEdict (self);
      return;
    }

  self->model = "models/items/healing/large/tris.md2";
  self->count = 25;
  SpawnItem (self, FindItem ("Health"));
  gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void
SP_item_health_mega (edict_t * self)
{
  if (1)			//deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
    {
      G_FreeEdict (self);
      return;
    }

  self->model = "models/items/mega_h/tris.md2";
  self->count = 100;
  SpawnItem (self, FindItem ("Health"));
  gi.soundindex ("items/m_health.wav");
  self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}


itemList_t items[ILIST_COUNT];

void InitItems (void)
{
	int i;

	game.num_items = sizeof (itemlist) / sizeof (itemlist[0]) - 1;

	for(i=0; i<ILIST_COUNT; i++)
	{
		items[i].index = ITEM_INDEX(FindItemByNum(i));
		items[i].flag = 0;
	}

	items[MK23_NUM].flag = WPF_MK23;
	items[MP5_NUM].flag = WPF_MP5;
	items[M4_NUM].flag = WPF_M4;
	items[M3_NUM].flag = WPF_M3;
	items[HC_NUM].flag = WPF_HC;
	items[SNIPER_NUM].flag = WPF_SNIPER;
	items[DUAL_NUM].flag = WPF_DUAL;
	items[KNIFE_NUM].flag = WPF_KNIFE;
	items[GRENADE_NUM].flag = WPF_GRENADE;
	items[PROXMINE_NUM].flag = WPF_PROXMINE;

	items[SIL_NUM].flag = ITF_SIL;
	items[SLIP_NUM].flag = ITF_SLIP;
	items[BAND_NUM].flag = ITF_BAND;
	items[KEV_NUM].flag = ITF_KEV;
	items[LASER_NUM].flag = ITF_LASER;
	items[HELM_NUM].flag = ITF_HELM;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int i;
	gitem_t *it;

	for (i = 0; i < game.num_items; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS + i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX (FindItem ("Jacket Armor"));
	combat_armor_index = ITEM_INDEX (FindItem ("Combat Armor"));
	body_armor_index = ITEM_INDEX (FindItem ("Body Armor"));
	power_screen_index = ITEM_INDEX (FindItem ("Power Screen"));
	power_shield_index = ITEM_INDEX (FindItem ("Power Shield"));


}