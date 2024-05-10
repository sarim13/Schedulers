#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//process control block (PCB)
struct pcb 
{
	unsigned int pid;
	char pname[20];
	unsigned int ptimeleft;
	unsigned int ptimearrival;
    int ptimestart;
    int ptimend;
};

typedef struct pcb pcb;

//queue node
struct dlq_node 
{
	struct dlq_node *pfwd;
	struct dlq_node *pbck;
	struct pcb *data;
}; 

typedef struct dlq_node dlq_node;

//queue
struct dlq 
{
	struct dlq_node *head;
	struct dlq_node *tail;
};

typedef struct dlq dlq;
 
//function to add a pcb to a new queue node
dlq_node * get_new_node (pcb *ndata) 
{
	if (!ndata)
		return NULL;

	dlq_node *new = malloc(sizeof(dlq_node));
	if(!new)
    {
		fprintf(stderr, "Error: allocating memory\n");exit(1);
	}

	new->pfwd = new->pbck = NULL;
	new->data = ndata;
	return new;
}

//function to add a node to the tail of queue
void add_to_tail (dlq *q, dlq_node *new)
{
	if (!new)
		return;

	if (q->head==NULL)
    {
		if(q->tail!=NULL)
        {
			fprintf(stderr, "DLList inconsitent.\n"); exit(1);
		}
		q->head = new;
		q->tail = q->head;
	}
	else 
    {		
		new->pfwd = q->tail;
		new->pbck = NULL;
		new->pfwd->pbck = new;
		q->tail = new;
	}
}

//function to remove a node from the head of queue
dlq_node* remove_from_head(dlq * const q){
	if (q->head==NULL){ //empty
		if(q->tail!=NULL){fprintf(stderr, "DLList inconsitent.\n"); exit(1);}
		return NULL;
	}
	else if (q->head == q->tail) { //one element
		if (q->head->pbck!=NULL || q->tail->pfwd!=NULL) {
			fprintf(stderr, "DLList inconsitent.\n"); exit(1);
		}

		dlq_node *p = q->head;
		q->head = NULL;
		q->tail = NULL;
	
		p->pfwd = p->pbck = NULL;
		return p;
	}
	else { // normal
		dlq_node *p = q->head;
		q->head = q->head->pbck;
		q->head->pfwd = NULL;
	
		p->pfwd = p->pbck = NULL;
		return p;
	}
}

//function to print our queue
void print_q (const dlq *q) 
{
	dlq_node *n = q->head;
	if (n == NULL)
		return;

	while (n) 
    {
		printf("%s(%d),", n->data->pname, n->data->ptimeleft);
		n = n->pbck;
	}
}

//function to check if the queue is empty
int is_empty (const dlq *q)
{
	if (q->head == NULL && q->tail==NULL)
		return 1;
	else if (q->head != NULL && q->tail != NULL)
		return 0;
	else 
    {
		fprintf(stderr, "Error: DLL queue is inconsistent."); exit(1);
	}
}

//function to sort the queue on completion time
void sort_by_timetocompletion(const dlq *q) 
{ 
	// bubble sort
	dlq_node *start = q->tail;
	dlq_node *end = q->head;
	
	while (start != end) 
    {
		dlq_node *node = start;
		dlq_node *next = node->pfwd;

		while (next != NULL) 
        {
			if (node->data->ptimeleft < next->data->ptimeleft)
            {
				// do a swap
				pcb *temp = node->data;
				node->data = next->data;
				next->data = temp;
			}
			node = next;
			next = node->pfwd;
		}
		end = end ->pbck;
	} 
}

//function to sort the queue on arrival time
void sort_by_arrival_time (const dlq *q) 
{
	// bubble sort
	dlq_node *start = q->tail;
	dlq_node *end = q->head;
	
	while (start != end) 
    {
		dlq_node *node = start;
		dlq_node *next = node->pfwd;

		while (next != NULL) 
        {
			if (node->data->ptimearrival < next->data->ptimearrival)
            {
				// do a swap
				pcb *temp = node->data;
				node->data = next->data;
				next->data = temp;
			}
			node = next;
			next = node->pfwd;
		}
		end = end->pbck;
	}
}

