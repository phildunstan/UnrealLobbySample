// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "LSLocalPlayer.generated.h"

UCLASS()
class LOBBYSAMPLE_API ULSLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	// Changed the implementation of GetNickname because the default creates a lot of log spam with EOS
	virtual FString GetNickname() const override;
};
