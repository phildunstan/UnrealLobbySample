// Copyright Philip Dunstan


#include "LSSessionSubsystemOSSv2.h"

#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

#if !LS_USE_OSSV1

#include "Engine/AssetManager.h"

#include "Online/Auth.h"
#include "Online/Connectivity.h"
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineError.h"
#include "Online/OnlineResult.h"
#include "Online/Presence.h"
#include "Online/Privileges.h"
#include "Online/UserInfo.h"

#include <imgui.h>

using namespace UE::Online;

const FName LOBBY_SETTING_MATCHTYPE_KEY(TEXT("MATCH_TYPE"));
const FName LOBBY_SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv2"));

void ULSSessionSubsystemOSSv2::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	OnlineServices = UE::Online::GetServices();
	check(OnlineServices); // Is OnlineServices set up in the DefaultEngine.ini (and OnlineSubsystem disabled)?
	if (OnlineServices)
	{
		AuthService = OnlineServices->GetAuthInterface();
		ConnectivityService = OnlineServices->GetConnectivityInterface();
		PrivilegesService = OnlineServices->GetPrivilegesInterface();
		PresenceService = OnlineServices->GetPresenceInterface();
		LobbiesService = OnlineServices->GetLobbiesInterface();
		UserInfoService = OnlineServices->GetUserInfoInterface();
	}
}

void ULSSessionSubsystemOSSv2::Deinitialize()
{
	Super::Deinitialize();
}

void ULSSessionSubsystemOSSv2::Login(const ULocalPlayer* const LocalPlayer)
{
	FPlatformUserId PlatformUserId = LocalPlayer->GetPlatformUserId();
	FAuthLogin::Params LoginParameters;
	LoginParameters.PlatformUserId = PlatformUserId;
	LoginParameters.CredentialsType = LoginCredentialsType::Auto;
	
	if (AuthService->IsLoggedIn(PlatformUserId))
	{
		TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> GetUsersResult = AuthService->GetLocalOnlineUserByPlatformUserId(FAuthGetLocalOnlineUserByPlatformUserId::Params { PlatformUserId });
		if (GetUsersResult.IsOk())
		{
			UpdatePlayerUniqueNetId(PlatformUserId, GetUsersResult.GetOkValue().AccountInfo->AccountId);
			return;
		}
	}
	AuthService->Login(MoveTemp(LoginParameters)).OnComplete(this, &ThisClass::OnLoginComplete, LoginParameters.PlatformUserId);
}

