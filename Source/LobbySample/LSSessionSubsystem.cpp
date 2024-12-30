// Copyright Philip Dunstan


#include "LSSessionSubsystem.h"

#if LS_USE_OSSV1
#include "LSSessionSubsystemOSSv1.h"
#else // if !LS_USE_OSSV1
#include "LSSessionSubsystemOSSv2.h"
#endif // !LS_USE_OSSV1

void ULSSessionSubsystem::Login(const ULocalPlayer* const LocalPlayer)
{
#if LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv1>()->Login(LocalPlayer);
#else // if !LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv2>()->Login(LocalPlayer);
#endif // !LS_USE_OSSV1
}

void ULSSessionSubsystem::HostSession(const FPrimaryAssetId& LobbyMapID)
{
#if LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv1>()->HostSession(LobbyMapID);
#else // if !LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv2>()->HostLobby(LobbyMapID);
#endif // !LS_USE_OSSV1
}

void ULSSessionSubsystem::FindAndJoinSession()
{
#if LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv1>()->FindAndJoinSession();
#else // if !LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv2>()->FindAndJoinLobby();
#endif // !LS_USE_OSSV1
}

void ULSSessionSubsystem::LeaveSession()
{
#if LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv1>()->LeaveSession();
#else // if !LS_USE_OSSV1
	GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv2>()->LeaveLobby();
#endif // !LS_USE_OSSV1
}

FString ULSSessionSubsystem::GetNickname(const ULocalPlayer* const LocalPlayer)
{
#if LS_USE_OSSV1
	return GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv1>()->GetNickname(LocalPlayer);
#else // if !LS_USE_OSSV1
	return GetGameInstance()->GetSubsystem<ULSSessionSubsystemOSSv2>()->GetNickname(LocalPlayer);
#endif // !LS_USE_OSSV1
}
		

void ULSSessionSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);
	
	NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::OnNetworkFailure);
	TravelFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::OnTravelFailure);
}

void ULSSessionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	GEngine->OnTravelFailure().Remove(TravelFailureDelegateHandle);
}

void ULSSessionSubsystem::OnNetworkFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString &ErrorString)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange, FString::Printf(TEXT("OnNetworkError %s %s"), *UEnum::GetValueAsString(FailureType), *ErrorString));
    UE_LOG(LogTemp, Error, TEXT("OnNetworkError %s %s"), *UEnum::GetValueAsString(FailureType), *ErrorString);
}

void ULSSessionSubsystem::OnTravelFailure(UWorld *InWorld, ETravelFailure::Type FailureType, const FString &ErrorString)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange, FString::Printf(TEXT("OnTravelFailure %s %s"), *UEnum::GetValueAsString(FailureType), *ErrorString));
    UE_LOG(LogTemp, Error, TEXT("OnTravelFailure %s %s"), *UEnum::GetValueAsString(FailureType), *ErrorString);
}
