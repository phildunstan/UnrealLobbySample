// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LSFrontEndGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LOBBYSAMPLE_API ALSFrontEndGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALSFrontEndGameMode();
	
	virtual void StartToLeaveMap() override;
	virtual void PostSeamlessTravel() override;

	virtual APlayerController *ProcessClientTravel(FString &URL, bool bSeamless, bool bAbsolute) override;
	virtual void ProcessServerTravel(const FString &URL, bool bAbsolute = false) override;
};
