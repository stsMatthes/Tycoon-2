class WorkerTaskRec {

	Identity destination;
	int count;

	WorkerTaskRec() {destination = Global.HandlerA; count = 0;}
	int Count() {return count;}
	void SetCount(int n) {count = n;}
	Identity Destination() {return destination;}
	void SetDestination(Identity d) {destination = d;}
	void Print() {}
}