//function to tokenize the one row of data
pcb* tokenize_pdata (char *buf) 
{
	pcb* p = (pcb*) malloc(sizeof(pcb));
	if(!p)
    { 
        fprintf(stderr, "Error: allocating memory.\n"); exit(1);
    }

	char *token = strtok(buf, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token pname\n"); exit(1);
    }  
	strcpy(p->pname, token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token pid\n"); exit(1);
    }  
	p->pid = atoi(token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token duration\n"); exit(1);
    } 
	 
	p->ptimeleft= atoi(token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token arrival time\n"); exit(1);
    }  
	p->ptimearrival = atoi(token);

	token = strtok(NULL, ":\n");
	if(token)
    { 
        fprintf(stderr, "Error: Oh, what've you got at the end of the line?\n");exit(1);
    } 

	return p;
}

//implement the FIFO scheduling code
void sched_FIFO(dlq *const p_fq, int *p_time)
{
    dlq nq;
    nq.tail = NULL;
    nq.head = NULL;
    dlq_node* node = p_fq->head;
    dlq_node* node2 = node->pbck;
    while (node){
        if (*p_time < node->data->ptimearrival){
            printf("%d:idle:empty:\n",((*p_time)++)+1);
        }
        else{
            if (nq.head){
                remove_from_head(&nq);
            }
            node->data->ptimestart = *p_time;
            while (node->data->ptimeleft > 0) {
                while (node2){
                    if ((node2->data->ptimearrival)== *p_time){
                        add_to_tail(&nq, get_new_node(node2->data));
                        node2 = node2->pbck;
                    }
                    else {
                        break;
                    }
                }
                printf("%d:%s:", ((*p_time)++)+1, node->data->pname);
                if (nq.head){
                    print_q(&nq);
                }
                else{
                    printf("empty");
                }
                printf(":\n");
                node->data->ptimeleft = node->data->ptimeleft - 1;
            }
            node->data->ptimend = *p_time;
            node = nq.head;
        }
    }
}
//implement the SJF scheduling code
void sched_SJF(dlq *const p_fq, int *p_time)
{
    dlq nq;
    nq.tail = NULL;
    nq.head = NULL;
    dlq_node* node = p_fq->head;
    dlq_node* node2 = node->pbck;
    add_to_tail(&nq, get_new_node(node->data));
    while (node){
        if (*p_time < node->data->ptimearrival){
            printf("%d:idle:empty:\n",((*p_time)++)+1);
        }
        else{
            if (nq.head){
                remove_from_head(&nq);
            }
            node->data->ptimestart = *p_time;
            while (node->data->ptimeleft > 0) {
                while (node2){
                    if ((node2->data->ptimearrival)== *p_time){
                        add_to_tail(&nq, get_new_node(node2->data));
                        sort_by_timetocompletion(&nq);
                        node2 = node2->pbck;
                    }
                    else {
                        break;
                    }
                }
                printf("%d:%s:", ((*p_time)++)+1, node->data->pname);
                if (nq.head){
                    print_q(&nq);
                }
                else{
                    printf("empty");
                }
                printf(":\n");
                node->data->ptimeleft = node->data->ptimeleft - 1;
            }
            node->data->ptimend = *p_time;
            node = nq.head;
        }
    }
}
//implement the STCF scheduling code
void sched_STCF(dlq *const p_fq, int *p_time)
{
    dlq nq;
    nq.tail = NULL;
    nq.head = NULL;
    dlq_node* node = p_fq->head;
    dlq_node* node2 = node->pbck;
    add_to_tail(&nq, get_new_node(node->data));
    while (node){
        if (*p_time < node->data->ptimearrival){
            printf("%d:idle:empty:\n",((*p_time)++)+1);
        }
        else{
            if (nq.head){
                remove_from_head(&nq);
            }
            if (node->data->ptimestart == -1){
                node->data->ptimestart = *p_time;
            } 
            while (node->data->ptimeleft > 0) {
                while (node2){
                    if ((node2->data->ptimearrival)== *p_time){
                        add_to_tail(&nq, get_new_node(node2->data));
                        sort_by_timetocompletion(&nq);
                        node2 = node2->pbck;
                    }
                    else {
                        break;
                    }
                }
                if ((nq.head)&&(node->data->ptimeleft > nq.head->data->ptimeleft)){
                    add_to_tail(&nq, get_new_node(node->data));
                    sort_by_timetocompletion(&nq);
                    break;
                }
                printf("%d:%s:", ((*p_time)++)+1, node->data->pname);
                if (nq.head){
                    print_q(&nq);
                }
                else{
                    printf("empty");
                }
                printf(":\n");
                node->data->ptimeleft = node->data->ptimeleft - 1;
            }
            if (node->data->ptimeleft == 0){
                node->data->ptimend = *p_time;
            }
            node = nq.head;
        }
    }
}
//implement the RR scheduling code
void sched_RR(dlq *const p_fq, int *p_time)
{
    dlq nq;
    nq.tail = NULL;
    nq.head = NULL;
    dlq_node* node = p_fq->head;
    dlq_node* node2 = node->pbck;
    add_to_tail(&nq, get_new_node(node->data));
    while (node){
        if (*p_time < node->data->ptimearrival){
            printf("%d:idle:empty:\n",((*p_time)++)+1);
        }
        else{
            if (nq.head){
                remove_from_head(&nq);
            }
            if (node->data->ptimestart == -1){
                node->data->ptimestart = *p_time;
            } 
            while (node->data->ptimeleft > 0) {
                while (node2){
                    if ((node2->data->ptimearrival)== *p_time){
                        add_to_tail(&nq, get_new_node(node2->data));
                        node2 = node2->pbck;
                    }
                    else {
                        break;
                    }
                }
                printf("%d:%s:", ((*p_time)++)+1, node->data->pname);
                if (nq.head){
                    print_q(&nq);
                }
                else{
                    printf("empty");
                }
                printf(":\n");
                node->data->ptimeleft = node->data->ptimeleft - 1;
                if ((nq.head)&&(node->data->ptimeleft > 0)){
                    add_to_tail(&nq, get_new_node(node->data));
                    break;
                }
            }
            if (node->data->ptimeleft == 0){
                node->data->ptimend = *p_time;
            }
            node = nq.head;
        }
    }
}

