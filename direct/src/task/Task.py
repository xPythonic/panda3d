from libpandaexpressModules import *
from DirectNotifyGlobal import *
from PythonUtil import *
import time
import fnmatch
import string

# MRM: Need to make internal task variables like time, name, index
# more unique (less likely to have name clashes)

exit = -1
done = 0
cont = 1

# Store the global clock
globalClock = ClockObject.getGlobalClock()

def getTimeFrame():
    # WARNING: If you are testing tasks without an igloop,
    # you must manually tick the clock

    # Ask for the time last frame
    t = globalClock.getFrameTime()

    # Get the new frame count
    f = globalClock.getFrameCount()

    return t, f


class Task:
    def __init__(self, callback, priority = 0):
        self.__call__ = callback
        self._priority = priority
        self.uponDeath = None
        self.dt = 0.0
        self.maxDt = 0.0
        self.avgDt = 0.0
        self.runningTotal = 0.0

    def getPriority(self):
        return self._priority

    def setPriority(self, pri):
        self._priority = pri

    def setStartTimeFrame(self, startTime, startFrame):
        self.starttime = startTime
        self.startframe = startFrame

    def setCurrentTimeFrame(self, currentTime, currentFrame):
        # Calculate and store this task's time (relative to when it started)
        self.time = currentTime - self.starttime
        self.frame = currentFrame - self.startframe

def doLater(delayTime, task, taskName):
    task.name = taskName
    # make a sequence out of the delay and the task
    seq = sequence(pause(delayTime), task)
    return seq

def spawnMethodNamed(self, func, name):
        task = Task(func)
        self.spawnTaskNamed(task, name)

def pause(delayTime):
    def func(self):
        if (self.time < self.delayTime):
            return cont
        else:
            # Time is up, return done
            TaskManager.notify.debug('pause done: ' + self.name)
            return done
    task = Task(func)
    task.name = 'pause'
    task.delayTime = delayTime
    return task


def release():
    def func(self):
        # A release is immediately done
        TaskManager.notify.debug('release done: ' + self.name)
        return done
    task = Task(func)
    task.name = 'release'
    return task


def sequence(*taskList):
    return make_sequence(taskList)


def make_sequence(taskList):
    def func(self):
        # If we got to the end of the list, this sequence is done
        if (self.index >= len(self.taskList)):
            TaskManager.notify.debug('sequence done: ' + self.name)
            return done
        else:
            task = self.taskList[self.index]
            # If this is a new task, set its start time and frame
            if (self.index > self.prevIndex):
                task.setStartTimeFrame(self.time, self.frame)
            self.prevIndex = self.index

            # Calculate this task's time since it started
            task.setCurrentTimeFrame(self.time, self.frame)

            # Execute the current task
            ret = task(task)

            # Check the return value from the task
            # If this current task wants to continue,
            # come back to it next frame
            if (ret == cont):
                return cont

            # If this task is done, increment the index so that next frame
            # we will start executing the next task on the list
            elif (ret == done):
                self.index = self.index + 1
                return cont

            # If this task wants to exit, the sequence exits
            elif (ret == exit):
                return exit

    task = Task(func)
    task.name = 'sequence'
    task.taskList = taskList
    task.prevIndex = -1
    task.index = 0
    return task

def resetSequence(task):
    # Should this automatically be done as part of spawnTaskNamed?
    # Or should one have to create a new task instance every time
    # one wishes to spawn a task (currently sequences and can
    # only be fired off once
    task.index = 0
    task.prevIndex = -1

def loop(*taskList):
    return make_loop(taskList)

def make_loop(taskList):
    def func(self):
        # If we got to the end of the list, this sequence is done
        if (self.index >= len(self.taskList)):
            TaskManager.notify.debug('sequence done, looping: ' + self.name)
            self.prevIndex = -1
            self.index = 0
            return cont
        else:
            task = self.taskList[self.index]
            # If this is a new task, set it's start time and frame
            if (self.index > self.prevIndex):
                task.setStartTimeFrame(self.time, self.frame)
            self.prevIndex = self.index

            # Calculate this task's time since it started
            task.setCurrentTimeFrame(self.time, self.frame)

            # Execute the current task
            ret = task(task)

            # Check the return value from the task
            # If this current task wants to continue,
            # come back to it next frame
            if (ret == cont):
                return cont

            # If this task is done, increment the index so that next frame
            # we will start executing the next task on the list
            elif (ret == done):
                self.index = self.index + 1
                return cont

            # If this task wants to exit, the sequence exits
            elif (ret == exit):
                return exit

    task = Task(func)
    task.name = 'loop'
    task.taskList = taskList
    task.prevIndex = -1
    task.index = 0
    return task


def makeSpawner(task, taskName, taskMgr):
    def func(self):
        self.taskMgr.spawnTaskNamed(self.task, self.taskName)
        return done
    newTask = Task(func)
    newTask.name = taskName + "-spawner"
    newTask.task = task
    newTask.taskName = taskName
    newTask.taskMgr = taskMgr
    return newTask


