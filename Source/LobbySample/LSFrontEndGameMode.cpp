// Copyright Philip Dunstan


#include "LSFrontEndGameMode.h"

ALSFrontEndGameMode::ALSFrontEndGameMode()
{
	// we can't seamless travel from the front end to the lobby
	bUseSeamlessTravel = false;
}

void ALSFrontEndGameMode::StartToLeaveMap()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSFrontEndGameMode::StartToLeaveMap %hs"), (GetWorld()->IsInSeamlessTravel() ? "seamless" : "not seamless"));
	Super::StartToLeaveMap();
}

void ALSFrontEndGameMode::PostSeamlessTravel()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSFrontEndGameMode::PostSeamlessTravel"));
	Super::PostSeamlessTravel();
}

APlayerController* ALSFrontEndGameMode::ProcessClientTravel(FString& URL, bool bSeamless, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSFrontEndGameMode::ProcessClientTravel %hs %hs"), (bSeamless ? "seamless" : "not seamless"), (bAbsolute ? "absolute" : "relative"));
	return Super::ProcessClientTravel(URL, bSeamless, bAbsolute);
}

void ALSFrontEndGameMode::ProcessServerTravel(const FString& URL, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSFrontEndGameMode::ProcessServerTravel %hs"), (bAbsolute ? "absolute" : "relative"));
	Super::ProcessServerTravel(URL, bAbsolute);
}
