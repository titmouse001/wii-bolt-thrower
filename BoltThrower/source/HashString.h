#ifndef HashString_H
#define HashString_H

#include "HashLabel.h"

class HashString
{
public:
	const static HashLabel BLANK;

	// Main menu buttons
	const static HashLabel Credits;
	const static HashLabel Start_Game;
	const static HashLabel Quit;
	const static HashLabel Intro;
	const static HashLabel Controls;
	const static HashLabel Options;
	const static HashLabel Change_Tune;
	const static HashLabel download_extra_music;
	
	// Option menu buttons
	const static HashLabel IngameMusicState;
	const static HashLabel DifficultySetting;
	const static HashLabel IngameMusicVolumeState;
	const static HashLabel LanguageSetting;

	// fonts names
	const static HashLabel LargeFont;
	const static HashLabel SmallFont;

	// sound names
	const static HashLabel FireMissle;
	const static HashLabel AfterBurn;
	const static HashLabel DropMine;
	const static HashLabel Explode1;
	const static HashLabel Explode2;
	const static HashLabel Explode3;
	const static HashLabel ExplodeBig1;
	const static HashLabel ExplodeBig2;
	const static HashLabel Hull_bang1;
	const static HashLabel Hull_bang2;
	const static HashLabel Hull_bang3;

	const static HashLabel Hull1;
	const static HashLabel Hull2;
	const static HashLabel Hull3;

	const static HashLabel HullCreak1;
	const static HashLabel HullCreak2;

	const static HashLabel PickUphealth;
	const static HashLabel Metal_pickup;
	

	const static HashLabel GunFire1;
	const static HashLabel GunFire2;
	const static HashLabel GunFire3;
	const static HashLabel GunFire4;


	const static HashLabel ExplodeWithSparks;
	const static HashLabel HitShield;

	const static HashLabel Alarm2;
	
	// graphics names
	const static HashLabel PlayersShip32x32;
	const static HashLabel AimingPointer32x32;

	
	const static HashLabel HealthPickUp32x32;


	
	const static HashLabel MediumEnemyShipType01_32x32;
	const static HashLabel MediumEnemyShipType02_32x32;
	const static HashLabel MediumEnemyShipType03_32x32;
	const static HashLabel MediumEnemyShipType04_32x32;

	
	const static HashLabel ShieldIcon32x32;
	const static HashLabel SmallMissile16x16;
	const static HashLabel SmallWhiteEnemyShip16x16x2;
	const static HashLabel SmallRedEnemyShip16x16x2;
	const static HashLabel ExplosionFire1Type16x16x9;
	const static HashLabel ExplosionFire2Type16x16x9;
	const static HashLabel SpinningSpore16x16x9;
	const static HashLabel ProbeMine16x16x2;

	const static HashLabel ProbeMineUp16x16;
	const static HashLabel ProbeMineRight16x16;
	const static HashLabel ProbeMineDown16x16;
	const static HashLabel ProbeMineLeft16x16;

	const static HashLabel ProbeMineUpThrust16x16x5;
	const static HashLabel ProbeMineRightThrust16x16x5;
	const static HashLabel ProbeMineDownThrust16x16x5;
	const static HashLabel ProbeMineLeftThrust16x16x5;
	const static HashLabel ExplosionThrust1Type16x16x10;
	const static HashLabel ExplosionSolidType32x32x10;
	const static HashLabel ExplosionRingType32x32x7;
	const static HashLabel ExplosionStarType32x32x4;
	
	const static HashLabel ShieldRed;
	const static HashLabel ShieldBlue;
	const static HashLabel ExplosionDull1Type16x16x10;
	const static HashLabel ExplosionSmoke1Type16x16x10;
	const static HashLabel ExplosionSmoke2Type16x16x10;

	const static HashLabel SmokeTrail16x16x10;

	const static HashLabel Shot;

	const static HashLabel MiniMoon16x16;
	const static HashLabel YellowRadarPing32x32;
	const static HashLabel SmallGunTurret;

