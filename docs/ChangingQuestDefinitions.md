# Quest Definition Changes

As your game evolves, you're going to want to make changes to the quests. But, 
what if you already have save games with quest data in them?

The [saved data](Saving.md) for quest [progression](Progression.md) does *not* 
include the quest definitions, merely the references to quest and task identifiers.
The save doesn't even store objectives; the state for those is derived from the 
saved task state. 

Therefore, you *can* alter quests during development without breaking save games,
so long as you understand the impact.

## Changes which are always safe

The following changes to the [quest library](Quests.md) are always safe to make and won't
break any save games:

* Adding a new quest
* Removing a quest which has never been accepted, or if accepted, was never progressed

## Changes which are usually safe

* Adding new optional tasks
* Adding alternate branches to a quest, which are NOT activated for existing saves

These 2 are usually safe because the save will simply fill in the data that's 
already there, and any new parts will just get their default state. It may
change what the player sees in their quest log if the quest is active, but not
in any breaking way. 

## Changes which MAY cause problems

* Adding or removing objectives
* Adding or removing mandatory tasks
* Changing tasks from mandatory to optional or vice versa
* Changing whether tasks are sequential or not
* Changing the number of tasks required to complete an objective
* Adding new branches which are activated for existing saves

Doing any of these things won't break the save game, but it will mean that
the quest structure will change. Any saved data will simply fill in what matches
in the new quest structure. 

If a quest is still active (not completed / failed), then the quest will be
re-evaluated based on the new structure. This could mean that the player's position
in the quest could jump to a different place, if for example a new Objective was
added *before* the step they're currently on. The quest will require that they
do this new Objective before getting back to where they were. So this could be
confusing, and potentially even impossible if the player can't do that because the
rest of the world has changed state. So be careful with these.

If a quest is no longer active (completed or failed), then it will remain in
that state, and won't be re-opened if you add new things to do. However, querying
the quest archive may be problematic because the quest will have potentially been
completed in such a way that is *now* no longer considered full completion.

## Changes which WILL break a save

* Changing a quest identifier
* Changing a task identifier
* Deleting a quest

These straight-up lose data and you should avoid doing them.

## Avoiding breaking changes

### Option 1: Parallel Quests

One way to deal with breaking changes to quest definitions is 
to create a new quest instead, with a new Identifier, to replace it. 
You essentially deprecate the old quest and only accept the new one in future games,
but leave the old one in the quest library with the original Identifier.
Players who are part way through the old quest can complete it as before (you 
should leave all your existing triggers for the old quest in place, 
but add new ones for the new quest as well), but players reaching that quest for 
the first time get the new quest. 

### Option 2: Data Migration

An alternative is to migrate existing saves over to the new quest structure more 
explicitly. You will still want to change the quest identifier, just because it's
the easiest way to detect whether you *need* to perform the upgrade. But the old
old quest doesn't need to remain in the library, and you can remove triggers in 
the world that refer to it, which is a bit tidier.

The key is to subscribe to the `USuqsProgression::OnPreLoad` event. This fires after 
quest data has been loaded into memory, but before it's been combined with the 
quest library to establish the actual quest state. You are able to change any of
this data before that happens. 

So, if you wanted to make breaking changes to quest "Q_Main", you would 
change its identifier to "Q_Main_v2" after making them. Then in the `OnPreLoad` callback, you 
detect the presence of active state for "Q_Main" but not "Q_Main_v2" - this indicates you
need to upgrade some old quest data. You can restructure the state, rename it to "Q_Main_v2" 
changing data however you like, and then that data will be loaded instead. 
This way you can have a completely custom upgrade procedure for quest data
if you need one.
