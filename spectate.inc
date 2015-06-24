/*
	Spectate Include (spectate.inc)
	* An advance spectate system for SAMP.
	* Camera freezer, now don't show the ugly sea view camera. Freeze the camera when the player is lost (died or spawned)
	* Advance spectate pooling techique to easily manage limited spectating
	* Crash free, bug free and safe!
	* Auto redirecting so no need to repeat code
	* Auto adjustments, adjust interior and virtual world automatically
	* New callbacks to handle spectating!

 	Author: (creator)
	* Gammix

	(c) Copyright 2015
  	* This file is provided as is (no warranties).
*/

/*
FUNCTIONS:
native GetPlayerSpectateID(playerid);
native GetPlayerSpectateType(playerid);
native GetPlayerSpectateMode(playerid);
native PlayerSpectatePlayer(playerid, targetplayerid, mode = SPECTATE_MODE_NORMAL);
native PlayerSpectateVehicle(playerid, targetvehicleid, mode = SPECTATE_MODE_NORMAL);
native TogglePlayerSpectating(playerid, bool:toggle);
native SetPlayerSpectateKeys(playerid, keytype, key);
native GetPlayerSpectateKeys(playerid, keytype);
native IsPlayerSpectating(playerid);
native UpdatePlayerSpectate(playerid, bool:next = true);
native SetPlayerSpectatePool(playerid, index, id);
native GetPlayerSpectatePool(playerid, index);
native ResetPlayerSpectatePool(playerid);

CALLBACKS:
public OnPlayerSpecUpdate(playerid, targetplayerid, updatetype, newvalue, oldvalue)
*/

#define SPEC_TYPE_NONE      	0
#define SPEC_TYPE_PLAYER    	1
#define SPEC_TYPE_VEHICLE   	2
#define SPEC_TYPE_FREECAM   	3

#define SPEC_UPDATE_WORLDID     0
#define SPEC_UPDATE_INTERIORID  1
#define SPEC_UPDATE_DEATH       2
#define SPEC_UPDATE_DISCONNECT  3
#define SPEC_UPDATE_START       4
#define SPEC_UPDATE_STOP        5
#define SPEC_UPDATE_NEXT    	6
#define SPEC_UPDATE_PREVIOUS    7
#define SPEC_UPDATE_SPAWN       8
#define SPEC_UPDATE_CHANGE      9

#define SPEC_KEY_INVALID        -1
#define SPEC_KEY_NEXT        	0
#define SPEC_KEY_PREVIOUS       1
#define SPEC_KEY_STOP        	2

#define SPEC_MAX_POOL_SIZE  	50 //maximum ids in a spectate pool

#define SPEC_LOOP_PLAYERS(%0)   		for(new %0, _%0 = GetPlayerPoolSize(); %0 <= _%0, IsPlayerConnected(%0); %0++)
#define SPEC_LOOP_PLAYERS_NEXT(%0,%1)   for(new %0 = %1; %0 < MAX_PLAYERS; %0++)
#define SPEC_LOOP_PLAYERS_PREV(%0,%1)   for(new %0 = %1; %0 > -1; %0--)

#if ! defined IsPlayerSpectating
	stock IsPlayerSpectating(playerid)
	{
	    if(GetPlayerState(playerid) == PLAYER_STATE_SPECTATING) return true;
	    return false;
	}
#endif

enum SpectateEnum
{
	SPEC_ID,
	SPEC_TYPE,
	SPEC_MODE,
	#if ! defined OnPlayerVirtualWorldChange
		SPEC_VIRTUAL_WORLD,
	#endif
	SPEC_POOL[SPEC_MAX_POOL_SIZE],
	SPEC_KEY_SET_NEXT,
	SPEC_KEY_SET_PREVIOUS,
	SPEC_KEY_SET_STOP
};
static gSpectate[MAX_PLAYERS][SpectateEnum];

stock GetPlayerSpectateID(playerid)
{
	return gSpectate[playerid][SPEC_ID];
}

stock GetPlayerSpectateType(playerid)
{
	return gSpectate[playerid][SPEC_TYPE];
}

