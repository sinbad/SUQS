#include "SuqsProgression.h"

#include <algorithm>


#include "EngineUtils.h"
#include "Suqs.h"
#include "SuqsObjectiveState.h"
#include "SuqsQuestState.h"
#include "SuqsTaskState.h"
#include "SuqsWaypointComponent.h"
#include "SuqsWaypointSubsystem.h"
#include "Kismet/GameplayStatics.h"

#define SuqsCurrentDataVersion 1

//PRAGMA_DISABLE_OPTIMIZATION
void USuqsProgression::InitWithQuestDataTables(TArray<UDataTable*> Tables)
{
	QuestDataTables = Tables;
	RebuildAllQuestData();
}

void USuqsProgression::InitWithQuestDataTablesInPath(const FString& Path)
{
	InitWithQuestDataTablesInPaths(TArray<FString> { Path });
	
}

void USuqsProgression::InitWithQuestDataTablesInPaths(const TArray<FString>& Paths)
{
	TArray<UDataTable*> DataTables;
	for (const FString& Path : Paths)
	{
		TArray<UObject*> Assets;
		EngineUtils::FindOrLoadAssetsByPath(Path, Assets, EngineUtils::ATL_Regular);
		for (auto Asset : Assets)
		{
			if (auto DataTable = Cast<UDataTable>(Asset))
			{
				if (DataTable->RowStruct == FSuqsQuest::StaticStruct())
				{
					DataTables.Add(DataTable);
				}
			}
		}
	}

	InitWithQuestDataTables(DataTables);
}

bool USuqsProgression::GetQuestDefinitionCopy(FName QuestID, FSuqsQuest& OutQuest)
{
	auto QDef = QuestDefinitions.Find(QuestID);
	if (QDef)
	{
		OutQuest = *QDef;
		return true;
	}

	return false;
}

bool USuqsProgression::CreateQuestDefinition(const FSuqsQuest& NewQuest, bool bOverwriteIfExists)
{
	if (NewQuest.Identifier.IsNone())
	{
		UE_LOG(LogSUQS, Error, TEXT("CreateQuestDefinition: Identifier is None"));
		return false;
	}

	auto QDef = QuestDefinitions.Find(NewQuest.Identifier);
	if (QDef)
	{
		if (!bOverwriteIfExists)
		{
			UE_LOG(LogSUQS, Warning, TEXT("CreateQuestDefinition: Identifier '%s' exists, not overwriting"), *NewQuest.Identifier.ToString());
			return false;
		}
		// Delete to re-use add impl
		DeleteQuestDefinition(NewQuest.Identifier);
	}

	// Add new
	AddQuestDefinitionInternal(NewQuest);
	return true;
}

bool USuqsProgression::DeleteQuestDefinition(FName QuestID)
{
	// Remove quest status first, since that holds raw pointers to quest defs
	RemoveQuest(QuestID, true, true);
	// Remove definition
	return QuestDefinitions.Remove(QuestID) > 0;
}

void USuqsProgression::SetDefaultProgressionTimeDelays(float QuestDelay, float TaskDelay)
{
	DefaultQuestResolveTimeDelay = QuestDelay;
	DefaultTaskResolveTimeDelay = TaskDelay;
}


void USuqsProgression::RebuildAllQuestData()
{
	QuestDefinitions.Empty();
	QuestCompletionDeps.Empty();
	QuestFailureDeps.Empty();
	ActiveQuests.Empty();
	QuestArchive.Empty();
	GlobalActiveBranches.Empty();
	
	// Build unified quest table
	if (QuestDataTables.Num() > 0)
	{
		for (auto Table : QuestDataTables)
		{
			UE_LOG(LogSUQS, Verbose, TEXT("Loading quest definitions from %s"), *Table->GetName());
			Table->ForeachRow<FSuqsQuest>("", [this, Table](const FName& Key, const FSuqsQuest& Quest)
            {
				if (QuestDefinitions.Contains(Quest.Identifier))
				{
					UE_LOG(LogSUQS, Error, TEXT("Quest ID '%s' has been used more than once! Duplicate entry was in %s"), *Quest.Identifier.ToString(), *Table->GetName());
				}
				else
				{
					AddQuestDefinitionInternal(Quest);
				}
            });
		}
	}

	const auto GI = UGameplayStatics::GetGameInstance(this);
	if (IsValid(GI))
	{
		auto WaypointsSubSys = GI->GetSubsystem<USuqsWaypointSubsystem>();
		WaypointsSubSys->SetProgression(this);
	}
	
}

