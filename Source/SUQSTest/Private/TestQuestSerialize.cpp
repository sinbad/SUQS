#include "CallbackCatcher.h"
#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"
#include "TestQuestData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestSerialize, "SUQSTest.QuestSerialize",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestSerialize::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	// Add a decent number of quest defs
	// note that only the ones we accept will be saved
	constexpr int NumQuestTables = 9;
	FString JsonStrings[NumQuestTables]
	{
		SimpleMainQuestJson,
		SimpleSideQuestJson,
		SmallestPossibleQuestJson,
		OrderedTasksQuestJson,
		UnorderedTasksQuestJson,
		AnyOfTasksQuestJson,
		TargetNumberQuestJson,
		BranchingQuestJson,
		TimeLimitQuestJson
	};

	for (int i = 0; i < NumQuestTables; ++i)
	{
		Progression->QuestDataTables.Add(USuqsProgression::MakeQuestDataTableFromJSON(JsonStrings[i]));
	}

	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Main1"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Side1"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Smol"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Ordered"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Unordered"));
	// Do NOT accept Q_Any2Of to test that it's not in the save
	//TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Any2Of"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_TargetNumbers"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Branching"));
	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_TimeLimits"));
	
	// Complete simple quest
	TestTrue("Complete smol task should work", Progression->CompleteTask("Q_Smol", "T_Smol"));
	
	// Fail ordered quest, but complete all but the last task
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Ordered", "T_1"));
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Ordered", "T_2"));
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Ordered", "T_3"));
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Ordered", "T_11"));
	Progression->FailTask("Q_Ordered", "T_12");

	// Partially complete main quest
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Main1", "T_ReachThePlace"));
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Main1", "T_DoTheThing"));
	TestTrue("Complete ordered task should work", Progression->CompleteTask("Q_Main1", "T_Something1"));
	// This leaves one task incomplete
	// Don't progress Q_Side1 at all
	// Partially progress target number task
	// First task gets completed
	TestEqual("Remaining number should be correct", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf1", 1), 0);
	// Second task is partial
	TestEqual("Remaining number should be correct", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf3", 2), 1);
	// Tick so that the time limited task changes a little
	Progression->Tick(10);

	// Set some branches both globally and on branching quest
	Progression->SetGlobalQuestBranchActive("BranchB", true);
	Progression->SetGlobalQuestBranchActive("BranchNonExistent", true);
	Progression->SetQuestBranchActive("Q_Branching", "BranchA", true);
	Progression->SetQuestBranchActive("Q_Branching", "BranchDoesntExist", true);

	// OK save all this
	TArray<uint8> Data;
	FMemoryWriter Writer(Data);
	Progression->Serialize(Writer);
	TestTrue("Serialize should have written some data", Data.Num() > 0);
	

	// Read the data back into a new progression
	USuqsProgression* LoadedProgression = NewObject<USuqsProgression>();
	// Need to set up the quest definitions separately, those aren't loaded
	for (int i = 0; i < NumQuestTables; ++i)
	{
		LoadedProgression->QuestDataTables.Add(USuqsProgression::MakeQuestDataTableFromJSON(JsonStrings[i]));				
	}

	// register listeners to confirm we don't get events during loading
	UCallbackCatcher* CallbackCatcher = NewObject<UCallbackCatcher>();
	CallbackCatcher->Subscribe(LoadedProgression);
	
	FMemoryReader Reader(Data);
	LoadedProgression->Serialize(Reader);

	TestEqual("Should not have received any callbacks during load", CallbackCatcher->AcceptedQuests.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->CompletedObjectives.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->CompletedQuests.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->CompletedTasks.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->FailedObjectives.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->FailedQuests.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->FailedTasks.Num(), 0);
	TestEqual("Should not have received any callbacks during load", CallbackCatcher->UpdatedTasks.Num(), 0);

	TArray<USuqsQuestState*> OrigAcceptedQuests;
	TArray<USuqsQuestState*> LoadedAcceptedQuests;
	
	// Test everything is back to how it was
	// Global branches
	TestEqual("Global branches should be the same", LoadedProgression->GetGlobalActiveQuestBranches(), Progression->GetGlobalActiveQuestBranches());

	// Quests
	// Note: deliberately not refactoring this into functions because it's much easier to trace failures when unrolled
	Progression->GetAcceptedQuests(OrigAcceptedQuests);
	LoadedProgression->GetAcceptedQuests(LoadedAcceptedQuests);
	TestEqual("Should be the same number of accepted quests", LoadedAcceptedQuests.Num(), OrigAcceptedQuests.Num());
	// We can't guarantee what order quests will be in, so iterate originals and look up
	for (auto OrigQ : OrigAcceptedQuests)
	{
		auto LoadedQ = LoadedProgression->GetQuest(OrigQ->GetIdentifier());
		FString QContext = FString::Printf(TEXT("Quest %s"), *OrigQ->GetIdentifier().ToString());
		TestNotNull(FString::Printf(TEXT("Should have found %s in loaded version"), *QContext), LoadedQ);
		if (LoadedQ)
		{
			TestEqual(QContext + " status should match", LoadedQ->GetStatus(), OrigQ->GetStatus());
			TestEqual(QContext + " active branches should match", LoadedQ->GetActiveBranches(), OrigQ->GetActiveBranches());
			
			TestEqual(QContext + " objectives count wrong", LoadedQ->GetObjectives().Num(), OrigQ->GetObjectives().Num());
			if (LoadedQ->GetObjectives().Num() == OrigQ->GetObjectives().Num())
			{
				// Objectives and tasks ordering *is* predictable
				for (int ObjIdx = 0; ObjIdx < OrigQ->GetObjectives().Num(); ++ObjIdx)
				{
					auto OrigO = OrigQ->GetObjectives()[ObjIdx];
					auto LoadedO = LoadedQ->GetObjectives()[ObjIdx];
					FString OContext = FString::Printf(TEXT("Quest %s objective %s"), *OrigQ->GetIdentifier().ToString(), *OrigO->GetIdentifier().ToString());

					TestEqual(OContext + " identifier wrong", LoadedO->GetIdentifier(), OrigO->GetIdentifier());
					TestEqual(OContext + " status wrong", LoadedO->GetStatus(), OrigO->GetStatus());
					TestEqual(OContext + " should be the same number of tasks", LoadedO->GetTasks().Num(), OrigO->GetTasks().Num());
					if (LoadedO->GetTasks().Num() == OrigO->GetTasks().Num())
					{
						for (int TaskIdx = 0; TaskIdx < OrigO->GetTasks().Num(); ++TaskIdx)
						{
							auto OrigT = OrigO->GetTasks()[TaskIdx];
							auto LoadedT = LoadedO->GetTasks()[TaskIdx];
							FString TContext = FString::Printf(TEXT("Quest %s objective %s task %s"),
								*OrigQ->GetIdentifier().ToString(), *OrigO->GetIdentifier().ToString(), *OrigT->GetIdentifier().ToString());

							TestEqual(TContext + " identifier wrong", LoadedT->GetIdentifier(), OrigT->GetIdentifier());
							TestEqual(TContext + " status wrong", LoadedT->GetStatus(), OrigT->GetStatus());
							TestEqual(TContext + " number wrong", LoadedT->GetNumber(), OrigT->GetNumber());
							TestEqual(TContext + " number wrong", LoadedT->GetTimeRemaining(), OrigT->GetTimeRemaining());
							TestEqual(TContext + " hidden wrong", LoadedT->GetHidden(), OrigT->GetHidden());

						}
					}

				}
			}
			
		}
	}
	
	
	
	return true;
}
