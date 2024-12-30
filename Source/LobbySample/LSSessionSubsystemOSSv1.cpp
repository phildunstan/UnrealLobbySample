// Copyright Philip Dunstan

#include "LSSessionSubsystemOSSv1.h"

#if LS_USE_OSSV1

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemEOSTypesPublic.h"

#include <imgui.h>

#include "Engine/AssetManager.h"

void ULSSessionSubsystemOSSv1::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);
	
	OnlineSubsystem = Online::GetSubsystem(GetWorld());
	IdentityInterface = Online::GetIdentityInterface(GetWorld());
	UserInterface = Online::GetUserInterface(GetWorld());
	SessionInterface = Online::GetSessionInterface(GetWorld());
	PresenceInterface = Online::GetPresenceInterface(GetWorld());
	EventsInterface = Online::GetEventsInterface(GetWorld());
	StatsInterface = Online::GetStatsInterface(GetWorld());
	PartyInterface = Online::GetPartyInterface(GetWorld());
	TimeInterface = Online::GetTimeInterface(GetWorld());
}

void ULSSessionSubsystemOSSv1::Deinitialize()
{
	Super::Deinitialize();
}

void ULSSessionSubsystemOSSv1::Login(const ULocalPlayer* const LocalPlayer)
{
	if (!IdentityInterface)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("OnlineSubsystem is not available")));
		return;
	}
	IdentityInterface->AutoLogin(LocalPlayer->GetControllerId());
}

namespace
{
	FString ConstructServerTravelURL(const FString& MapName, const TMap<FString, FString>& Args)
	{
		FString URL = MapName;
		
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

void ULSSessionSubsystemOSSv1::HostSession(const FPrimaryAssetId& LobbyMapID)
{
	SessionInterface->DestroySession(ActiveSessionName);

	FAssetData MapAssetData;
	if (!UAssetManager::Get().GetPrimaryAssetData(LobbyMapID, MapAssetData))
	{
		OnCreateSessionComplete(ActiveSessionName, false);
		return;
	}
	FString MapName = MapAssetData.PackageName.ToString();

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.BuildUniqueId = 1;
	SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, ActiveSessionName.ToString());
	// SessionSettings.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY, FString(TEXT("BUCKET_ID_DEV")), EOnlineDataAdvertisementType::ViaOnlineService);
	// SessionSettings.Set(SETTING_GAMEMODE, FString(TEXT("")), EOnlineDataAdvertisementType::Type::ViaOnlineService);
	SessionSettings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::Type::ViaOnlineService);
	SessionSettings.Set(LOBBY_SETTING_MATCHTYPE_KEY, FString(TEXT("Race")), EOnlineDataAdvertisementType::Type::ViaOnlineService);
	CreateSessionCompleteDelegateHandle = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ULSSessionSubsystemOSSv1::OnCreateSessionComplete);
	
	PendingTravelURL = ConstructServerTravelURL(MapName, {});
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(LocalPlayer->GetControllerId(), ActiveSessionName, SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionComplete(ActiveSessionName, false);
	}
}

void ULSSessionSubsystemOSSv1::FindAndJoinSession()
{
	LastSessionSearch = MakeShared<FOnlineSessionSearch>();
	LastSessionSearch->MaxSearchResults = 4;
	LastSessionSearch->TimeoutInSeconds = 10.0f;
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	// LastSessionSearch->QuerySettings.Set(OSSEOS_BUCKET_ID_ATTRIBUTE_KEY, FString(TEXT("BUCKET_ID_DEV")), EOnlineComparisonOp::Equals);
	LastSessionSearch->QuerySettings.Set(LOBBY_SETTING_MATCHTYPE_KEY, FString(TEXT("Race")), EOnlineComparisonOp::Equals);
	
	FindSessionsCompleteDelegateHandle = SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ULSSessionSubsystemOSSv1::OnFindSessionsComplete);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(LocalPlayer->GetControllerId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete(false);
	}
}

void ULSSessionSubsystemOSSv1::LeaveSession()
{
	DestroySessionCompleteDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &ULSSessionSubsystemOSSv1::OnDestroySessionComplete);
	if (!SessionInterface->DestroySession(ActiveSessionName))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionComplete(ActiveSessionName, false);
	}
}

FString ULSSessionSubsystemOSSv1::GetNickname(const ULocalPlayer* const LocalPlayer)
{
	// return LocalPlayer->GetNickname();
	return "";
}


