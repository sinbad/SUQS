#include "SuqsProgression.h"

#include <algorithm>


#include "EngineUtils.h"
#include "Suqs.h"
#include "SuqsObjectiveState.h"
#include "SuqsQuestState.h"
#include "SuqsTaskState.h"

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
                	UE_LOG(LogSUQS, Error, TEXT("Quest ID '%s' has been used more than once! Duplicate entry was in %s"), *Quest.Identifier.ToString(), *Table->GetName());

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
            });
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

			Quest->Reset();
		}
		else
		{
			// New quest
			Quest = NewObject<USuqsQuestState>(this);
			Quest->Initialise(QDef, this);
			ActiveQuests.Add(QuestID, Quest);

			// Propagate global quest branches
			for (auto& Branch : GlobalActiveBranches)
			{
				Quest->SetBranchActive(Branch, true);
			}
		}

		if (!bSuppressEvents)
			OnQuestAccepted.Broadcast(Quest);
		return true;
	}
	else
	{
		UE_LOG(LogSUQS, Error , TEXT("Attempted to accept a non-existent quest %s"), *QuestID.ToString());
		return false;
	}
	
}


void USuqsProgression::AutoAcceptQuests(const FName& FinishedQuestID, bool bFailed)
{
	TArray<FName> DependentQuestIDs;
	if (bFailed)
		QuestFailureDeps.MultiFind(FinishedQuestID, DependentQuestIDs);
	else
		QuestCompletionDeps.MultiFind(FinishedQuestID, DependentQuestIDs);
	
	for (const auto& DepQuestID : DependentQuestIDs)
	{
		if (!IsQuestAccepted(DepQuestID) && QuestDependenciesMet(DepQuestID))
			AcceptQuest(DepQuestID);
	}
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
		return bCompleted;
	}
	else
	{
		auto T = FindTaskStatus(QuestID, TaskIdentifier);
		if (T)
		{
			return T->Complete();
		}
	}
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

bool USuqsProgression::IsQuestIncomplete(FName QuestID) const
{
	if (auto Q = FindQuestState(QuestID))
	{
		return Q->IsIncomplete();
	}
	return false;
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
	return false;
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

void USuqsProgression::RaiseTaskUpdated(USuqsTaskState* Task)
{
	// might be worth queuing these up and raising combined?
	if (!bSuppressEvents)
		OnTaskUpdated.Broadcast(Task);
}

void USuqsProgression::RaiseTaskCompleted(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
		OnTaskCompleted.Broadcast(Task);
}
void USuqsProgression::RaiseTaskFailed(USuqsTaskState* Task)
{
	if (!bSuppressEvents)
		OnTaskFailed.Broadcast(Task);
}


void USuqsProgression::RaiseObjectiveCompleted(USuqsObjectiveState* Objective)
{
	if (!bSuppressEvents)
		OnObjectiveCompleted.Broadcast(Objective);
}

void USuqsProgression::RaiseObjectiveFailed(USuqsObjectiveState* Objective)
{
	if (!bSuppressEvents)
		OnObjectiveFailed.Broadcast(Objective);
}


void USuqsProgression::RaiseQuestCompleted(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	ActiveQuests.Remove(Quest->GetIdentifier());
	QuestArchive.Add(Quest->GetIdentifier(), Quest);

	if (!bSuppressEvents)
		OnQuestCompleted.Broadcast(Quest);

	AutoAcceptQuests(Quest->GetIdentifier(), false);
}

void USuqsProgression::RaiseQuestFailed(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	ActiveQuests.Remove(Quest->GetIdentifier());
	QuestArchive.Add(Quest->GetIdentifier(), Quest);

	if (!bSuppressEvents)
		OnQuestFailed.Broadcast(Quest);

	AutoAcceptQuests(Quest->GetIdentifier(), true);
}

void USuqsProgression::RaiseQuestReset(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	QuestArchive.Remove(Quest->GetIdentifier());
	ActiveQuests.Add(Quest->GetIdentifier(), Quest);
	
	if (!bSuppressEvents)
		OnQuestAccepted.Broadcast(Quest);
}

const FSuqsQuest* USuqsProgression::GetQuestDefinition(const FName& QuestID)
{
	return QuestDefinitions.Find(QuestID);
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

	bSuppressEvents = true;
	
	for (auto& QData : Data.QuestData)
	{
		if (auto QDef = GetQuestDefinition(FName(QData.Identifier)))
		{
			auto Q = NewObject<USuqsQuestState>();
			// This will re-create the quest structure, including objectives and tasks, based on *current* definition
			Q->Initialise(QDef, this);

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
				}		
			}
			
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
	

	bSuppressEvents = false;
}

void USuqsProgression::SaveToData(FSuqsSaveData& Data) const
{
	Data.QuestData.Empty();
	Data.GlobalActiveBranches.Empty();
	for (FName Branch : GlobalActiveBranches)
	{
		Data.GlobalActiveBranches.Add(Branch.ToString());		
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

		for (auto O : Q->Objectives)
		{
			for (auto T : O->GetTasks())
			{
				auto& TData = QData.TaskData.Emplace_GetRef();
				TData.Identifier = T->GetIdentifier().ToString();
				TData.Number = T->GetNumber();
				TData.TimeRemaining = T->GetTimeRemaining();
			}
		}
	}
}