void USuqsProgression::AddQuestDefinitionInternal(const FSuqsQuest& Quest)
{
	// Check task IDs are unique
	TSet<FName> TaskIDSet;
	for (auto& Objective : Quest.Objectives)
	{
		for (auto& Task : Objective.Tasks)
		{
			bool bDuplicate;
			TaskIDSet.Add(Task.Identifier, &bDuplicate);
			if (bDuplicate)
				UE_LOG(LogSUQS, Error, TEXT("Task ID '%s' has been used more than once! Duplicate entry title: %s"), *Task.Identifier.ToString(), *Task.Title.ToString());
		}
	}
				
	QuestDefinitions.Add(Quest.Identifier, Quest);

	// Record dependencies
	if (Quest.AutoAccept)
	{
		for (auto& CompletedQuest : Quest.PrerequisiteQuests)
		{
			QuestCompletionDeps.Add(CompletedQuest, Quest.Identifier);
		}
		for (auto& FailedQuest : Quest.PrerequisiteQuestFailures)
		{
			QuestFailureDeps.Add(FailedQuest, Quest.Identifier);
		}
	}
}

const TMap<FName, FSuqsQuest>& USuqsProgression::GetQuestDefinitions(bool bForceRebuild)
{
	if (bForceRebuild)
	{
		RebuildAllQuestData();
	}
	return QuestDefinitions;
}

ESuqsQuestStatus USuqsProgression::GetQuestStatus(FName QuestID) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	const auto State = FindQuestState(QuestID);

	if (State)
		return State->GetStatus();
	else
		return ESuqsQuestStatus::Unavailable;
	
}

USuqsQuestState* USuqsProgression::GetQuest(FName QuestID)
{
	auto Status = FindQuestState(QuestID);
	return Status;
}


USuqsQuestState* USuqsProgression::FindQuestState(const FName& QuestID)
{
	auto PQ = ActiveQuests.Find(QuestID);
	if (PQ)
		return *PQ;
	PQ = QuestArchive.Find(QuestID);
	if (PQ)
		return *PQ;

	return nullptr;
	
}

const USuqsQuestState* USuqsProgression::FindQuestState(const FName& QuestID) const
{
	return const_cast<USuqsProgression*>(this)->FindQuestState(QuestID);
}

USuqsTaskState* USuqsProgression::FindTaskStatus(const FName& QuestID, const FName& TaskID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
	{
		return Q->GetTask(TaskID);
	}
	
	return nullptr;
}


void USuqsProgression::GetAcceptedQuestIdentifiers(TArray<FName>& AcceptedQuestIDsOut) const
{
	ActiveQuests.GenerateKeyArray(AcceptedQuestIDsOut);
}

void USuqsProgression::GetArchivedQuestIdentifiers(TArray<FName>& ArchivedQuestIDsOut) const
{
	QuestArchive.GenerateKeyArray(ArchivedQuestIDsOut);
}


void USuqsProgression::GetAcceptedQuests(TArray<USuqsQuestState*>& AcceptedQuestsOut) const
{
	ActiveQuests.GenerateValueArray(AcceptedQuestsOut);
}

void USuqsProgression::GetArchivedQuests(TArray<USuqsQuestState*>& ArchivedQuestsOut) const
{
	QuestArchive.GenerateValueArray(ArchivedQuestsOut);
}

bool USuqsProgression::AcceptQuest(FName QuestID, bool bResetIfFailed, bool bResetIfComplete, bool bResetIfInProgress)
{
	auto QDef = QuestDefinitions.Find(QuestID);
	if (QDef)
	{
		// Check that we don't already have this quest
		auto Quest = FindQuestState(QuestID);
		if (Quest)
		{
			bool bReset;
			switch(Quest->Status)
			{
			case ESuqsQuestStatus::Incomplete:
				bReset = bResetIfInProgress;
				break;
			case ESuqsQuestStatus::Completed: 
				bReset = bResetIfComplete;
				break;
			case ESuqsQuestStatus::Failed: 
				bReset = bResetIfFailed;
				break;
			default:
				bReset = true;
				break;
			}

			if (!bReset)
			{
				UE_LOG(LogSUQS, Warning, TEXT("Ignoring request to accept quest %s because it has status %d"), *QuestID.ToString(), Quest->Status);
				return false;
			}

			// Quest reset will raise all the necessary events
			Quest->Reset();
		}
		else
		{
			// New quest
			Quest = NewObject<USuqsQuestState>(GetOuter());
			// The problem is that initialisation could trigger detail events while it sorts itself out, out of order
			// Let's suppress that
			const bool bPrevSuppressed = bSuppressEvents;
			bSuppressEvents = true;
			Quest->Initialise(QDef, this);
			bSuppressEvents = bPrevSuppressed;
			
			ActiveQuests.Add(QuestID, Quest);
			
			if (!bSuppressEvents)
			{
				// Manually raise all the initialisation events since these will have been suppressed
				OnQuestAccepted.Broadcast(Quest);
				OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestAccepted, Quest));
				OnActiveQuestsListChanged.Broadcast();
				OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::ActiveQuestsChanged));

				RaiseCurrentObjectiveChanged(Quest);
			}

			// Propagate global quest branches
			for (auto& Branch : GlobalActiveBranches)
			{
				Quest->SetBranchActive(Branch, true);
			}
		}

		return true;
	}
	else
	{
		UE_LOG(LogSUQS, Error , TEXT("Attempted to accept a non-existent quest %s"), *QuestID.ToString());
		return false;
	}
	
}


