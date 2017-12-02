// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CatAirBalloonGameMode.h"
#include "CatAirBalloonPawn.h"

ACatAirBalloonGameMode::ACatAirBalloonGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ACatAirBalloonPawn::StaticClass();
}
