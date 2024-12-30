// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LSSessionSubsystem.generated.h"

UCLASS(BlueprintType, Config=Engine)
class LOBBYSAMPLE_API ULSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Login(const ULocalPlayer* const LocalPlayer);

	UFUNCTION(BlueprintCallable)
	void HostSession(const FPrimaryAssetId& LobbyMapID);
	
	UFUNCTION(BlueprintCallable)
	void FindAndJoinSession();
	
	UFUNCTION(BlueprintCallable)
	void LeaveSession();

	
	FString GetNickname(const ULocalPlayer* const LocalPlayer);
	

	void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;

private:
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	FDelegateHandle NetworkFailureDelegateHandle;
	
	void OnTravelFailure(UWorld *InWorld, ETravelFailure::Type FailureType, const FString &ErrorString);
	FDelegateHandle TravelFailureDelegateHandle;
};
