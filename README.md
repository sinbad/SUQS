# Steve's Unreal Quest System (SUQS)

## What Is It?

SUQS is a simple, data-driven quest system for UE4. It helps you define quest structures for your game, 
and track progress against those quests for a player in a simple way. 

All quests are defined in static asset datatables, which can either be edited directly in UE4, or written
as JSON - a schema is provided to provide handy autocomplete and validation in most editors 
(see [Editor Setup](docs/EditorSetup.md)).

At runtime, you use a simple API to record progress against these quests: accept quests, complete tasks,
increment progress etc. As tasks are completed they trigger progression based on simple rules, and callbacks
and a query API let you reflect those changes in your world.

## Defining Quests

As mentioned, quests are static data tables. They can't be altered* dynamically, and this is deliberate;
not only does it keep the implementation simpler, it means you always have a single source of truth for
you quest sequence.

That doesn't mean you can't have branching quests. Branching within a quest is supported (but the branches
are predefined in the data), and also quests can be dependent on each other's success or failure. But importantly,
code can't add whole new quest branches at runtime, they're predefined.

> *Well, actually it *is* possible to add quests dynamically, but it's not recommended.

Let's use the JSON example because it's the easiest. See [Editor Setup](docs/EditorSetup.md) for how to
configure your editor to autocomplete. Here is the simplest possible quest:

```json
[
  {
    "Identifier": "Q_Smol",
    "Title": "NSLOCTEXT(\"TestQuests\", \"SmolQuestTitle\", \"Smol Quest\")",
    "DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"SmolQuestDesc\", \"The smallest possible quest\")",
    "Objectives": [
      {
        "Tasks": [
          {
            "Identifier": "T_Smol",
            "Title": "NSLOCTEXT(\"TestQuests\", \"SmolQuestTaskTitle\", \"Easiest possible thing\")"
          }
        ]
      }
    ]
  }
]
```

Some notes:
* A single JSON file can include multiple [Quests](docs/Quests.md), each of which must have a globally unique Identifier
* All player-visible text can be localised using NSLOCTEXT
* Each quest is comprised of 1 or more [Objectives](docs/Objectives.md)
  * Objectives are always sequential (but can [branch](docs/Branching.md))
  * Objectives contain [Tasks](docs/Tasks.md), which are the unit of progression
  * Objectives group Tasks, determining whether they are sequential or flexibly ordered, or whether all mandatory tasks have to be completed or only 1 (or 2, or 3...) of those in the group, 
  * Tasks track actual progress, can be mandatory or optional, and can have target numbers (e.g. collect 3 otter's noses), and time limits

You can import this as a Datatable asset just by putting the JSON file in your
Content folder and importing like this:

![Quest JSON Import](docs/img/quest_json_import.png)


More details on the specifics can be found in [Quests](docs/Quests.md).

## Tracking Progress

The root object you need to use in SUQS is `USuqsProgression`. This tracks progress
for a single player, and can be serialized with save games etc.

The best thing to do is to put this in a property on your GameInstance or wherever
else you're keeping permanent state. It needs to be given a list of data tables
which contain your quest definitions, which you can define in class defaults, then 
at runtime you can call the API to accept quests and advance progress.

See [Advancing Quest Progress](docs/Progress.md) for more details.


