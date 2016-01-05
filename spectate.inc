#include <a_samp>

#if defined _spectate_included
	#endinput
#endif
#define _spectate_included

#include <SAMP_Fixer/fix_vehicles>
#include <foreach>

/*
* [Include] Spectate enhancer (spectate.inc)
* An advance spectate system include with camera freezer, crash free, bug free, auto redirecting, auto adjustments and supports callbacks.
*
* By Gammix - (c) Copyright 2015
* This file is provided as is (no warranties).
*/

/*
FUNCTIONS:
native GetPlayerSpectateData(playerid, &spectatetype, &targetid, &mode);
native UpdatePlayerSpectate(playerid, bool:advance = true);
*/

#if ! defined Vehicle
	new Iterator:Vehicle<MAX_VEHICLES>;

	stock Spec_CreateVehicle(modelid, Float:spawn_x, Float:spawn_y, Float:spawn_z, Float:z_angle, color1, color2, respawn_delay)
	{
 		new ret = CreateVehicle(modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2, respawn_delay);
	    Iter_Add(Vehicle, ret);
     	return ret;
	}
	#if defined _ALS_CreateVehicle
 		#undef CreateVehicle
	#else
 		#define _ALS_CreateVehicle
	#endif
	#define CreateVehicle Spec_CreateVehicle

	stock Spec_AddStaticVehicle(modelid, Float:spawn_x, Float:spawn_y, Float:spawn_z, Float:z_angle, color1, color2)
	{
 		new ret = AddStaticVehicle(modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2);
	    Iter_Add(Vehicle, ret);
     	return ret;
	}
	#if defined _ALS_AddStaticVehicle
 		#undef AddStaticVehicle
	#else
	    #define _ALS_AddStaticVehicle
	#endif
	#define AddStaticVehicle Spec_AddStaticVehicle

	stock Spec_AddStaticVehicleEx(modelid, Float:spawn_x, Float:spawn_y, Float:spawn_z, Float:z_angle, color1, color2, respawn_delay)
	{
 		new ret = AddStaticVehicleEx(modelid, spawn_x, spawn_y, spawn_z, z_angle, color1, color2, respawn_delay);
	    Iter_Add(Vehicle, ret);
     	return ret;
	}
	#if defined _ALS_AddStaticVehicleEx
 		#undef AddStaticVehicleEx
	#else
	    #define _ALS_AddStaticVehicleEx
	#endif
	#define AddStaticVehicleEx Spec_AddStaticVehicleEx

	stock Spec_DestroyVehicle(vehicleid)
	{
 		Iter_Remove(Vehicle, vehicleid);
   		return DestroyVehicle(vehicleid);
	}
	#if defined _ALS_DestroyVehicle
 		#undef DestroyVehicle
	#else
 		#define _ALS_DestroyVehicle
	#endif
	#define DestroyVehicle Spec_DestroyVehicle
#endif

#define SPECTATE_TYPE_NONE (0)
#define SPECTATE_TYPE_PLAYER (1)
#define SPECTATE_TYPE_VEHICLE (2)
#define SPECTATE_TYPE_FREE (3)

enum e_SPECTATE_DATA
{
	Spec_Id,
	Spec_Type,
	Spec_Mode
};
static pSpecData[MAX_PLAYERS][e_SPECTATE_DATA];
static pSpecWorldid[MAX_PLAYERS];

stock Spec_PlayerSpectatePlayer(playerid, targetplayerid, mode = SPECTATE_MODE_NORMAL)
{
	if (PlayerSpectatePlayer(playerid, targetplayerid, mode))
	{
	    pSpecData[playerid][Spec_Id] = targetplayerid;
	    pSpecData[playerid][Spec_Type] = SPECTATE_TYPE_PLAYER;
	    pSpecData[playerid][Spec_Mode] = mode;
	  
		SetPlayerInterior(playerid, GetPlayerInterior(targetplayerid));
		SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetplayerid));

		if (IsPlayerInAnyVehicle(targetplayerid))
		{
  			PlayerSpectateVehicle(playerid, GetPlayerVehicleID(targetplayerid), mode);
		}
		
		return true;
	}
	
	return false;
}
#if defined _ALS_PlayerSpectatePlayer
    #undef PlayerSpectatePlayer
