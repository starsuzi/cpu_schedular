#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TRUE	1
#define FALSE	0

//Related to Process
typedef struct _process {
	int pid;
	int cpu_burst;
	int io_burst;
	int io_start;
	int ent_btime;
	int arr_time;
	int comp_time;		//completion time
	int remain_time;	//for p_sjf
	int ent_remain_time;//for p_sjf_io
	int remain_time_io;	//for p_ljf
	int progress;
	int priority;
	bool start;
	int order;			//for sjf
	int time_quan;		//for RR
	int preemptive;
} Process;

typedef struct _evaluation {
	float avg_turnaround;
	float avg_waiting;
	int idle_time;
	int check;
	int finish_time;
} Evaluation;

//Related to Queue
typedef int Data;

typedef struct _node
{
	Data data;
	struct _node * next;
} Node;

typedef struct _lQueue
{
	Node * front;
	Node * rear;
} LQueue;

typedef LQueue Queue;

void QueueInit(Queue * pq);
int QIsEmpty(Queue * pq);

void Enqueue(Queue * pq, Data data);
Data Dequeue(Queue * pq);
Data QPeek(Queue * pq);

//Declare constants.
#define MAX_P_NUM 10

int p_num = 0; // Number of process
Process *process[MAX_P_NUM];

// Declare each algorithms' evaluation
Evaluation fcfs, np_sjf, p_sjf, np_pri, p_pri, rr, np_sjf_io, p_sjf_io, p_ljf;

// Define Queue
Queue ready_q, running_q;
int waiting_q[MAX_P_NUM];

// Functions related to Queue
void QueueInit(Queue * pq) {
	pq->front = NULL;
	pq->rear = NULL;
}

int QIsEmpty(Queue * pq) {
	if (pq->front == NULL) return TRUE;
	else return FALSE;
}

void Enqueue(Queue * pq, Data data) {
	Node * newNode = (Node*)malloc(sizeof(Node));
	newNode->next = NULL;
	newNode->data = data;

	if (QIsEmpty(pq)) {
		pq->front = newNode;
		pq->rear = newNode;
	}
	else {
		pq->rear->next = newNode;
		pq->rear = newNode;
	}
}

Data Dequeue(Queue * pq) {
	Node * delNode;
	Data retData;

	if (QIsEmpty(pq)) {
		printf("Queue Memory Error![dequeue]");
		exit(-1);
	}
	delNode = pq->front;
	retData = delNode->data;
	pq->front = pq->front->next;

	free(delNode);
	return retData;
}

Data QPeek(Queue * pq) {
	if (QIsEmpty(pq)) {
		printf("Queue Memory Error![peek]");
		exit(-1);
	}
	return pq->front->data;
}

//Function of creating processes
void create_process() {
	//모든 알고리즘에서는 공통적으로 프로세싱을 할 때 현재 시간을 나타내는 time 변수를 1씩 증가시키는데, 
	//이를 마치는 시기는 Evaluation 구조체에서 각 프로세스가 끝날 때 마다
	//int형의 변수인 check를 1씩 증가시키며 check가 프로세스의 개수가 될 때이다.
	//또한 프로세스의 진행 중에 progress와 arrival time이 같아질 때
	// I/O operation이 발생했다고 하며, running queue에 있는 프로세스를 꺼내 Waiting queue에 넣고(waiting_q[i]에 I/O burst 할당)
	//시간이 지날 때마다 1씩 감소시켜 0이 되면 I/O operation을 종료한다.(waiting_q에서 ready_q나 running_q로 이동)
	printf("< CPU SCHEDULER >\n");
	printf("* Please enter the number of processes to be created.(1 ~ 10): ");
	while (1) {
		scanf("%d", &p_num);
		if (p_num > MAX_P_NUM || p_num < 0) printf("[ERROR] Number of processes must be 1 to 10. Please enter number of processes again: ");
		else break;
	}

	srand(time(NULL));
	int found[10]; int pri, flag;
	int i, j;
	for (i = 0; i < p_num; i++) {
		Process *p_process = (Process*)malloc(sizeof(Process) * 1);
		p_process->pid = i;														//Process ID: 0 ~ 9(orderly)
		p_process->cpu_burst = (int)(rand() % 8 + 2);							//CPU burst time: 2 ~ 10
		p_process->io_burst = (int)(rand() % 4 + 1);							//I/O burst time: 1 ~ 5
		p_process->arr_time = (int)(rand() % (4 * p_num) + 1);					//Arrival time: 1 ~ 4 x # of processes
																				//Duplicate prevention function(priority)
		while (1) {
			pri = (rand() % p_num + 1);
			flag = 0;
			for (j = 0; j <= i; j++) {
				if (pri != found[j]) { flag++; }
			}
			if (flag == i + 1) { found[i] = pri; break; }
		}
		p_process->priority = found[i];											//Priority: 1 ~ # of processes
		p_process->io_start = (int)(rand() % (p_process->cpu_burst - 1) + 1);	//I/O start time: after 1 ~ n - 1 (randomly)
		p_process->remain_time = p_process->cpu_burst;
		p_process->ent_btime = p_process->cpu_burst + p_process->io_burst;
		p_process->ent_remain_time = p_process->ent_btime;
		p_process->remain_time_io = p_process->io_burst;
		p_process->start = FALSE;
		p_process->comp_time = 0;												//Completion time
		p_process->progress = 0;
		p_process->time_quan = 0;
		p_process->preemptive = 0;												//Indicating that it can be preemptive.
		process[i] = p_process;
	}
	// At least one process can arrive at the start.
	process[(int)(rand() % p_num)]->arr_time = 1;

	printf("----------------------------------------------------------------------------------------\n");
	printf("Process\t\tCPU Burst\tI/O Burst\tI/O Start\tArrival Time\tPriority\n");
	printf("----------------------------------------------------------------------------------------\n");
	for (i = 0; i < p_num; i++) {
		printf("P%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", process[i]->pid, process[i]->cpu_burst, process[i]->io_burst, process[i]->io_start, process[i]->arr_time - 1, process[i]->priority);
	}
	printf("----------------------------------------------------------------------------------------\n");
	printf("- Finished creating the process.\n\n");
}

