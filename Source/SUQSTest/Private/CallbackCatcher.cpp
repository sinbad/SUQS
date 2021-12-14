#include "CallbackCatcher.h"

void UCallbackCatcher::Subscribe(USuqsProgression* Progression)
{
	Progression->OnQuestAccepted.AddDynamic(this, &UCallbackCatcher::OnQuestAccepted);
	Progression->OnQuestFailed.AddDynamic(this, &UCallbackCatcher::OnQuestFailed);
	Progression->OnQuestCompleted.AddDynamic(this, &UCallbackCatcher::OnQuestCompleted);
	
	Progression->OnObjectiveFailed.AddDynamic(this, &UCallbackCatcher::OnObjectiveFailed);
	Progression->OnObjectiveCompleted.AddDynamic(this, &UCallbackCatcher::OnObjectiveCompleted);

	Progression->OnTaskUpdated.AddDynamic(this, &UCallbackCatcher::OnTaskUpdated);
	Progression->OnTaskFailed.AddDynamic(this, &UCallbackCatcher::OnTaskFailed);
	Progression->OnTaskCompleted.AddDynamic(this, &UCallbackCatcher::OnTaskCompleted);

	Progression->OnProgressionEvent.AddDynamic(this, &UCallbackCatcher::OnProgression);
	
}
