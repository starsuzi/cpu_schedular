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
	//��� �˰��򿡼��� ���������� ���μ����� �� �� ���� �ð��� ��Ÿ���� time ������ 1�� ������Ű�µ�, 
	//�̸� ��ġ�� �ñ�� Evaluation ����ü���� �� ���μ����� ���� �� ����
	//int���� ������ check�� 1�� ������Ű�� check�� ���μ����� ������ �� ���̴�.
	//���� ���μ����� ���� �߿� progress�� arrival time�� ������ ��
	// I/O operation�� �߻��ߴٰ� �ϸ�, running queue�� �ִ� ���μ����� ���� Waiting queue�� �ְ�(waiting_q[i]�� I/O burst �Ҵ�)
	//�ð��� ���� ������ 1�� ���ҽ��� 0�� �Ǹ� I/O operation�� �����Ѵ�.(waiting_q���� ready_q�� running_q�� �̵�)
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
	// ���� ������ ���μ����� ���� �����ϴ� �˰���
	//�� ���μ����� arrival time�� time�� ������ �� ready queue�� ���������� ����.
	//�׸��� running queue�� ��������� ready queue�� ����ִ��� Ȯ���ؼ�
	//����ִٸ� idle�� ����ϰ�, �׷��� �ʴٸ� queue�� ���� �տ� �ִ� ���μ����� ���� running queue�� �ִ´�.
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
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ� running ť�� ������� ���
			pick = Dequeue(&ready_q);
			Enqueue(&running_q, pick); //ready ť���� running ť��
			process[pick]->start = TRUE;
		}

		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); fcfs.idle_time++; } //running ť�� ��������� idle time�̶�� ��� �� idle ++
		else { //running ť�� ������ ���
			if (process[pick]->start == TRUE) { //���� ����
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; //progress ����

			if (process[pick]->progress == process[pick]->io_start) { //progress�� io start �ð��� ���ٸ�, �� io operation�� �߻��ߴٸ�
				int waiting = Dequeue(&running_q); 					  //running ť�� �ִ� ���μ����� ���� waiting ť�� ����
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waitingť�� io burst�� �Ҵ�
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

void NP_SJF() {	//���μ��� ���� �� �Ҵ����� cpu burst time�� �̿��Ѵ�.. �ӽ� ��� arr�� ���� cpu burst�� �־��� �� bubble sort�� ���� cpu burst time�� ���� ������� ����
				//���⿡���� ���μ��� ���� �� �Ҵ����� cpu burst time�� �̿��ϴµ�,
				//�ӽ� ��� arr�� ���� cpu burst�� �־��� �� bubble sort�� ���� cpu burst time�� ���� ������� �����Ѵ�.
				//�̸� ���� ���μ����� arrive�� ������ ready queue���� �ڽ��� cpu burst�� �´� ��ġ�� ���� �Ǹ�
				// running queue�� �ִ� ���μ����� ���� running queue�� ��� �׶� ready queue�� �ִ� ���μ����� running���� ����.
	printf("[Non-preemptive Shortest Job First Scheduling]\n");
	int i, j, tmp, pick, time;
	np_sjf.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 }; //�ӽ� ��� arr�� ����� cpu burst�� �־��� �κ�
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->cpu_burst;
	}
	for (i = 0; i < p_num; i++) { //bubble sort�� ���� cpu burst�� ���� ������� ����
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
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waitingť�� ����ְų�, ���μ����� arrive�ߴٸ�
				if (waiting_q[i] == 0) { waiting_q[i]--; } 			 //�̷��ų�

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid); //ready ť�� ����ִٸ� readyť�� ����
				else {														//ready ť�� ������� �ʴٸ�
					while (process[QPeek(&ready_q)]->order < process[i]->order) { //ready ť�� order < ���� ���μ����� order
						Enqueue(&temp, Dequeue(&ready_q));	//ready ť���� ������ tempť�� �ִ´�
						if (QIsEmpty(&ready_q)) break; 		//ready ť�� ����ִٸ� break
					}
					Enqueue(&temp, process[i]->pid);		//tempť�� process[i] �ִ´�
					while (!QIsEmpty(&ready_q)) {			//readyť�� ���ִµ���
						Enqueue(&temp, Dequeue(&ready_q));	//readyť���� ������ tempť�� �ִ´�
					}
					while (!QIsEmpty(&temp)) {				//tempť�� ���ִµ���
						Enqueue(&ready_q, Dequeue(&temp));	//tempť���� ������ readyť�� �ִ´�
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ� running ť�� ����ִٸ�
			pick = Dequeue(&ready_q); //ready ť���� �������� pick
			Enqueue(&running_q, pick); //pick�� running ť�� ����
			process[pick]->start = TRUE; //����
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf.idle_time++; } //running ť�� ����ִٸ�, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick�� ���μ����� ����

			if (process[pick]->progress == process[pick]->io_start) { //pick �� ���μ����� progress�� io start�� �������ٸ�
				int waiting = Dequeue(&running_q); //running ť���� ������ waiting ť�� �ִ´�.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1; //waitingť�� io burst�� �Ҵ�
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
	//NP_SJF������ �ʱ⿡ cpu burst�� sort�� �ݸ鿡 
	//���⿡�� �� ���μ����� remain_time���� �־ ���� ���μ����� ���� �� ���� ���� �ð��� ��Ÿ���ش�.
	//�׸��� ���μ����� arrive�� ������ ���� running queue���� ����ǰ� �ִ� ���μ����� remain time�� ���� 
	//�� ���μ����� remain time�� �� �۴ٸ�(Ȥ�� �����鼭 �� ���μ����� pid�� �۴ٸ�) running queue�� ���� �ǰ�(preemptive �߻�) 
	// �ű⿡ �ִ� ���μ����� ready queue�� �ֻ������ ���� �Ǹ�, 
	//�׷��� �ʴٸ� �� ���μ����� cpu burst �� �۴ٸ� ready queue���� �ڽ��� cpu burst�� �´� ��ġ�� ���� �ȴ�.(temp Queue�� �����ؼ� ����)
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

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //readyť�� ����ְ�, running ť�� ��������� ready ť�� �ִ´�
				else if (!QIsEmpty(&running_q)) { //���� running ť�� ���ִٸ� 
					if (process[i]->remain_time < process[QPeek(&running_q)]->remain_time) { //���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ����� remain time�� ����
						preem = Dequeue(&running_q);										 //�� ���μ����� remain time�� �� �۴ٸ�(Ȥ�� �����鼭 �� ���μ����� pid�� �۴ٸ�) running ť�� ����
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }   //ready ť�� ���ִ� ���� ready ť���� ������ temp ť�� �ִ´�
						Enqueue(&ready_q, preem);											 //running ť���� ���� preem�� ready ť�� �ִ´�
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }		 //temp�� ���ִ� ����, temp���� ���� �ֵ��� ready ť�� �ִ´� (������ �ִ� ���μ����� ready ť�� �ֻ������ ���� �ϱ� ����)
						Enqueue(&running_q, i);												 //���μ��� i�� running ť����..
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ����� remain time�� ���� �� ���μ����� remain time�� �� ���� �ʴٸ�
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i); //���� ready ť�� ����ִٸ�, ���μ��� i�� ready ť�� �ִ´�
						else { //ready ť�� ������� �ʴٸ�
							while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) { //���� ready ť�� remain time < �� ���μ��� i�� remain time �ϴ� ����
								Enqueue(&temp, Dequeue(&ready_q));									  //ready ť�� ������ temp ť�� �ִ´�.
								if (QIsEmpty(&ready_q)) break;										  //���� ready�� ��� break;
							}
							Enqueue(&temp, process[i]->pid);										  //���μ���i�� pid�� temp ť�� �ִ´�
							while (!QIsEmpty(&ready_q)) {											  //ready ť�� ���ִ� ����
								Enqueue(&temp, Dequeue(&ready_q));									  //readyť���� ������ tempť�� �ִ´�.
							}
							while (!QIsEmpty(&temp)) {												  //temp ť�� ���ִµ���
								Enqueue(&ready_q, Dequeue(&temp));									  //temp ť���� ������ ready ť�� �ִ´�
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time < process[i]->remain_time) { //���� ready ť�� ���μ����� remain time < ���ο� ���μ����� remain time �ϴ� ����
						Enqueue(&temp, Dequeue(&ready_q));									  //ready ť���� ������ temp ť�� ����
						if (QIsEmpty(&ready_q)) break;										  //���� ready ť�� ����ִٸ� break
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
		if (!QIsEmpty(&running_q)) { 	//running ť�� ���ִٸ�
			pick = QPeek(&running_q);	//running ť�� ����� pick�̶��..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ�, running ť�� ����ִٸ�
			pick = Dequeue(&ready_q);					   //ready ť���� ������ pick�Ѵ�
			Enqueue(&running_q, pick);					   //pick�� running ť�� �ִ´�
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf.idle_time++; } //running ť�� ����ִٸ�, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) { //progress�� io start �ð��� ���ٸ�, �� io operation�� �߻��ߴٸ�
				int waiting = Dequeue(&running_q);					  //running ť�� �ִ� ���μ����� ���� waiting ť�� ����
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waitingť�� io burst�� �Ҵ�
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
	//P_SJF���� ���μ����� ��ġ�� ���ϴ� ������ remain time�̾��ٸ�, ���⼭�� �� ���� priority�̴�. 
	//���μ����� ������ �� ���� running ť�� ���ϸ� preemptive, ready ť�� ���� np
	printf("[Non-preemptive Priority Scheduling]\n"); //��� np��
	int i, pick, time;
	Queue temp; QueueInit(&temp);
	np_pri.idle_time = 0, np_pri.check = 0;

	Initalize();
	
	for (time = 1; np_pri.check != p_num; time++) { //Terminate when all processes are finished.
		for (i = 0; i < p_num; i++) {
			if (waiting_q[i] > 0) { waiting_q[i]--; } //Doing I/O operation...
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting ť�� ����ְų�, ���μ����� ������ �ð��� time�� ���ٸ�
				if (waiting_q[i] == 0) { waiting_q[i]--; }			 //waiting ť�� ����ִٸ�, waiting ť --

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid); //ready ť�� ����ִٸ�, �� ���μ����� pid�� ready ť�� ����
				else { //ready ť�� ������� �ʴٸ�
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) { //ready ť�� ���μ����� priority < �� ���μ����� priority �ϴ� ����
						Enqueue(&temp, Dequeue(&ready_q));									//ready ť���� ������ temp�� �ִ´�
						if (QIsEmpty(&ready_q)) break;										//���� ready ť�� ��� break;
					}
					Enqueue(&temp, process[i]->pid);	//�� ���μ��� i�� pid�� temp ť�� ����
					while (!QIsEmpty(&ready_q)) {			//ready ť�� ���ִµ���
						Enqueue(&temp, Dequeue(&ready_q));	//ready ť���� ������ temp ť�� ����
					}
					while (!QIsEmpty(&temp)) {				//temp�� �� �ִµ���
						Enqueue(&ready_q, Dequeue(&temp));	//tempť���� ������ ready ť�� ����
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ�, running ť�� ����ִٸ�
			pick = Dequeue(&ready_q);					   //ready ť���� ���� �ָ� pick
			Enqueue(&running_q, pick);					   //pick �� �ָ� running ť�� ����
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_pri.idle_time++; }  //running ť�� ����ִٸ�, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick�� ���μ����� ����

			if (process[pick]->progress == process[pick]->io_start) { //pick �� ���μ����� progress�� io start�� �������ٸ�
				int waiting = Dequeue(&running_q);					  //running ť���� ������ waiting ť�� �ִ´�.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waitingť�� io burst�� �Ҵ�
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
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waiting ť�� ����ְų�, ���μ����� ������ �ð��� time�� ���ٸ�
				if (waiting_q[i] == 0) { waiting_q[i]--; }			 //waiting ť�� ����ִٸ�, waiting ť --

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //ready ť, running ť�� ����ִٸ�, �� ���μ����� pid�� ready ť�� ����
				else if (!QIsEmpty(&running_q)) { //running ť�� ���ִٸ�
					if (process[i]->priority < process[QPeek(&running_q)]->priority) { //���ο� process�� �켱���� < running ť�� �켱����
						preem = Dequeue(&running_q);								   //running ť�� ������ preem��
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); } //ready ť�� ���ִ� ����, readyť���� ������ temp ť�� �ִ´�
						Enqueue(&ready_q, preem);										   //preem�� readyť�� ����
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }	   //tempť�� ���ִ� ���� temp�� ť�� ���� ready ť�� ����
						Enqueue(&running_q, i);											   //���μ���i�� running ť�� ����
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //non�̶� ���
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i); //ready ť�� ����ִٸ�, ���μ���i�� ready ť�� ����
						else {
							while (process[QPeek(&ready_q)]->priority < process[i]->priority) { // ready ť�� ���μ����� �켱���� < ���ο� ���μ����� �켱���� �ε���
								Enqueue(&temp, Dequeue(&ready_q));								//ready ť�� ������ temp ť�� ����
								if (QIsEmpty(&ready_q)) break;									//ready ť�� ��� break
							}
							Enqueue(&temp, process[i]->pid);									//�� ���μ��� i�� pid�� temp ť�� ����
							while (!QIsEmpty(&ready_q)) {										//ready ť�� ���ִµ���
								Enqueue(&temp, Dequeue(&ready_q));								//readyť�� ������ temp ť�� ����
							}
							while (!QIsEmpty(&temp)) {											//temp ť�� ����ִ� ����
								Enqueue(&ready_q, Dequeue(&temp));								//temp ť���� ������ ready ť�� ����
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->priority < process[i]->priority) {			// ready ť�� �켱���� < ���ο� ���μ����� �켱���� �ε���
						Enqueue(&temp, Dequeue(&ready_q));										//ready ť���� ������ temp ť�� ����
						if (QIsEmpty(&ready_q)) break;											//readyť�� ��� break;
					}
					Enqueue(&temp, process[i]->pid);											//���μ��� i�� pid�� temp ť�� ����
					while (!QIsEmpty(&ready_q)) {												//readyť�� ���ִ� ����
						Enqueue(&temp, Dequeue(&ready_q));										//readyť���� ������ tempť�� ����
					}
					while (!QIsEmpty(&temp)) {													//temp ť�� ���ִ� ����
						Enqueue(&ready_q, Dequeue(&temp));										//temp ť���� ������ ready ť�� ����
					}
				}
			}
		}
		if (!QIsEmpty(&running_q)) {	//running ť�� ���ִٸ�
			pick = QPeek(&running_q);	//running ť�� pick
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ� running ť�� ��� �ִٸ�
			pick = Dequeue(&ready_q);					   //ready ť���� ���� �ָ� pick
			Enqueue(&running_q, pick);					   //pick �� �ָ� running ť�� ����
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_pri.idle_time++; } //running ť�� ����ִٸ�, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++; // pick�� ���μ����� ����

			if (process[pick]->progress == process[pick]->io_start) { //pick �� ���μ����� progress�� io start�� �������ٸ�
				int waiting = Dequeue(&running_q);					  //running ť���� ������ waiting ť�� �ִ´�.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waitingť�� io burst�� �Ҵ�
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
	//FCFS���� time quantum���� �߰��� �˰���
	//�� �Լ��� ����Ǹ� ���� time quantum ���� ����ڿ��� �Է� �޴´�
	//�� ���μ����� progress ���� 1�� ������Ű�鼭 ���ÿ� time_quan ���� 1�� ������Ŵ (time_quan�� Process ����ü�� ����)
	//time_quan�� �Է¹��� time quantum�� �������� readyť�� ����ִٸ� time_quan�� 0���� �ʱ�ȭ�� �ϰ�,
	//ready ť�� �ٸ� ���μ����� �����Ѵٸ�, ���� ����ǰ� �ִ� ���μ����� ���� ready ť�� �ְ� ready ť�� ���μ����� running ť�� �ִ´�
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
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) { //ready ť�� ���ְ�, running ť�� ������� ���
			pick = Dequeue(&ready_q);	//ready ť���� ���� �ָ� pick
			Enqueue(&running_q, pick);	//pick �� �ָ� running ť�� ����
			process[pick]->start = TRUE;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); rr.idle_time++; } 	//running ť�� ��������� idle time�̶�� ��� �� idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;
			process[pick]->time_quan++;

			if (process[pick]->progress == process[pick]->io_start) { //progress�� io start �ð��� ���ٸ�, �� io operation�� �߻��ߴٸ�
				int waiting = Dequeue(&running_q);					  //running ť�� �ִ� ���μ����� ���� waiting ť�� ����
				waiting_q[waiting] = process[waiting]->io_burst + 1;  //waitingť�� io burst�� �Ҵ�
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
	//�۵������ NP_SJF, P_SJF�� ����
	//������ NP_SJF�� P_SJF������ �Ǵ� ������ cpu_burst, remain_time�̾��ٸ�
	//���⿡���� io burst time���� �Բ� ����� ���̴�
	printf("(Additional)[Non-preemptive Shortest Job First Scheduling (I/O + CPU burst)]\n");
	int i, j, tmp, pick, time;
	np_sjf_io.idle_time = 0;

	Initalize();

	int arr[10][2] = { 11 };	//�ӽ� ��� arr�� ����� cpu burst+io burst�� �־��ش� (ent_btime = p_process->cpu_burst + p_process->io_burst;)
	for (i = 0; i < p_num; i++) {
		arr[i][0] = i;
		arr[i][1] = process[i]->ent_btime;
	}
	for (i = 0; i < p_num; i++) { //bubble sort�� ���� cpu burst +io burst�� ���� ������� ����
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
			if (waiting_q[i] == 0 || time == process[i]->arr_time) { //waitingť�� ����ְų�, ���μ����� arrive�ߴٸ�
				if (waiting_q[i] == 0) { waiting_q[i]--; }

				if (QIsEmpty(&ready_q))	Enqueue(&ready_q, process[i]->pid);	//ready ť�� ����ִٸ� readyť�� ����
				else {														//ready ť�� ������� �ʴٸ�
					while (process[QPeek(&ready_q)]->order < process[i]->order) {	//ready ť�� order < ���� ���μ����� order
						Enqueue(&temp, Dequeue(&ready_q));					//ready ť���� ������ tempť�� �ִ´�
						if (QIsEmpty(&ready_q)) break;						//ready ť�� ����ִٸ� break
					}
					Enqueue(&temp, process[i]->pid);						//tempť�� process[i] �ִ´�
					while (!QIsEmpty(&ready_q)) {							//readyť�� ���ִµ���
						Enqueue(&temp, Dequeue(&ready_q));					//readyť���� ������ tempť�� �ִ´�
					}
					while (!QIsEmpty(&temp)) {								//tempť�� ���ִµ���
						Enqueue(&ready_q, Dequeue(&temp));					//tempť���� ������ readyť�� �ִ´�
					}
				}
			}
		}
		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {					//ready ť�� ���ְ� running ť�� ����ִٸ�
			pick = Dequeue(&ready_q);										//ready ť���� �������� pick
			Enqueue(&running_q, pick);										//pick�� running ť�� ����
			process[pick]->start = TRUE;									//����
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); np_sjf_io.idle_time++; }	//running ť�� ����ִٸ�, idle ++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//pick �� ���μ����� progress�� io start�� �������ٸ�
				int waiting = Dequeue(&running_q);						//running ť���� ������ waiting ť�� �ִ´�.. 
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waitingť�� io burst�� �Ҵ�
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
	//�۵������ NP_SJF, P_SJF�� ����
	//������ NP_SJF�� P_SJF������ �Ǵ� ������ cpu_burst, remain_time�̾��ٸ�
	//���⿡���� io burst time���� �Բ� ����� ���̴�
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

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid);	//readyť�� ����ְ�, running ť�� ��������� ready ť�� �ִ´�
				else if (!QIsEmpty(&running_q)) {	//���� running ť�� ���ִٸ� 
					if (process[i]->ent_remain_time < process[QPeek(&running_q)]->ent_remain_time) { //���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ�����  io burst time�� ����� remain time�� ����
						preem = Dequeue(&running_q);												 //�� ���μ����� io burst time�� �����remain time�� �� �۴ٸ�(Ȥ�� �����鼭 �� ���μ����� pid�� �۴ٸ�) running ť�� ����
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }			 //ready ť�� ���ִ� ���� ready ť���� ������ temp ť�� �ִ´�
						Enqueue(&ready_q, preem);													 //running ť���� ���� preem�� ready ť�� �ִ´�
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }				 //temp�� ���ִ� ����, temp���� ���� �ֵ��� ready ť�� �ִ´� (������ �ִ� ���μ����� ready ť�� �ֻ������ ���� �ϱ� ����)
						Enqueue(&running_q, i);
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else { //���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ����� io burst time�� ����� remain time�� ���� �� ���μ����� io burst time�� ����� remain time�� �� ���� �ʴٸ�
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);	//���� ready ť�� ����ִٸ�, ���μ��� i�� ready ť�� �ִ´�
						else {	//ready ť�� ������� �ʴٸ�
							while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {	//���� ready ť�� io burst time�� ����� remain time < �� ���μ��� i�� io burst time�� ����� remain time �ϴ� ����
								Enqueue(&temp, Dequeue(&ready_q));												//ready ť�� ������ temp ť�� �ִ´�.
								if (QIsEmpty(&ready_q)) break;													//���� ready�� ��� break;
							}
							Enqueue(&temp, process[i]->pid);													//���μ���i�� pid�� temp ť�� �ִ´�
							while (!QIsEmpty(&ready_q)) {														//ready ť�� ���ִ� ����
								Enqueue(&temp, Dequeue(&ready_q));												//readyť���� ������ tempť�� �ִ´�.
							}
							while (!QIsEmpty(&temp)) {															//temp ť�� ���ִµ���
								Enqueue(&ready_q, Dequeue(&temp));												//temp ť���� ������ ready ť�� �ִ´�
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->ent_remain_time < process[i]->ent_remain_time) {			//���� ready ť�� ���μ����� io burst time�� ����� remain time < ���ο� ���μ����� io burst time�� �����  remain time �ϴ� ����
						Enqueue(&temp, Dequeue(&ready_q));														//ready ť���� ������ temp ť�� ����
						if (QIsEmpty(&ready_q)) break;															//���� ready ť�� ����ִٸ� break
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
		if (!QIsEmpty(&running_q)) {		//running ť�� ���ִٸ�
			pick = QPeek(&running_q);		//running ť�� ����� pick�̶��..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {	//ready ť�� ���ְ�, running ť�� ����ִٸ�
			pick = Dequeue(&ready_q);						//ready ť���� ������ pick�Ѵ�
			Enqueue(&running_q, pick);						//pick�� running ť�� �ִ´�
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_sjf_io.idle_time++; }	//running ť�� ����ִٸ�, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->ent_remain_time--;
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//progress�� io start �ð��� ���ٸ�, �� io operation�� �߻��ߴٸ�
				int waiting = Dequeue(&running_q);						//running ť�� �ִ� ���μ����� ���� waiting ť�� ����
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waitingť�� io burst�� �Ҵ�
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
	//io busrt time �� ª�� ������� ���� ���μ����� �����Ѵ�
	//cpu utilization�� ���̴� �Ϳ� ����
	//P_SJF�� �۵������ ����ϴ�. 
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

				if (QIsEmpty(&ready_q) && QIsEmpty(&running_q))	Enqueue(&ready_q, process[i]->pid); //readyť�� ����ְ�, running ť�� ��������� ready ť�� �ִ´�
				else if (!QIsEmpty(&running_q)) { //���� running ť�� ���ִٸ�
					if (process[i]->remain_time_io > process[QPeek(&running_q)]->remain_time_io) {	 //���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ����� io burst time�� ����
						preem = Dequeue(&running_q);												 //�� ���μ����� io burst time�� �� �۴ٸ�(Ȥ�� �����鼭 �� ���μ����� pid�� �۴ٸ�) running ť�� ����
						while (!QIsEmpty(&ready_q)) { Enqueue(&temp, Dequeue(&ready_q)); }			 //ready ť�� ���ִ� ���� ready ť���� ������ temp ť�� �ִ´�
						Enqueue(&ready_q, preem);													 //running ť���� ���� preem�� ready ť�� �ִ´�
						while (!QIsEmpty(&temp)) { Enqueue(&ready_q, Dequeue(&temp)); }				 //temp�� ���ִ� ����, temp���� ���� �ֵ��� ready ť�� �ִ´� (������ �ִ� ���μ����� ready ť�� �ֻ������ ���� �ϱ� ����)
						Enqueue(&running_q, i);													     //���μ��� i�� running ť����..
						if (process[preem]->preemptive == 1) {
							process[preem]->preemptive = 0;
						}
					}
					else {	//���μ����� arrive�� ������ ���� running ť���� ����ǰ� �ִ� ���μ����� io burst time�� ���� �� ���μ����� io burst time�� �� ���� �ʴٸ�
						if (QIsEmpty(&ready_q)) Enqueue(&ready_q, i);	//���� ready ť�� ����ִٸ�, ���μ��� i�� ready ť�� �ִ´�
						else {	//ready ť�� ������� �ʴٸ�
							while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {	//���� ready ť��  io burst time < �� ���μ��� i��  io burst time�ϴ� ����
								Enqueue(&temp, Dequeue(&ready_q));											//ready ť�� ������ temp ť�� �ִ´�.
								if (QIsEmpty(&ready_q)) break;												//���� ready�� ��� break;
							}
							Enqueue(&temp, process[i]->pid);												//���μ���i�� pid�� temp ť�� �ִ´�
							while (!QIsEmpty(&ready_q)) {													//ready ť�� ���ִ� ����
								Enqueue(&temp, Dequeue(&ready_q));											//readyť���� ������ tempť�� �ִ´�.
							}
							while (!QIsEmpty(&temp)) {														//temp ť�� ���ִµ���
								Enqueue(&ready_q, Dequeue(&temp));											//temp ť���� ������ ready ť�� �ִ´�
							}
						}
					}
				}
				else {
					while (process[QPeek(&ready_q)]->remain_time_io > process[i]->remain_time_io) {			//���� ready ť�� ���μ����� io burst time  < ���ο� ���μ����� io burst time  �ϴ� ����
						Enqueue(&temp, Dequeue(&ready_q));													//ready ť���� ������ temp ť�� ����
						if (QIsEmpty(&ready_q)) break;														//���� ready ť�� ����ִٸ� break	
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
		if (!QIsEmpty(&running_q)) {		//running ť�� ���ִٸ�
			pick = QPeek(&running_q);		//running ť�� ����� pick�̶��..
			process[pick]->start = TRUE;
		}

		if (!QIsEmpty(&ready_q) && QIsEmpty(&running_q)) {	//ready ť�� ���ְ�, running ť�� ����ִٸ�
			pick = Dequeue(&ready_q);						//ready ť���� ������ pick�Ѵ�
			Enqueue(&running_q, pick);						//pick�� running ť�� �ִ´�
			process[pick]->start = TRUE;
			process[pick]->preemptive = 1;
		}
		if (QIsEmpty(&running_q)) { printf("TIME %d ~ %d\t: IDLE\n", time - 1, time); p_ljf.idle_time++; }	//running ť�� ����ִٸ�, idle time++
		else {
			if (process[pick]->start == TRUE) {
				process[pick]->start = FALSE;
			}
			printf("TIME %d ~ %d\t: P[%d]\n", time - 1, time, pick);
			process[pick]->progress++;

			if (process[pick]->progress == process[pick]->io_start) {	//progress�� io start �ð��� ���ٸ�, �� io operation�� �߻��ߴٸ�
				int waiting = Dequeue(&running_q);						//running ť�� �ִ� ���μ����� ���� waiting ť�� ����
				waiting_q[waiting] = process[waiting]->io_burst + 1;	//waitingť�� io burst�� �Ҵ�
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