void ULSSessionSubsystemOSSv2::HostLobby(const FPrimaryAssetId& LobbyMapID)
{
	FCreateLobby::Params CreateLobbyParameters;
	CreateLobbyParameters.LocalAccountId = GetLocalAccountId();
	CreateLobbyParameters.LocalName = NAME_GameSession;
	CreateLobbyParameters.SchemaId = FSchemaId(TEXT("GameLobby"));
	CreateLobbyParameters.MaxMembers = 4;
	CreateLobbyParameters.JoinPolicy = ELobbyJoinPolicy::PublicAdvertised;
	CreateLobbyParameters.bPresenceEnabled = true;
	CreateLobbyParameters.Attributes.Emplace(SETTING_GAMEMODE, FString("Race"));
	CreateLobbyParameters.Attributes.Emplace(SETTING_MAPNAME, LobbyMapID.ToString());
	CreateLobbyParameters.Attributes.Emplace(SETTING_MATCHING_TIMEOUT, 120.0f);
	CreateLobbyParameters.Attributes.Emplace(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")));
	CreateLobbyParameters.Attributes.Emplace(LOBBY_SETTING_ONLINESUBSYSTEM_VERSION, true);

	CreateLobbyParameters.UserAttributes.Emplace(SETTING_GAMEMODE, FString(TEXT("GameSession")));
	
	LobbiesService->CreateLobby(MoveTemp(CreateLobbyParameters)).OnComplete(this, &ThisClass::OnCreateLobbyComplete, LobbyMapID);
}

void ULSSessionSubsystemOSSv2::FindAndJoinLobby()
{
	FFindLobbies::Params FindLobbiesParameters;
	FindLobbiesParameters.LocalAccountId = GetLocalAccountId();
	LobbiesService->FindLobbies(MoveTemp(FindLobbiesParameters)).OnComplete(this, &ThisClass::OnFindLobbiesComplete);
}

void ULSSessionSubsystemOSSv2::LeaveLobby()
{
	FLeaveLobby::Params LeaveLobbyParameters;
	LeaveLobbyParameters.LocalAccountId = GetLocalAccountId();
	// LeaveLobbyParameters.LobbyId = ;
	LobbiesService->LeaveLobby(MoveTemp(LeaveLobbyParameters)).OnComplete(this, &ThisClass::OnLeaveLobbyComplete, LeaveLobbyParameters.LocalAccountId);
}

FString ULSSessionSubsystemOSSv2::GetNickname(const ULocalPlayer* const LocalPlayer)
{
	if (UWorld* World = GetWorld())
	{
		const FUniqueNetIdRepl LocalUserId = LocalPlayer->GetPreferredUniqueNetId();
		if (LocalUserId.IsValid())
		{
			FAccountId AccountId = LocalUserId.GetV2();
			const TOnlineResult<FGetUserInfo> GetUserInfoResult = UserInfoService->GetUserInfo(FGetUserInfo::Params { AccountId, AccountId });
			if (GetUserInfoResult.IsOk())
			{
				// return GetUserInfoResult.GetOkValue().UserInfo->DisplayName;
				return TEXT("");
			}
		}
	}
	return TEXT("");
}


void ULSSessionSubsystemOSSv2::JoinLobby(const FLobbyId LobbyId)
{
	FJoinLobby::Params JoinLobbyParameters;
	JoinLobbyParameters.LocalAccountId = GetLocalAccountId();
	JoinLobbyParameters.LocalName = NAME_GameSession;
	JoinLobbyParameters.LobbyId = LobbyId;
	JoinLobbyParameters.bPresenceEnabled  = true;
	LobbiesService->JoinLobby(MoveTemp(JoinLobbyParameters)).OnComplete(this, &ThisClass::OnJoinLobbyComplete, JoinLobbyParameters.LocalAccountId);
}

void ULSSessionSubsystemOSSv2::OnLoginComplete(const TOnlineResult<FAuthLogin>& Result, const FPlatformUserId PlatformUserId)
{
	if (!Result.IsOk())
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnUserLoginComplete failed: %s."), *ToLogString(Result.GetErrorValue())));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("OnUserLoginComplete succeeded for platform user id %d: %s"), PlatformUserId.GetInternalId(), *ToLogString(Result.GetOkValue().AccountInfo->AccountId)));

	FAccountId AccountId = Result.GetOkValue().AccountInfo->AccountId;
	UpdatePlayerUniqueNetId(PlatformUserId, AccountId);
}

namespace
{
	FString ConstructServerTravelURL(const FPrimaryAssetId& MapID, const TMap<FString, FString>& Args)
	{
		FAssetData MapAssetData;
		if (!UAssetManager::Get().GetPrimaryAssetData(MapID, MapAssetData))
		{
			return FString();
		}
		
		FString URL = MapAssetData.PackageName.ToString();
		
		URL += TEXT("?listen");
		
		for (const auto& KVP : Args)
		{
			if (!KVP.Key.IsEmpty())
			{
				if (KVP.Value.IsEmpty())
				{
					URL += FString::Printf(TEXT("?%s"), *KVP.Key);
				}
				else
				{
					URL += FString::Printf(TEXT("?%s=%s"), *KVP.Key, *KVP.Value);
				}
			}
		}

		return URL;
	}
}

void ULSSessionSubsystemOSSv2::OnCreateLobbyComplete(const TOnlineResult<FCreateLobby>& Result, const FPrimaryAssetId LobbyMapID)
{
	if (!Result.IsOk())
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnCreateLobbyComplete failed: %s."), *ToLogString(Result.GetErrorValue())));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("OnCreateLobbyComplete succeeded. Created lobby %s."), *ToLogString(Result.GetOkValue().Lobby->LobbyId)));

	const FString TravelURL = ConstructServerTravelURL(LobbyMapID, {});
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString::Printf(TEXT("ServerTravel initiated to %s"), *TravelURL));
	GetWorld()->ServerTravel(TravelURL);
}