bool USuqsProgression::AutoAcceptQuests(const FName& FinishedQuestID, bool bFailed)
{
	bool bAnyAccepted = false;
	TArray<FName> DependentQuestIDs;
	if (bFailed)
		QuestFailureDeps.MultiFind(FinishedQuestID, DependentQuestIDs);
	else
		QuestCompletionDeps.MultiFind(FinishedQuestID, DependentQuestIDs);
	
	for (const auto& DepQuestID : DependentQuestIDs)
	{
		if (!IsQuestAccepted(DepQuestID) && QuestDependenciesMet(DepQuestID))
		{
			bAnyAccepted = AcceptQuest(DepQuestID) || bAnyAccepted;
		}
	}

	return bAnyAccepted;
}

void USuqsProgression::ResetQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Reset();

	// note we don't update quest lists here, but we rely on callbacks since you could reset or re-activate using the quest itself
	
}

void USuqsProgression::RemoveQuest(FName QuestID, bool bRemoveActive, bool bRemoveArchived)
{
	if (bRemoveActive)
		ActiveQuests.Remove(QuestID);
	if (bRemoveArchived)
		QuestArchive.Remove(QuestID);
}

void USuqsProgression::FailQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Fail();

	// note we don't update quest lists here, but we rely on callbacks since you could fail via the quest itself
}

void USuqsProgression::CompleteQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Complete();
	
}

void USuqsProgression::ResolveQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Resolve();
	
}

void USuqsProgression::FailTask(FName QuestID, FName TaskIdentifier)
{
	if (QuestID.IsNone())
	{
		for (auto Pair : ActiveQuests)
		{
			Pair.Value->FailTask(TaskIdentifier);
		}
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			T->Fail();
		}
	}
}


bool USuqsProgression::CompleteTask(FName QuestID, FName TaskIdentifier)
{
	if (QuestID.IsNone())
	{
		bool bCompleted = false;
		for (auto Pair : ActiveQuests)
		{
			bCompleted = Pair.Value->CompleteTask(TaskIdentifier) || bCompleted;
		}
		if (bCompleted)
			return true;
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			return T->Complete();
		}
	}
	UE_LOG(LogSUQS, Warning, TEXT("Attempted to complete task %s/%s but it was not found in the active quests"),
		*QuestID.ToString(), *TaskIdentifier.ToString());
	return false;
}

int USuqsProgression::ProgressTask(FName QuestID, FName TaskIdentifier, int Delta)
{
	if (QuestID.IsNone())
	{
		int MaxLeft = 0;
		for (auto Pair : ActiveQuests)
		{
			MaxLeft = std::max(Pair.Value->ProgressTask(TaskIdentifier, Delta), MaxLeft);
		}
		return MaxLeft;
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			return T->Progress(Delta);
		}
		return 0;
	}
}

void USuqsProgression::SetTaskNumberCompleted(FName QuestID, FName TaskIdentifier, int Number)
{
	if (QuestID.IsNone())
	{
		int MaxLeft = 0;
		for (auto Pair : ActiveQuests)
		{
			Pair.Value->SetTaskNumberCompleted(TaskIdentifier, Number);
		}
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			return T->SetNumber(Number);
		}
	}
}

void USuqsProgression::ResolveTask(FName QuestID, FName TaskIdentifier)
{
	if (QuestID.IsNone())
	{
		for (auto Pair : ActiveQuests)
		{
			Pair.Value->ResolveTask(TaskIdentifier);
		}
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			T->Resolve();
		}
	}
}

USuqsObjectiveState* USuqsProgression::GetCurrentObjective(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->GetCurrentObjective();
	}
	return nullptr;
}


bool USuqsProgression::IsQuestAccepted(FName QuestID) const
{
	if (FindQuestState(QuestID))
	{
		return true;
	}
	return false;
}

bool USuqsProgression::IsQuestActive(FName QuestID) const
{
	return ActiveQuests.Find(QuestID) != nullptr;
}

