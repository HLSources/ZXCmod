/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef GAME_H
#define GAME_H

extern void GameDLLInit( void );


extern cvar_t	displaysoundlist;

// multiplayer server rules
extern cvar_t	teamplay;
extern cvar_t	fraglimit;
extern cvar_t	timelimit;
extern cvar_t	friendlyfire;
extern cvar_t	falldamage;
//extern cvar_t	weaponstay;
extern cvar_t	forcerespawn;
extern cvar_t	flashlight;
extern cvar_t	aimcrosshair;
extern cvar_t	decalfrequency;
extern cvar_t	teamlist;
extern cvar_t	teamoverride;
extern cvar_t	defaultteam;
extern cvar_t	allowmonsters;
extern cvar_t	allowmonsters2; //1.27
extern cvar_t	allowmonsters3; //1.27
extern cvar_t	allowmonsters4;
extern cvar_t	allowmonsters5;
extern cvar_t	allowmonsters6;
extern cvar_t	allowmonsters7;
extern cvar_t	allowmonsters8;
extern cvar_t	allowmonsters9;
extern cvar_t	allowmonsters10;
extern cvar_t	allowmonsters11; // SF radius
extern cvar_t	allowmonsters12; // 
extern cvar_t	allowmonsters13; //
extern cvar_t	allowmonsters14; //
extern cvar_t	allowmonsters15; // Triple rocket mod
extern cvar_t	allowmonsters16; // 
extern cvar_t	allowmonsters17; // 
extern cvar_t	allowmonsters18; // 
extern cvar_t	allowmonsters19; // Prevent another models for player
extern cvar_t	allowmonsters20; //
extern cvar_t	g_flWeaponCheat; //
extern cvar_t	g_bestfrags; //

// Engine Cvars
extern cvar_t	*g_psv_gravity;
extern cvar_t	*g_psv_aim;
extern cvar_t	*g_footsteps;

#endif		// GAME_H
