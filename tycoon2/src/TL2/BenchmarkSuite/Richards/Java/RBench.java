class RBench {

	TaskControlBlock taskList, currentTask;
	Identity currentTaskIdent;
	TaskControlBlock taskTable[] = {null, null, null, null, null, null};
	int layout;
	int holdCount, queuePacketCount;
    	boolean tracing;

	/* creation */
	void CreateDevice (Identity id, int prio,  Packet work,
                       TaskState state)
	{   DeviceTaskRec data;
    	TaskControlBlock t;

    	data = new DeviceTaskRec();
    	t = new DeviceTCB(taskList, id, prio, work, state, data);
    	EnterTask(id, t);
	}
    void CreateHandler(Identity id, int prio,  Packet work,
                       TaskState state)
	{   HandlerTaskRec data;
    	TaskControlBlock t;

    	data = new HandlerTaskRec();
    	t = new HandlerTCB(taskList, id, prio, work, state, data);
    	EnterTask(id, t);
	}
    void CreateIdler  (Identity id, int prio,  Packet work,
                       TaskState state)
	{   IdleTaskRec data;
    	TaskControlBlock t;

    	data = new IdleTaskRec();
    	t = new IdlerTCB(taskList, id, prio, work, state, data);
    	EnterTask(id, t);
	}
    void CreateWorker (Identity id, int prio,  Packet work,
                       TaskState state)
	{   WorkerTaskRec data;
    	TaskControlBlock t;

    	data = new WorkerTaskRec();
    	t = new WorkerTCB(taskList, id, prio, work, state, data);
    	EnterTask(id, t);
	}
    void EnterTask(Identity id, TaskControlBlock t)
	{
    	taskList = t;
    	taskTable[id.type] = t;
	}

    /* scheduling */
    void Schedule()
	{
    	currentTask = taskList;
    	while (currentTask != Global.NoTask) {
         	if (currentTask.IsTaskHoldingOrWaiting()) 
              	currentTask = currentTask.Link();
         	else {
              	currentTaskIdent = currentTask.Ident();
              	if (tracing) Trace(currentTaskIdent.type, "i");
              	currentTask = currentTask.RunTask();
         	}
    	}
	}

    /* initializing */
    void InitScheduler()
	{
    	queuePacketCount = 0;
    	holdCount = 0;
    	for (int i = 0; i < Global.NTASKS; i++) {taskTable[i] = Global.NoTask;}
    	taskList = Global.NoTask;
	}

    void InitTrace()
	{
    	tracing = false;
	}

    /* task management */
    TaskControlBlock FindTask(Identity id)
	{   TaskControlBlock t;

    	t = taskTable[id.type];
    	if (t == null) System.out.println("***error: FindTask failed! ");
    	return t;
	}

    TaskControlBlock HoldSelf()
	{
    	holdCount++;
    	currentTask.SetTaskHolding(true);
    	return currentTask.Link();
	}

    TaskControlBlock QueuePacket(Packet p)
	{   TaskControlBlock t;

    	t = FindTask(p.Ident());
    	queuePacketCount++;
    	p.SetLink(Global.NoWork);
    	p.SetIdent(currentTaskIdent);
    	return t.AddPacket(p, currentTask);
	}

    TaskControlBlock Release(Identity id)
	{   TaskControlBlock t;

    	t = FindTask(id);
    	t.SetTaskHolding(false);
    	return (t.Priority() > currentTask.Priority()) ? t : currentTask;
	}
    TaskControlBlock Wait()
	{
    	currentTask.SetTaskWaiting(true);
    	return currentTask;
	}

    /* tracing */
    void Trace(int id, String s)
	{
	}

    void Start(boolean askTrace)
	{	TaskState t;
    	Packet workQ;

    	if (askTrace) InitTrace(); else tracing = false;
    	InitScheduler();
    	t = new TaskState(); t.Running();				// Idler
    	CreateIdler(Global.Idler, 0, Global.NoWork, t);

    	workQ = new Packet(Global.NoWork, Global.Worker, Global.WorkPacket);		// Worker
    	workQ = new Packet(workQ , Global.Worker, Global.WorkPacket);
    	t = new TaskState(); t.WaitingWithPacket();
    	CreateWorker(Global.Worker, 1000, workQ, t);

    	workQ = new Packet(Global.NoWork, Global.DeviceA, Global.DevicePacket);		// HandlerA
    	workQ = new Packet(workQ , Global.DeviceA, Global.DevicePacket);
   		workQ = new Packet(workQ , Global.DeviceA, Global.DevicePacket);
    	t = new TaskState(); t.WaitingWithPacket();
    	CreateHandler(Global.HandlerA, 2000, workQ, t);

    	workQ = new Packet(Global.NoWork, Global.DeviceB, Global.DevicePacket);		// HandlerB
    	workQ = new Packet(workQ , Global.DeviceB, Global.DevicePacket);
    	workQ = new Packet(workQ , Global.DeviceB, Global.DevicePacket);
    	t = new TaskState(); t.WaitingWithPacket();
    	CreateHandler(Global.HandlerB, 3000, workQ, t);

    	t = new TaskState(); t.Waiting();				// DeviceA
    	CreateDevice(Global.DeviceA, 4000, Global.NoWork, t);
    	t = new TaskState(); t.Waiting();				// DeviceB
    	CreateDevice(Global.DeviceB, 5000, Global.NoWork, t);

    	Schedule();

    	if (! (queuePacketCount == 23246 && holdCount == 9297)) {
      		System.out.println("error: richards results are incorrect");
    	}
	}
}


