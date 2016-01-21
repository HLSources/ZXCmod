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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

LINK_ENTITY_TO_CLASS( weapon_glock, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock );


void CGlock::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CGlock::Precache( void )
{
	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");
	PRECACHE_MODEL("sprites/blflare.spr");
	PRECACHE_MODEL("sprites/blueflare2.spr");
	
	

	PRECACHE_SOUND ("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun3.wav");//handgun
	//1.27
	PRECACHE_SOUND ("zxc/GaussGun.wav");
	PRECACHE_SOUND ("zxc/Build1.wav");
	PRECACHE_SOUND ("zxc/Build2.wav");
	PRECACHE_SOUND ("zxc/Build3.wav");
	PRECACHE_SOUND ("zxc/Build4.wav");
	PRECACHE_SOUND ("zxc/LsrExpl2.wav");
	//
	m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
	m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}



void CGlock::Reload( void )
{
	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 17, GLOCK_RELOAD, 1.5 );
	else
		iResult = DefaultReload( 20, GLOCK_RELOAD_NOT_EMPTY, 1.5 );

	if (iResult)
	{
		m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
	}
}


BOOL CGlock::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 280 );
	// pev->body = 1;
	return DefaultDeploy( "models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CGlock::PrimaryAttack( void )
{
	GlockFire( 0.0075, 0.3, FALSE );

}

void CGlock::SecondaryAttack( void )
{
	GlockFire( 0.105, 0.18, FALSE );
}

void CGlock::ThirdAttack( void )
{
	if (  m_iClip >= 4 ) //need delay
		{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zxc/GaussGun.wav", 1.0, ATTN_NORM); //play sound
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
		Vector vecSrc = m_pPlayer->pev->origin;
		Vector vecThrow = gpGlobals->v_forward * 540; //init and start speed of core
		#ifndef CLIENT_DLL
			CBaseEntity *pSatchel = Create( "weapon_minigun", m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12, m_pPlayer->pev->v_angle, m_pPlayer->edict() );
			pSatchel->pev->velocity = vecThrow;
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
			m_iClip-=4;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1;
			SendWeaponAnim( GLOCK_SHOOT );
		#endif
		return;
		}

}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_GLOBAL;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	

		vecAiming = gpGlobals->v_forward;
	

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( 0, flSpread, 0 ), 8192, RANDOM_LONG( 9, 13 ), 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CGlock::GlockFire2( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
}




void CGlock::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim( iAnim, 1 );
	}
}








class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );








///////////////////
///////////////////





//////////////NEW weapon

class   CGB : public CBaseEntity
{
        public:

        void    Spawn           ( );
		void Precache ();
        void    MoveThink       ( );
        void    Explode         ( );
		int m_flDie;
		int m_flDie2;
		int     BeamSprite;
		int m_iSpriteTexture;
		short		m_LaserSprite;
		int m_iBalls;
		
		static CGB* Create( Vector, Vector, CBaseEntity* );
		void EXPORT Hit         ( CBaseEntity* );
		
};

LINK_ENTITY_TO_CLASS( weapon_minigun, CGB );



void    CGB :: Spawn( )
{
		Precache( );
		
        SET_MODEL( ENT(pev), "sprites/blflare.spr" );
        pev->movetype = MOVETYPE_FLY;
        pev->solid = SOLID_BBOX;
        UTIL_SetSize( pev, Vector(0,0,0), Vector(0,0,0) );
		pev->angles.x = -(pev->angles.x);
        UTIL_SetOrigin( pev, pev->origin );
        pev->classname = MAKE_STRING( "weapon_9mmhandgun" );
		m_flDie = gpGlobals->time + 5;
		pev->dmg = 85; //dynamyc value
		pev->takedamage = DAMAGE_YES;
		pev->nextthink = gpGlobals->time + 0.1;
		
		pev->rendermode = kRenderTransAdd; //kRenderTransAlpha
        // pev->rendercolor.x = 100;  // red
        // pev->rendercolor.y = 100;  // green
        // pev->rendercolor.z = 255; // blue
        pev->renderamt = 195;
		
		SetThink( MoveThink );
		pev->health			= 9999; 
		
		
		
		//pev->model = MAKE_STRING("sprites/bigflare1.spr");

		
		SetTouch( Hit );
		
		
}

void CGB :: Precache( void )
{
m_iSpriteTexture = PRECACHE_MODEL( "sprites/shockwave.spr" );
m_LaserSprite = PRECACHE_MODEL( "sprites/laserbeam.spr" );
m_iBalls = PRECACHE_MODEL( "sprites/gradbeam.spr" );
}