void ULSSessionSubsystemOSSv1::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.f, (bWasSuccessful ? FColor::Green : FColor::Orange), FString::Printf(TEXT("OnCreateSessionComplete %s"), (bWasSuccessful ? TEXT("succeeded") : TEXT("failed"))));
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString::Printf(TEXT("ServerTravel initiated to %s"), *PendingTravelURL));
	GetWorld()->ServerTravel(PendingTravelURL);
}

void ULSSessionSubsystemOSSv1::OnFindSessionsComplete(bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.f, (bWasSuccessful ? FColor::Green : FColor::Orange), FString::Printf(TEXT("OnFindSessionsComplete %s. Found %d sessions."), (bWasSuccessful ? TEXT("succeeded") : TEXT("failed")), LastSessionSearch->SearchResults.Num()));
	
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if (!LastSessionSearch->SearchResults.IsEmpty())
	{
		JoinSessionCompleteDelegateHandle = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ULSSessionSubsystemOSSv1::OnJoinSessionComplete);
		
		FOnlineSessionSearchResult& SearchResult = LastSessionSearch->SearchResults[0];
		// bUseLobbiesIfAvailable isn't automatically copied to the search result.
		SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		SessionInterface->JoinSession(LocalPlayer->GetControllerId(), ActiveSessionName, SearchResult);
	}
}

void ULSSessionSubsystemOSSv1::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.f, ((Result == EOnJoinSessionCompleteResult::Type::Success) ? FColor::Green : FColor::Orange), FString::Printf(TEXT("OnJoinSessionComplete %s: %s"), ((Result == EOnJoinSessionCompleteResult::Type::Success) ? TEXT("succeeded") : TEXT("failed")), LexToString(Result)));
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	FString URL;
	if (!SessionInterface->GetResolvedConnectString(SessionName, URL))
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, FString::Printf(TEXT("GetResolvedConnectString failed")));
		UE_LOG(LogOnlineSession, Error, TEXT("ULSSessionSubsystemOSSv1::OnJoinSessionComplete GetResolvedConnectString failed"));
		return;
	}
	
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ensure(PlayerController);
	UE_LOG(LogOnlineSession, Log, TEXT("ULSSessionSubsystemOSSv1::OnJoinSessionComplete Starting ClientTravel to %s"), *URL);
	PlayerController->ClientTravel(URL, TRAVEL_Absolute, false);
}

void ULSSessionSubsystemOSSv1::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	
	GEngine->AddOnScreenDebugMessage(-1, 8.f, (bWasSuccessful ? FColor::Green : FColor::Orange), FString::Printf(TEXT("OnDestroySessionComplete %s"), (bWasSuccessful ? TEXT("succeeded") : TEXT("failed"))));
}

void ULSSessionSubsystemOSSv1::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void ULSSessionSubsystemOSSv1::Tick(float DeltaTime)
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
	
