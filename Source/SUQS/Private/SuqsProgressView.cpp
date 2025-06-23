
#include "SuqsProgressView.h"

#include "SuqsProgression.h"

FSuqsTaskStateView::FSuqsTaskStateView(): CompletedNumber(0), bHidden(false)
{
}

void FSuqsTaskStateView::FromUObject(USuqsTaskState* State)
{
	Identifier = State->GetIdentifier();
	Title = State->GetTitle();
	bMandatory = State->IsMandatory();
	TargetNumber = State->GetTargetNumber();
	CompletedNumber = 0;
	TimeRemaining = State->GetTimeRemaining();
	Status = State->GetStatus();
	bHidden = State->GetHidden();
	
}

FSuqsQuestStateView::FSuqsQuestStateView()
{
}

void FSuqsQuestStateView::FromUObject(USuqsQuestState* State)
{
	Identifier = State->GetIdentifier();
	Labels = State->GetLabels();
	Title = State->GetTitle();

	Description = State->GetDescription();
	CurrentObjectiveIdentifier = State->GetCurrentObjective()->GetIdentifier();
	CurrentObjectiveDescription = State->GetCurrentObjective()->GetDescription();

	auto& Tasks = State->GetCurrentObjective()->GetTasks();
	CurrentTasks.Reset(Tasks.Num());
	for (USuqsTaskState* TaskState : Tasks)
	{
		auto& Task = CurrentTasks.AddDefaulted_GetRef();
		Task.FromUObject(TaskState);
	}
}

FSuqsProgressView::FSuqsProgressView()
{
}

void FSuqsProgressView::FromUObject(USuqsProgression* State)
{
	TArray<USuqsQuestState*> QuestStates;
	State->GetAcceptedQuests(QuestStates);
	ActiveQuests.Reset(QuestStates.Num());

	for (USuqsQuestState* QuestState : QuestStates)
	{
		if (QuestState->IsPlayerVisible())
		{
			auto& Quest = ActiveQuests.AddDefaulted_GetRef();
			Quest.FromUObject(QuestState);
		}
	}
	// If not player visible, ignore quest
}


