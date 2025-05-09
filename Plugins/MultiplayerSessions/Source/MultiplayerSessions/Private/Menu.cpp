// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
// #include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/KismetStringLibrary.h"


void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath, FString GameMapPath, bool bAllowCharacterControl)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	PathToGameMap = FString::Printf(TEXT("%s?listen"), *GameMapPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetIsFocusable(true);
	UWorld* World = GetWorld();
	if (World)
	{
		FString OutPath = World->GetCurrentLevel()->GetPathName();
		FString Name = World->GetName();
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				OutPath
			);
			
			/*
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				Name
			);
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				LobbyPath
			);
			*/
		}
		
		if (World->IsNetMode(ENetMode::NM_ListenServer) && 
			UKismetStringLibrary::FindSubstring(World->GetCurrentLevel()->GetPathName(), LobbyPath) == 0)
		{
			if (StartButton)
			{
				StartButton->SetVisibility(ESlateVisibility::Visible);
			}
			if (FindFriendsButton)
			{
				FindFriendsButton->SetVisibility(ESlateVisibility::Visible);
				FindFriendsButton->SetIsEnabled(true);
			}
		}
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			if (bAllowCharacterControl)
			{
				FInputModeGameAndUI InputModeData;
				InputModeData.SetWidgetToFocus(TakeWidget());
				InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetWidgetInputMode(InputModeData, PlayerController);
			}
			else
			{
				FInputModeUIOnly InputModeData;
				InputModeData.SetWidgetToFocus(TakeWidget());
				InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetWidgetInputMode(InputModeData, PlayerController);
			}
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnReadFriendsListComplete.BindUObject(this, &ThisClass::OnFindFriendsComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnSessionInviteComplete.AddDynamic(this, &ThisClass::OnSessionInviteComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnSendInviteComplete.AddDynamic(this, &ThisClass::OnSendInviteComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnSessionInviteReceived.AddDynamic(this, &ThisClass::OnSessionInviteReceived);
	}
}

void UMenu::SetWidgetInputMode(FInputModeDataBase& InputModeData, APlayerController* PlayerController)
{
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
	}
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMenu::StartButtonClicked);
		StartButton->SetVisibility(ESlateVisibility::Hidden);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMenu::QuitButtonClicked);
	}
	if (FindFriendsButton)
	{
		FindFriendsButton->OnClicked.AddDynamic(this, &UMenu::FindFriend);
		FindFriendsButton->SetIsEnabled(false);
		FindFriendsButton->SetVisibility(ESlateVisibility::Hidden);
	}
	if (InviteFriendButton)
	{
		InviteFriendButton->OnClicked.AddDynamic(this, &UMenu::InviteFriend);
		InviteFriendButton->SetIsEnabled(false);
		InviteFriendButton->SetVisibility(ESlateVisibility::Hidden);
	}
	
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTeardown();

	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		/*
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString("Session created successfully!!")
			);
		}
		*/
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		/*
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString("Failed to create session")
			);
		}
		*/
		HostButton->SetIsEnabled(true);
		bIsSessionCreated = false;
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			FString ID = Result.GetSessionIdStr();
			FString User = Result.Session.OwningUserName;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("ID: %s, user: %s"), *ID, *User)
				);
			}
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address(TEXT("None."));
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			/*
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					Address
				);
			}
			*/
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
	bIsSessionCreated = false;
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		/*
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				3,
				30.f,
				FColor::Green,
				FString("Started session!!!")
			);
		}
		*/
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToGameMap);
		}
	}
	else
	{
		StartButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindFriendsComplete(bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends)
{
	if (bWasSuccessful)
	{
		InGameFriends.Empty();
		if (Friends.Num() > 0)
		{
			InGameFriends = Friends;

			for (TSharedRef<FOnlineFriend> Friend : InGameFriends)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("%s"), *Friend->GetDisplayName()));
				}
			}
			if (InviteFriendButton)
			{
				InviteFriendButton->SetVisibility(ESlateVisibility::Visible);
				InviteFriendButton->SetIsEnabled(true);
				InviteText->SetText(FText::FromString(FString::Printf(TEXT("Invite %s"), *InGameFriends[0]->GetDisplayName())));
			}
		}
		else if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("FOUND NO FRIENDS!!!"));
		}
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("FindFriends Failed!!!"));
	}
	FindFriendsButton->SetIsEnabled(true);
}

void UMenu::OnSessionInviteComplete(bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("SessionInviteComplete finished!"));
	}
	if (InviteFriendButton)
	{
		InviteFriendButton->SetIsEnabled(true);
		InviteFriendButton->SetVisibility(ESlateVisibility::Visible);
	}
}


void UMenu::OnSendInviteComplete(bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("OnSendInviteComplete finished!"));
	}
	if (InviteFriendButton)
	{
		InviteFriendButton->SetIsEnabled(true);
		InviteFriendButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMenu::OnSessionInviteReceived(bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("OnSessionInviteReceived finished!"));
	}
	if (InviteFriendButton)
	{
		InviteFriendButton->SetIsEnabled(true);
		InviteFriendButton->SetVisibility(ESlateVisibility::Visible);
	}
}


void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	bIsSessionCreated = true;
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::StartButtonClicked()
{
	StartButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->StartSession();
	}
}

void UMenu::QuitButtonClicked()
{
	QuitButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->ConsoleCommand("quit");
		}
	}
}

void UMenu::FindFriend()
{
	FindFriendsButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindFriends();
	}
}

void UMenu::InviteFriend()
{
	InviteFriendButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		if (InGameFriends.Num() > 0)
		{
			MultiplayerSessionsSubsystem->InviteFriend(InGameFriends[0]->GetUserId());
		}
	}
}

void UMenu::MenuTeardown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	/*
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString("TEARDOWN")
		);
	}
	*/
}