bool USuqsProgression::IsQuestIncomplete(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsIncomplete();
	}
	return true;
}

bool USuqsProgression::IsQuestCompleted(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsCompleted();
	}
	return false;
}

bool USuqsProgression::IsQuestFailed(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsFailed();
	}
	return false;
}

bool USuqsProgression::IsObjectiveIncomplete(FName QuestID, FName ObjectiveID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsObjectiveIncomplete(ObjectiveID);
	}
	return false;
	
}

bool USuqsProgression::IsObjectiveCompleted(FName QuestID, FName ObjectiveID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsObjectiveCompleted(ObjectiveID);
	}
	return false;
}

bool USuqsProgression::IsObjectiveFailed(FName QuestID, FName ObjectiveID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsObjectiveFailed(ObjectiveID);
	}
	return false;
}

void USuqsProgression::ResetObjective(FName QuestID, FName ObjectiveID)
{
	if (auto Q = FindQuestState(QuestID))
	{
		Q->ResetObjective(ObjectiveID);
	}	
}

USuqsTaskState* USuqsProgression::GetNextMandatoryTask(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->GetNextMandatoryTask();
	}
	return nullptr;
	
}


bool USuqsProgression::IsTaskIncomplete(FName QuestID, FName TaskID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsTaskIncomplete(TaskID);
	}
	// Should default to TRUE not false if missing (assume incomplete if never accepted)
	return true;
}

bool USuqsProgression::IsTaskCompleted(FName QuestID, FName TaskID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsTaskCompleted(TaskID);
	}
	return false;
}

bool USuqsProgression::IsTaskFailed(FName QuestID, FName TaskID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsTaskFailed(TaskID);
	}
	return false;
}

bool USuqsProgression::IsTaskRelevant(FName QuestID, FName TaskID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsTaskRelevant(TaskID);
	}
	return false;
	
}

USuqsTaskState* USuqsProgression::GetTaskState(FName QuestID, FName TaskID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->GetTask(TaskID);
	}
	return nullptr;
	
}

void USuqsProgression::ResetTask(FName QuestID, FName TaskID)
{
	if (auto Q = FindQuestState(QuestID))
	{
		Q->ResetTask(TaskID);
	}
}

void USuqsProgression::SetQuestBranchActive(FName QuestID, FName Branch, bool bActive)
{
	if (auto Q = FindQuestState(QuestID))
	{
		Q->SetBranchActive(Branch, bActive);
	}
}


bool USuqsProgression::IsQuestBranchActive(FName QuestID, FName Branch)
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsBranchActive(Branch);
	}
	return false;
}

void USuqsProgression::SetGlobalQuestBranchActive(FName Branch, bool bActive)
{
	if (Branch.IsNone())
		return;

	bool bChanged = false;;
	if (bActive)
	{
		if (!GlobalActiveBranches.Contains(Branch))
		{
			GlobalActiveBranches.Add(Branch);
			bChanged = true;
		}
	}
	else
		bChanged = GlobalActiveBranches.Remove(Branch) > 0;

	if (bChanged)
	{
		// Copy into temporary list because this can change quest state and move between lists
		TArray<USuqsQuestState*> ListCopy;
		ActiveQuests.GenerateValueArray(ListCopy);
		for (auto Q : ListCopy)
		{
			Q->SetBranchActive(Branch, bActive);
		}
	}
}

void USuqsProgression::ResetGlobalQuestBranches()
{
	for (auto& Branch : GlobalActiveBranches)
	{
		// Copy into temporary list because this can change quest state and move between lists
		TArray<USuqsQuestState*> ListCopy;
		ActiveQuests.GenerateValueArray(ListCopy);
		for (auto Q : ListCopy)
		{
			Q->SetBranchActive(Branch, false);
		}
	}
	GlobalActiveBranches.Empty();
}

bool USuqsProgression::IsGlobalQuestBranchActive(FName Branch)
{
	// No branch is always active
	if (Branch.IsNone())
		return true;

	return GlobalActiveBranches.Contains(Branch);
}

const TArray<FName>& USuqsProgression::GetGlobalActiveQuestBranches() const
{
	return GlobalActiveBranches;
}

void USuqsProgression::SetGateOpen(FName GateName, bool bOpen)
{
	// Ignore nonsense
	if (GateName.IsNone())
		return;
	
	if (bOpen)
	{
		bool bWasAlreadyPresent;
		OpenGates.Add(GateName, &bWasAlreadyPresent);
		if (!bWasAlreadyPresent)
		{
			TArray<USuqsQuestState*> ActiveQuestList;
			// Need to copy since this change may cascade to completing quests
			ActiveQuests.GenerateValueArray(ActiveQuestList);
			for (auto Quest : ActiveQuestList)
			{
				Quest->NotifyGateOpened(GateName);
			}
		}
	}
	else
		OpenGates.Remove(GateName);
}

