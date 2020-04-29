#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <fstream>
#include <string>
#include "limits.h"
using namespace std;
/*******************************/
/*   some define (changable)   */
/*******************************/
#define MaxExecTime 10000
#define MaxProNum   10

/**********************/
/*   struct Process   */
/**********************/
typedef struct Process {
    // constructors
    Process(string s, int r, int e): finSche(false), leftExec_t(e) {
        name = s;
        ready_t = r;
        exec_t = e;
    }

    Process() {}

    // class member
    string  name;
    int     ready_t;
    int     exec_t;
    int     leftExec_t;
    bool    finSche; // already scheduled
} Process;


/****************************/
/*   struct SJF scheduler   */
/****************************/
typedef struct SJF {
    //constructor
    SJF(): numPro(0), totalExecTime(0), doneSche(false) { // totalExecTime start with 0, init = -1
    }

    // not yet implement
    void readfile() {
        // while(getline) {
            // new Process pro;
            // pro_arr[i] = pro;
            // numPro ++
        // }
    }

    Process nextProcess(int recent_t) {
        Process nPro("NULL", -1, INT_MAX); 
        int position = -1; // because nPro = pro_arr[i] is pass by value, we need to record its position and change its finSche

        for(int i = 0; i < numPro; ++i) {
            if(pro_arr[i].finSche == false 
                    && pro_arr[i].exec_t < nPro.exec_t
                    && pro_arr[i].ready_t <= recent_t ) {
                nPro = pro_arr[i];
                position = i;
            }
        }

        if(position > -1) { // not the initial one
            pro_arr[position].finSche = true;
        }
        return nPro;
    }

    void scheduling() {
        // check if the processes have been scheduled
        if(doneSche) cerr << "The processes are already scheduled(SJF)" << endl;

        int finSche = 0; // finish scheduling, already finish scheduling to finSche msec
        int finPro = 0;  // number of process finish scheduling

        for(int i = 0; i < MaxExecTime; ++i) {
            if(finPro == numPro) // finish scheduling every process
                break;
            Process nPro = nextProcess(i);

            if(nPro.name != "NULL") {
                for(int j = 0; j < nPro.exec_t; ++j) {
                    schedule_arr[i+j] = nPro;
                    totalExecTime ++;
                }
                i += (nPro.exec_t - 1); // for loop initially add 1
                finPro ++;
                continue; // don't go through totalExecTime ++;
            }

            schedule_arr[i] = nPro;
            totalExecTime ++;
        }

        doneSche = true;
    }

    void start() {
        readfile();
        scheduling();
    }

    // class member
    Process pro_arr[MaxProNum];            
    Process schedule_arr[MaxExecTime];    // output this array
    int     numPro;             // record total process number
    int     totalExecTime;      // for schedule_arr use, record its valid length, 
                                // totalExecTime = end_of_scheduler_time + 1 !!!!!
    bool    doneSche;           // check if the processes are being scheduled

} SJF;


/*****************************/
/*   struct FIFO scheduler   */
/*****************************/
typedef struct FIFO {
     
    //constructor
    FIFO(): numPro(0), totalExecTime(0), doneSche(false) { // totalExecTime start with 0, init = -1
    }

    // not yet implement
    void readfile() {
        // while(getline) {
            // new Process pro;
            // pro_arr[i] = pro;
            // numPro ++
        // }
    }

    Process nextProcess(int recent_t) {
        Process nPro("NULL", INT_MAX, 0); 
        int position = -1; // because nPro = pro_arr[i] is pass by value, we need to record its position and change its finSche

        for(int i = 0; i < numPro; ++i) {
            if(pro_arr[i].finSche == false 
                    && pro_arr[i].ready_t < nPro.ready_t
                    && pro_arr[i].ready_t <= recent_t ) {
                nPro = pro_arr[i];
                position = i;
            }
        }

        if(position > -1) { // not the initial one
            pro_arr[position].finSche = true;
        }
        return nPro;
    }

    void scheduling() {
        // check if the processes have been scheduled
        if(doneSche) cerr << "The processes are already scheduled(FIFO)" << endl;

        int finSche = 0; // finish scheduling, already finish scheduling to finSche msec
        int finPro = 0;  // number of process finish scheduling

        for(int i = 0; i < MaxExecTime; ++i) {
            if(finPro == numPro) { // finish scheduling every process
                break;
            }
            Process nPro = nextProcess(i);

            if(nPro.name != "NULL") {
                for(int j = 0; j < nPro.exec_t; ++j) {
                    schedule_arr[i+j] = nPro;
                    totalExecTime ++;
                }
                i += (nPro.exec_t - 1); // for loop initially add 1
                finPro ++;
                continue; // don't go through totalExecTime ++;
            }

            schedule_arr[i] = nPro;
            totalExecTime ++;
        }

        doneSche = true;
    }

    void start() {
        readfile();
        scheduling();
    }
    // class member
    Process pro_arr[MaxProNum];            
    Process schedule_arr[MaxExecTime];    // output this array
    int     numPro;             // record total process number
    int     totalExecTime;      // for schedule_arr use, record its valid length, 
                                // totalExecTime = end_of_scheduler_time + 1 !!!!!
    bool    doneSche;           // check if the processes are being scheduled
} FIFO;

/*****************************/
/*   struct PSJF scheduler   */
/*****************************/
typedef struct PSJF {
    //constructor
    PSJF(): numPro(0), totalExecTime(0), doneSche(false), finPro(0) { 
        // totalExecTime start with 0, init = -1
    }

    // not yet implement
    void readfile() {
        // while(getline) {
            // new Process pro;
            // pro_arr[i] = pro;
            // numPro ++
        // }
    }

    Process nextProcess(int recent_t) {
        Process nPro("NULL", -1, INT_MAX); 
        int position = -1; // because nPro = pro_arr[i] is pass by value, we need to record its position and change its finSche

        for(int i = 0; i < numPro; ++i) {
            if(pro_arr[i].finSche == false 
                    && pro_arr[i].exec_t < nPro.leftExec_t
                    && pro_arr[i].ready_t <= recent_t ) {
                nPro = pro_arr[i];
                position = i;
            }
        }

        if(position > -1) {
            pro_arr[position].leftExec_t --;
            if(pro_arr[position].leftExec_t == 0) { // not the initial one
                pro_arr[position].finSche = true;
                finPro ++;
            }
        }

        return nPro;
    }

    void scheduling() {
        // check if the processes have been scheduled
        if(doneSche) cerr << "The processes are already scheduled(PSJF)" << endl;

        int finSche = 0; // finish scheduling, already finish scheduling to finSche msec

        for(int i = 0; i < MaxExecTime; ++i) {
            if(finPro == numPro) // finish scheduling every process
                break;
            Process nPro = nextProcess(i);

            schedule_arr[i] = nPro;
            totalExecTime ++;
        }

        doneSche = true;
    }

    void start() {
        readfile();
        scheduling();
    }

    // class member
    Process pro_arr[MaxProNum];            
    Process schedule_arr[MaxExecTime];    // output this array
    int     numPro;             // record total process number
    int     totalExecTime;      // for schedule_arr use, record its valid length, 
                                // totalExecTime = end_of_scheduler_time + 1 !!!!!
    int     finPro;             // number of process finish scheduling
    bool    doneSche;           // check if the processes are being scheduled

} PSJF;

#endif
