# Objectives

Objectives are the backbone of a quest. They define the major sequence(s) of the
quest, and how it progresses from the start to an eventual conclusion.

Objectives don't cover the individual actions you do to progress the quest, that
is handled by child [Tasks](Tasks.md). Objectives give everything structure and
group potentially multiple tasks together in an overal sub-goal. Objectives are
implicitly completed when mandatory child tasks are completed.

Objectives are always sequential. Thus, a quest will typically run down the list of
objectives from top to bottom; however there can be [branching](Branching.md) within that.
There will always be ONE 'next objective' in a quest, although that may be
comprised of multiple tasks.

As a grouping level, Objectives allow you to combine multiple tasks in different
ways, depending on the Objective attributes. 

## Completing Objectives

Objectives are completed when the *required number of mandatory tasks* within them
are completed. By default, that's "all mandatory tasks". However you can also
tell an objective that completing just one of the mandatory tasks within it fulfils
the objective, or an arbitrary number out of the ones defined. 

Optional tasks never complete objectives, they're just a bonus.

## Failing Objectives

An objective fails when it's impossible to complete it any more given the 
number of mandatory tasks left unresolved, and the number of mandatory
tasks required to complete it. At the simplest level, that means if an objective
requires all mandatory tasks to be completed (the default), then failing *any* mandatory
task fails the objective.

When the current objective fails, the quest fails. How you handle that is up to
you; that could be the end of it, or you could have other fallback quests 
automatically be accepted (see Auto-Accept in [Quests](Quests.md)). Or you could
reset the quest back to the beginning, or even activate another [quest branch](Branching.md)
within the same quest.


## Sequential and Non-sequential Tasks

One option on Objective is whether the mandatory tasks contained within it need to be
completed in order (default true). Calls to complete sequential tasks out of order 
will be ignored. This can let you simplify your world triggers so that they can
always send the message to complete a task, but it won't actually progress the 
quest unless that task is the next one to be done.

Non-sequential tasks can be done in any order, so long as the Objective is active.
Again, any calls to complete the task while the Objective isn't the current one
in the quest will be ignored.

Optional tasks can always be completed at any time; if you need them to be 
ordered then use Objectives to sequence that.

## Objective Identifiers

Like Quests, Objectives can have identifiers which must be unique within the Quest
if supplied. However, you only need to assign an Identifier to an Objective if 
you need one to perform queries later - e.g. you need to know whether/when a
specific Objective is completed in full or failed, rather than individual tasks or the whole
quest.

If you don't need to know this, you can leave the Identifier out of your definition
(it will default to "None").

## Objective Titles and Descriptions

These are optional, and dependent on your quest UI needs, but if you want to
display the collective name of the Objective to the player, or have it add extra lines to
the quest description, then you can supply these in the JSON. They're only used
by any quest UI you build.

### Progression Delay

When this objective completes or fails, a time delay can be added before the knock-on effects
of this are resolved, such as activating the next objective, or completing/failing the quest.

It defaults to -1 which means to not add a specific delay to this quest, and to use whatever
defaults are set in [Progression](Progression.md).

### Progression Gate

Much like Progression Delay, this adds a delay between completing/failing the objective, and
the knock-on effects; except this time the delay is user-controlled, and will only be
activated when the named "Gate" is opened on [Progression](Progression.md).

This lets you precisely control when the next steps in the quest line are activated, instead
of happening immediately on completion/failure, which can be useful for narrative pacing.


## More info

* [Tasks](Tasks.md)
* [Branching Objectives](Branching.md)