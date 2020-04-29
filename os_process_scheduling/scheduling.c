#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

const int max_name_length = 32;
const int max_numP = 100;
const int time_quantum = 500;
const long long n = 1000000000;
// malloc should be used if array may be too large
int numP_now = 0;
int numP_finish = 0;
#define FIFO 0
#define RR 1
#define SJF 2
#define PSJF 3
typedef struct process
{
    char *name;
    int readyT;
    int execT;
    int ID;
} Process;
// function for Process sorting
int compare_Process(const void *a, const void *b)
{
    Process *P1 = (Process *)a;
    Process *P2 = (Process *)b;
    if(P1->readyT < P2->readyT)
        return -1;
    if(P1->readyT > P2->readyT)
        return 1;
    // if readyT is equal, compare their ID
    if(P1->ID < P2->ID)
        return -1;
    if(P1->ID > P2->ID)
        return 1;
    return 0;
}
// function for process list
void insertP(Process **waiting_list, int policy, Process *P)
{
    numP_now++; 
    if(policy == FIFO) // just insert new process to the list
        waiting_list[numP_now - 1] = P;
    if(policy == RR){
        waiting_list[numP_now - 1] = P;
        if(numP_now > 1){
            if (waiting_list[numP_now-1]->readyT != waiting_list[numP_now-2]->readyT){
                Process *temp = waiting_list[numP_now - 1];
                waiting_list[numP_now-1] = waiting_list[numP_now - 2];
                waiting_list[numP_now - 2] = temp;
            }
        }
    }
    if(policy == SJF || policy == PSJF) // need to find appropriate place according to execT
    {
        waiting_list[numP_now - 1] = P;
        for(int i = numP_now - 1; i > 0 && waiting_list[i]->execT < waiting_list[i - 1]->execT; i--)
        {
            // new process should be swapped to left
            Process *temp = waiting_list[i];
            waiting_list[i] = waiting_list[i - 1];
            waiting_list[i - 1] = temp;
        }
    }
}
int execP(Process **waiting_list, int policy) // return how long the process should be executed continuously
{
    if(policy == FIFO || policy == SJF) // first process in the waiting_list will be done
    {
        int exec_length = waiting_list[0]->execT;
        waiting_list[0]->execT = 0;
        waiting_list[0] = NULL;
        numP_now--;
        numP_finish++;
        for(int i = 1; i <= numP_now; i++)
        {
            Process *temp = waiting_list[i];
            waiting_list[i] = waiting_list[i - 1];
            waiting_list[i - 1] = temp;
        }
        return exec_length; 
    }
    if(policy == RR)
    {
        int exec_length;
        if(waiting_list[0]->execT > time_quantum)
        {
            exec_length = time_quantum;
            waiting_list[0]->execT -= time_quantum;
        }
        else
        {
            exec_length = waiting_list[0]->execT;
            waiting_list[0]->execT = 0;
            waiting_list[0] = NULL; // this process is done
        }
        for(int i = 1; i < numP_now; i++)
        {
            Process *temp = waiting_list[i];
            waiting_list[i] = waiting_list[i - 1];
            waiting_list[i - 1] = temp;
        }
        if(waiting_list[numP_now - 1] == NULL)
        {
            numP_now--;
            numP_finish++;
        }
        

        return exec_length;
    }
    if(policy == PSJF) // execute only one time unit
    {
        waiting_list[0]->execT--;
        if(waiting_list[0]->execT == 0)
        {
            waiting_list[0] = NULL;
            numP_now--;
            numP_finish++;
            for(int i = 1; i <= numP_now; i++)
            {
                Process *temp = waiting_list[i];
                waiting_list[i] = waiting_list[i - 1];
                waiting_list[i - 1] = temp;
            }
        }
        return 1;
    }
}
// main scheduling program
int main()
{
    Process P[max_numP];
    char policy_name[5];
    scanf("%s", policy_name);
    int numP;
    scanf("%d", &numP);
    for(int i = 0; i < numP; i++)
    {
        char *process_name = malloc(max_name_length * sizeof(char));
        int ready_time;
        int execution_time;
        scanf("%s %d %d", process_name, &ready_time, &execution_time);
        P[i].name = process_name;
        P[i].readyT = ready_time;
        P[i].execT = execution_time;
        P[i].ID = i;
    }
    // sort P according to readyT
    qsort(P, numP, sizeof(Process), compare_Process);
    int policy = 0;
    char policy_list[4][5] = {"FIFO", "RR", "SJF", "PSJF"};
    for(int i = 0; i < 4; i++)
        if(strcmp(policy_name, policy_list[i]) == 0)
            policy = i;
    Process *waiting_list[max_numP];
    for(int i = 0; i < max_numP; i++)
        waiting_list[i] = NULL;
    
    //set CPU
    
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(0, &cpu_mask);
    
    if(sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask) != 0){
        perror("sched_setaffinity error");
        exit(EXIT_FAILURE);
    }

    //raise the priority
    
    const int priorityH = 80;//sched_get_priority_max(SCHED_RR);
    const int priorityL = 10;//sched_get_priority_min(SCHED_RR);
    //printf("H:%d L:%d\n",priorityH,priorityL); 
    struct sched_param param;
    param.sched_priority = 50;
    //Just want to make sure it won't get preempted by other processes on its CPU
    pid_t pidP = getpid();
    if(sched_setscheduler(pidP, SCHED_FIFO, &param) != 0) {
        perror("sched_setscheduler error");
        exit(EXIT_FAILURE);  
    }
    
    // P already sorted in ascending order of readyT
    int time_count = 0;
    int fork_count = 0;
    Process *exec_process = NULL;
    Process *exec_process_last = NULL;
    int numP_finish_last = 0;
    int exec_length = 0;
    pid_t pids[max_numP];
    while(!(numP_finish == numP && exec_length == 0))
    {
        //fork the processes that are ready at time_count
        //long start_time = syscall(411);
        //long start_time_s = start_time / n;
        //long start_time_ns = start_time % n;
        while(P[fork_count].readyT <= time_count && fork_count < numP)
        {
            
            //printf("%s : fork at time %d\n", P[fork_count].name, time_count);
            pid_t pid = fork();
            
            if(pid < 0)   
                printf("error in fork!");   
            else if(pid == 0) {
                // child
                char exec_time[10];
                sprintf(exec_time, "%d", P[fork_count].execT);
                char start_time_string[20]; 
                //char start_time_ns_string[20];
                //sprintf(start_time_string, "%ld", start_time);
                //sprintf(start_time_ns_string, "%lld", start_time_ns);
                if(execlp("./process", "process", P[fork_count].name, exec_time, (char *)NULL) < 0){
                    perror("execlp error");
                    exit(EXIT_FAILURE);
                }
            }  
            // parent
            //change the newly forked child's CPU
            cpu_set_t cpu_mask;
            CPU_ZERO(&cpu_mask);
            CPU_SET(1, &cpu_mask);

            if(sched_setaffinity(pid, sizeof(cpu_set_t), &cpu_mask) != 0){
                perror("sched_setaffinity error");
                exit(EXIT_FAILURE);
            }

            pids[P[fork_count].ID] = pid; // use ID (also 0 ~ numP - 1) as index to store pids
            //not RR
            //if policy != 1:
            insertP(waiting_list, policy, (P + fork_count));
            fork_count++;
        }
        
        // decide next process  
        
        // no process is waiting
        if(waiting_list[0] == NULL && exec_length == 0)
        {
            time_count++;
            volatile unsigned long i;
            for(i=0;i<1000000UL;i++); 
            exec_process_last = NULL;
        }
        else
        {
            // find next process in the list
            if(exec_length == 0)
            {
                exec_process = waiting_list[0];
                exec_length = execP(waiting_list, policy);
                //change priority if a different process is going to run
                if(exec_process_last == NULL || exec_process_last->execT == 0)
                {
		//printf("Set %s to high at time %d\n", exec_process->name, time_count);
                    pid_t pid = pids[exec_process->ID];
		    //printf("PID: %d\n",pid);
                    param.sched_priority = priorityH;
                    if(sched_setscheduler(pid, SCHED_RR, &param) != 0) {
                        perror("sched_setscheduler error");
                        exit(EXIT_FAILURE);
                    }
			//if(sched_getscheduler(pid) == SCHED_RR)
			//puts("RR");
                }
                // recover priority of last process
                else
		{
		//printf("Set %s to high, recover %s to low at time %d\n", exec_process->name, exec_process_last->name, time_count);
                    pid_t pid_last = pids[exec_process_last->ID];
                    pid_t pid = pids[exec_process->ID];
		    //printf("PID:%d\n",pid);
                    param.sched_priority = priorityL;
                    if(sched_setscheduler(pid_last, SCHED_RR, &param) != 0) {
                        perror("sched_setscheduler error");
                        exit(EXIT_FAILURE);  
                    }

                    param.sched_priority = priorityH;
                    if(sched_setscheduler(pid, SCHED_FIFO, &param) != 0) {
                        perror("sched_setscheduler error");
                        exit(EXIT_FAILURE);  
                    }
                }	
            }

            exec_length--;
            time_count++;
            volatile unsigned long i;
            for(i=0;i<1000000UL;i++); 
            
            // exec time section for this process is over
            if(exec_length == 0)
            {
                // check if the process has terminated, if so, wait for it
                if(numP_finish == numP_finish_last + 1)
                {
                    //printf("%s : end at time %d\n", exec_process->name, time_count);
                    int status;
                    if(waitpid(pids[exec_process->ID], &status, 0) == -1)
                    {
                        perror("waitpid error");
                        exit(EXIT_FAILURE);
                    }
                }

                exec_process_last = exec_process;
                numP_finish_last = numP_finish;
            }   
        }    
    }

    for(int i = 0; i < numP; i++){
        printf("%s ", P[i].name);
        printf("%d\n", pids[P[i].ID]);
    }

}
