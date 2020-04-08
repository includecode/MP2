/* Copyright (c) 2013 Pablo Oliveira <pablo.oliveira@prism.uvsq.fr>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.  All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "sched.h"
#include "task_description.h"

/* --Scheduler random--*/
tproc *randomscheduler(tlist *procs, tlist *ready, int *delta)
{
    int length = len(ready);
    int selected = rand() % length;
    tnode *p = ready->first;
    for (int i = 0; i < selected; i++)
    {
        p = p->next;
    }
    *delta = rand() % p->proc->remaining + 1;
    return p->proc;
}
/* --Scheduler random--*/

/* --Scheduler fcfs-- */
tproc *fcfs(tlist *procs, tlist *ready, int *delta)
{
    /* take the first in the queue (first come)*/
    tnode *p = ready->first;
    *delta = 1;
    return p->proc;
}
/* --Scheduler fcfs-- */

/* --Scheduler rr-- */
tproc *rr(tlist *procs, tlist *ready, int *delta)
{
    tnode *p = ready->last;

    //Select the last process at each turn (newest process)
    //Send the last proc back to the head of the queue (for an eventual come back if not finished)
    if (len(ready) > 1)
    {
        tnode *f = NULL;

        for (f = ready->first; f->next != NULL; f = f->next)
            ; //this loop just finds the last process in queue
        f->next = ready->first;
        ready->last = ready->first;
        ready->first = ready->last->next;
        ready->last->next = NULL;
    }
    *delta = 1;
    return p->proc;
}
/* --Scheduler rr-- */

/* --Scheduler sjf-- */
tproc *sjf(tlist *procs, tlist *ready, int *delta)
{
    tnode *srt_process = ready->first;

    //find the process which has the shortest lenght
    for (tnode *p = srt_process->next; p != NULL; p = p->next)
    {
        if (p->proc->length < srt_process->proc->length)
            srt_process = p;
    }
fprintf(stderr, " +++++l is %d proc = %d rem = %d\n", srt_process->proc->length, srt_process->proc->pid, srt_process->proc->remaining);
    //Simulate execution of the same process untill the end
    while (srt_process->proc->length > 0)
    {
        srt_process->proc->length--;
        (*delta)++;
    }
fprintf(stderr, " +++++delta is %d proc = %d rem = %d\n", *delta, srt_process->proc->pid, srt_process->proc->remaining);

    return srt_process->proc;
}
/* --Scheduler sjf-- */

/* --Scheduler srtf-- */
tproc *srtf(tlist *procs, tlist *ready, int *delta)
{
    tnode *srt_process = ready->first;

    //find the process which has the shortest remaining time
    for (tnode *p = srt_process->next; p != NULL; p = p->next)
    {
        if (p->proc->remaining < srt_process->proc->remaining)
            srt_process = p;
    }

    *delta = 1;
    return srt_process->proc;
}
/* --Scheduler srtf-- */

/* --Scheduler edf-- */
tproc *edf(tlist *procs, tlist *ready, int *delta)
{

    tnode *edf_process = ready->first;

    //find the process which has the nearest deadline
    for (tnode *p = edf_process->next; p != NULL; p = p->next)
    {
        if (p->proc->activation + p->proc->period < edf_process->proc->activation + edf_process->proc->period)
            edf_process = p;
        else
        {
            //If same activation time, take the shortest length
            if (p->proc->activation + edf_process->proc->period == edf_process->proc->activation + edf_process->proc->period)
            {
                if (p->proc->length < edf_process->proc->length)
                    edf_process = p;
            }
        }
    }
    *delta = 1;
    return edf_process->proc;
}
/* --Scheduler edf-- */

/* --Scheduler rm-- */
tproc *rm(tlist *procs, tlist *ready, int *delta)
{
    tnode *rm_process = ready->first;

    //find the process which has the highest priority
    for (tnode *p = rm_process->next; p != NULL; p = p->next)
    {
        if ((float)1 / p->proc->period > (float)1 / rm_process->proc->period)
        {
            rm_process = p;
        }
        else
        {
            //If same period, take the shortest length
            if ((float)1 / p->proc->period == (float)1 / rm_process->proc->period)
            {
                if (p->proc->length < rm_process->proc->length)
                    rm_process = p;
            }
        }
    }
    *delta = 1;
    return rm_process->proc;
}
/* --Scheduler rm-- */

/* List of ready procs */
tlist ready;

/* List of other procs */
tlist procs;

/* The selected scheduler */
tscheduler scheduler;

/* The scheduling statistics */
tstats stats = {0};

/* display usage message */
void usage()
{
    fail("usage: sched [fcfs, rr, sjf, srtf]\n");
}