bool USuqsProgression::IsGateOpen(FName GateName)
{
	// No gate is always OK
	if (GateName.IsNone())
		return true;

	return OpenGates.Contains(GateName);
}

bool USuqsProgression::QuestDependenciesMet(const FName& QuestID)
{
	if (auto QuestDef = QuestDefinitions.Find(QuestID))
	{
		for (auto& RequiredCompletedID : QuestDef->PrerequisiteQuests)
		{
			if (!IsQuestCompleted(RequiredCompletedID))
				return false;
		}
		for (auto& RequiredFailedID : QuestDef->PrerequisiteQuestFailures)
		{
			if (!IsQuestFailed(RequiredFailedID))
				return false;
		}
		return true;
	}
	return false;
	
}

void USuqsProgression::AddParameterProvider(UObject* Provider)
{
	if (IsValid(Provider) && Provider->Implements<USuqsParameterProvider>())
	{
		const int PrevNum = ParameterProviders.Num();
		const int NewIdx = ParameterProviders.AddUnique(Provider);
		if (NewIdx >= PrevNum)
		{
			// This might cause active tasks to change their text
			OnParameterProvidersChanged.Broadcast(this);
		}
	}
	else
	{
		UE_LOG(LogSUQS, Error, TEXT("Provider passed to AddFormatter is either invalid or doesn't implement ISuqsParameterProvider, ignoring."))
	}
}

void USuqsProgression::RemoveParameterProvider(UObject* Provider)
{
	ParameterProviders.Remove(Provider);
}

void USuqsProgression::RemoveAllParameterProviders()
{
	ParameterProviders.Empty();
}

FText USuqsProgression::FormatQuestOrTaskText(const FName& QuestID, const FName& TaskID, const FText& FormatText)
{
	if (!IsValid(FormatParams))
		FormatParams = NewObject<USuqsNamedFormatParams>();
	else
		FormatParams->Empty();

	for (int i = 0; i < ParameterProviders.Num(); ++i)
	{
		auto F = ParameterProviders[i];
		if (F.IsValid())
		{
			ISuqsParameterProvider::Execute_GetQuestParameters(F.Get(), QuestID, TaskID, FormatParams);
		}
		else
		{
			// Weak pointer to deleted object, tidy up so these don't accumulate
			ParameterProviders.RemoveAt(i);
			--i;
		}
	}

	return FormatParams->Format(FormatText);
}

FText USuqsProgression::FormatQuestText(const FName& QuestID, const FText& FormatText)
{
	static FName NoTaskID;
	return FormatQuestOrTaskText(QuestID, NoTaskID, FormatText);

}

FText USuqsProgression::FormatTaskText(const FName& QuestID, const FName& TaskID, const FText& FormatText)
{
	return FormatQuestOrTaskText(QuestID, TaskID, FormatText);
}

bool USuqsProgression::GetTextNeedsFormatting(const FText& Text)
{
	// Determine whether there are any parameters in the text
	// This picks up both ordered and named, but we don't support ordered for simplicity (and it's generally a bad idea)
	TArray<FString> Params;
	FText::GetFormatPatternParameters(Text, Params);

	return Params.Num() > 0;
}

void USuqsProgression::RaiseTaskUpdated(USuqsTaskState* Task)
{
	// might be worth queuing these up and raising combined?
	if (!bSuppressEvents)
	{
		OnTaskUpdated.Broadcast(Task);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::TaskUpdated, Task));
		
		// A task that hasn't changed visibility but has changed status may need its waypoints enabling/disabling
		auto Waypoints = Task->GetWaypoints(false);
		for (auto W : Waypoints)
		{
			W->SetIsCurrent(Task->IsIncomplete());
		}
	}
}

void USuqsProgression::RaiseTaskCompleted(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
	{
		OnTaskCompleted.Broadcast(Task);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::TaskCompleted, Task));
	}
	// A task being completed means its waypoints are no longer needed
	auto Waypoints = Task->GetWaypoints(false);
	for (auto W : Waypoints)
	{
		W->SetIsCurrent(false);
	}
	
}

