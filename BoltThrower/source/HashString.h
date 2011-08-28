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
	
	// Option menu buttons
	const static HashLabel IngameMusicState;
	const static HashLabel DifficultySetting;
	const static HashLabel LanguageSetting;

	// fonts names
	const static HashLabel LargeFont;
	const static HashLabel SmallFont;

	// sound names
	const static HashLabel FireMissle;
	const static HashLabel AfterBurn;
	const static HashLabel DropMine;
	const static HashLabel Explode01;
	const static HashLabel Explode11;
	const static HashLabel hitmetal;

	// graphics names
	const static HashLabel ShipFrames;
	const static HashLabel AimingPointer;
	const static HashLabel MissileFrames;
	const static HashLabel Bad1Frames;
	const static HashLabel Bad2Frames;
	const static HashLabel BoomFrames;
	const static HashLabel Boom2Frames;
	const static HashLabel ThingFrames;
	const static HashLabel StarFrames;
	const static HashLabel ProbeMineFrames;
	const static HashLabel ProbeMineUpThrusterFrames;
	const static HashLabel ProbeMineRightThrusterFrames;
	const static HashLabel ProbeMineDownThrusterFrames;
	const static HashLabel ProbeMineLeftThrusterFrames;
	const static HashLabel SimpleMineFrames;
	const static HashLabel Boom3Frames;
	const static HashLabel Explosion64x64;
	const static HashLabel ShieldRed;
	const static HashLabel ShieldBlue;
	const static HashLabel Boom4Frames;
	const static HashLabel Boom5Frames;
	const static HashLabel Boom6Frames;

	const static HashLabel Shot;

	const static HashLabel MiniMoon;
	const static HashLabel SmallTurrent;
	const static HashLabel YellowCircleWithHole;
	const static HashLabel SmallGunTurret;

	const static HashLabel Material_PickUp;

	const static HashLabel LargeRedExplosion;
	const static HashLabel LargeYellowExplosion;

	// Variables
	const static HashLabel WiiMoteIdleTimeoutInSeconds;
	//const static HashLabel WaitTimeForGameCompletedMessage;
	const static HashLabel AsteroidTotal;
	const static HashLabel AmountBadShipsFromSpore;
	const static HashLabel AmountBadShips;
	const static HashLabel AmountBadSpores;
	const static HashLabel PlayerMaxShieldLevel;
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
	const static HashLabel WiiMoteInfraRedPointer;
	const static HashLabel WiiMoteButtonHome;


	const static HashLabel TurretForGunShip;
	const static HashLabel BrokenTurretForGunShip;
	const static HashLabel GunShip;
	const static HashLabel SmokeTrailFrames;


	const static HashLabel TurretNo1ForGunShipOriginX;
	const static HashLabel TurretNo2ForGunShipOriginX;
	const static HashLabel TurretForGunShipOriginY;
	const static HashLabel AmountOfGunShipsAtStartUp;

	const static HashLabel GunShipProjectileFrames;


	const static HashLabel LargeExplosion;
	const static HashLabel VeryLargeExplosion;

	const static HashLabel NebulaGass01;

	const static HashLabel Rock1;
	const static HashLabel Rock2;
	const static HashLabel MoonLowRess;
	const static HashLabel MoonHiRess;
	const static HashLabel Viper;
	const static HashLabel WiiMote;
	const static HashLabel Satellite;

	const static HashLabel MoonShield;
	const static HashLabel ShieldGenerator;
	const static HashLabel Skull;



	const static HashLabel RadarCircle;

	const static HashLabel SpaceBackground01;
	
	const static HashLabel TinyLogo;

	const static HashLabel TinyLogoForMineIntroLayout;

	const static HashLabel AsteroidWidthCoverage;
	const static HashLabel AsteroidHeightCoverage;

	const static HashLabel  ViewRadiusForAsteroids;
	
	const static HashLabel  EnermyAmardaArroundLastShieldGenerator;


};



#endif