stock GetPlayerSpectateMode(playerid)
{
	return gSpectate[playerid][SPEC_MODE];
}

stock SPEC_PlayerSpectatePlayer(playerid, targetplayerid, mode = SPECTATE_MODE_NORMAL)
{
	if(PlayerSpectatePlayer(playerid, targetplayerid, mode))
	{
	    new empty_counts = 0;
	    for(new i; i < SPEC_MAX_POOL_SIZE; i++)
	    {
	        if(	GetPlayerSpectatePool(playerid, i) == targetplayerid &&
				GetPlayerSpectatePool(playerid, i) != -1)
	        {
	            gSpectate[playerid][SPEC_ID] = targetplayerid;
			    gSpectate[playerid][SPEC_TYPE] = SPEC_TYPE_PLAYER;
			    gSpectate[playerid][SPEC_MODE] = mode;

			    SetPlayerInterior(playerid, GetPlayerInterior(targetplayerid));
			    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetplayerid));

				if(! IsPlayerSpectating(playerid))
				{
					CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_START, SPEC_TYPE_PLAYER, SPEC_TYPE_NONE);
				}
				else if(GetPlayerSpectateType(playerid) != SPEC_TYPE_PLAYER)
				{
					CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_CHANGE, SPEC_TYPE_PLAYER, GetPlayerSpectateType(playerid));
				}
				return true;
	        }
	        else if(GetPlayerSpectatePool(playerid, i) == -1)
	        {
	            empty_counts += 1;
	        }
	    }
		if(empty_counts >= SPEC_MAX_POOL_SIZE)
		{
		    gSpectate[playerid][SPEC_ID] = targetplayerid;
		    gSpectate[playerid][SPEC_TYPE] = SPEC_TYPE_PLAYER;
			gSpectate[playerid][SPEC_MODE] = mode;

		    SetPlayerInterior(playerid, GetPlayerInterior(targetplayerid));
		    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetplayerid));

			if(! IsPlayerSpectating(playerid))
			{
				CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_START, SPEC_TYPE_PLAYER, SPEC_TYPE_NONE);
			}
			else if(GetPlayerSpectateType(playerid) != SPEC_TYPE_PLAYER)
			{
				CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_CHANGE, SPEC_TYPE_PLAYER, GetPlayerSpectateType(playerid));
			}
			return true;
		}

	    return UpdatePlayerSpectate(playerid, true);
	}
	return false;
}
#if defined _ALS_PlayerSpectatePlayer
    #undef PlayerSpectatePlayer
#else
    #define _ALS_PlayerSpectatePlayer
#endif
#define PlayerSpectatePlayer SPEC_PlayerSpectatePlayer

stock SPEC_PlayerSpectateVehicle(playerid, targetvehicleid, mode = SPECTATE_MODE_NORMAL)
{
	if(PlayerSpectateVehicle(playerid, targetvehicleid, mode))
	{
	    gSpectate[playerid][SPEC_ID] = targetvehicleid;
		gSpectate[playerid][SPEC_TYPE] = SPEC_TYPE_VEHICLE;
  		gSpectate[playerid][SPEC_MODE] = mode;

		SetPlayerInterior(playerid, GetPlayerInterior(targetplayerid));
		SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetplayerid));

		if(! IsPlayerSpectating(playerid))
		{
			CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_START, SPEC_TYPE_VEHICLE, SPEC_TYPE_NONE);
		}
		if(GetPlayerSpectateType(playerid) != SPEC_TYPE_VEHICLE)
		{
			CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, targetplayerid, SPEC_UPDATE_CHANGE, SPEC_TYPE_VEHICLE, GetPlayerSpectateType(playerid));
		}
		return true;
	}
	return false;
}
#if defined _ALS_PlayerSpectateVehicle
    #undef PlayerSpectateVehicle
#else
    #define _ALS_PlayerSpectateVehicle
#endif
#define PlayerSpectateVehicle SPEC_PlayerSpectateVehicle