///////////////
CGB* CGB :: Create( Vector Pos, Vector Aim, CBaseEntity* Owner )
{
        CGB* Beam = GetClassPtr( (CGB*)NULL );

        UTIL_SetOrigin( Beam->pev, Pos );
        Beam->pev->angles = Aim;
        Beam->Spawn( );
        Beam->SetTouch( CGB :: Hit );
        Beam->pev->owner = Owner->edict( );
        return Beam;
}

void    CGB :: Hit( CBaseEntity* Target )
{
        TraceResult TResult;
        Vector      StartPosition;
        pev->enemy = Target->edict( );
        StartPosition = pev->origin - pev->velocity.Normalize() * 32;

        UTIL_TraceLine( StartPosition,
                        StartPosition + pev->velocity.Normalize() * 64,
                        ignore_monsters,
                        ENT( pev ),
                        &TResult );
   
		
		
//full explode after touch with wall
		::RadiusDamage( pev->origin, pev, VARS( pev->owner ), 12, 128, CLASS_NONE, DMG_MORTAR  ); //end blast
/* 		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_BEAMCYLINDER );
			WRITE_COORD( pev->origin.x);
			WRITE_COORD( pev->origin.y);
			WRITE_COORD( pev->origin.z);
			WRITE_COORD( pev->origin.x + 350);
			WRITE_COORD( pev->origin.y + 250);
			WRITE_COORD( pev->origin.z + 150 ); // reach damage radius over .2 seconds
			WRITE_SHORT( m_iSpriteTexture );
			WRITE_BYTE( 0 ); // startframe
			WRITE_BYTE( 1 ); // framerate
			WRITE_BYTE( 12 ); // life
			WRITE_BYTE( 25 );  // width
			WRITE_BYTE( 51 );   // noise
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 128 );   // r, g, b
			WRITE_BYTE( 0 );   // r, g, b
			WRITE_BYTE( 128 ); // brightness
			WRITE_BYTE( 12 );		// speed
		MESSAGE_END(); */



// TraceResult tr, beam_tr;
		//balls
		// MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos );
			// WRITE_BYTE( TE_SPRITETRAIL );// TE_RAILTRAIL);
			// WRITE_COORD( pev->origin.x );
			// WRITE_COORD( pev->origin.y );
			// WRITE_COORD( pev->origin.z );
			// WRITE_COORD( pev->origin.x + tr.vecPlaneNormal.x*3 );
			// WRITE_COORD( pev->origin.y  + tr.vecPlaneNormal.y*3 );
			// WRITE_COORD( pev->origin.z + tr.vecPlaneNormal.z*3  );
			// WRITE_SHORT( m_iBalls );		// model
			// WRITE_BYTE( 20  );				// count
			// WRITE_BYTE( 7 );				// life * 10
			// WRITE_BYTE( RANDOM_LONG( 1, 2 ) );				// size * 10
			// WRITE_BYTE( 90 );				// amplitude * 0.1
			// WRITE_BYTE( 2 );				// speed * 100
		// MESSAGE_END();
		
		//sprite 
		pev->model = MAKE_STRING("sprites/blueflare2.spr");
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/blueflare2.spr", pev->origin, TRUE );
		if ( pSprite )
			{

				//pev->ltime = pev->ltime;
				pSprite->pev->nextthink = gpGlobals->time + 2.4;
				pSprite->pev->scale = pev->dmg/40; /// 85 / 40 = 2.125
				pSprite->SetThink( SUB_Remove );
				pSprite->SetTransparency( kRenderTransAdd, 128, 128, 128, 200, kRenderFxGlowShell );
			}
		
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "zxc/LsrExpl2.wav", 1.0, ATTN_NORM);
	pev->takedamage = DAMAGE_NO;
	SetThink( SUB_Remove );
}


void    CGB:: Explode()
{


		
		//set trails
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(entindex()); // entity
		WRITE_SHORT(g_sModelIndexSmokeTrail ); // model
		WRITE_BYTE( 30 ); // life
		WRITE_BYTE( 2 ); // width
		WRITE_BYTE( 10 ); // r, g, b
		WRITE_BYTE( 200 ); // r, g, b
		WRITE_BYTE( 10 ); // r, g, b
		WRITE_BYTE( 40 ); // brightness
		MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

		




pev->nextthink = gpGlobals->time + 0.12;
//pev->effects |= EF_LIGHT;
SetThink(MoveThink);
}


