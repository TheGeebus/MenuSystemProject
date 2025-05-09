// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
* Delegates for the menu class to bind callbacks to
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

DECLARE_DELEGATE_TwoParams(FMultiplayerOnReadFriendsListComplete, bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionInviteComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSendInviteComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionInviteReceived, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionsSubsystem();

	/**
	* To handle session functionality, the menu class will call these 
	*/
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	void FindFriends();

	void InviteFriend(const FUniqueNetIdRef& FriendNetID);

	/**
	* Custom delegates for the Menu class to bind callbacks to
	*/
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	
	FMultiplayerOnReadFriendsListComplete MultiplayerOnReadFriendsListComplete;
	FMultiplayerOnSessionInviteComplete MultiplayerOnSessionInviteComplete;
	FMultiplayerOnSendInviteComplete MultiplayerOnSendInviteComplete;
	FMultiplayerOnSessionInviteReceived MultiplayerOnSessionInviteReceived;

	// FMultiplayerOnSessionUserInviteAcceptedDelegate MultiplayerOnSessionUserInviteAccepted;
	// FMultiplayerOnAcceptInviteComplete MultiplayerOnAcceptInviteComplete;
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	* Internal callbacks for the delegates to be added to the Online Session Interface delegate list
	* These don't need to be called outside of this class
	*/
	void OnCreateSessionsComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void OnSendInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerID, FUniqueNetIdPtr UserID, const FOnlineSessionSearchResult& InviteResult);
	// void OnAcceptInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

private:
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	/**
	* Added to the Online Session Interface delegate list
	* We'll bind our MultiplayerSessionsSubsystem internal callbacks to these 
	*/
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;

	FOnSendInviteComplete SendInviteCompleteDelegate;
	
	FOnSessionUserInviteAcceptedDelegate SessionUserInviteAcceptedDelegate;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

	FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
	FDelegateHandle SessionInviteReceivedDelegateHandle;

	// FOnInviteAcceptedDelegate InviteAcceptedDelegate;
	// FDelegateHandle InviteAcceptedDelegateHandle;

	// FOnAcceptInviteComplete AcceptInviteCompleteDelegate;


	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
