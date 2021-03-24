# Quest Events

As [progress](Progression.md) is made on quests, you can receive events so that you can update dependent
state. All the events are on the `USuqsProgression` class:

* Quest Accepted
* Quest Completed
* Quest Failed
* Objective Completed
* Objective Failed
* Task Completed
* Task Failed
* Task Updated (in any way)

Each gives you the identifier required to figure out if it's the event you're
waiting for. 
