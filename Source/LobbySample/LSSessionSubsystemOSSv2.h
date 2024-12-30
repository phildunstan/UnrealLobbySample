// Copyright Philip Dunstan

#pragma once

#include "CoreMinimal.h"

#if !LS_USE_OSSV1
#include "Online/Auth.h"
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineServices.h"
#endif // !LS_USE_OSSV1

#include "LSSessionSubsystemOSSv2.generated.h"

UCLASS()
class LOBBYSAMPLE_API ULSSessionSubsystemOSSv2 : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
#if !LS_USE_OSSV1

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;

	
	void Login(const ULocalPlayer* const LocalPlayer);

	void HostLobby(const FPrimaryAssetId& LobbyMapID);
	
	void FindAndJoinLobby();
	
	void LeaveLobby();
	
	FString GetNickname(const ULocalPlayer *LocalPlayer);

private:
	void JoinLobby(const UE::Online::FLobbyId LobbyId);

	void OnLoginComplete(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, const FPlatformUserId PlatformUserId);
	void OnCreateLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FCreateLobby>& Result, const FPrimaryAssetId LobbyMapID);
	void OnFindLobbiesComplete(const UE::Online::TOnlineResult<UE::Online::FFindLobbies>& Result);
	void OnLeaveLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FLeaveLobby>& Result, const UE::Online::FAccountId AccountId);
	void OnJoinLobbyComplete(const UE::Online::TOnlineResult<UE::Online::FJoinLobby>& Result, const UE::Online::FAccountId AccountId);

	void UpdatePlayerUniqueNetId(const FPlatformUserId PlatformUserId, const UE::Online::FAccountId AccountId);
	UE::Online::FAccountId GetLocalAccountId() const;
	FString GetUserDisplayName(const UE::Online::FAccountId AccountId) const;


	UE::Online::IOnlineServicesPtr OnlineServices;
	UE::Online::IAuthPtr AuthService;
	UE::Online::IConnectivityPtr ConnectivityService;
	UE::Online::IPrivilegesPtr PrivilegesService;
	UE::Online::IPresencePtr PresenceService;
	UE::Online::ILobbiesPtr LobbiesService;
	UE::Online::IUserInfoPtr UserInfoService;

public:
	// ImGui tick and draw functions
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULSSessionSubsystemOSSv2, STATGROUP_Tickables); }
	void DrawImGui();
	
#else // if LS_USE_OSSV1
	
	virtual void Tick(float DeltaTime) override {}
	virtual bool IsTickable() const override { return false; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULSSessionSubsystemOSSv2, STATGROUP_Tickables); }
	
#endif // LS_USE_OSSV1
};
