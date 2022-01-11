# Tasks

Tasks are the primary way that a player makes [progress](Progression.md) on quests. They are grouped
underneath [Objectives](Objectives.md) in order to provide more flexibility
around whether the lowest level tasks need to be completed in order, or whether
all of them need to be completed or not to deem the whole objective as complete.

## Definition of a Task

A task represents either a single action the player needs to complete (e.g. 
reach a location), or a number of identical actions (e.g. kill 5 boars).

Once a task is completed or failed, its parent Objective will evaluate whether
that change completes or fails the Objective as a whole or not, and the change
"bubbles up" as necessary, potentially completing or failing the entire quest.

A task's parent [Objective](Objectives.md) determines whether the tasks need
to be completed in order (sequentially) or not, and whether all mandatory tasks need 
to be completed or just a subset (at least X).

## Task Attributes

### Identifier

Tasks must have identifiers, which must be unique within the quest. When you're
triggering the completion or failure of task from something in the world, 
you need to know both the quest identifier and the task identifier.

### Title

A player-visible description of the task that needs doing. As with other text
attributes, you can localise this with the NSLOCTEXT macro:

```json
"Title": "NSLOCTEXT(\"Namespace\", \"Key\", \"Default Text\")",
```

### bMandatory

Whether this task is mandatory or not (default true). If false, this task does 
not contribute to the success or failure of the parent Objective. If true 
(the default), then this task will contribute to success or failure; if the
parent Objective requires that all mandatory tasks are completed, then its failure
will fail the objective. 

It's possible to say at the [Objective](Objectives.md) level that only
a certain number of mandatory tasks need to be completed (e.g. you have to complete
1 of 2 mandatory tasks, so failing one doesn't fail the objective yet). But by
default all mandatory tasks have to be completed.

### Target Number

This is used when you need a task to represent performing a number of identical
tasks, to reach a threshold which completes the task, e.g. "Skin 3 deer".
Rather than completing the task outright, triggers in the world add [progress](Progression.md) to
the task, which automatically completes when it hits this target number.

### Time Limit

Use this to set time limits on tasks. The timer ticks automatically, and if it
expires the task will be automatically failed. 

The timer starts as soon as the Task becomes available to complete. For sequential
tasks, this will be when the task becomes the next in the list, or for non-sequential
tasks it will be when the parent Objective becomes current. You can reset the
timer by resetting the task.

### Resolve Automatically

Whether to automatically resolve the outcome of this task completing / failing,
such as activating the next task, or completing/failing the quest. Defaults to true.
If set to false, this task will only be resolved after a call to `ResolveTask`.
Combines with Resolve Delay and Resolve Gate below.

This lets you precisely control when the next steps in the quest line are activated, instead
of happening immediately on completion/failure, which can be useful for narrative pacing.

### Resolve Delay

When this objective completes or fails, a time delay can be added before the knock-on effects
of this are resolved, such as activating the next task, or completing/failing the quest.

It defaults to -1 which means to not add a specific delay to this quest, and to use whatever
defaults are set in [Progression](Progression.md).

Combines with Resolve Automatically and Resolve Gate - all must be cleared to resolve.

### Resolve Gate

Much like Resolve Delay, this adds a delay between completing/failing the task, and
the knock-on effects; except this time the delay is user-controlled, and will only be
activated when the named "Gate" is opened on [Progression](Progression.md).

Combines with Resolve Automatically and Resolve Delay - all must be cleared to resolve.


## Task Progression

This covers how tasks are defined, see [Progression](Progression.md) for more 
information on how tasks are progressed.

## Waypoints

It's possible to associate a world location (or locations) with a task, by
using [Waypoints](Waypoints.md).


## More Info

* [Waypoints](Waypoints.md)
* [Objectives](Objectives.md)
* [Quest Progression](Progression.md)
* [Changing Quest Definitions](ChangingQuestDefinitions.md)
