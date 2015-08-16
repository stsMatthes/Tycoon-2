class IdleTaskRec {

	int control, count;

	IdleTaskRec() {control = 1; count = 10000;}
	int Control() {return control;}
	void SetControl(int n) {control = n;}
	int Count() {return count;}
	void SetCount(int n) {count = n;}
	void Print() {}
}
