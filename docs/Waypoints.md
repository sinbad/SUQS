# Waypoints

Everything else in SUQS is merely state data and has no connection with any
world objects. However, often you'll want to put markers in the world to guide
players to the location of tasks in the world; these are called Waypoints.

SUQS provides a few helpful tools in this area; you don't have to use them,
but they can be convenient.

## Waypoint component

SUQS contains a scene component subclass called `USuqsWaypointComponent`. 
If you attach this to an actor in your scene, then it will connect SUQS tasks to 
locations in the world.

Here's how you configure it:

![Waypoint Config](img/waypointdetails.png)

You need to supply a Quest ID and Task ID at a minimum, neither of which can be
changed at runtime. You can associate multiple waypoints with a single task.

## Waypoint Actors

You can attach a waypoint component to any actor, but for convenience SUQS provides
a base class called `ASuqsWaypointActor`, which contains both a waypoint component,
and a `UWidgetComponent` which will display the location of the waypoint on screen.

You only have to provide the QuestID, TaskID and provide a user widget of your 
choice to represent the waypoint visually. Everything else is done for you;
`ASuqsWaypointActor` is smart enough to only display the waypoint when the task
it's associated with becomes active, and when the waypoint itself is enabled. 
So you can place these actors in the world, attach them to other actors etc, 
and they will start showing up in the UI when the task they reference becomes relevant.

Alternatively, you can retrieve waypoints manually.

## Getting Waypoints

Waypoints are loosely associated with quest data via IDs, but at runtime
when a level is loaded, any waypoints in the level will be registered and
connected to quest state data. 

See the [Progression](Progression.md) documentation on how quest state is handled.
Normally you'll want to access waypoints when a new task becomes active; one way
is listening in to the `USuqsProgression::OnProgressionEvent` callback,
and looking for the `ESuqsProgressionEventType::TaskAdded` event.

On receipt of this, you can access waypoints via `USuqsTaskState::GetWaypoints`. 
You can then track this in your UI relative to your camera, map etc.

## Multiple Waypoints per Task

You can associate multiple waypoints with a single task - this might be because
it's possible to complete a task in multiple places, or that there's a sequence
of waypoints. 

In the latter case, SequenceIndex can allow you to order them
reliably in the return from `GetWaypoints`. However, consider whether it might
be better to split the task up rather than have a sequence of waypoints, if it's
any more complicated than just describing a path.

## Waypoint enable/disable

Waypoints can also be enabled / disabled, in case you want to hide or
show them independently of the task state. This can be useful for example if
you have multiple waypoints for a single count-based task, like "Collect 10 nuts".
Once a nut has been collected, the task isn't complete, but this specific waypoint
should be disabled.


## More Info

* [Tasks](Tasks.md)
* [Quest Progression](Progression.md)
* [Changing Quest Definitions](ChangingQuestDefinitions.md)