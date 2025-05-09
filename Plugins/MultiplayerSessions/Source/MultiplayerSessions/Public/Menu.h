// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

class UButton;
class UTextBlock;
class UMultiplayerSessionsSubsystem;



/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")), FString GameMapPath = FString(TEXT("/Game/ThirdPerson/Maps/StartingLevel")), bool bAllowCharacterControl = false);

	void SetWidgetInputMode(FInputModeDataBase& InputModeData, APlayerController* PlayerController);


protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/**
	* Callbacks for custom delegates on the MultiplayerSessionsSubsystem
	*/
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	void OnFindFriendsComplete(bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends);
	void OnSessionInviteComplete(bool bWasSuccessful);
	void OnSendInviteComplete(bool bWasSuccessful);
	void OnSessionInviteReceived(bool bWasSuccessful);

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FindFriendsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InviteFriendButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InviteText;

	TArray<TSharedRef<FOnlineFriend>> InGameFriends;

	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();
	
	UFUNCTION()
	void StartButtonClicked();

	UFUNCTION()
	void QuitButtonClicked();

	UFUNCTION()
	void FindFriend();

	UFUNCTION()
	void InviteFriend();
	
	void MenuTeardown();

	// The subsystem designed to handle all online session functionality
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};

	FString PathToLobby{TEXT("")};
	FString PathToGameMap{TEXT("")};

	bool bIsSessionCreated = false;
};