void ULSSessionSubsystemOSSv2::OnFindLobbiesComplete(const TOnlineResult<FFindLobbies>& Result)
{
	if (!Result.IsOk())
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnFindLobbiesComplete failed: %s."), *ToLogString(Result.GetErrorValue())));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("OnFindLobbiesComplete succeeded. Found %d lobbies."), Result.GetOkValue().Lobbies.Num()));

	const TArray<TSharedRef<const FLobby>>& Lobbies = Result.GetOkValue().Lobbies;
	if (!Lobbies.IsEmpty())
	{
		JoinLobby(Lobbies[0]->LobbyId);
	}
}

void ULSSessionSubsystemOSSv2::OnLeaveLobbyComplete(const TOnlineResult<FLeaveLobby>& Result, const FAccountId AccountId)
{
	if (!Result.IsOk())
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnLeaveLobbyComplete failed: %s."), *ToLogString(Result.GetErrorValue())));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("OnLeaveLobbyComplete succeeded for user %s."), *ToLogString(AccountId)));
}

void ULSSessionSubsystemOSSv2::OnJoinLobbyComplete(const TOnlineResult<FJoinLobby>& Result, const FAccountId AccountId)
{
	if (!Result.IsOk())
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnJoinLobbyComplete failed %s"), *ToLogString(Result.GetErrorValue())));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("OnJoinLobbyComplete succeeded for user %s from lobby %s."), *ToLogString(AccountId), *ToLogString(Result.GetOkValue().Lobby->LobbyId)));

	const FLobby& Lobby = *Result.GetOkValue().Lobby;
	
	const FAccountId LocalAccountId = GetLocalAccountId();
	if (LocalAccountId.IsValid())
	{
		TOnlineResult<FGetResolvedConnectString> GetResolvedConnectStringResult = OnlineServices->GetResolvedConnectString({LocalAccountId, Lobby.LobbyId});
		if (!GetResolvedConnectStringResult.IsOk())
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("GetResolvedConnectString failed %s"), *ToLogString(Result.GetErrorValue())));
			return;
		}
		const FString TravelURL = GetResolvedConnectStringResult.GetOkValue().ResolvedConnectString;
		
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		ensure(PlayerController);
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString::Printf(TEXT("ClientTravel initiated to %s"), *TravelURL));
		PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute);
	}
}

void ULSSessionSubsystemOSSv2::UpdatePlayerUniqueNetId(const FPlatformUserId PlatformUserId, const FAccountId AccountId)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerControllerFromPlatformUser(GetWorld(), PlatformUserId);
	check(PlayerController->IsLocalController());
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->GetLocalPlayer());
	check(LocalPlayer);
	LocalPlayer->SetCachedUniqueNetId(FUniqueNetIdRepl(AccountId));
}


FAccountId ULSSessionSubsystemOSSv2::GetLocalAccountId() const
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	const FPlatformUserId PlatformUserId = LocalPlayer->GetPlatformUserId();
	const TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> GetUsersResult = AuthService->GetLocalOnlineUserByPlatformUserId(FAuthGetLocalOnlineUserByPlatformUserId::Params { PlatformUserId });
	if (GetUsersResult.IsOk())
		return GetUsersResult.GetOkValue().AccountInfo->AccountId;
	else
		return FAccountId();
}


FString ULSSessionSubsystemOSSv2::GetUserDisplayName(const FAccountId AccountId) const
{
	return TEXT("");
	// if (UserInfoService)
	// {
	// 	const FAccountId LocalAccountId = GetLocalAccountId();
	// 	const TOnlineResult<FGetUserInfo> GetUserInfoResult = UserInfoService->GetUserInfo(FGetUserInfo::Params { LocalAccountId, AccountId });
	// 	if (GetUserInfoResult.IsOk())
	// 	{
	// 		return GetUserInfoResult.GetOkValue().UserInfo->DisplayName;
	// 	}
	// 	else
	// 	{
	// 		return GetUserInfoResult.GetErrorValue().GetLogString(true);
	// 	}
	// }
	//
	// return TEXT("UserInfo service is unavailable");
}