def makeSequenceFromTimeline(timelineList, taskMgr):
    timeline = []
    lastPause = 0
    sortedList = list(timelineList)
    sortedList.sort()
    for triple in sortedList:
        t = triple[0] - lastPause
        lastPause = triple[0]
        task = triple[1]
        taskName = triple[2]
        timeline.append(pause(t))
        timeline.append(makeSpawner(task, taskName, taskMgr))
    return make_sequence(timeline)


def timeline(*timelineList):
    def func(self):
        # Step our sub task manager (returns the number of tasks remaining)
        lenTaskList = self.taskMgr.step()

        # The sequence start time is the same as our start time
        self.sequence.time = self.time
        self.sequence.frame = self.frame

        if (not self.sequenceDone):
            # Execute the sequence for this frame
            seqRet = self.sequence(self.sequence)
            # See if sequence is done
            if (seqRet == done):
                self.sequenceDone = 1
                # See if the timeline is done
                if (lenTaskList == 0):
                    TaskManager.notify.debug('timeline done: ' + self.name)
                    return done
                else:
                    return cont
            else:
                return cont
        else:
            return cont
    task = Task(func)
    task.name = 'timeline'
    task.taskMgr = TaskManager()
    task.sequence = makeSequenceFromTimeline(timelineList, task.taskMgr)
    task.sequenceDone = 0
    return task


class TaskManager:

    notify = None

    def __init__(self):
        self.running = 0
        self.stepping = 0
        self.taskList = []
        self.currentTime, self.currentFrame = getTimeFrame()
        if (TaskManager.notify == None):
            TaskManager.notify = directNotify.newCategory("TaskManager")
        # TaskManager.notify.setDebug(1)
        self.taskTimerVerbose = 0

    def stepping(value):
        self.stepping = value

    def spawnMethodNamed(self, func, name):
        task = Task(func)
        return self.spawnTaskNamed(task, name)

    def spawnTaskNamed(self, task, name):
        TaskManager.notify.debug('spawning task named: ' + name)
        task.name = name
        task.setStartTimeFrame(self.currentTime, self.currentFrame)
        # search back from the end of the list until we find a
        # task with a lower priority, or we hit the start of the list
        index = len(self.taskList) - 1
        while (1):
            if (index < 0):
                break
            if (self.taskList[index].getPriority() <= task.getPriority()):
                break
            index = index - 1
        index = index + 1
        self.taskList.insert(index, task)
        return task

    def doMethodLater(self, delayTime, func, taskName):
        task = Task(func)
        seq = doLater(delayTime, task, taskName)
        return self.spawnTaskNamed(seq, 'doLater-' + taskName)

    def removeAllTasks(self):
        # Make a shallow copy so we do not modify the list in place
        taskListCopy = self.taskList[:]
        for task in taskListCopy:
            self.removeTask(task)

    def removeTask(self, task):
        TaskManager.notify.debug('removing task: ' + `task`)
        try:
            self.taskList.remove(task)
        except:
            pass
        if task.uponDeath:
            task.uponDeath(task)

    def removeTasksNamed(self, taskName):
        TaskManager.notify.debug('removing tasks named: ' + taskName)
        removedTasks = []

        # Find the tasks that match by name and make a list of them
        for task in self.taskList:
            if (task.name == taskName):
                removedTasks.append(task)

        # Now iterate through the tasks we need to remove and remove them
        for task in removedTasks:
            self.removeTask(task)

        # Return the number of tasks removed
        return len(removedTasks)

    def removeTasksMatching(self, taskPattern):
        """removeTasksMatching(self, string taskPattern)

        Removes tasks whose names match the pattern, which can include
        standard shell globbing characters like *, ?, and [].

        """
        TaskManager.notify.debug('removing tasks matching: ' + taskPattern)
        removedTasks = []

        # Find the tasks that match by name and make a list of them
        for task in self.taskList:
            if (fnmatch.fnmatchcase(task.name, taskPattern)):
                removedTasks.append(task)

        # Now iterate through the tasks we need to remove and remove them
        for task in removedTasks:
            self.removeTask(task)

        # Return the number of tasks removed
        return len(removedTasks)

    def step(self):
        TaskManager.notify.debug('step')
        self.currentTime, self.currentFrame = getTimeFrame()
        for task in self.taskList:
            task.setCurrentTimeFrame(self.currentTime, self.currentFrame)

            if not self.taskTimerVerbose:
                # don't record timing info
                ret = task(task)
            else:
                # Run the task and check the return value
                startTime = time.clock()
                ret = task(task)
                endTime = time.clock()

                # Record the dt
                dt = endTime - startTime
                task.dt = dt

                # See if this is the new max
                if dt > task.maxDt:
                    task.maxDt = dt

                # Record the running total of all dts so we can compute an average
                task.runningTotal = task.runningTotal + dt
                if (task.frame > 0):
                    task.avgDt = (task.runningTotal / task.frame)
                else:
                    task.avgDt = 0

            # See if the task is done
            if (ret == cont):
                continue
            elif (ret == done):
                self.removeTask(task)
            elif (ret == exit):
                self.removeTask(task)
            else:
                raise 'invalid task state'
        return len(self.taskList)

    def run(self):
        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        t = globalClock.getFrameTime()
        globalClock.setRealTime(t)
        if self.stepping:
            self.step()
        else:
            self.running = 1
            while self.running:
                try:
                    self.step()
                except KeyboardInterrupt:
                    self.stop()
                except:
                    raise

    def stop(self):
        # Set a flag so we will stop before beginning next frame
        self.running = 0

    def replaceMethod(self, oldMethod, newFunction):
        import new
        for task in self.taskList:
            method = task.__call__
            if (type(method) == types.MethodType):
                function = method.im_func
            else:
                function = method
            #print ('function: ' + `function` + '\n' +
            #       'method: ' + `method` + '\n' +
            #       'oldMethod: ' + `oldMethod` + '\n' +
            #       'newFunction: ' + `newFunction` + '\n')
            if (function == oldMethod):
                newMethod = new.instancemethod(newFunction,
                                               method.im_self,
                                               method.im_class)
                task.__call__ = newMethod
                # Found it return true
                return 1
        return 0

    def __repr__(self):
        import fpformat
        taskNameWidth = 32
        dtWidth = 7
        priorityWidth = 10
        totalDt = 0
        totalAvgDt = 0
        str = ('taskList'.ljust(taskNameWidth)
               + 'dt(ms)'.rjust(dtWidth)
               + 'avg'.rjust(dtWidth)
               + 'max'.rjust(dtWidth)
               + 'priority'.rjust(priorityWidth)
               + '\n')
        str = str + '---------------------------------------------------------------\n'
        for task in self.taskList:
            totalDt = totalDt + task.dt
            totalAvgDt = totalAvgDt + task.avgDt
            if (self.taskTimerVerbose):
                str = str + (task.name.ljust(taskNameWidth)
                             + fpformat.fix(task.dt*1000, 2).rjust(dtWidth)
                             + fpformat.fix(task.avgDt*1000, 2).rjust(dtWidth)
                             + fpformat.fix(task.maxDt*1000, 2).rjust(dtWidth)
                             + `task.getPriority()`.rjust(priorityWidth)
                             + '\n')
            else:
                str = str + (task.name.ljust(taskNameWidth)
                             + '----'.rjust(dtWidth)
                             + '----'.rjust(dtWidth)
                             + '----'.rjust(dtWidth)
                             + `task.getPriority()`.rjust(priorityWidth)
                             + '\n')
        str = str + '---------------------------------------------------------------\n'
        if (self.taskTimerVerbose):
            str = str + ('total'.ljust(taskNameWidth)
                         + fpformat.fix(totalDt*1000, 2).rjust(dtWidth)
                         + fpformat.fix(totalAvgDt*1000, 2).rjust(dtWidth)
                         + '\n')
        else:
            str = str + ('total'.ljust(taskNameWidth)
                         + '----'.rjust(dtWidth)
                         + '----'.rjust(dtWidth)
                         + '\n')
        return str