void ULSSessionSubsystemOSSv1::DrawImGui()
{
#ifndef IMGUI_DISABLE
	if (!GetWorld())
		return;
	
	static bool bShowWindow = true;
	if (ImGui::Begin("Session Subsystem", &bShowWindow, 0))
	{
		if (OnlineSubsystem)
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("OnlineSubsystemName: %s"), *OnlineSubsystem->GetOnlineServiceName().ToString())));
		else
			ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("OnlineSubsystemName: None"))));
	
		if (ImGui::TreeNodeEx("Users", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (IdentityInterface.IsValid())
			{
				const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
				const FUniqueNetIdRepl LocalUniqueNetId = LocalPlayer ? LocalPlayer->GetPreferredUniqueNetId() : FUniqueNetIdRepl();
				const TSharedPtr<FUserOnlineAccount> LocalUserAccount = LocalUniqueNetId.IsValid() ? IdentityInterface->GetUserAccount(*LocalUniqueNetId) : TSharedPtr<FUserOnlineAccount>();
				if (!LocalUserAccount.IsValid())
				{
					ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("No local logged in user"))));
				}
				else
				{
					DrawImGui(*LocalUserAccount);
				}
				
				TArray<TSharedPtr<FUserOnlineAccount>> UserAccounts = IdentityInterface->GetAllUserAccounts();
				if (UserAccounts.IsEmpty())
				{
					ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("GetAllUserAccounts returned 0 users"))));
				}
				else
				{
					for (const TSharedPtr<FUserOnlineAccount>& UserAccount : UserAccounts)
					{
						if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("UserAccount %s"), *UserAccount->GetUserId()->ToDebugString())), ImGuiTreeNodeFlags_DefaultOpen))
						{
							DrawImGui(*UserAccount);
							ImGui::TreePop();
						}
					}
				}
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Identity interface is unavailable"))));
			}
			
			ImGui::TreePop();
		}
	
		if (ImGui::TreeNodeEx("Session", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (SessionInterface.IsValid())
			{
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d sessions"), SessionInterface->GetNumSessions())));

				if (!ActiveSessionName.IsNone())
				{
					if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("ActiveSession %s"), *ActiveSessionName.ToString())), ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(ActiveSessionName))
						{
							DrawImGui(*Session);
						}

						ImGui::TreePop();
					}
				}
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Session interface is unavailable"))));
			}
	
			if (PresenceInterface.IsValid())
			{
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Presence interface is unavailable"))));
			}
	
			if (EventsInterface.IsValid())
			{
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Events interface is unavailable"))));
			}
	
			if (StatsInterface.IsValid())
			{
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Stats interface is unavailable"))));
			}
	
			if (PartyInterface.IsValid())
			{
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Party interface is unavailable"))));
			}
	
			if (TimeInterface.IsValid())
			{
			}
			else
			{
				ImGui::TextColored(FColorToImVec4(FColorList::Orange), TCHAR_TO_UTF8(*FString::Printf(TEXT("Time interface is unavailable"))));
			}
	
			ImGui::TreePop();
		}

		if (LastSessionSearch)
		{
			ImGui::Separator();
		
			if (ImGui::TreeNodeEx("Last Search Request", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawImGui(*LastSessionSearch);
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
#endif
}

void ULSSessionSubsystemOSSv1::DrawImGui(const FUserOnlineAccount& LocalUserAccount) const
{
	if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Local UserAccount %s"), *LocalUserAccount.GetUserId()->ToDebugString())), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("RealName: %s"), *LocalUserAccount.GetRealName())));
		if (!OnlineSubsystem->GetOnlineServiceName().EqualTo(FText::FromString(TEXT("EOS")))) // uncomment this when it doesn't spam the log
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("DisplayName: %s"), *LocalUserAccount.GetDisplayName())));
		ImGui::TreePop();
	}
}

void ULSSessionSubsystemOSSv1::DrawImGui(const FNamedOnlineSession& Session) const
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	const FUniqueNetIdRepl LocalUniqueNetId = LocalPlayer->GetPreferredUniqueNetId();
	
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SessionName: %s"), *Session.SessionName.ToString())));	
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("HostingPlayerNum: %d"), Session.HostingPlayerNum)));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SessionState: %s"), EOnlineSessionState::ToString(Session.SessionState))));
	if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("RegisteredPlayers: %d"), Session.RegisteredPlayers.Num())), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (Session.RegisteredPlayers.IsEmpty())
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT(" 0 registered players"))));
		}
		else
		{
			// start the async query for all of the user info and presence. we won't get the result this frame but that is okay.
			// we shouldn't need to do this every frame, only if we can't get the info for some of the users.
			// also, we should start this query from the session code when a new member joins the lobby.
			if (UserInterface.IsValid())
			{
				UserInterface->QueryUserInfo(LocalUserNum, Session.RegisteredPlayers);
			}
			if (PresenceInterface.IsValid())
			{
				PresenceInterface->QueryPresence(*LocalUniqueNetId,	Session.RegisteredPlayers, IOnlinePresence::FOnPresenceTaskCompleteDelegate());
			}

			for (const FUniqueNetIdRef& RegisteredPlayerId : Session.RegisteredPlayers)
			{
				if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("RegisteredPlayer %s"), *RegisteredPlayerId->ToDebugString()))))
				{
					if (UserInterface.IsValid())
					{
						if (TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(LocalUserNum, *RegisteredPlayerId); UserInfo.IsValid())
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("RealName: %s"), *UserInfo->GetRealName())));
							if (!OnlineSubsystem->GetOnlineServiceName().EqualTo(FText::FromString(TEXT("EOS")))) // uncomment this when it doesn't spam the log
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("DisplayName: %s"), *UserInfo->GetDisplayName())));
						}
					}

					if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Presence"))), ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (PresenceInterface.IsValid())
						{
							TSharedPtr<FOnlineUserPresence> Presence;
							if (PresenceInterface->GetCachedPresence(*RegisteredPlayerId, Presence) == EOnlineCachedResult::Type::Success)
							{
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("Status: %s"), *Presence->Status.ToDebugString())));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsOnline: %d"), Presence->bIsOnline)));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsPlaying: %d"), Presence->bIsPlaying)));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsPlayingThisGame %d"), Presence->bIsPlayingThisGame)));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsJoinable %d"), Presence->bIsJoinable)));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bHasVoiceSupport %d"), Presence->bHasVoiceSupport)));
							}
						}
			
						ImGui::TreePop();
					}
		
					ImGui::TreePop();
				}
			}

		}
		ImGui::TreePop();
	}

	DrawImGui(static_cast<const FOnlineSession&>(Session));
}