stock SPEC_TogglePlayerSpectating(playerid, bool:toggle)
{
	if(TogglePlayerSpectating(playerid, toggle))
	{
		CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, GetPlayerSpectateID(playerid), SPEC_UPDATE_STOP, gSpectate[playerid][SPEC_TYPE], SPEC_TYPE_NONE);

	    gSpectate[playerid][SPEC_ID] = INVALID_PLAYER_ID;
	    gSpectate[playerid][SPEC_TYPE] = SPEC_TYPE_NONE;
		return true;
	}
	return false;
}
#if defined _ALS_TogglePlayerSpectating
    #undef TogglePlayerSpectating
#else
    #define _ALS_TogglePlayerSpectating
#endif
#define TogglePlayerSpectating SPEC_TogglePlayerSpectating

stock SetPlayerSpectateKeys(playerid, keytype, key)
{
	switch(keytype)
	{
		case SPEC_KEY_NEXT: gSpectate[playerid][SPEC_KEY_SET_NEXT] = key;
		case SPEC_KEY_PREVIOUS: gSpectate[playerid][SPEC_KEY_SET_PREVIOUS] = key;
		case SPEC_KEY_STOP: gSpectate[playerid][SPEC_KEY_SET_STOP] = key;
		default: return false;
	}
	return true;
}

stock GetPlayerSpectateKeys(playerid, keytype)
{
	switch(keytype)
	{
		case SPEC_KEY_NEXT: return gSpectate[playerid][SPEC_KEY_SET_NEXT];
		case SPEC_KEY_PREVIOUS: return gSpectate[playerid][SPEC_KEY_SET_PREVIOUS];
		case SPEC_KEY_STOP: return gSpectate[playerid][SPEC_KEY_SET_STOP];
	}
	return -1;
}

#if ! defined IsPlayerSpectating
	stock IsPlayerSpectating(playerid)
	{
		if(GetPlayerState(playerid) == PLAYER_STATE_SPECTATING) return true;
		return false;
	}
#endif

stock UpdatePlayerSpectate(playerid, bool:next = true)
{
	switch(GetPlayerSpectateMode(playerid))
	{
	    case SPEC_TYPE_PLAYER:
	    {
			if(next)
			{
			    SPEC_LOOP_PLAYERS_NEXT(i, (GetPlayerSpectateID(playerid) + 1))
				{
					if(	IsPlayerConnected(i) &&
						! IsPlayerSpectating(i) &&
						i != playerid)
			    	{
			   			CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, i, SPEC_UPDATE_NEXT, i, GetPlayerSpectateID(playerid));
			    		PlayerSpectatePlayer(playerid, i, GetPlayerSpectateMode(playerid));
			    		return true;
			    	}
					else if(i < GetPlayerPoolSize())
					{
					    SPEC_LOOP_PLAYERS_PREV(p, (GetPlayerSpectateID(playerid) - 1))
					    {
					        if(	IsPlayerConnected(p) &&
								! IsPlayerSpectating(p) &&
								i != playerid)
					        {
				   				CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, p, SPEC_UPDATE_PREVIOUS, p, GetPlayerSpectateID(playerid));
					            PlayerSpectatePlayer(playerid, p, GetPlayerSpectateMode(playerid));
					            return true;
					        }
					        if(p < 0)
					        {
					            return false;
					        }
					    }
					}
				}
			}
			else
			{
			    SPEC_LOOP_PLAYERS_PREV(p, (GetPlayerSpectateID(playerid) - 1))
				{
					if(	IsPlayerConnected(p) &&
						! IsPlayerSpectating(p) &&
						p != playerid)
					{
						CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, p, SPEC_UPDATE_PREVIOUS, p, GetPlayerSpectateID(playerid));
		    			PlayerSpectatePlayer(playerid, p, GetPlayerSpectateMode(playerid));
		       			return true;
					}
					if(p < 0)
					{
		   				SPEC_LOOP_PLAYERS_NEXT(i, (GetPlayerSpectateID(playerid) + 1))
						{
							if(	IsPlayerConnected(i) &&
								! IsPlayerSpectating(i) &&
								i != playerid)
					    	{
					   			CallLocalFunction("OnPlayerSpecUpdate", "iiiii", playerid, i, SPEC_UPDATE_NEXT, i, GetPlayerSpectateID(playerid));
					    		PlayerSpectatePlayer(playerid, i, GetPlayerSpectateMode(playerid));
					    		return true;
					    	}
					    	else if(i < GetPlayerPoolSize())
					    	{
					    	    return false;
					    	}
					    }
		       		}
			  	}
			}
		}
		default: return false;
	}
	return false;
}