void USuqsProgression::RaiseTaskAdded(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
	{
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::TaskAdded, Task));
	}
	// A task being added means its waypoints become relevant, so we should get events for them being changed
	auto Waypoints = Task->GetWaypoints(false);
	for (auto W : Waypoints)
	{
		W->SetIsCurrent(true);

		// Take this opportunity to sub to events
		if (!bSubcribedToWaypointEvents)
		{
			const auto GI = UGameplayStatics::GetGameInstance(W);
			if (IsValid(GI))
			{
				auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
				Suqs->OnAnyWaypointMoved.AddDynamic(this, &USuqsProgression::OnWaypointMoved);
				Suqs->OnAnyWaypointEnabledChanged.AddDynamic(this, &USuqsProgression::OnWaypointEnabledChanged);
				bSubcribedToWaypointEvents = true;
			}
		}
	}
}

void USuqsProgression::RaiseTaskRemoved(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
	{
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::TaskRemoved, Task));
	}
	// A task being removed means its waypoints are no longer relevant
	auto Waypoints = Task->GetWaypoints(false);
	for (auto W : Waypoints)
	{
		W->SetIsCurrent(false);
	}
}

void USuqsProgression::RaiseTaskFailed(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
	{
		OnTaskFailed.Broadcast(Task);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::TaskFailed, Task));
	}
	// A task being failed means its waypoints are no longer needed
	auto Waypoints = Task->GetWaypoints(false);
	for (auto W : Waypoints)
	{
		W->SetIsCurrent(false);
	}
	
}


void USuqsProgression::RaiseObjectiveCompleted(USuqsObjectiveState* Objective)
{
	if (!bSuppressEvents)
	{
		OnObjectiveCompleted.Broadcast(Objective);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::ObjectiveCompleted, Objective));
	}
}

void USuqsProgression::RaiseObjectiveFailed(USuqsObjectiveState* Objective)
{
	if (!bSuppressEvents)
	{
		OnObjectiveFailed.Broadcast(Objective);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::ObjectiveFailed, Objective));
	}
}


void USuqsProgression::RaiseQuestCompleted(USuqsQuestState* Quest)
{
	if (!bSuppressEvents)
	{
		OnQuestCompleted.Broadcast(Quest);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestCompleted, Quest));
	}

	// We don't process changes caused by complete / fail until barriers resolved (see ProcessQuestStatusChange)
	
}

void USuqsProgression::RaiseQuestFailed(USuqsQuestState* Quest)
{
	if (!bSuppressEvents)
	{
		OnQuestFailed.Broadcast(Quest);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestFailed, Quest));
	}

	// We don't process changes caused by complete / fail until barriers resolved (see ProcessQuestStatusChange)
	
}

void USuqsProgression::RaiseQuestReset(USuqsQuestState* Quest)
{
	// Move quest to the correct list immediately, unlike complete / fail
	const int NumRemoved = QuestArchive.Remove(Quest->GetIdentifier());
	ActiveQuests.Add(Quest->GetIdentifier(), Quest);
	
	if (!bSuppressEvents)
	{
		// Always raise as new acceptance
		OnQuestAccepted.Broadcast(Quest);
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestAccepted, Quest));
		
		// Only raise list change if moved from archived
		if (NumRemoved > 0)
		{
			OnActiveQuestsListChanged.Broadcast();
			OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::ActiveQuestsChanged));
		}
	}
}

void USuqsProgression::RaiseCurrentObjectiveChanged(USuqsQuestState* Quest)
{
	if (!bSuppressEvents)
	{
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestCurrentObjectiveChanged, Quest));

		// Also raise events for every task which is now relevant
		if (const auto Obj = Quest->GetCurrentObjective())
		{
			TArray<USuqsTaskState*> Tasks;
			Obj->GetAllRelevantTasks(Tasks);
			for (const auto T : Tasks)
			{
				RaiseTaskAdded(T);
			}
		}
		
	}
}

void USuqsProgression::ProcessQuestStatusChange(USuqsQuestState* Quest)
{
	// Quest list and dependent quest acceptance is potentially delayed for completion / failed
	const ESuqsQuestStatus Status = Quest->GetStatus();
	if (Status == ESuqsQuestStatus::Completed ||
		Status == ESuqsQuestStatus::Failed)
	{
		// Move quest to the correct list
		ActiveQuests.Remove(Quest->GetIdentifier());
		QuestArchive.Add(Quest->GetIdentifier(), Quest);
		if (!bSuppressEvents)
		{
			OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::QuestArchived, Quest));
			// Raise this here for the purpose of archive quests
			// Auto-accepts will raise this again
			OnActiveQuestsListChanged.Broadcast();
			OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::ActiveQuestsChanged));
		}
		
		AutoAcceptQuests(Quest->GetIdentifier(), Status == ESuqsQuestStatus::Failed);
		
	}
}

const FSuqsQuest* USuqsProgression::GetQuestDefinition(const FName& QuestID)
{
	return QuestDefinitions.Find(QuestID);
}

