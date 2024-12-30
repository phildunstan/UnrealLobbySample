// Copyright Philip Dunstan


#include "LSLobbyGameMode.h"

ALSLobbyGameMode::ALSLobbyGameMode()
{
	bUseSeamlessTravel = true;
}

void ALSLobbyGameMode::StartToLeaveMap()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSLobbyGameMode::StartToLeaveMap %hs"), (GetWorld()->IsInSeamlessTravel() ? "seamless" : "not seamless"));
	Super::StartToLeaveMap();
}

void ALSLobbyGameMode::PostSeamlessTravel()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSLobbyGameMode::PostSeamlessTravel"));
	Super::PostSeamlessTravel();
}

APlayerController* ALSLobbyGameMode::ProcessClientTravel(FString& URL, bool bSeamless, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSLobbyGameMode::ProcessClientTravel %hs %hs"), (bSeamless ? "seamless" : "not seamless"), (bAbsolute ? "absolute" : "relative"));
	return Super::ProcessClientTravel(URL, bSeamless, bAbsolute);
}

void ALSLobbyGameMode::ProcessServerTravel(const FString& URL, bool bAbsolute)
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSLobbyGameMode::ProcessServerTravel %hs"), (bAbsolute ? "absolute" : "relative"));
	Super::ProcessServerTravel(URL, bAbsolute);
}
