// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#if LS_USE_OSSV1
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#endif // LS_USE_OSSV1

#include "LSSessionSubsystemOSSv1.generated.h"

UCLASS()
class LOBBYSAMPLE_API ULSSessionSubsystemOSSv1 : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

#if LS_USE_OSSV1
	
public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;

	
	void Login(const ULocalPlayer* const LocalPlayer);

	void HostSession(const FPrimaryAssetId& LobbyMapID);
	
	void FindAndJoinSession();
	
	void LeaveSession();

	FString GetNickname(const ULocalPlayer* const LocalPlayer);

	
private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
	IOnlineSubsystem* OnlineSubsystem = nullptr;
	IOnlineIdentityPtr IdentityInterface;
	IOnlineUserPtr UserInterface;
	IOnlineSessionPtr SessionInterface;
	IOnlinePresencePtr PresenceInterface;
	IOnlineEventsPtr EventsInterface;
	IOnlineStatsPtr StatsInterface;
	IOnlinePartyPtr PartyInterface;
	IOnlineTimePtr TimeInterface;

	const FName ActiveSessionName = NAME_GameSession;
	const FName LOBBY_SETTING_MATCHTYPE_KEY = FName(TEXT("MATCH_TYPE"));

	FString PendingTravelURL;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	
public:
	// ImGui tick and draw functions
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULSSessionSubsystemOSSv1, STATGROUP_Tickables); }
	void DrawImGui();
	void DrawImGui(const FUserOnlineAccount& LocalUserAccount) const;
	void DrawImGui(const FNamedOnlineSession &Session) const;
	void DrawImGui(const FOnlineSession &Session) const;
	void DrawImGui(const FOnlineSessionSearch &SessionSearch) const;

#else // if !LS_USE_OSSV1
	
	virtual void Tick(float DeltaTime) override {}
	virtual bool IsTickable() const override { return false; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULSSessionSubsystemOSSv1, STATGROUP_Tickables); }

#endif // !LS_USE_OSSV1
};