/* simulate a single core scheduler, from time 0 to `max_time` */
void simulate(int max_time)
{
    /* array to know when a process is being runned for the first time */
    /* (0) ==> process has never been runned, or (1) ==> process had a previous run */
    char *process_first_touch = malloc(sizeof(char) * len(&procs));
    for (tnode *p = procs.first; p != NULL; p = p->next)
        process_first_touch[p->proc->pid] = 0;

    /* [PERIODIC] array to keep initial activation time of periodic processes */
    char *initial_remainings = malloc(sizeof(char) * len(&procs));
    for (tnode *p = procs.first; p != NULL; p = p->next)
        initial_remainings[p->proc->pid] = p->proc->remaining;

    int time = 0;
    stats.waiting = 0;
    stats.response = 0;
    stats.completion = 0;
    tnode *f = NULL;
    for (f = procs.first; f != NULL; f = f->next)
        stats.completion += f->proc->length;
    while (time <= max_time)
    {
        int old_time = time;
        /* Activate process */
        for (tnode *p = procs.first; p != NULL;)
        {
            tproc *proc = p->proc;
            p = p->next;

            /* Move every process which should be activated,
             * from the procs list to the ready list */
            if (proc->activation <= time)
            {
                del(&procs, proc);
                add(&ready, proc);
                //on sjf process are not added at the time they should be added beacause another
                //process has running priority, so ad this to response time
                stats.response += (time - proc->activation);
            }
        }

        /* If any process is ready, then we can schedule! */
        if (ready.first != NULL)
        {

            int delta = 0;
            /* Call the scheduler */
            tproc *proc = scheduler(&procs, &ready, &delta);

            /*Set process to taken */
            process_first_touch[proc->pid] = 1;

            /* Ensure the scheduler has advanced at least one unit of time */
            assert(delta > 0);

            /* Output task execution */
            /* red color for periodic tasks that dont meet the deadline */
            if (time >= (proc->activation + proc->period) && proc->period > 0)
                printf("\\TaskExecution[color=red]{%d}{%d}{%d}\n", proc->pid, time, time + delta);
            else
                printf("\\TaskExecution{%d}{%d}{%d}\n", proc->pid, time, time + delta);

            /* Advance time by delta */
            time += delta;

            /* Remove delta from chosen process */
            proc->remaining -= delta;

            /* If the process remaining time is less zero or less, 
             * delete it */
            if (proc->remaining <= 0)
            {
                stats.completion += (time - (proc->activation + proc->length));
                stats.waiting += (time - (proc->activation + proc->length));
                del(&ready, proc);
                del(&procs, proc);
                if (proc->period > 0) // periodic task
                {
                    //Update & Send periodic tasks back to the list of processes
                    proc->remaining = initial_remainings[proc->pid];
                    proc->activation = proc->activation + proc->period;
                    add(&procs, proc);
                    printf("\\TaskArrival{%d}{%d}\n", proc->pid, proc->activation);
                    printf("\\TaskDeadline{%d}{%d}\n", proc->pid, proc->activation);
                }
            }

            /* Update response time for other processes */
            for (tnode *p = ready.first; p != NULL; p = p->next)
            {

                if (process_first_touch[p->proc->pid] == 0)
                    stats.response += (time - old_time);
            }
        }
        /* If no process is ready, just advance the simulation timer */
        else
        {
            time += 1;
        }
    }
    /* release memory */
    free(process_first_touch);
    free(initial_remainings);
}

int main(int argc, char *argv[])
{

    /* Parse arguments */
    if (argc != 2)
        usage();

    /* Seed random number generator */
    srand(time(NULL) ^ getpid());

    char *method = argv[1];

    /* The sched argument should be one of fcfs, rr, sjf, srtf */
    if (strcmp(method, "fcfs") == 0)
    {
        scheduler = fcfs;
    }
    else if (strcmp(method, "rr") == 0)
    {
        scheduler = rr;
    }
    else if (strcmp(method, "sjf") == 0)
    {
        scheduler = sjf;
    }
    else if (strcmp(method, "srtf") == 0)
    {
        scheduler = srtf;
    }
    else if (strcmp(method, "edf") == 0)
    {
        scheduler = edf;
    }
    else if (strcmp(method, "rm") == 0)
    {
        scheduler = rm;
    }
    else
    {
        usage();
    }

    /* Add all tasks to the procs queue */
    for (int i = 0; i < sizeof(tasks) / sizeof(tproc); i++)
    {
        add(&procs, &(tasks[i]));
    }

    /* Output RTGrid header */
    printf("\\begin{RTGrid}[width=0.8\\textwidth]{%d}{%d}\n", len(&procs), max_time);

    /* Output task arrivals for all tasks */
    for (tnode *p = procs.first; p != NULL; p = p->next)
    {
        printf("\\TaskArrival{%d}{%d}\n", p->proc->pid, p->proc->activation);
    }

    /* Start scheduling simulation */
    simulate(max_time);

    /* Close RTGrid environment */
    printf("\\end{RTGrid}\\newline\\newline\n");

    /* Print statistics */
    printf("Total completion time = %d\\newline\n", stats.completion);
    printf("Total waiting time = %d\\newline\n", stats.waiting);
    printf("Total response time = %d\\newline\n", stats.response);

    /* Empty the lists if needed */
    del_all(&ready);
    del_all(&procs);

    return 0;
}