stock SetPlayerSpectatePool(playerid, index, id)
{
    gSpectate[playerid][SPEC_POOL][index] = id;
	return true;
}

stock GetPlayerSpectatePool(playerid, index)
{
    return gSpectate[playerid][SPEC_POOL][index];
}

stock ResetPlayerSpectatePool(playerid)
{
	for(new i; i < SPEC_MAX_POOL_SIZE; i++)
	{
		gSpectate[playerid][SPEC_POOL][i] = -1;
	}
	return true;
}

public OnPlayerConnect(playerid)
{
	gSpectate[playerid][SPEC_ID] = INVALID_PLAYER_ID;
	gSpectate[playerid][SPEC_TYPE] = SPEC_TYPE_NONE;
	gSpectate[playerid][SPEC_MODE] = -1;
	#if ! defined OnPlayerVirtualWorldChange
		gSpectate[playerid][SPEC_VIRTUAL_WORLD] = GetPlayerVirtualWorld(playerid);
	#endif
	ResetPlayerSpectatePool(playerid);
	gSpectate[playerid][SPEC_KEY_SET_NEXT] = KEY_RIGHT;
	gSpectate[playerid][SPEC_KEY_SET_PREVIOUS] = KEY_LEFT;
	gSpectate[playerid][SPEC_KEY_SET_STOP] = KEY_SECONDARY_ATTACK;

    #if defined SPEC_OnPlayerConnect
       	return SPEC_OnPlayerConnect(playerid);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect SPEC_OnPlayerConnect
#if defined SPEC_OnPlayerConnect
    forward SPEC_OnPlayerConnect(playerid);
#endif

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
	if(GetPlayerSpectateType(playerid) == SPEC_TYPE_PLAYER)
	{
		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != SPEC_KEY_INVALID)
		{
			if(	GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != KEY_UP &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != KEY_DOWN &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != KEY_LEFT &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != KEY_RIGHT)
			{
			    if(newkeys == GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT))
			    {
			        UpdatePlayerSpectate(playerid, true);
			    }
			}
		}

		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != SPEC_KEY_INVALID)
		{
			if(	GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != KEY_UP &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != KEY_DOWN &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != KEY_LEFT &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != KEY_RIGHT)
			{
			    if(newkeys == GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT))
			    {
			        UpdatePlayerSpectate(playerid, false);
			    }
			}
		}

		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != SPEC_KEY_INVALID)
		{
			if(	GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != KEY_UP &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != KEY_DOWN &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != KEY_LEFT &&
				GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != KEY_RIGHT)
			{
			    if(newkeys == GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP))
			    {
			        TogglePlayerSpectating(playerid, false);
			    }
			}
		}
	}
	
	#if defined SPEC_OnPlayerKeyStateChange
       	return SPEC_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerKeyStateChange
    #undef OnPlayerKeyStateChange
#else
    #define _ALS_OnPlayerKeyStateChange
#endif
#define OnPlayerKeyStateChange SPEC_OnPlayerKeyStateChange
#if defined SPEC_OnPlayerKeyStateChange
    forward SPEC_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
#endif