float turnTime(dlq* const q){
    dlq_node* node = q->head;
    int sum = 0;
    while(node){
        sum += (node->data->ptimend) - (node->data->ptimearrival);
        node = node->pbck;
    }
    return sum;
}

float respTime(dlq* const q){
    dlq_node* node = q->head;
    int sum = 0;
    while(node){
        sum += (node->data->ptimestart) - (node->data->ptimearrival);
        node = node->pbck;
    }
    return sum;
}

int main()
{
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
	FILE* file = fopen("test13.txt", "r");
    int N = 0;
    char tech[20]={'\0'};
    char buffer[100]={'\0'};
	fscanf(file,"%d", &N);
	fgets(tech, sizeof(tech), file);
    fgets(tech, sizeof(tech), file);
    dlq queue;
    queue.head = NULL;
    queue.tail = NULL;
    for(int i=0; i<N; ++i)
    {   
    fgets(buffer, sizeof(buffer), file);
        pcb *p = tokenize_pdata(buffer);
        p->ptimend = -1;
        p->ptimestart = -1;
        add_to_tail (&queue, get_new_node(p) );  
    }
    // print_q(&queue);
    unsigned int system_time = 0;
    sort_by_arrival_time (&queue);
    // print_q(&queue);
    
    // run scheduler
    if(!strncmp(tech,"FIFO",4))
        sched_FIFO(&queue, &system_time);
    else if(!strncmp(tech,"SJF",3))
        sched_SJF(&queue, &system_time);
    else if(!strncmp(tech,"STCF",4))
        sched_STCF(&queue, &system_time);
    else if(!strncmp(tech,"RR",2))
        sched_RR(&queue, &system_time);
    else
        fprintf(stderr, "Error: unknown POLICY\n");
    float tput = (float)N/(float)system_time;
    float resT = respTime(&queue)/(float)N;
    float turnT = turnTime(&queue)/(float)N;
    printf("Performance metrics for %s",tech);
    printf("Avg throughput: %f\n",tput);
    printf("Avg response time: %f\n",resT);
    printf("Avg turnaround time: %f\n",turnT);
    return 0;
}