#else
    #define _ALS_PlayerSpectatePlayer
#endif
#define PlayerSpectatePlayer Spec_PlayerSpectatePlayer

stock Spec_PlayerSpectateVehicle(playerid, targetvehicleid, mode = SPECTATE_MODE_NORMAL)
{
	if(PlayerSpectateVehicle(playerid, targetvehicleid, mode))
	{
	    pSpecData[playerid][Spec_Id] = targetvehicleid;
	    pSpecData[playerid][Spec_Type] = SPECTATE_TYPE_VEHICLE;
	    pSpecData[playerid][Spec_Mode] = mode;

		SetPlayerInterior(playerid, GetVehicleInterior(targetvehicleid));
		SetPlayerVirtualWorld(playerid, GetVehicleVirtualWorld(targetvehicleid));
		
		return true;
	}
	
	return false;
}
#if defined _ALS_PlayerSpectateVehicle
    #undef PlayerSpectateVehicle
#else
    #define _ALS_PlayerSpectateVehicle
#endif
#define PlayerSpectateVehicle Spec_PlayerSpectateVehicle

stock Spec_TogglePlayerSpectating(playerid, bool:toggle)
{
	if (TogglePlayerSpectating(playerid, toggle))
	{
	    pSpecData[playerid][Spec_Id] = INVALID_PLAYER_ID;
	    pSpecData[playerid][Spec_Type] = SPECTATE_TYPE_FREE;
	    pSpecData[playerid][Spec_Mode] = 0;
	    
		return true;
	}
	
	return false;
}
#if defined _ALS_TogglePlayerSpectating
    #undef TogglePlayerSpectating
#else
    #define _ALS_TogglePlayerSpectating
#endif
#define TogglePlayerSpectating Spec_TogglePlayerSpectating

stock GetPlayerSpectateData(playerid, &spectatetype, &targetid, &mode)
{
	if (playerid < 0 || player > MAX_PLAYERS)
	{
	    return false;
	}

    spectatetype = pSpecData[playerid][Spec_Type];
    targetid = pSpecData[playerid][Spec_Id];
    mode = pSpecData[playerid][Spec_Mode];
    
    return true;
}

