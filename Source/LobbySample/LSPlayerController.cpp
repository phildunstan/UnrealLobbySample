// Copyright Philip Dunstan


#include "LSPlayerController.h"

#include "LSGameInstance.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameModeBase.h"


void ALSPlayerController::StartGame_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, TEXT("Start game"));

	ensure(HasAuthority());
	
	ULSGameInstance* LSGameInstance = CastChecked<ULSGameInstance>(GetGameInstance());
	
	FPrimaryAssetId GameMapID = LSGameInstance->GameMapID;
	
	FAssetData MapAssetData;
	if (!UAssetManager::Get().GetPrimaryAssetData(GameMapID, MapAssetData))
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, TEXT("Unable to resolve game map name."));
		return;
	}
	const FString MapName = MapAssetData.PackageName.ToString();
	const FString GameModeName = LSGameInstance->GameMapGameModeClass->GetClassPathName().ToString();
	// const FString GameModeName = LSGameInstance->GameMapGameModeClass->GetClassPathName().GetPackageName().ToString();
	const FString URL = FString::Printf(TEXT("%s?Game=%s"), *MapName, *GameModeName);
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, *FString::Printf(TEXT("Starting server travel to %s"), *URL));
	UE_LOG(LogTemp, Verbose, TEXT("Starting server travel to %s"), *URL);
	
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	ensure(GameMode);
	GetWorld()->ServerTravel(URL);
}

void ALSPlayerController::PostSeamlessTravel()
{
	UE_LOG(LogTemp, Verbose, TEXT("ALSPlayerController::PostSeamlessTravel"));
	Super::PostSeamlessTravel();
}