FSuqsResolveBarrier USuqsProgression::GetResolveBarrierForTask(const FSuqsTask* Task,
	ESuqsTaskStatus Status) const
{
	FSuqsResolveBarrier Barrier;

	if (Status == ESuqsTaskStatus::Completed ||
		Status == ESuqsTaskStatus::Failed)
	{
		if (DefaultTaskResolveTimeDelay > 0)
		{
			Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Time);
			Barrier.TimeRemaining = DefaultTaskResolveTimeDelay;
		}

		if (Task->ResolveDelay >= 0) // >= because default is -1, so that 0 can override >0 default
		{
			Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Time);
			Barrier.TimeRemaining = Task->ResolveDelay;
		}
		if (!Task->ResolveGate.IsNone())
		{
			Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Gate);
			Barrier.Gate = Task->ResolveGate;
		}
		if (!Task->bResolveAutomatically)
		{
			Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Explicit);
		}
	}
	
	// Always pending, even if no condition, since need to raise event once
	Barrier.bPending = true;
	return Barrier;
}

FSuqsResolveBarrier USuqsProgression::GetResolveBarrierForQuest(const FSuqsQuest* Quest, ESuqsQuestStatus Status) const
{
	FSuqsResolveBarrier Barrier;

	if (DefaultQuestResolveTimeDelay > 0)
	{
		Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Time);
		Barrier.TimeRemaining = DefaultQuestResolveTimeDelay;
	}
	if (Quest->ResolveDelay >= 0) // >= because default is -1, so that 0 can override >0 default
	{
		Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Time);
		Barrier.TimeRemaining = Quest->ResolveDelay;
	}
	if (!Quest->ResolveGate.IsNone())
	{
		Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Gate);
		Barrier.Gate = Quest->ResolveGate;
	}
	if (!Quest->bResolveAutomatically)
	{
		Barrier.Conditions |= static_cast<int>(ESuqsResolveBarrierCondition::Explicit);
	}

	// Always pending, even if no condition, since need to raise event once
	Barrier.bPending = true;
	return Barrier;
}

// FTickableGameObject start
void USuqsProgression::Tick(float DeltaTime)
{
	// Copy into temporary list because ticking can fail quests and alter the collection
	TArray<USuqsQuestState*> ListCopy;
	ActiveQuests.GenerateValueArray(ListCopy);
	for (auto Q : ListCopy)
	{
		Q->Tick(DeltaTime);
	}
}

TStatId USuqsProgression::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USuqsStatus, STATGROUP_Tickables);
}

UDataTable* USuqsProgression::MakeQuestDataTableFromJSON(const FString& JsonString)
{
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->bIgnoreMissingFields = true;
	QuestTable->ImportKeyField = "Identifier";
	QuestTable->CreateTableFromJSONString(JsonString);

	return QuestTable;
}

FString USuqsProgression::GetProgressEventDescription(const FSuqsProgressionEventDetails& Evt)
{
	const FString Empty("");
	return FString::Printf(TEXT("%s : %s : %s%s"), *StaticEnum<ESuqsProgressionEventType>()->GetValueAsString(Evt.EventType),
	                       Evt.Quest ? *Evt.Quest->GetTitle().ToString() : *Empty,
	                       Evt.Objective ? *Evt.Objective->GetTitle().ToString() : *Empty,
	                       Evt.Task ? *Evt.Task->GetTitle().ToString() : *Empty);
}

// FTickableGameObject end

void USuqsProgression::Serialize(FArchive& Ar)
{
	FSuqsSaveData Data;
	if (Ar.IsLoading())
	{
		Data.Serialize(Ar);
		// Give hook the opportunity to fix up data
		OnPreLoad.ExecuteIfBound(this, Data);

		LoadFromData(Data);

		OnProgressionLoaded.Broadcast(this);
		
	}
	else
	{
		SaveToData(Data);
		Data.Serialize(Ar);
	}
	 	
}

