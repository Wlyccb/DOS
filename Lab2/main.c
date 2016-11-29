/*  main.c  - main */

#include <xinu.h>
#define N 3


/* Define your circular buffer structure and semaphore variables here */
/* */
struct mailbox {
    pid32 pid;
    int32 head;
    int32 tail;
    umsg32 box[10];
} ;

pid32 receive_id;
pid32 receive_id2;
pid32 send_id;
struct mailbox boxtable[N]; //??????
sid32 pmutex;
sid32 tmutex;


syscall sendMsg (pid32 pid,umsg32 msg ) {
    intmask mask;
    struct procent *prptr;
    struct mailbox *mybox;
    bool8 flag = 0;
    mask = disable();
    if (isbadpid(pid)) {
        restore(mask);
        return SYSERR;
    }
    prptr = &proctab[pid];
    wait(tmutex);
    int32 i;
    for (i=0;i<N;i++) {
        if (boxtable[i].pid == pid) {
            mybox = &boxtable[i];
            flag = 1;
        }
    }
    if (flag) {
        mybox->box[mybox->head] = msg;
        mybox->head++;
        if (mybox->head == 10) mybox->head = 0;
        wait(pmutex);
        printf("send one message %d to %d. \n",msg,pid);
        signal(pmutex);
    }
    else {
        signal(tmutex);
        restore(mask);
        return SYSERR;
    }
    signal(tmutex);
    if (prptr->prstate == PR_RECV) ready(pid);
    restore(mask);
    return OK;
}

umsg32 receiveMsg (void) {
    intmask	mask;    umsg32	msg;
    struct procent *prptr;
    struct mailbox *mybox;
    bool8 flag = 0;
    mask = disable();
    prptr = &proctab[currpid];
    wait(tmutex);
    int32 i;
    for (i=0;i<N;i++) {
        if (boxtable[i].pid == currpid) {
            mybox = &boxtable[i];
            flag = 1;
        }
    }
    if (flag) {
        if (mybox->head == mybox->tail) {
            prptr->prstate = PR_RECV;
            printf("block");
            signal(tmutex);
            resched();  //block until message arrive
            wait(tmutex);
        }
        else {
            msg = mybox->box[mybox->tail];
            mybox->tail++;
            if (mybox->tail == 10) mybox->tail = 0;
            wait(pmutex);
            printf("%d receive one message %d. \n" , currpid,msg);
            signal(pmutex);
        }
    }
    else {
        signal(tmutex);
        restore(mask);
        return -1;
    }
    signal(tmutex);
    restore(mask);
    return msg;
    
}

int32 sendMsgs (pid32 pid,umsg32* msgs,int32 msg_count ){
    intmask	mask;
    struct procent *prptr;
    struct mailbox *mybox;
    bool8 flag = 0;
    mask = disable();
    if (isbadpid(pid)) {
        signal(tmutex);
        restore(mask);
        return 0;
    }
    prptr = &proctab[pid];
    wait(tmutex);
    int32 i;
    for (i=0;i<N;i++) {
        if (boxtable[i].pid == pid) {
            mybox = &boxtable[i];
            flag = 1;
        }
    }    int32 succmsg = 0;
    if (flag) {
        wait(pmutex);
        printf("send messages to %d:\n",pid);
        signal(pmutex);
        for (i=0;i<msg_count;i++) {
            mybox->box[mybox->head] = *(msgs+i);
            mybox->head++;
            if (mybox->head == 10) mybox->head = 0;
            wait(pmutex);
            printf(" %d \n",*(msgs+i));
            signal(pmutex);
            succmsg++;
            
        }
    }
    else {
        signal(tmutex);
        restore(mask);
        return 0;
    }
    signal(tmutex);
    if (prptr->prstate == PR_RECV) ready(pid);
    restore(mask);
    return succmsg;
}

syscall receiveMsgs (umsg32* msgs,int32 msg_count ){
    intmask mask;
    struct mailbox *mybox;
    struct procent *prptr;
    bool8 flag = 0;
    mask = disable();
    prptr = &proctab[currpid];
    wait(tmutex);
    int32 i;
    for (i=0;i<N;i++) {
        if (boxtable[i].pid == currpid) {
            mybox = &boxtable[i];
            flag = 1;
        }
    }
    if(flag) {
        if (mybox->head == mybox->tail) {
            prptr->prstate = PR_RECV;
            signal(tmutex);
            resched();  //block until message arrive
            wait(tmutex);
        }
        else {
            wait(pmutex);
            printf("%d receive messages:\n",currpid);
            signal(pmutex);
            for (i=0;i<msg_count;i++) {
                *(msgs+i) = mybox->box[mybox->tail];
                mybox->tail++;
                if(mybox->tail == 10) mybox->tail = 0;
                wait(pmutex);
                printf(" %d \n",*(msgs+i));
                signal(pmutex);

            }
        }
    }
    else {
        signal(tmutex);
        restore(mask);
        return SYSERR;
    }
    signal(tmutex);
    restore(mask);
    return OK;
}

int32 sendnMsg (int32 pid_count,pid32* pids, umsg32 msg) {
    int32 i;
    int32 succmsg = 0;
    for (i=0;i<pid_count;i++) {
        if (sendMsg(pids[i],msg) == OK) {
            succmsg++;
            wait(pmutex);
            printf("send %d to %d. \n",msg,pids[i]);
            signal(pmutex);
        }
    }
    return succmsg;
}


process sender (void) {
    //send msg 100  to rveiver1 100
    sendMsg(receive_id,100);
    //send 2 msgs(200,300) to receiver1
    umsg32 msgs[2]={200,300};
    int32 n1 = sendMsgs (receive_id,msgs,2);
    //send 2 msgs to receiver1 and receiver2
    pid32 pids[2] = {receive_id,receive_id2};
    int32 n2 = sendnMsg(2,&pids,1);
}

process receiver1 (void) {
    //receiver1 receive msg from sender
    umsg32 msg1 = receiveMsg();
    //receiver1 receive msgs(200,300) from sender
    umsg32 msgs[2]={};
    receiveMsgs(msgs,2);
    //receiver1 receive msg from sender
    umsg32 msg2 = receiveMsg();
    
}
process receiver2 (void) {
    //receiver2 receive msg from sender
    umsg32 msg = receiveMsg();
    
}


process	main(void)
{
	recvclr();

	/* Create the shared circular buffer and semaphores here */
    
    pmutex = semcreate(1);
    tmutex = semcreate(1);
	/* */
    
    send_id = create(sender, 4096, 50, "sender", 0);
    receive_id = create(receiver1, 4096, 50, "receiver", 0);
    receive_id2 = create(receiver2, 4096, 50, "receiver", 0);
    struct mailbox onebox = {.pid = receive_id,.head = 0, .tail = 0,.box = {0}};
    struct mailbox twobox = {.pid = receive_id2,.head = 0, .tail = 0,.box = {0}};
    boxtable[0]=onebox;
    boxtable[1]=twobox;
    
    resume(send_id);
    resume(receive_id);
    resume(receive_id2);
    
	return OK;
}