void    CGB :: MoveThink( )
{
	// Make a lightning strike
	Vector vecEnd;
	TraceResult tr;
	vecEnd.x = RANDOM_FLOAT(-360,360);	// Pick a random direction
	vecEnd.y = RANDOM_FLOAT(-360,360);
	vecEnd.z = RANDOM_FLOAT(-360,360);
	// vecEnd = vecEnd.Normalize();
	vecEnd = pev->origin + vecEnd.Normalize() * pev->dmg/2;
	UTIL_TraceLine( pev->origin, vecEnd, ignore_monsters, ENT(pev), &tr);


//sounds
	switch(RANDOM_LONG(0,8))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "zxc/Build1.wav", 0.4, ATTN_NORM); break;
	case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "zxc/Build2.wav", 0.5, ATTN_NORM); break;
	case 5: EMIT_SOUND(ENT(pev), CHAN_BODY, "zxc/Build3.wav", 0.6, ATTN_NORM); break;
	case 8: EMIT_SOUND(ENT(pev), CHAN_BODY, "zxc/Build4.wav", 0.7, ATTN_NORM); break;
	}

/* TraceResult tr, beam_tr;
		// balls
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos );
			WRITE_BYTE( TE_SPRITETRAIL );// TE_RAILTRAIL);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( m_iBalls );		// model
			WRITE_BYTE( 3  );				// count
			WRITE_BYTE( 2 );				// life * 10
			WRITE_BYTE( 1 );				// size * 10
			WRITE_BYTE( 24 );				// amplitude * 0.1
			WRITE_BYTE( 24 );				// speed * 100
		MESSAGE_END(); */
		
//lightings 1
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
                WRITE_BYTE( TE_BEAMPOINTS );
                WRITE_COORD(pev->origin.x);
                WRITE_COORD(pev->origin.y);
                WRITE_COORD(pev->origin.z);
                WRITE_COORD( tr.vecEndPos.x );
                WRITE_COORD( tr.vecEndPos.y );
                WRITE_COORD( tr.vecEndPos.z );
                WRITE_SHORT( m_LaserSprite );
                WRITE_BYTE( 0 ); // Starting frame
                WRITE_BYTE( 16  ); // framerate * 0.1
                WRITE_BYTE( 1 ); // life * 0.1
                WRITE_BYTE( 3 ); // width
                WRITE_BYTE( 128 ); // noise
                WRITE_BYTE( 64 ); // color r,g,b
                WRITE_BYTE( 128 ); // color r,g,b
                WRITE_BYTE( 255 ); // color r,g,b
                WRITE_BYTE( 140 ); // brightness
                WRITE_BYTE( 16 ); // scroll speed
        MESSAGE_END();
		
	///gravity
		
	CBaseEntity *pEntity = NULL;
	Vector	vecDir;
	vecDir = Vector( 0, 0, 0 );
	

	while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, pev->dmg )) != NULL)
       		 {
				if ((pEntity->edict() != pev->owner) && pEntity->pev->takedamage && (pEntity->edict() != edict()) && pEntity->pev->health >= 3) //!(pEntity->pev->movetype == MOVETYPE_FLY)
					{
					vecDir = ( pEntity->Center() - Vector ( 0, 0, 5 ) - Center() ).Normalize();
					pEntity->pev->velocity = vecDir * -575; //700 550
					//UTIL_ScreenShake( pEntity->pev->origin, 1024.0, 90.5, 154.7, 1 );
					//if (pev->movetype != MOVETYPE_NONE)
					//pEntity->TakeDamage(pev, VARS( pev->owner ), RANDOM_LONG(77,110), DMG_BURN); //destroy all near thinks
					//UTIL_ScreenFade( pEntity, Vector(RANDOM_LONG(128,255),RANDOM_LONG(0,64),0), 300, 30, 100, FFADE_IN );
					::RadiusDamage( pev->origin, pev, VARS( pev->owner ), pev->dmg/5, pev->dmg, CLASS_NONE, DMG_SHOCK  );
					} 
			}
	
	

Explode();
if (gpGlobals->time >= m_flDie) //full explode and self destroy
	{
	::RadiusDamage( pev->origin, pev, VARS( pev->owner ), 100, 200, CLASS_NONE, DMG_MORTAR  ); //end blast
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "zxc/LsrExpl2.wav", 1.0, ATTN_NORM);
	pev->takedamage = DAMAGE_NO;
	SetThink( SUB_Remove );
	}
}


/* 
	if (!g_pGameRules->FAllowMonsters())
	{
		pev->flags |= FL_KILLME;		// Post this because some monster code modifies class data after calling this function
//		REMOVE_ENTITY(ENT(pev));
		return;
	}
 */