void USuqsProgression::LoadFromData(const FSuqsSaveData& Data)
{
	// Save / load from data is deliberately self-contained here in progression and not distributed around
	// the quest / objective / task classes. Partly this is because we compress out Objectives in the save data
	// and partly it's because it's just easier to follow, considering it's only a few lines of code
	ActiveQuests.Empty();
	QuestArchive.Empty();
	GlobalActiveBranches.Empty();
	OpenGates.Empty();

	bSuppressEvents = true;
	
	for (auto& QData : Data.QuestData)
	{
		if (auto QDef = GetQuestDefinition(FName(QData.Identifier)))
		{
			auto Q = NewObject<USuqsQuestState>(GetOuter());
			// This will re-create the quest structure, including objectives and tasks, based on *current* definition
			Q->Initialise(QDef, this);
			Q->StartLoad();

			for (FString Branch : QData.ActiveBranches)
			{
				Q->SetBranchActive(FName(Branch), true);
			}

			for (auto& TData : QData.TaskData)
			{
				// Discard task state which isn't in the quest any more
				if (auto T = Q->GetTask(FName(TData.Identifier)))
				{
					T->SetNumber(TData.Number);
					T->SetTimeRemaining(TData.TimeRemaining);
					// It's important this is done LAST, because completion triggered from the above can generate
					// a new barrier
					T->SetResolveBarrier(TData.ResolveBarrier);
				}		
			}

			// Again, set the resolve barrier last to ensure we overwrite any new generated one
			Q->SetResolveBarrier(QData.ResolveBarrier);

			Q->FinishLoad();

            if (QData.Status == ESuqsQuestDataStatus::Incomplete)
            	ActiveQuests.Add(QDef->Identifier, Q);
			else
			{
				// Manually set the status, in case this isn't borne out by the current quest def
				Q->OverrideStatus(QData.Status == ESuqsQuestDataStatus::Failed ?
					ESuqsQuestStatus::Failed : ESuqsQuestStatus::Completed);

				QuestArchive.Add(QDef->Identifier, Q);
			}
		}
		else
		{
			UE_LOG(LogSUQS, Warning, TEXT("Ignoring saved quest data for %s because that quest no longer exists"), *QData.Identifier);
		}
		
	}

	// Now load global branches
	for (FString Branch : Data.GlobalActiveBranches)
	{
		SetGlobalQuestBranchActive(FName(Branch), true);
	}
	for (FString Gate : Data.OpenGates)
	{
		SetGateOpen(FName(Gate), true);
	}
	

	bSuppressEvents = false;
}

void USuqsProgression::SaveToData(FSuqsSaveData& Data) const
{
	Data.Version = SuqsCurrentDataVersion;
	Data.QuestData.Empty();
	Data.GlobalActiveBranches.Empty();
	Data.OpenGates.Empty();
	
	for (FName Branch : GlobalActiveBranches)
	{
		Data.GlobalActiveBranches.Add(Branch.ToString());		
	}
	for (FName Gate : OpenGates)
	{
		Data.OpenGates.Add(Gate.ToString());		
	}
	SaveToData(ActiveQuests, Data);
	SaveToData(QuestArchive, Data);
}

void USuqsProgression::SaveToData(TMap<FName, USuqsQuestState*> Quests, FSuqsSaveData& Data)
{
	for (auto Pair : Quests)
	{
		auto Q = Pair.Value;
		auto& QData = Data.QuestData.Emplace_GetRef();
		
		QData.Identifier = Q->GetIdentifier().ToString();
		// Status is kept as a separate enum for future insulation
		switch (Q->Status)
		{
		case ESuqsQuestStatus::Incomplete:
			QData.Status = ESuqsQuestDataStatus::Incomplete;
			break;
		case ESuqsQuestStatus::Completed:
			QData.Status = ESuqsQuestDataStatus::Completed;
			break;
		case ESuqsQuestStatus::Failed:
			QData.Status = ESuqsQuestDataStatus::Failed;
			break;
		default: ;
		}

		for (auto Branch : Q->GetActiveBranches())
		{
			QData.ActiveBranches.Add(Branch.ToString());
		}

		QData.ResolveBarrier = Q->ResolveBarrier;

		for (auto O : Q->Objectives)
		{
			for (auto T : O->GetTasks())
			{
				auto& TData = QData.TaskData.Emplace_GetRef();
				TData.Identifier = T->GetIdentifier().ToString();
				TData.Number = T->GetNumber();
				TData.TimeRemaining = T->GetTimeRemaining();
				TData.ResolveBarrier = T->GetResolveBarrier();
			}
		}
	}
}

void USuqsProgression::OnWaypointMoved(USuqsWaypointComponent* Waypoint)
{
	if (!bSuppressEvents)
	{
		const auto Task = GetTaskState(Waypoint->GetQuestID(), Waypoint->GetTaskID());
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::WaypointMoved, Waypoint, Task));
	}
	
}

void USuqsProgression::OnWaypointEnabledChanged(USuqsWaypointComponent* Waypoint)
{
	if (!bSuppressEvents)
	{
		const auto Task = GetTaskState(Waypoint->GetQuestID(), Waypoint->GetTaskID());
		OnProgressionEvent.Broadcast(FSuqsProgressionEventDetails(ESuqsProgressionEventType::WaypointEnabledOrDisabled, Waypoint, Task));
	}
}
//PRAGMA_ENABLE_OPTIMIZATION