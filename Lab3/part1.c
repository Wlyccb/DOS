/*  main.c  - main */

#include <xinu.h>

struct subs {
    pid32 p_id;
    void (*handler)(topic16, uint32);
    struct subs *nextsub;
};

struct topics {
    struct subs *head;
    int32 count;
};

struct brokers{
    topic16 topic_id;
    struct brokers *next;
    uint32 data;
};

struct topics table[256];
//struct brokers headnode;
struct brokers *head;
struct brokers *tail;

sid32 tmutex;
sid32 bmutex;
sid32 lock;
pid32 A_id;
pid32 B_id;
pid32 C_id;
pid32 broker_pid;

status ini () {
    //topics table
    int32 i;
    for (i=0;i<256;i++) {
        table[i].head = (struct subs*)NULL;
        table[i].count = 0;
    }
    if ((tmutex=semcreate(1)) == SYSERR) return SYSERR;
    
    //brokers
//    headnode.topic_id = -1;
//    headnode.data = 0;
//    headnode.next = (struct brokers*)NULL;
//    head = &headnode;
//    tail = &headnode;
    head = (struct brokers*)getmem(sizeof(struct brokers));
    head->topic_id = -1;
    head->data = 0;
    head->next =(struct brokers*)NULL;
    tail = head;
    if ((bmutex=semcreate(1)) == SYSERR) return SYSERR;
    
    return OK;
}



syscall subscribe(topic16 topic, void (*callback)(topic16, uint32)) {
    intmask mask;
    mask = disable();
    
    if (table[topic].count == 8 || topic>255) {
        restore(mask);
        return SYSERR;
    }
    //printf("start subscribe... 1\n");
    struct subs *temp = (struct subs*)getmem(sizeof(struct subs));
    temp->p_id = currpid;
    temp->nextsub = table[topic].head;
    temp->handler = callback;
    //printf("start subscribe... 2\n");
    wait(tmutex);
    //printf("start subscribe... 3\n");
    table[topic].head = temp;
    table[topic].count++;
    //printf("end subscribe... \n");
    signal(tmutex);
    
    restore(mask);
    return OK;
}


syscall unsubscribe(topic16 topic) {
    intmask mask;
    mask = disable();
    
    wait(tmutex);
    printf("start unsubscribe... 1\n");
    struct subs *p = table[topic].head;
    if (p->p_id == currpid) {
        printf("start unsubscribe... 2\n");
        struct subs* freep = table[topic].head;
        table[topic].head = table[topic].head->nextsub;
        freemem(freep,sizeof(struct subs));
    }
    else {
        printf("start unsubscribe... 3\n");
        while (p->nextsub != (struct subs*)NULL && p->nextsub->p_id != currpid) {
            printf("start unsubscribe... 4\n");
            p = p->nextsub;
        }
        if (p->nextsub == (struct subs*)NULL) {
            signal(tmutex);
            restore(mask);
            return SYSERR;
        }
        else {
            printf("start unsubscribe... 5\n");
            
            p->nextsub = p->nextsub->nextsub;
            table[topic].count--;
            printf("%d %d \n",table[topic].head,table[topic].head->nextsub);
        }
    }
    signal(tmutex);
    printf("finish unsubscribe... \n");
    restore(mask);
    return OK;
}

syscall publish(topic16 topic, uint32 data) {
    intmask mask;
    mask = disable();
    
    if (topic > 255) {
        restore(mask);
        return SYSERR;
    }
    //printf("start publish... 1\n");
    struct brokers *temp = (struct brokers*)getmem(sizeof(struct brokers));
    temp->topic_id = topic;
    temp->data = data;
    temp->next = (struct brokers*)NULL;
    //printf("start publish...2 \n");
    wait(bmutex);
    printf("start publish...3 \n");
    tail->next = temp;
    tail = temp;
    signal(bmutex);
    printf("head of brokers \n",head->next);
    //signal(lock);
    //printf("end publish... \n");
    
    restore(mask);
    return OK;
}



void foo(topic16 top_id, uint32 data) {
    printf("Function foo() is called with arguments %d and %d \n",top_id,data);
}

void bar(topic16 top_id, uint32 data) {
    printf("Function bar() is called with arguments %d and %d \n",top_id,data);
}

process broken (void){
    //printf("broker mark \n");
    while (1) {
        //wait(lock);
        wait(bmutex);
//        printf("aftet wait(lock) \n");
        //printf("%d \n",head->next);
        if (head->next != (struct brokers*)NULL) {
            printf("start broker... \n");
            topic16 id = head->next->topic_id;
            uint32 data = head->next->data;
            struct brokers *freep = head->next;
            if (tail == head->next) tail = head;
            head->next = head->next->next;
            freemem(freep,sizeof(struct brokers));
            wait(tmutex);
            struct subs *p = table[id].head;
            while (p!=(struct subs*)NULL) {
                p->handler(id,data);
                p = p->nextsub;
            }
            signal(tmutex);
            printf("end broker... \n");
        }
        signal(bmutex);
        sleep(5);
    }
}


process A (void) {
    //int32 i = 10;
    int32 issub[256] = {0};
    
    if (subscribe(10, &foo)) {
        issub[10] = 1;
        printf("Process A subscribes to topic 10 with handler foo() \n");
    }
    else printf("Process A fail to subscribe. \n");
    sleep(10);
    if (unsubscribe(10)) {
        issub[10] = 0;
        printf("Process A unsubscribes to topic 10 \n");
    }
    else printf("Process A fail to unsubscribe. \n");
    
    int32 i;
    for (i = 0;i<256;i++) {
        if (issub[i] == 1) unsubscribe(i);
    }
    
}
process B (void) {
    if (publish(10,77) == OK) {
        printf("Process B publishes %d to topic %d. \n",77,10);
    }
    else printf("Process B fail to publish. \n");
    sleep(20);
    if (publish(10,42) == OK) {
        printf("Process B publishes %d to topic %d. \n",42,10);
    }
    else printf("Process B fail to publish. \n");
    
}

process C (void) {
    int32 issub[256] = {0};
    //int32 i = 10;
    if (subscribe(10, &bar)) {
        issub[10] = 1;
        printf("Process C subscribes to topic 10 with handler bar() \n");
    }
    else printf("Process C fail to subscribe. \n");
    int32 i;
    for (i = 0;i<256;i++) {
        if (issub[i] == 1) unsubscribe(i);
    }
}


process	main(void)
{

	/* Run the Xinu shell */

	recvclr();
    lock = semcreate(0);
    
    if (ini() != OK) return SYSERR;
    //printf("after ini \n");
    broker_pid = create(broken, 4096, 50, "broken", 0);
    A_id = create(A, 4096, 50, "A", 0);
    B_id = create(B, 4096, 50, "B", 0);
    C_id = create(C, 4096, 50, "C", 0);
    
	
    resume(A_id);
    resume(B_id);
    resume(C_id);
    resume(broker_pid);
    
	return OK;
    
}
