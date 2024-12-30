// Copyright Philip Dunstan


#include "LSGameGameMode.h"

ALSGameGameMode::ALSGameGameMode()
{
	bUseSeamlessTravel = true;
}

void ALSGameGameMode::StartToLeaveMap()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSGameGameMode::StartToLeaveMap %hs"), (GetWorld()->IsInSeamlessTravel() ? "seamless" : "not seamless"));
	Super::StartToLeaveMap();
}

void ALSGameGameMode::PostSeamlessTravel()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSGameGameMode::PostSeamlessTravel"));
	Super::PostSeamlessTravel();
}

APlayerController* ALSGameGameMode::ProcessClientTravel(FString& URL, bool bSeamless, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSGameGameMode::ProcessClientTravel %hs %hs"), (bSeamless ? "seamless" : "not seamless"), (bAbsolute ? "absolute" : "relative"));
	return Super::ProcessClientTravel(URL, bSeamless, bAbsolute);
}

void ALSGameGameMode::ProcessServerTravel(const FString& URL, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSGameGameMode::ProcessServerTravel %hs"), (bAbsolute ? "absolute" : "relative"));
	Super::ProcessServerTravel(URL, bAbsolute);
}