	const static HashLabel Material_PickUp;

	const static HashLabel RedEdgeExplosion64x64;
	const static HashLabel YellowEdgeExplosion64x64;

	const static HashLabel FireBallExplosion64x64;
	const static HashLabel HotYellowOrangeExplosion80x80;


	const static HashLabel  AmountBadSatellites;
	const static HashLabel  EnemySatellite80x64x2;

	const static HashLabel  GreenLockOn;

	const static HashLabel  SmallShot16x16;
	


	// Variables
	const static HashLabel WiiMoteIdleTimeoutInSeconds;
	//const static HashLabel WaitTimeForGameCompletedMessage;
	const static HashLabel AsteroidTotal;
	const static HashLabel AmountBadShipsFromSpore;
	const static HashLabel AmountBadShips;
	const static HashLabel AmountBadSpores;
	const static HashLabel PlayerMaxShieldLevel;
	const static HashLabel AmountSporesNeededForShiled;
	const static HashLabel ReducePlayerShieldLevelByAmountForShipToShipCollision;
	const static HashLabel BadShipType1MaxShieldLevel;
	const static HashLabel BadShipType2MaxShieldLevel;
	const static HashLabel ViewRadiusForSprites;
	const static HashLabel ViewRadiusForIntroSprites;
	const static HashLabel PlayerCollisionRadius;
//	const static HashLabel PlayerShipCollisionFactor;
//	const static HashLabel FoeShipCollisionFactor;
	const static HashLabel RocketCollisionRadius;
	const static HashLabel PlayerStartingPointX;
	const static HashLabel PlayerStartingPointY;
	const static HashLabel PlayerFacingDirection;
	const static HashLabel PlayerStartingVelocityX;
	const static HashLabel PlayerStartingVelocityY;

	const static HashLabel SpinningSpaceDebre01;
	const static HashLabel SpinningSpaceDebre02;

	//WiiMote
	const static HashLabel WiiMoteButtonA;
	const static HashLabel WiiMoteButtonB;
	const static HashLabel WiiMoteDirectionDownMarkedRed;
	const static HashLabel WiiMoteDirectionUpMarkedRed;
	const static HashLabel WiiMoteInfraRedPointer;
	const static HashLabel WiiMoteButtonHome;


	const static HashLabel TurretForGunShip;
	const static HashLabel BrokenTurretForGunShip;
	const static HashLabel GunShip;


	const static HashLabel TurretNo1ForGunShipOriginX;
	const static HashLabel TurretNo2ForGunShipOriginX;
	const static HashLabel TurretForGunShipOriginY;
	const static HashLabel AmountOfGunShipsAtStartUp;

	const static HashLabel GunShipProjectileFrames;

	const static HashLabel NebulaGass01;


	const static HashLabel Earth_lowres;
	const static HashLabel Earth_medres;

	const static HashLabel Rock1;
	const static HashLabel Rock2;
	const static HashLabel MoonLowRes;
	const static HashLabel MoonHiRes;
	const static HashLabel Viper;
	const static HashLabel WiiMote;
	const static HashLabel Satellite;

	const static HashLabel MoonShield;
	const static HashLabel ShieldGenerator;
	const static HashLabel Skull;

	const static HashLabel BarRightSmall;

	const static HashLabel RadarCircle;

	const static HashLabel SpaceBackground01;
	const static HashLabel TinyLogoForMineIntroLayout;

	const static HashLabel AsteroidWidthCoverage;
	const static HashLabel AsteroidHeightCoverage;

	const static HashLabel  ViewRadiusForAsteroids;
	
	const static HashLabel  EnermyAmardaArroundLastShieldGenerator;

	const static HashLabel TurretTarget_SmallShip;
	const static HashLabel TurretTarget_GunShip;

	const static HashLabel OptionsPopUpMessage;
	const static HashLabel Start_GamePopUpMessage;
	const static HashLabel IntroPopUpMessage;

	const static HashLabel Version;
};



#endif
