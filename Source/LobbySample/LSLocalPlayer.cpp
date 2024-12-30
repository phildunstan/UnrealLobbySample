// Copyright Philip Dunstan


#include "LSLocalPlayer.h"

#include "LSSessionSubsystem.h"

FString ULSLocalPlayer::GetNickname() const
{
	return GetGameInstance()->GetSubsystem<ULSSessionSubsystem>()->GetNickname(this);
}