//Function of system environment setting (ready queue(queue) & running queue(queue) & waiting queue(array))
void config() {
	int i = 0;
	QueueInit(&ready_q);
	QueueInit(&running_q);
	for (i = 0; i < p_num; i++) {
		waiting_q[i] = -1;
	}
	printf("- Finished creating the queue.\n\n");
}

//Initializing processes' environment for scheduling algorithms
void Initalize() {
	int i = 0;
	for (i = 0; i < p_num; i++) {
		waiting_q[i] = -1;
		process[i]->progress = 0;
		process[i]->comp_time = 0;
	}
}

//CPU Scheduling algorithms
void FCFS() {
	// 먼저 들어오는 프로세스를 먼저 수행하는 알고리즘
	//각 프로세스는 arrival time이 time과 같아질 때 ready queue에 순차적으로 들어간다.
	//그리고 running queue가 비어있으면 ready queue가 비어있는지 확인해서
	//비어있다면 idle을 출력하고, 그렇지 않다면 queue의 가장 앞에 있는 프로세스를 꺼내 running queue에 넣는다.
	printf("[FCFS Scheduling]\n");
	int i, pick, time;
	fcfs.idle_time = 0, fcfs.check = 0;
	Initalize();

	for (time = 1; fcfs.check != p_num; time++) {	//Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (time == process[i]->arr_time) { Enqueue(&ready_q, process[i]->pid); }
			if (waiting_q[i] > 0) { waiting_q[i]--; }						//Doing I/O operation...
			if (waiting_q[i] == 0) { Enqueue(&ready_q, i); waiting_q[i]--; }//When the process waits for io burst, it re-enters the ready queue.
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고 running 큐는 비어있을 경우
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick); //ready 큐에서 running 큐로
			process[pick]->start = TRUE;
		}

		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); fcfs.idle_time++; } //running 큐가 비어있으면 idle time이라고 출력 후 idle ++
		else { //running 큐가 차있을 경우
			if (process[pick]->start == TRUE) { //여기 뭘까
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; //progress 진행

			if (process[pick]->progress == process[pick]->io_start) { //progress와 io start 시간이 같다면, 즉 io operation이 발생했다면
				int waiting = Dequeue(&running_q); 					  //running 큐에 있는 프로세스를 꺼내 waiting 큐에 넣음
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				fcfs.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	fcfs.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	fcfs.avg_turnaround = (float)tot_turn / p_num;
	fcfs.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", fcfs.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", fcfs.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void NP_SJF() {	//프로세스 생성 시 할당해준 cpu burst time을 이용한다.. 임시 행렬 arr을 만들어서 cpu burst를 넣어준 뒤 bubble sort를 통해 cpu burst time이 적은 순서대로 정렬
				//여기에서는 프로세스 생성 시 할당해준 cpu burst time을 이용하는데,
				//임시 행렬 arr을 만들어서 cpu burst를 넣어준 뒤 bubble sort를 통해 cpu burst time이 적은 순서대로 정렬한다.
				//이를 통해 프로세스가 arrive할 때마다 ready queue에서 자신의 cpu burst에 맞는 위치에 들어가게 되며
				// running queue에 있는 프로세스가 끝나 running queue가 비면 그때 ready queue에 있는 프로세스가 running으로 들어간다.
	printf("[Non-preemptive Shortest Job First Scheduling]\n");
	int i, j, tmp, pick, time;
	np_sjf.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 }; //임시 행렬 arr을 만들어 cpu burst를 넣어준 부분
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->cpu_burst;
	}
	for (i = 0; i < p_num; i++) { //bubble sort를 통해 cpu burst가 적은 순서대로 정렬
		for (j = i + 1; j < p_num; j++) {
			if (arr[i][1] > arr[j][1]) {
				tmp = arr[i][0]; arr[i][0] = arr[j][0]; arr[j][0] = tmp;
				tmp = arr[i][1]; arr[i][1] = arr[j][1]; arr[j][1] = tmp;
			}
		}
	}
	for (i = 0; i < p_num; i++) {
		for (j = 0; j < p_num; j++) {
			if (process[i]->pid == arr[j][0])
				process[i]->order = j;
		}
	}
	Queue temp; QueueInit(&temp);

	for (time = 1; np_sjf.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }	//Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting큐가 비어있거나, 프로세스가 arrive했다면
				if (waiting_q[i] == 0) { waiting_q[i]--; } 			 //이러거나

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid); //ready 큐가 비어있다면 ready큐에 넣음
				else {														//ready 큐가 비어있지 않다면
					while (process[QPeek(&ready_q)]->order < process[i]->order) { //ready 큐의 order < 현재 프로세스의 order
						Enqueue(&temp, Dequeue(&ready_q));	//ready 큐에서 꺼내서 temp큐에 넣는다
						if (QIsEmpty(&ready_q)) break; 		//ready 큐가 비어있다면 break
					}
					Enqueue(&temp, process[i]->pid);		//temp큐에 process[i] 넣는다
					while (!QIsEmpty(&ready_q)) {			//ready큐가 차있는동안
						Enqueue(&temp, Dequeue(&ready_q));	//ready큐에서 꺼내서 temp큐에 넣는다
					}
					while (!QIsEmpty(&temp)) {				//temp큐가 차있는동안
						Enqueue(&ready_q, Dequeue(&temp));	//temp큐에서 꺼내서 ready큐에 넣는다
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고 running 큐가 비어있다면
			pick = Dequeue(&ready_q); //ready 큐에서 꺼낸것을 pick
			Enqueue(&running_q, pick); //pick을 running 큐에 넣음
			process[pick]->start = TRUE; //시작
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf.idle_time++; } //running 큐가 비어있다면, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick한 프로세스를 진행

			if (process[pick]->progress == process[pick]->io_start) { //pick 한 프로세스의 progress와 io start이 같아진다면
				int waiting = Dequeue(&running_q); //running 큐에서 꺼내서 waiting 큐에 넣는다.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1; //waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_sjf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_sjf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_sjf.avg_turnaround = (float)tot_turn / p_num;
	np_sjf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_sjf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_sjf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_SJF() {
	//NP_SJF에서는 초기에 cpu burst를 sort한 반면에 
	//여기에는 각 프로세스에 remain_time값을 넣어서 현재 프로세스가 끝날 때 까지 남은 시간을 나타내준다.
	//그리고 프로세스가 arrive할 때마다 현재 running queue에서 실행되고 있는 프로세스와 remain time을 비교해 
	//새 프로세스의 remain time이 더 작다면(혹은 같으면서 새 프로세스의 pid가 작다면) running queue에 들어가게 되고(preemptive 발생) 
	// 거기에 있던 프로세스는 ready queue의 최상단으로 가게 되며, 
	//그렇지 않다면 새 프로세스의 cpu burst 가 작다면 ready queue에서 자신의 cpu burst에 맞는 위치에 들어가게 된다.(temp Queue를 정의해서 구현)
	printf("[Preemptive Shortest Job First Scheduling]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_sjf.idle_time = 0, p_sjf.check = 0;

	Initalize();

	for (time = 1; p_sjf.check != p_num; time++) { //Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; } //Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //ready큐가 비어있고, running 큐가 비어있으면 ready 큐에 넣는다
				else if (!QIsEmpty(&running_q)) { //만일 running 큐가 차있다면 
					if (process[i]->remain_time < process[QPeek(&running_q)]->remain_time) { //프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와 remain time을 비교해
						preem = Dequeue(&running_q);										 //새 프로세스의 remain time이 더 작다면(혹은 같으면서 새 프로세스의 pid가 작다면) running 큐에 들어간다
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }   //ready 큐가 차있는 동안 ready 큐에서 꺼내서 temp 큐에 넣는다
						Enqueue(&ready_q, preem);											 //running 큐에서 꺼낸 preem을 ready 큐에 넣는다
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }		 //temp가 차있는 동안, temp에서 꺼낸 애들을 ready 큐에 넣는다 (기존에 있던 프로세스를 ready 큐의 최상단으로 가게 하기 위함)
						Enqueue(&running_q, i);												 //프로세스 i를 running 큐에서..
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와 remain time을 비교해 새 프로세스의 remain time이 더 작지 않다면
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i); //만일 ready 큐가 비어있다면, 프로세스 i를 ready 큐에 넣는다
						else { //ready 큐가 비어있지 않다면
							while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) { //현재 ready 큐의 remain time < 새 프로세스 i의 remain time 하는 동안
								Enqueue(&temp, Dequeue(&ready_q));									  //ready 큐를 꺼내서 temp 큐에 넣는다.
								if (QIsEmpty(&ready_q)) break;										  //만일 ready가 비면 break;
							}
							Enqueue(&temp, process[i]->pid);										  //프로세스i의 pid를 temp 큐에 넣는다
							while (!QIsEmpty(&ready_q)) {											  //ready 큐가 차있는 동안
								Enqueue(&temp, Dequeue(&ready_q));									  //ready큐에서 꺼내서 temp큐에 넣는다.
							}
							while (!QIsEmpty(&temp)) {												  //temp 큐가 차있는동안
								Enqueue(&ready_q, Dequeue(&temp));									  //temp 큐에서 꺼내서 ready 큐에 넣는다
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) { //현재 ready 큐의 프로세스의 remain time < 새로운 프로세스의 remain time 하는 동안
						Enqueue(&temp, Dequeue(&ready_q));									  //ready 큐에서 꺼내서 temp 큐에 삽입
						if (QIsEmpty(&ready_q)) break;										  //만일 ready 큐가 비어있다면 break
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) { 	//running 큐가 차있다면
			pick = QPeek(&running_q);	//running 큐의 상단을 pick이라고..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고, running 큐가 비어있다면
			pick = Dequeue(&ready_q);					   //ready 큐에서 꺼내서 pick한다
			Enqueue(&running_q, pick);					   //pick을 running 큐에 넣는다
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf.idle_time++; } //running 큐가 비어있다면, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) { //progress와 io start 시간이 같다면, 즉 io operation이 발생했다면
				int waiting = Dequeue(&running_q);					  //running 큐에 있는 프로세스를 꺼내 waiting 큐에 넣음
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_sjf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_sjf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_sjf.avg_turnaround = (float)tot_turn / p_num;
	p_sjf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_sjf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_sjf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void NP_PRIORITY() {
	//P_SJF에서 프로세스의 위치를 정하는 기준이 remain time이었다면, 여기서는 그 값이 priority이다. 
	//프로세스가 들어왔을 때 곧장 running 큐와 비교하면 preemptive, ready 큐에 들어가면 np
	printf("[Non-preemptive Priority Scheduling]\n"); //얘는 np임
	int i, pick, time;
	Queue temp; QueueInit(&temp);
	np_pri.idle_time = 0, np_pri.check = 0;

	Initalize();
	
	for (time = 1; np_pri.check != p_num; time++) { //Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; } //Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting 큐가 비어있거나, 프로세스가 도착한 시간이 time과 같다면
				if (waiting_q[i] == 0) { waiting_q[i]--; }			 //waiting 큐가 비어있다면, waiting 큐 --

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid); //ready 큐가 비어있다면, 새 프로세스의 pid를 ready 큐에 삽입
				else { //ready 큐가 비어있지 않다면
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) { //ready 큐의 프로세스의 priority < 새 프로세스의 priority 하는 동안
						Enqueue(&temp, Dequeue(&ready_q));									//ready 큐에서 꺼내고 temp에 넣는다
						if (QIsEmpty(&ready_q)) break;										//만일 ready 큐가 비면 break;
					}
					Enqueue(&temp, process[i]->pid);	//새 프로세스 i의 pid를 temp 큐에 삽입
					while (!QIsEmpty(&ready_q)) {			//ready 큐가 차있는동안
						Enqueue(&temp, Dequeue(&ready_q));	//ready 큐에서 꺼내서 temp 큐에 삽입
					}
					while (!QIsEmpty(&temp)) {				//temp가 차 있는동안
						Enqueue(&ready_q, Dequeue(&temp));	//temp큐에서 꺼내서 ready 큐에 삽입
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고, running 큐가 비어있다면
			pick = Dequeue(&ready_q);					   //ready 큐에서 꺼낸 애를 pick
			Enqueue(&running_q, pick);					   //pick 한 애를 running 큐에 삽입
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_pri.idle_time++; }  //running 큐가 비어있다면, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick한 프로세스를 진행

			if (process[pick]->progress == process[pick]->io_start) { //pick 한 프로세스의 progress와 io start이 같아진다면
				int waiting = Dequeue(&running_q);					  //running 큐에서 꺼내서 waiting 큐에 넣는다.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_pri.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_pri.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_pri.avg_turnaround = (float)tot_turn / p_num;
	np_pri.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_pri.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_pri.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_PRIORITY() {
	printf("[Preemptive Priority Scheduling]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_pri.idle_time = 0, p_pri.check = 0;

	Initalize();

	for (time = 1; p_pri.check != p_num; time++) {	//Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; }	//Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting 큐가 비어있거나, 프로세스가 도착한 시간이 time과 같다면
				if (waiting_q[i] == 0) { waiting_q[i]--; }			 //waiting 큐가 비어있다면, waiting 큐 --

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //ready 큐, running 큐가 비어있다면, 새 프로세스의 pid를 ready 큐에 삽입
				else if (!QIsEmpty(&running_q)) { //running 큐가 차있다면
					if (process[i]->priority < process[QPeek(&running_q)]->priority) { //새로운 process의 우선순위 < running 큐의 우선순위
						preem = Dequeue(&running_q);								   //running 큐를 꺼내서 preem에
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); } //ready 큐가 차있는 동안, ready큐에서 꺼내서 temp 큐에 넣는다
						Enqueue(&ready_q, preem);										   //preem을 ready큐에 삽입
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }	   //temp큐가 차있는 동안 temp의 큐를 꺼내 ready 큐에 삽입
						Enqueue(&running_q, i);											   //프로세스i를 running 큐에 삽입
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //non이랑 비슷
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i); //ready 큐가 비어있다면, 프로세스i를 ready 큐에 삽입
						else {
							while (process[QPeek(&ready_q)]->priority < process[i]->priority) { // ready 큐의 프로세스의 우선순위 < 새로운 프로세스의 우선순위 인동안
								Enqueue(&temp, Dequeue(&ready_q));								//ready 큐를 꺼내서 temp 큐에 삽입
								if (QIsEmpty(&ready_q)) break;									//ready 큐가 비면 break
							}
							Enqueue(&temp, process[i]->pid);									//새 프로세스 i의 pid를 temp 큐에 삽입
							while (!QIsEmpty(&ready_q)) {										//ready 큐가 차있는동안
								Enqueue(&temp, Dequeue(&ready_q));								//ready큐를 꺼내서 temp 큐에 삽입
							}
							while (!QIsEmpty(&temp)) {											//temp 큐가 비어있는 동안
								Enqueue(&ready_q, Dequeue(&temp));								//temp 큐에서 꺼내서 ready 큐에 삽입
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) {			// ready 큐의 우선순위 < 새로운 프로세스의 우선순위 인동안
						Enqueue(&temp, Dequeue(&ready_q));										//ready 큐에서 꺼내서 temp 큐에 삽입
						if (QIsEmpty(&ready_q)) break;											//ready큐가 비면 break;
					}
					Enqueue(&temp, process[i]->pid);											//프로세스 i의 pid를 temp 큐에 삽입
					while (!QIsEmpty(&ready_q)) {												//ready큐가 차있는 동안
						Enqueue(&temp, Dequeue(&ready_q));										//ready큐에서 꺼내고 temp큐에 삽입
					}
					while (!QIsEmpty(&temp)) {													//temp 큐가 차있는 동안
						Enqueue(&ready_q, Dequeue(&temp));										//temp 큐에서 꺼내서 ready 큐에 삽입
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {	//running 큐가 차있다면
			pick = QPeek(&running_q);	//running 큐를 pick
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고 running 큐가 비어 있다면
			pick = Dequeue(&ready_q);					   //ready 큐에서 꺼낸 애를 pick
			Enqueue(&running_q, pick);					   //pick 한 애를 running 큐에 삽입
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_pri.idle_time++; } //running 큐가 비어있다면, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick한 프로세스를 진행

			if (process[pick]->progress == process[pick]->io_start) { //pick 한 프로세스의 progress와 io start이 같아진다면
				int waiting = Dequeue(&running_q);					  //running 큐에서 꺼내서 waiting 큐에 넣는다.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_pri.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_pri.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_pri.avg_turnaround = (float)tot_turn / p_num;
	p_pri.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_pri.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_pri.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void RR() {
	//FCFS에서 time quantum값만 추가한 알고리즘
	//이 함수가 실행되면 먼저 time quantum 값을 사용자에게 입력 받는다
	//각 프로세스의 progress 값을 1씩 증가시키면서 동시에 time_quan 값을 1씩 증가시킴 (time_quan은 Process 구조체의 변수)
	//time_quan이 입력받은 time quantum과 같아지면 ready큐가 비어있다면 time_quan을 0으로 초기화만 하고,
	//ready 큐에 다른 프로세스가 존재한다면, 현재 실행되고 있는 프로세스를 꺼내 ready 큐에 넣고 ready 큐의 프로세스를 running 큐에 넣는다
	printf("[Round Robin Scheduling]\n");
	int i, pick, time;
	rr.idle_time = 0, rr.check = 0;

	Initalize();

	int time_quantum = 0;
	printf("* Please enter time quantum.(1 ~ 5): ");
	while (1) {
		scanf("%d", &time_quantum);
		if (time_quantum > 7 || time_quantum < 1) printf("[ERROR] Time Quantum must be 1 to 4. Please enter time quantum again: ");
		else break;
	}
	for (time = 1; rr.check != p_num; time++) { //Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (time == process[i]->arr_time) { Enqueue(&ready_q, process[i]->pid); }
			if (waiting_q[i] > 0) { waiting_q[i]--; }	//Doing I/O operation...
			if (waiting_q[i] == 0) { Enqueue(&ready_q, i); waiting_q[i]--; } //When the process waits for io burst, it re-enters the ready queue.
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready 큐가 차있고, running 큐가 비어있을 경우
			pick = Dequeue(&ready_q);	//ready 큐에서 뽑은 애를 pick
			Enqueue(&running_q, pick);	//pick 한 애를 running 큐에 삽입
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); rr.idle_time++; } 	//running 큐가 비어있으면 idle time이라고 출력 후 idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;
			process[pick]->time_quan++;

			if (process[pick]->progress == process[pick]->io_start) { //progress와 io start 시간이 같다면, 즉 io operation이 발생했다면
				int waiting = Dequeue(&running_q);					  //running 큐에 있는 프로세스를 꺼내 waiting 큐에 넣음
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waiting큐에 io burst를 할당
				process[waiting]->time_quan = 0;
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				rr.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
			else if (process[pick]->time_quan == time_quantum) {
				process[pick]->time_quan = 0;
				Enqueue(&ready_q, (Dequeue(&running_q)));
			}
		}
	}
	rr.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	rr.avg_turnaround = (float)tot_turn / p_num;
	rr.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", rr.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", rr.avg_turnaround);	printf("*****************************************************************************\n\n");
}

void NP_SJF_IO() {
	//작동방식은 NP_SJF, P_SJF와 같다
	//하지만 NP_SJF과 P_SJF에서의 판단 기준이 cpu_burst, remain_time이었다면
	//여기에서는 io burst time까지 함께 고려한 것이다
	printf("(Additional)[Non-preemptive Shortest Job First Scheduling (I/O + CPU burst)]\n");
	int i, j, tmp, pick, time;
	np_sjf_io.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 };	//임시 행렬 arr을 만들어 cpu burst+io burst를 넣어준다 (ent_btime = p_process->cpu_burst + p_process->io_burst;)
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->ent_btime;
	}
	for (i = 0; i < p_num; i++) { //bubble sort를 통해 cpu burst +io burst가 적은 순서대로 정렬
		for (j = i + 1; j < p_num; j++) {
			if (arr[i][1] > arr[j][1]) {
				tmp = arr[i][0]; arr[i][0] = arr[j][0]; arr[j][0] = tmp;
				tmp = arr[i][1]; arr[i][1] = arr[j][1]; arr[j][1] = tmp;
			}
		}
	}
	for (i = 0; i < p_num; i++) {
		for (j = 0; j < p_num; j++) {
			if (process[i]->pid == arr[j][0])
				process[i]->order = j;
		}
	}
	Queue temp; QueueInit(&temp);

	for (time = 1; np_sjf_io.check != p_num; time++) {
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; } 				 //Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting큐가 비어있거나, 프로세스가 arrive했다면
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid);	//ready 큐가 비어있다면 ready큐에 넣음
				else {														//ready 큐가 비어있지 않다면
					while (process[QPeek(&ready_q)]->order < process[i]->order) {	//ready 큐의 order < 현재 프로세스의 order
						Enqueue(&temp, Dequeue(&ready_q));					//ready 큐에서 꺼내서 temp큐에 넣는다
						if (QIsEmpty(&ready_q)) break;						//ready 큐가 비어있다면 break
					}
					Enqueue(&temp, process[i]->pid);						//temp큐에 process[i] 넣는다
					while (!QIsEmpty(&ready_q)) {							//ready큐가 차있는동안
						Enqueue(&temp, Dequeue(&ready_q));					//ready큐에서 꺼내서 temp큐에 넣는다
					}
					while (!QIsEmpty(&temp)) {								//temp큐가 차있는동안
						Enqueue(&ready_q, Dequeue(&temp));					//temp큐에서 꺼내서 ready큐에 넣는다
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {					//ready 큐가 차있고 running 큐가 비어있다면
			pick = Dequeue(&ready_q);										//ready 큐에서 꺼낸것을 pick
			Enqueue(&running_q, pick);										//pick을 running 큐에 넣음
			process[pick]->start = TRUE;									//시작
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf_io.idle_time++; }	//running 큐가 비어있다면, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//pick 한 프로세스의 progress와 io start이 같아진다면
				int waiting = Dequeue(&running_q);						//running 큐에서 꺼내서 waiting 큐에 넣는다.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				np_sjf_io.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	np_sjf_io.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	np_sjf_io.avg_turnaround = (float)tot_turn / p_num;
	np_sjf_io.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", np_sjf_io.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", np_sjf_io.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void P_SJF_IO() {
	//작동방식은 NP_SJF, P_SJF와 같다
	//하지만 NP_SJF과 P_SJF에서의 판단 기준이 cpu_burst, remain_time이었다면
	//여기에서는 io burst time까지 함께 고려한 것이다
	printf("(Additional)[Preemptive Shortest Job First Scheduling(I/O + CPU burst)]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_sjf_io.idle_time = 0, p_sjf_io.check = 0;

	Initalize();

	for (time = 1; p_sjf_io.check != p_num; time++) {	//Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; process[i]->ent_remain_time--; }	//Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);	//ready큐가 비어있고, running 큐가 비어있으면 ready 큐에 넣는다
				else if (!QIsEmpty(&running_q)) {	//만일 running 큐가 차있다면 
					if (process[i]->ent_remain_time < process[QPeek(&running_q)]->ent_remain_time) { //프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와  io burst time을 고려한 remain time을 비교해
						preem = Dequeue(&running_q);												 //새 프로세스의 io burst time을 고려한remain time이 더 작다면(혹은 같으면서 새 프로세스의 pid가 작다면) running 큐에 들어간다
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }			 //ready 큐가 차있는 동안 ready 큐에서 꺼내서 temp 큐에 넣는다
						Enqueue(&ready_q, preem);													 //running 큐에서 꺼낸 preem을 ready 큐에 넣는다
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }				 //temp가 차있는 동안, temp에서 꺼낸 애들을 ready 큐에 넣는다 (기존에 있던 프로세스를 ready 큐의 최상단으로 가게 하기 위함)
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와 io burst time을 고려한 remain time을 비교해 새 프로세스의 io burst time을 고려한 remain time이 더 작지 않다면
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);	//만일 ready 큐가 비어있다면, 프로세스 i를 ready 큐에 넣는다
						else {	//ready 큐가 비어있지 않다면
							while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {	//현재 ready 큐의 io burst time을 고려한 remain time < 새 프로세스 i의 io burst time을 고려한 remain time 하는 동안
								Enqueue(&temp, Dequeue(&ready_q));												//ready 큐를 꺼내서 temp 큐에 넣는다.
								if (QIsEmpty(&ready_q)) break;													//만일 ready가 비면 break;
							}
							Enqueue(&temp, process[i]->pid);													//프로세스i의 pid를 temp 큐에 넣는다
							while (!QIsEmpty(&ready_q)) {														//ready 큐가 차있는 동안
								Enqueue(&temp, Dequeue(&ready_q));												//ready큐에서 꺼내서 temp큐에 넣는다.
							}
							while (!QIsEmpty(&temp)) {															//temp 큐가 차있는동안
								Enqueue(&ready_q, Dequeue(&temp));												//temp 큐에서 꺼내서 ready 큐에 넣는다
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {			//현재 ready 큐의 프로세스의 io burst time을 고려한 remain time < 새로운 프로세스의 io burst time을 고려한  remain time 하는 동안
						Enqueue(&temp, Dequeue(&ready_q));														//ready 큐에서 꺼내서 temp 큐에 삽입
						if (QIsEmpty(&ready_q)) break;															//만일 ready 큐가 비어있다면 break
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {		//running 큐가 차있다면
			pick = QPeek(&running_q);		//running 큐의 상단을 pick이라고..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {	//ready 큐가 차있고, running 큐가 비어있다면
			pick = Dequeue(&ready_q);						//ready 큐에서 꺼내서 pick한다
			Enqueue(&running_q, pick);						//pick을 running 큐에 넣는다
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf_io.idle_time++; }	//running 큐가 비어있다면, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->ent_remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//progress와 io start 시간이 같다면, 즉 io operation이 발생했다면
				int waiting = Dequeue(&running_q);						//running 큐에 있는 프로세스를 꺼내 waiting 큐에 넣음
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_sjf_io.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_sjf_io.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_sjf_io.avg_turnaround = (float)tot_turn / p_num;
	p_sjf_io.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_sjf_io.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_sjf_io.avg_turnaround);
	printf("*****************************************************************************\n\n");
}
void P_LJF() {
	//io busrt time 이 짧은 순서대로 먼저 프로세싱을 진행한다
	//cpu utilization을 높이는 것에 집중
	//P_SJF와 작동방식은 비슷하다. 
	printf("(Additional)[Preemptive Longest Job First Scheduling(I/O burst)]\n");
	int i, pick, time, preem = 0;
	Queue temp; QueueInit(&temp);
	p_ljf.idle_time = 0, p_ljf.check = 0;

	Initalize();

	for (time = 1; p_ljf.check != p_num; time++) {	//Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; process[i]->remain_time_io--; } //(p_process->remain_time_io = p_process->io_burst;)
			if (waiting_q[i] == 0 || time == process[i]->arr_time) {
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //ready큐가 비어있고, running 큐가 비어있으면 ready 큐에 넣는다
				else if (!QIsEmpty(&running_q)) { //만일 running 큐가 차있다면
					if (process[i]->remain_time_io > process[QPeek(&running_q)]->remain_time_io) {	 //프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와 io burst time을 비교해
						preem = Dequeue(&running_q);												 //새 프로세스의 io burst time이 더 작다면(혹은 같으면서 새 프로세스의 pid가 작다면) running 큐에 들어간다
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }			 //ready 큐가 차있는 동안 ready 큐에서 꺼내서 temp 큐에 넣는다
						Enqueue(&ready_q, preem);													 //running 큐에서 꺼낸 preem을 ready 큐에 넣는다
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }				 //temp가 차있는 동안, temp에서 꺼낸 애들을 ready 큐에 넣는다 (기존에 있던 프로세스를 ready 큐의 최상단으로 가게 하기 위함)
						Enqueue(&running_q, i);													     //프로세스 i를 running 큐에서..
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {	//프로세스가 arrive할 때마다 현재 running 큐에서 실행되고 있는 프로세스와 io burst time을 비교해 새 프로세스의 io burst time이 더 작지 않다면
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);	//만일 ready 큐가 비어있다면, 프로세스 i를 ready 큐에 넣는다
						else {	//ready 큐가 비어있지 않다면
							while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {	//현재 ready 큐의  io burst time < 새 프로세스 i의  io burst time하는 동안
								Enqueue(&temp, Dequeue(&ready_q));											//ready 큐를 꺼내서 temp 큐에 넣는다.
								if (QIsEmpty(&ready_q)) break;												//만일 ready가 비면 break;
							}
							Enqueue(&temp, process[i]->pid);												//프로세스i의 pid를 temp 큐에 넣는다
							while (!QIsEmpty(&ready_q)) {													//ready 큐가 차있는 동안
								Enqueue(&temp, Dequeue(&ready_q));											//ready큐에서 꺼내서 temp큐에 넣는다.
							}
							while (!QIsEmpty(&temp)) {														//temp 큐가 차있는동안
								Enqueue(&ready_q, Dequeue(&temp));											//temp 큐에서 꺼내서 ready 큐에 넣는다
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {			//현재 ready 큐의 프로세스의 io burst time  < 새로운 프로세스의 io burst time  하는 동안
						Enqueue(&temp, Dequeue(&ready_q));													//ready 큐에서 꺼내서 temp 큐에 삽입
						if (QIsEmpty(&ready_q)) break;														//만일 ready 큐가 비어있다면 break	
					}
					Enqueue(&temp, process[i]->pid);
					while (!QIsEmpty(&ready_q)) {
						Enqueue(&temp, Dequeue(&ready_q));
					}
					while (!QIsEmpty(&temp)) {
						Enqueue(&ready_q, Dequeue(&temp));
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {		//running 큐가 차있다면
			pick = QPeek(&running_q);		//running 큐의 상단을 pick이라고..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {	//ready 큐가 차있고, running 큐가 비어있다면
			pick = Dequeue(&ready_q);						//ready 큐에서 꺼내서 pick한다
			Enqueue(&running_q, pick);						//pick을 running 큐에 넣는다
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_ljf.idle_time++; }	//running 큐가 비어있다면, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//progress와 io start 시간이 같다면, 즉 io operation이 발생했다면
				int waiting = Dequeue(&running_q);						//running 큐에 있는 프로세스를 꺼내 waiting 큐에 넣음
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waiting큐에 io burst를 할당
			}
			else if (process[pick]->progress == process[pick]->cpu_burst) {
				p_ljf.check++;
				process[Dequeue(&running_q)]->comp_time = time + 1;
			}
		}
	}
	p_ljf.finish_time = time;

	int tot_turn = 0, tot_burst = 0;
	for (i = 0; i < p_num; i++) {
		tot_turn += process[i]->comp_time - process[i]->arr_time;
		tot_burst += process[i]->cpu_burst;
	}
	p_ljf.avg_turnaround = (float)tot_turn / p_num;
	p_ljf.avg_waiting = (float)(tot_turn - tot_burst) / p_num;
	printf("\n* Average Waiting Time = %.4f", p_ljf.avg_waiting);
	printf("\n* Average Turnaround Time = %.4f\n", p_ljf.avg_turnaround);
	printf("*****************************************************************************\n\n");
}

void evaluation() {
	printf("[Evaluation of each scheduling algorithms]\n");
	printf("-----------------------------------------------------------------------------\n");
	printf("1. FCFS Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", fcfs.avg_waiting, fcfs.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((fcfs.finish_time - fcfs.idle_time) * 100) / fcfs.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("2. Non-preemtive SJF Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_sjf.avg_waiting, np_sjf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_sjf.finish_time - np_sjf.idle_time) * 100) / np_sjf.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("3. Preemptive SJF Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_sjf.avg_waiting, p_sjf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_sjf.finish_time - p_sjf.idle_time) * 100) / p_sjf.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("4. Non-preemptive Priority Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_pri.avg_waiting, np_pri.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_pri.finish_time - np_pri.idle_time) * 100) / np_pri.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("5. Preemptive Priority Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_pri.avg_waiting, p_pri.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_pri.finish_time - p_pri.idle_time) * 100) / p_pri.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("6. Round-Robin Scheduling\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", rr.avg_waiting, rr.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((rr.finish_time - rr.idle_time) * 100) / rr.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("7. (Additional) Non-preemtive SJF Scheduling(I/O + CPU burst)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", np_sjf_io.avg_waiting, np_sjf_io.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((np_sjf_io.finish_time - np_sjf_io.idle_time) * 100) / np_sjf_io.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("8. (Additional) Preemtive SJF Scheduling(I/O + CPU burst)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_sjf_io.avg_waiting, p_sjf_io.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_sjf_io.finish_time - p_sjf_io.idle_time) * 100) / p_sjf_io.finish_time);
	printf("-----------------------------------------------------------------------------\n");
	printf("9. (Additional) Preemtive LJF Scheduling(I/O)\n");
	printf("Average Waiting Time = %.4f, Average Turnaround Time = %.4f\n", p_ljf.avg_waiting, p_ljf.avg_turnaround);
	printf("CPU Utilization = %.2f%%\n", (float)((p_ljf.finish_time - p_ljf.idle_time) * 100) / p_ljf.finish_time);
}
void main() {
	create_process();
	config();
	FCFS();
	NP_SJF();
	P_SJF();
	NP_PRIORITY();
	P_PRIORITY();
	RR();
	NP_SJF_IO();
	P_SJF_IO();
	P_LJF();
	evaluation();
	system("pause");
}