"""
import Task
from ShowBaseGlobal import * # to get taskMgr, and run()

# sequence

def seq1(state):
    print 'seq1'
    return Task.done
def seq2(state):
    print 'seq2'
    return Task.done

t = Task.sequence(Task.pause(1.0), Task.Task(seq1), Task.release(),
                  Task.pause(3.0), Task.Task(seq2))
taskMgr.spawnTaskNamed(t, 'sequence')
run()

# If you want it to loop, make a loop
t = Task.loop(Task.pause(1.0), Task.Task(seq1), Task.release(),
              Task.pause(3.0), Task.Task(seq2))
taskMgr.spawnTaskNamed(t, 'sequence')
run()



# timeline

def keyframe1(state):
    print 'keyframe1'
    return Task.done
def keyframe2(state):
    print 'keyframe2'
    return Task.done
def keyframe3(state):
    print 'keyframe3'
    return Task.done

testtl = Task.timeline(
    (0.5, Task.Task(keyframe1), 'key1'),
    (0.6, Task.Task(keyframe2), 'key2'),
    (0.7, Task.Task(keyframe3), 'key3')
    )

t = taskMgr.spawnTaskNamed(testtl, 'timeline')
run()

# do later - returns a sequence

def foo(state):
    print 'later...'
    return Task.done

seq = Task.doLater(3.0, Task.Task(foo), 'fooLater')
t = taskMgr.spawnTaskNamed(seq, 'doLater-fooLater')
run()

# tasks with state
# Combined with uponDeath

someValue = 1

def func(state):
    if (state.someValue > 10):
        print 'true!'
        return Task.done
    else:
        state.someValue = state.someValue + 1
        return Task.cont

def deathFunc(state):
    print 'Value at death: ', state.someValue

task = Task.Task(func)

# set task state here
task.someValue = someValue

# Use instance variable uponDeath to specify function
# to perform on task removal
# Default value of uponDeath is None
task.uponDeath = deathFunc

t = taskMgr.spawnTaskNamed(task, 'funcTask')
run()
"""