void ULSSessionSubsystemOSSv1::DrawImGui(const FOnlineSession& Session) const
{
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("OwningPlayerName: %s"), *Session.OwningUserName)));
	if (Session.OwningUserId.IsValid())
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("OwningPlayerId: %s"), *Session.OwningUserId->ToDebugString())));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("NumOpenPrivateConnections: %d"), Session.NumOpenPrivateConnections)));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("NumOpenPublicConnections: %d"), Session.NumOpenPublicConnections)));
	if (Session.SessionInfo.IsValid())
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SessionInfo: %s"), *Session.SessionInfo->ToDebugString())));

	if (const FOnlineSessionSettings* SessionSettings = &Session.SessionSettings)
	{
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SessionSettings:"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tNumPublicConnections: %d"), SessionSettings->NumPublicConnections)));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tNumPrivateConnections: %d"), SessionSettings->NumPrivateConnections)));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbIsLanMatch: %s"), SessionSettings->bIsLANMatch ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbIsDedicated: %s"), SessionSettings->bIsDedicated ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbUsesStats: %s"), SessionSettings->bUsesStats ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbShouldAdvertise: %s"), SessionSettings->bShouldAdvertise ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbAllowJoinInProgress: %s"), SessionSettings->bAllowJoinInProgress ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbAllowInvites: %s"), SessionSettings->bAllowInvites ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbUsesPresence: %s"), SessionSettings->bUsesPresence ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbUseLobbiesIfAvailable: %s"), SessionSettings->bUseLobbiesIfAvailable ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbUseLobbiesVoiceChatIfAvailable: %s"), SessionSettings->bUseLobbiesVoiceChatIfAvailable ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbAllowJoinViaPresence: %s"), SessionSettings->bAllowJoinViaPresence ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tbAllowJoinViaPresenceFriendsOnly: %s"), SessionSettings->bAllowJoinViaPresenceFriendsOnly ? TEXT("true") : TEXT("false"))));
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("\tBuildUniqueId: 0x%08x"), SessionSettings->BuildUniqueId)));
		if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("SessionSettings"))), ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
			{
				FName Key = It.Key();
				const FOnlineSessionSetting& Setting = It.Value();
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s=%s"), *Key.ToString(), *Setting.ToString())));
			}
			if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("MemberSettings"))), ImGuiTreeNodeFlags_DefaultOpen))
			{
			
				for (const auto& MemberSettings : SessionSettings->MemberSettings)
				{
					if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s"), *MemberSettings.Key->ToDebugString())), ImGuiTreeNodeFlags_DefaultOpen))
					{
						for (FSessionSettings::TConstIterator It(MemberSettings.Value); It; ++It)
						{
							FName Key = It.Key();
							const FOnlineSessionSetting& Setting = It.Value();
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s=%s"), *Key.ToString(), *Setting.ToString())));
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

void ULSSessionSubsystemOSSv1::DrawImGui(const FOnlineSessionSearch& SessionSearch) const
{
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("SearchState: %s"), EOnlineAsyncTaskState::ToString(SessionSearch.SearchState))));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("MaxSearchResults: %d"), SessionSearch.MaxSearchResults)));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("bIsLanQuery: %s"), (SessionSearch.bIsLanQuery ? TEXT("true") : TEXT("false")))));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("PingBucketSize: %d"), SessionSearch.PingBucketSize)));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("PlatformHash: %d"), SessionSearch.PlatformHash)));
	ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("TimeoutInSeconds: %fs"), SessionSearch.TimeoutInSeconds)));
	if (ImGui::TreeNodeEx("Query Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const FOnlineSearchSettings& QuerySettings = SessionSearch.QuerySettings;
		for (const FOnlineKeyValuePairs<FName, FOnlineSessionSearchParam>::ElementType& SearchParam : QuerySettings.SearchParams)
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s: %s"), *SearchParam.Key.ToString(), *SearchParam.Value.ToString())));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Results", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d results"), SessionSearch.SearchResults.Num())));

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch.SearchResults)
		{
			if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*FString::Printf(TEXT("Search Result: Session %s"), *SearchResult.GetSessionIdStr())), ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawImGui(SearchResult.Session);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}



#endif // LS_USE_OSSV1
