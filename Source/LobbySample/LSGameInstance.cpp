// Copyright Philip Dunstan

#include "LSGameInstance.h"

#include <imgui.h>

#include "GameFramework/GameModeBase.h"

void ULSGameInstance::Tick(float DeltaTime)
{
	DrawImGui();
}

void ULSGameInstance::DrawImGui()
{
#ifndef IMGUI_DISABLE
	if (!GetWorld())
		return;
	
	static bool bShowWindow = true;
	if (ImGui::Begin("Game Modes", &bShowWindow, 0))
	{
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("Map: %s"), *GetWorld()->GetName())));
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("GameMode: %s"), *GameMode->GetClass()->GetName())));
		if (APlayerController* PlayerController = GetPrimaryPlayerController())
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("PlayerController: %s"), *PlayerController->GetClass()->GetName())));
	}
	ImGui::End();
#endif // IMGUI_DISABLE
}