void ULSSessionSubsystemOSSv2::Tick(float DeltaTime)
{
	DrawImGui();
}

namespace
{
	ImVec4 FColorToImVec4(const FColor &Color)
	{
		return ImVec4(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, Color.A / 255.0f);
	}
}

void ULSSessionSubsystemOSSv2::DrawImGui()
{
	if (!GetWorld())
		return;
	
	if (const ImGui::FScopedContext ScopedContext; ScopedContext)
	{
		static bool bShowWindow = true;
		if (ImGui::Begin("Session Subsystem", &bShowWindow, 0))
		{
			if (OnlineServices)
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("OnlineServices ServicesProvider: %s"), LexToString(OnlineServices->GetServicesProvider()))));
			else
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("OnlineServices ServicesProvider: None"))));
	
			if (ImGui::TreeNodeEx("Auth", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (AuthService)
				{
					TOnlineResult<FAuthGetAllLocalOnlineUsers> GetAllLocalOnlineUsersResult = AuthService->GetAllLocalOnlineUsers(FAuthGetAllLocalOnlineUsers::Params {});
					if (!GetAllLocalOnlineUsersResult.IsOk())
					{
						ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Error during GetAllLocalOnlineUsers %s"), *ToLogString(GetAllLocalOnlineUsersResult.GetErrorValue()))));
					}
					else
					{
						const TArray<TSharedRef<FAccountInfo>>& AccountInfos = GetAllLocalOnlineUsersResult.GetOkValue().AccountInfo;
						if (AccountInfos.IsEmpty())
						{
							ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("GetAllLocalOnlineUsers returned 0 users"))));
						}
						else
						{
							for (const auto& AccountInfo : AccountInfos)
							{
								if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("AccountInfo %s %s"), *GetUserDisplayName(AccountInfo->AccountId), *ToLogString(AccountInfo->AccountId))), ImGuiTreeNodeFlags_DefaultOpen))
								{
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("PlatformUserId: %d"), AccountInfo->PlatformUserId.GetInternalId())));
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("LoginStatus: %s"), LexToString(AccountInfo->LoginStatus))));
									for (const auto& Attribute : AccountInfo->Attributes)
									{
										ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s: %s"), *Attribute.Key.ToString(), *Attribute.Value.ToLogString())));
									}
									ImGui::TreePop();
								}
							}
						}
					}
				}
				else
				{
					ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Auth service is unavailable"))));
				}
				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNodeEx("Connectivity", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ConnectivityService)
				{
					TOnlineResult<FGetConnectionStatus> GetConnectionStatusResult = ConnectivityService->GetConnectionStatus(FGetConnectionStatus::Params {});
					if (!GetConnectionStatusResult.IsOk())
					{
						ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Error during GetGetConnectionStatus %s"), *ToLogString(GetConnectionStatusResult.GetErrorValue()))));
					}
					else
					{
						const EOnlineServicesConnectionStatus ConnectionStatus = GetConnectionStatusResult.GetOkValue().Status;
						if (ConnectionStatus == EOnlineServicesConnectionStatus::Connected)
						{
							ImGui::TextColored(FColorToImVec4(FColorList::Green), TCHAR_TO_UTF8(*FString::Printf(TEXT("Connected"))));
						}
						else if (ConnectionStatus == EOnlineServicesConnectionStatus::NotConnected)
						{
							ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Not Connected"))));
						}
					}
				}
				else
				{
					ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Connectivity service is unavailable"))));
				}
	
				ImGui::TreePop();
			}

			ImGui::Separator();

			const FAccountId LocalAccountId = GetLocalAccountId();
			if (LocalAccountId.IsValid())
			{
				if (ImGui::TreeNodeEx("Lobbies", ImGuiTreeNodeFlags_DefaultOpen))
				{
					check(LobbiesService);
					TOnlineResult<FGetJoinedLobbies> JoinedLobbiesResult = LobbiesService->GetJoinedLobbies(FGetJoinedLobbies::Params { LocalAccountId });
					if (!JoinedLobbiesResult.IsOk())
					{
						ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Error during GetJoinedLobbies %s"), *ToLogString(JoinedLobbiesResult.GetErrorValue()))));
					}
					else
					{
						const TArray<TSharedRef<const FLobby>>& Lobbies = JoinedLobbiesResult.GetOkValue().Lobbies;
						if (Lobbies.IsEmpty())
						{
							ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("GetJoinedLobbies returned 0 lobbies"))));
						}
						else
						{
							for (const auto& Lobby : Lobbies)
							{
								if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Lobby %s %s"), *Lobby->LocalName.ToString(), *ToLogString(Lobby->LobbyId))), ImGuiTreeNodeFlags_DefaultOpen))
								{
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("LocalName: %s"), *Lobby->LocalName.ToString())));
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("OwnerAccountId: %s"), *ToLogString(Lobby->OwnerAccountId))));
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SchemaId: %s"), *Lobby->SchemaId.ToString())));
									for (const auto& Attribute : Lobby->Attributes)
									{
										ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s: %s"), *Attribute.Key.ToString(), *Attribute.Value.ToLogString())));
									}
									if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Members: %d"), Lobby->Members.Num())), ImGuiTreeNodeFlags_DefaultOpen))
									{
										TArray<FAccountId> AllAccountIds;
										Lobby->Members.GetKeys(AllAccountIds);
							
										// start the async query for all of the user info and presence. we won't get the result this frame but that is okay.
										// we shouldn't need to do this every frame, only if we can't get the info for some of the users.
										// also, we should start this query from the session code when a new member joins the lobby.
										if (UserInfoService)
											UserInfoService->QueryUserInfo(FQueryUserInfo::Params {LocalAccountId, AllAccountIds});
										// if (PresenceService)
										// 	PresenceService->BatchQueryPresence(FBatchQueryPresence::Params { LocalAccountId, AllAccountIds, false });
						
										for	(const auto& Member : Lobby->Members)
										{
											const FAccountId AccountId = Member.Key;
											if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Member %s %s"), *GetUserDisplayName(AccountId), *ToLogString(AccountId))), ImGuiTreeNodeFlags_DefaultOpen))
											{
												ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsLocalMember: %d"), Member.Value->bIsLocalMember)));
												for (const auto& Attribute : Member.Value->Attributes)
												{
													ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s: %s"), *Attribute.Key.ToString(), *Attribute.Value.ToLogString())));
												}

												if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Presence"))), ImGuiTreeNodeFlags_DefaultOpen))
												{
													if (PresenceService)
													{
														const TOnlineResult<FGetCachedPresence> GetCachedPresenceResult = PresenceService->GetCachedPresence(FGetCachedPresence::Params {LocalAccountId, AccountId});
														if (GetCachedPresenceResult.IsOk())
														{
															const TSharedRef<const FUserPresence>& Presence = GetCachedPresenceResult.GetOkValue().Presence;
															ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("Status: %s"), LexToString(Presence->Status))));
															ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("GameStatus: %s"), LexToString(Presence->GameStatus))));
															ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("Joinability %s"), LexToString(Presence->Joinability))));
															ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("StatusString %s"), *Presence->StatusString)));
															ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("RichPresenceString %s"), *Presence->RichPresenceString)));
															for (const auto& Property : Presence->Properties)
															{
																ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s: %s"), *Property.Key, *Property.Value)));
															}
														}
													}
													else
													{
														ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Presence service is unavailable"))));
													}
										
													ImGui::TreePop();
												}
								
												ImGui::TreePop();
											}
										}
										ImGui::TreePop();
									}
					
									ImGui::TreePop();
								}
							}
						}
					}
	
					ImGui::TreePop();
				}
			}
		}
		ImGui::End();
	}
}

#endif // !LS_USE_OSSV1