stock UpdatePlayerSpectate(playerid, bool:advance = true)
{
	switch (pSpecData[playerid][Spec_Type])
	{
	    case SPECTATE_TYPE_PLAYER:
	    {
			if (advance)
			{
   			Spec_NextPlayer:

				new targetid = INVALID_PLAYER_ID;
    			targetid = Iter_Next(Player, targetid);
    			
    			if (targetid == INVALID_PLAYER_ID)
    			{
    			    return INVALID_PLAYER_ID;
    			}
                else if (GetPlayerState(targetid) == PLAYER_STATE_SPECTATING || GetPlayerState(targetid) == PLAYER_STATE_WASTED)
                {
                	goto Spec_NextPlayer;
                }
                else
                {
                	PlayerSpectatePlayer(playerid, targetid, pSpecData[playerid][Spec_Mode]);
       			}
       			
       			return targetid;
			}
			else
			{
			Spec_PreviousPlayer:

				new targetid = INVALID_PLAYER_ID;
                targetid = Iter_Prev(Player, targetid);
                
    			if (targetid == INVALID_PLAYER_ID)
    			{
    			    return INVALID_PLAYER_ID;
    			}
                else if (GetPlayerState(targetid) == PLAYER_STATE_SPECTATING || GetPlayerState(targetid) == PLAYER_STATE_WASTED)
                {
                	goto Spec_PreviousPlayer;
                }
                else
                {
                	PlayerSpectatePlayer(playerid, targetid, pSpecData[playerid][Spec_Mode]);
			    }

       			return targetid;
			}
		}
		case SPECTATE_TYPE_VEHICLE:
	    {
			if (advance)
			{
   			Spec_NextVehicle:

				new targetid = INVALID_VEHICLE_ID;
    			targetid = Iter_Next(Vehicle, targetid);

    			if (targetid == INVALID_VEHICLE_ID)
    			{
    			    return INVALID_VEHICLE_ID;
    			}
                else if (IsValidVehicle(targetid))
                {
                	goto Spec_NextVehicle;
                }
                else
                {
                	PlayerSpectateVehicle(playerid, targetid, pSpecData[playerid][Spec_Mode]);
       			}

       			return targetid;
			}
			else
			{
			Spec_PreviousVehicle:

				new targetid = INVALID_VEHICLE_ID;
                targetid = Iter_Prev(Vehicle, targetid);

    			if (targetid == INVALID_VEHICLE_ID)
    			{
    			    return INVALID_VEHICLE_ID;
    			}
                else if (IsValidVehicle(targetid))
                {
                	goto Spec_PreviousVehicle;
                }
                else
                {
                	PlayerSpectateVehicle(playerid, targetid, pSpecData[playerid][Spec_Mode]);
			    }

       			return targetid;
			}
		}
	}
	
	return -1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    new Float:pos[3];
	GetVehiclePos(vehicleid, pos[0], pos[1], pos[2]);
 	pos[2] += 5.0;

	foreach (new i : Player)
	{
		if (pSpecData[i][Spec_Type] == SPECTATE_TYPE_VEHICLE && pSpecData[i][Spec_Id] == vehicleid)
		{
			SetPlayerCameraPos(i, pos[0], pos[1], pos[2]);
			SetPlayerCameraLookAt(i, pos[0], pos[1], pos[2]);
		}
 	}

	#if defined Spec_OnVehicleDeath
       	return Spec_OnVehicleDeath(vehicleid, killerid);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnVehicleDeath
    #undef OnVehicleDeath
#else
    #define _ALS_OnVehicleDeath
#endif
#define OnVehicleDeath Spec_OnVehicleDeath
#if defined Spec_OnVehicleDeath
    forward Spec_OnVehicleDeath(vehicleid, killerid);
#endif

public OnVehicleSpawn(vehicleid)
{
	foreach (new i : Player)
	{
		if (pSpecData[i][Spec_Type] == SPECTATE_TYPE_VEHICLE && pSpecData[i][Spec_Id] == vehicleid)
		{
			PlayerSpectateVehicle(i, vehicleid, pSpecData[i][Spec_Mode]);
		}
 	}

	#if defined Spec_OnVehicleSpawn
       	return Spec_OnVehicleSpawn(vehicleid);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnVehicleSpawn
    #undef OnVehicleSpawn
#else
    #define _ALS_OnVehicleSpawn
#endif
#define OnVehicleSpawn Spec_OnVehicleSpawn
#if defined Spec_OnVehicleSpawn
    forward Spec_OnVehicleSpawn(vehicleid);
#endif

public OnPlayerStateChange(playerid, newstate, oldstate)
{
	if (newstate == PLAYER_STATE_SPECTATING)
	{
		foreach (new i : Player)
		{
			if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
	 		{
                UpdatePlayerSpectate(i);
			}
	  	}
  	}
	else if (((oldstate != PLAYER_STATE_DRIVER || oldstate != PLAYER_STATE_PASSENGER) && (newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER)) ||
			((oldstate != PLAYER_STATE_ONFOOT || oldstate != PLAYER_STATE_SPAWNED) && (newstate == PLAYER_STATE_ONFOOT || newstate == PLAYER_STATE_SPAWNED)))
	{
		foreach (new i : Player)
		{
			if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
	 		{
                PlayerSpectatePlayer(i, playerid, pSpecData[i][Spec_Mode]);
			}
	  	}
  	}

	#if defined Spec_OnPlayerStateChange
       	return Spec_OnPlayerStateChange(playerid, newstate, oldstate);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerStateChange
    #undef OnPlayerStateChange
#else
    #define _ALS_OnPlayerStateChange
#endif
#define OnPlayerStateChange Spec_OnPlayerStateChange
#if defined Spec_OnPlayerStateChange
    forward Spec_OnPlayerStateChange(playerid, newstate, oldstate);
#endif

public OnPlayerUpdate(playerid)
{
	new worldid = GetPlayerVirtualWorld(playerid);
	if (pSpecWorldid[playerid] != worldid)
	{
 		pSpecWorldid[playerid] = worldid;
	        
	    foreach (new i : Player)
		{
			if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
 			{
	            SetPlayerVirtualWorld(i, worldid);
		  	}
	    }
	}

 	#if defined Spec_OnPlayerUpdate
 		return Spec_OnPlayerUpdate(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerUpdate
	#undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif
#define OnPlayerUpdate Spec_OnPlayerUpdate
#if defined Spec_OnPlayerUpdate
    forward Spec_OnPlayerUpdate(playerid);
#endif

public OnPlayerSpawn(playerid)
{
	foreach (new i : Player)
	{
		if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
		{
			PlayerSpectateVehicle(i, playerid, pSpecData[i][Spec_Mode]);
		}
 	}

 	#if defined Spec_OnPlayerSpawn
 		return Spec_OnPlayerSpawn(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerSpawn
	#undef OnPlayerSpawn
#else
    #define _ALS_OnPlayerSpawn
#endif
#define OnPlayerSpawn Spec_OnPlayerSpawn
#if defined Spec_OnPlayerSpawn
    forward Spec_OnPlayerSpawn(playerid);
#endif

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	foreach (new i : Player)
	{
		if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
 		{
	    	SetPlayerInterior(i, newinteriorid);
		}
  	}

	#if defined Spec_OnPlayerInteriorChange
	  	return Spec_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerInteriorChange
    #undef OnPlayerInteriorChange
#else
    #define _ALS_OnPlayerInteriorChange
#endif
#define OnPlayerInteriorChange Spec_OnPlayerInteriorChange
#if defined Spec_OnPlayerInteriorChange
    forward Spec_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
#endif

public OnPlayerDeath(playerid, killerid, reason)
{
	new Float:cam[3];
	GetPlayerCameraPos(playerid, cam[0], cam[1], cam[2]);

	new Float:fpos[3];
	GetPlayerPos(playerid, fpos[0], fpos[1], fpos[2]);

	if (! GetPlayerInterior(playerid))
	{
		cam[2] += 5.0;
 	}
 	else
 	{
 	    cam[2] += 0.5;
 	}
 	
	foreach (new i : Player)
	{
		if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
		{
			SetPlayerCameraPos(i, cam[0], cam[1], cam[2]);
			SetPlayerCameraLookAt(i, fpos[0], fpos[1], fpos[2]);
		}
 	}

	#if defined Spec_OnPlayerDeath
	  	return Spec_OnPlayerDeath(playerid, killerid, reason);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerDeath
    #undef OnPlayerDeath
#else
    #define _ALS_OnPlayerDeath
#endif
#define OnPlayerDeath Spec_OnPlayerDeath
#if defined Spec_OnPlayerDeath
    forward Spec_OnPlayerDeath(playerid, killerid, reason);
#endif

public OnPlayerDisconnect(playerid, reason)
{
	foreach (new i : Player)
	{
		if (i != playerid && pSpecData[i][Spec_Type] == SPECTATE_TYPE_PLAYER && pSpecData[i][Spec_Id] == playerid)
		{
			UpdatePlayerSpectate(i);
		}
 	}

	#if defined Spec_OnPlayerDisconnect
	  	return Spec_OnPlayerDisconnect(playerid, reason);
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect Spec_OnPlayerDisconnect
#if defined Spec_OnPlayerDisconnect
    forward Spec_OnPlayerDisconnect(playerid, reason);
#endif