public OnPlayerUpdate(playerid)
{
	if(GetPlayerSpectateType(playerid) == SPEC_TYPE_PLAYER)
	{
		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) != SPEC_KEY_INVALID)
		{
			if(GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) == KEY_UP || GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) == KEY_DOWN)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(updown & GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT))
				{
					UpdatePlayerSpectate(playerid, true);
				}
			}
			else if(GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) == KEY_LEFT || GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT) == KEY_RIGHT)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(leftright & GetPlayerSpectateKeys(playerid, SPEC_KEY_NEXT))
				{
					UpdatePlayerSpectate(playerid, true);
				}
			}
		}

		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) != SPEC_KEY_INVALID)
		{
			if(GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) == KEY_UP || GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) == KEY_DOWN)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(updown & GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS))
				{
					UpdatePlayerSpectate(playerid, false);
				}
			}
			else if(GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) == KEY_LEFT || GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS) == KEY_RIGHT)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(leftright & GetPlayerSpectateKeys(playerid, SPEC_KEY_PREVIOUS))
				{
					UpdatePlayerSpectate(playerid, false);
				}
			}
		}

		if(GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) != SPEC_KEY_INVALID)
		{
			if(GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) == KEY_UP || GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) == KEY_DOWN)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(updown & GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP))
				{
					TogglePlayerSpectating(playerid, false);
				}
			}
			else if(GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) == KEY_LEFT || GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP) == KEY_RIGHT)
			{
				new keys, updown, leftright;
				GetPlayerKeys(playerid, keys, updown, leftright);

				if(leftright & GetPlayerSpectateKeys(playerid, SPEC_KEY_STOP))
				{
					TogglePlayerSpectating(playerid, false);
				}
			}
		}
	}
	
	#if ! defined OnPlayerVirtualWorldChange
	    new worldid = GetPlayerVirtualWorld(playerid);
	    if(gSpectate[playerid][SPEC_VIRTUAL_WORLD] != worldid)
	    {
	        CallLocalFunction("SPEC_OnPlayerWorldChange", "iii", playerid, worldid, gSpectate[playerid][SPEC_VIRTUAL_WORLD]);
	        gSpectate[playerid][SPEC_VIRTUAL_WORLD] = worldid;
	    }
	#endif

 	#if defined SPEC_OnPlayerUpdate
 		return SPEC_OnPlayerUpdate(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerUpdate
	#undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif
#define OnPlayerUpdate SPEC_OnPlayerUpdate
#if defined SPEC_OnPlayerUpdate
    forward SPEC_OnPlayerUpdate(playerid);
#endif

public OnPlayerSpawn(playerid)
{
	SPEC_LOOP_PLAYERS(i)
	{
        if(	GetPlayerSpectateID(i) == playerid &&
			GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
		{
			PlayerSpectatePlayer(i, playerid, GetPlayerSpectateMode(i));

            CallLocalFunction("OnPlayerSpecUpdate", "iiiii", i, playerid, SPEC_UPDATE_SPAWN, -1, -1);
        }
    }

 	#if defined SPEC_OnPlayerSpawn
 		return SPEC_OnPlayerSpawn(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerSpawn
	#undef OnPlayerSpawn
#else
    #define _ALS_OnPlayerSpawn
#endif
#define OnPlayerSpawn SPEC_OnPlayerSpawn
#if defined SPEC_OnPlayerSpawn
    forward SPEC_OnPlayerSpawn(playerid);
#endif

#if ! defined OnPlayerVirtualWorldChange
	forward SPEC_OnPlayerWorldChange(playerid, neworld, oldworld);
	public SPEC_OnPlayerWorldChange(playerid, neworld, oldworld)
	{
	    SPEC_LOOP_PLAYERS(i)
	 	{
	        if(	GetPlayerSpectateID(i) == playerid &&
				GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
	        {
	            SetPlayerVirtualWorld(i, neworld);

	            CallLocalFunction("OnPlayerSpecUpdate", "iiii", i, playerid, SPEC_UPDATE_WORLDID, neworld, oldworld);
	        }
	    }
	    return 1;
	}
#else
    public OnPlayerVirtualWorldChange(playeird, neworld, oldworld)
	{
	    new worldid = GetPlayerSpectateID(playerid);
	    SPEC_LOOP_PLAYERS(i)
	    {
	        if(	GetPlayerSpectateID(i) == playerid &&
				GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
	        {
	            SetPlayerVirtualWorld(i, worldid);

	            CallLocalFunction("OnPlayerSpecUpdate", "iiiii", i, playerid, SPEC_UPDATE_WORLDID, neworld, oldworld);
	        }
	    }

	    #if defined SPEC_OnPlayerVirtualWorldChange
	       	return SPEC_OnPlayerVirtualWorldChange(playeird, neworld, oldworld);
		#else
		   	return 1;
		#endif
	}
	#if defined _ALS_OnPlayerVirtualWorldChange
	    #undef OnPlayerVirtualWorldChange
	#else
	    #define _ALS_OnPlayerVirtualWorldChange
	#endif
	#define OnPlayerVirtualWorldChange SPEC_OnPlayerVirtualWorldChange
	#if defined SPEC_OnPlayerVirtualWorldChange
	    forward SPEC_OnPlayerVirtualWorldChange(playeird, neworld, oldworld);
	#endif
#endif

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	SPEC_LOOP_PLAYERS(i)
	{
        if(	GetPlayerSpectateID(i) == playerid &&
			GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
		{
			SetPlayerVirtualWorld(i, newinteriorid);

            CallLocalFunction("OnPlayerSpecUpdate", "iiiii", i, playerid, SPEC_UPDATE_INTERIORID, newinteriorid, oldinteriorid);
        }
    }

	#if defined SPEC_OnPlayerInteriorChange
	  	return SPEC_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerInteriorChange
    #undef OnPlayerInteriorChange
#else
    #define _ALS_OnPlayerInteriorChange
#endif
#define OnPlayerInteriorChange SPEC_OnPlayerInteriorChange
#if defined SPEC_OnPlayerInteriorChange
    forward SPEC_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
#endif

public OnPlayerDeath(playerid, killerid, reason)
{
	new Float:cam[3];
	GetPlayerCameraPos(playerid, cam[0], cam[1], cam[2]);

	new Float:fpos[3];
	GetPlayerPos(playerid, fpos[0], fpos[1], fpos[2]);

	SPEC_LOOP_PLAYERS(i)
	{
        if(	GetPlayerSpectateID(i) == playerid &&
			GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
		{
			if(! GetPlayerInterior(i))
			{
				cam[2] += 5.0;
		  	}
			SetPlayerCameraPos(i, cam[0], cam[1], cam[2]);
			SetPlayerCameraLookAt(i, fpos[0], fpos[1], fpos[2]);

            CallLocalFunction("OnPlayerSpecUpdate", "iiiii", i, playerid, SPEC_UPDATE_DEATH, killerid, reason);
        }
    }

	#if defined SPEC_OnPlayerDeath
	  	return SPEC_OnPlayerDeath(playerid, killerid, reason);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerDeath
    #undef OnPlayerDeath
#else
    #define _ALS_OnPlayerDeath
#endif
#define OnPlayerDeath SPEC_OnPlayerDeath
#if defined SPEC_OnPlayerDeath
    forward SPEC_OnPlayerDeath(playerid, killerid, reason);
#endif

public OnPlayerDisconnect(playerid, reason)
{
	new Float:cam[3];
	GetPlayerCameraPos(playerid, cam[0], cam[1], cam[2]);

	new Float:fpos[3];
	GetPlayerPos(playerid, fpos[0], fpos[1], fpos[2]);

	SPEC_LOOP_PLAYERS(i)
	{
        if(	GetPlayerSpectateID(i) == playerid &&
			GetPlayerSpectateMode(i) == SPEC_TYPE_PLAYER)
		{
			if(! GetPlayerInterior(i))
			{
				cam[2] += 5.0;
		  	}
			SetPlayerCameraPos(i, cam[0], cam[1], cam[2]);
			SetPlayerCameraLookAt(i, fpos[0], fpos[1], fpos[2]);

            CallLocalFunction("OnPlayerSpecUpdate", "iiiii", i, playerid, SPEC_UPDATE_DISCONNECT, reason, -1);
        }
    }

	#if defined SPEC_OnPlayerDisconnect
	  	return SPEC_OnPlayerDisconnect(playerid, reason);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect SPEC_OnPlayerDisconnect
#if defined SPEC_OnPlayerDisconnect
    forward SPEC_OnPlayerDisconnect(playerid, reason);
#endif

forward OnPlayerSpecUpdate(playerid, targetplayerid, updatetype, newvalue, oldvalue);

#undef SPEC_LOOP_PLAYERS
#undef SPEC_LOOP_PLAYERS_NEXT
#undef SPEC_LOOP_PLAYERS_PREV
