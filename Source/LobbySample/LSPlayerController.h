// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LSPlayerController.generated.h"

UCLASS()
class LOBBYSAMPLE_API ALSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "LobbySample")
	void StartGame();
	
protected:
	virtual void PostSeamlessTravel() override;
};
