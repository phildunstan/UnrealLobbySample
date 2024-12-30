// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LSGameInstance.generated.h"

UCLASS()
class LOBBYSAMPLE_API ULSGameInstance : public UGameInstance, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowedTypes="Map"), Category="LobbySample")
	FPrimaryAssetId FrontEndMapID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowedTypes="Map"), Category="LobbySample")
	FPrimaryAssetId LobbyMapID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowedTypes="Map"), Category="LobbySample")
	FPrimaryAssetId GameMapID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LobbySample")
	TSubclassOf<AGameModeBase> GameMapGameModeClass;

	
	// ImGui tick and draw functions
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULSGameInstance, STATGROUP_Tickables); }
	void DrawImGui();
};
