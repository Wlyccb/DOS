/*  main.c  - main */

#include <xinu.h>

struct subs {
    pid32 p_id;
    int32 group;
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
    int32 group;
};

struct topics table[256];
//struct brokers headnode;
struct brokers *head;
struct brokers *tail;

sid32 tmutex;
sid32 bmutex;
pid32 A_id;
pid32 B_id;
pid32 C_id;
pid32 broker_pid;

status ini () {
    int32 i;
    for (i=0;i<256;i++) {
        table[i].head = (struct subs*)NULL;
        table[i].count = 0;
    }
    if ((tmutex=semcreate(1)) == SYSERR) return SYSERR;

    head = (struct brokers*)getmem(sizeof(struct brokers));
    head->topic_id = -1;
    head->data = 0;
    head->next =(struct brokers*)NULL;
    head->group = -1;
    tail = head;
    if ((bmutex=semcreate(1)) == SYSERR) return SYSERR;
    
    return OK;
}



syscall subscribe(topic16 topic, void (*callback)(topic16, uint32),int32 group) {
    intmask mask;
    mask = disable();
    
    if (table[topic].count == 8 || topic>255) {
        restore(mask);
        return SYSERR;
    }
    struct subs *temp = (struct subs*)getmem(sizeof(struct subs));
    temp->p_id = currpid;
    temp->nextsub = table[topic].head;
    temp->handler = callback;
    temp->group = group;
    wait(tmutex);
    table[topic].head = temp;
    table[topic].count++;
    signal(tmutex);
    
    restore(mask);
    return OK;
}


syscall unsubscribe(topic16 topic) {
    intmask mask;
    mask = disable();
    
    wait(tmutex);
    struct subs *p = table[topic].head;
    if (p->p_id == currpid) {
        struct subs* freep = table[topic].head;
        table[topic].head = table[topic].head->nextsub;
        freemem(freep,sizeof(struct subs));
    }
    else {
        while (p->nextsub != (struct subs*)NULL && p->nextsub->p_id != currpid) {
            p = p->nextsub;
        }
        if (p->nextsub == (struct subs*)NULL) {
            signal(tmutex);
            restore(mask);
            return SYSERR;
        }
        else {
            struct subs* freep = p->nextsub;
            p->nextsub = p->nextsub->nextsub;
            freemem(freep,sizeof(struct subs));
        }
    }
    table[topic].count--;
    signal(tmutex);
    restore(mask);
    return OK;
}

syscall publish(topic16 topic, uint32 data, int32 group) {
    intmask mask;
    mask = disable();
    
    if (topic > 255) {
        restore(mask);
        return SYSERR;
    }
    struct brokers *temp = (struct brokers*)getmem(sizeof(struct brokers));
    temp->topic_id = topic;
    temp->data = data;
    temp->next = (struct brokers*)NULL;
    temp->group = group;
    wait(bmutex);
    tail->next = temp;
    tail = temp;
    signal(bmutex);
    
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
    while (1) {
        wait(bmutex);
        if (head->next != (struct brokers*)NULL) {
            topic16 id = head->next->topic_id;
            uint32 data = head->next->data;
            int32 g = head->next->group;
            struct brokers *freep = head->next;
            if (tail == head->next) tail = head;
            head->next = head->next->next;
            freemem(freep,sizeof(struct brokers));
            wait(tmutex);
            struct subs *p = table[id].head;
            while (p!=(struct subs*)NULL) {
                if (p->group == g || g == 1) {
                    p->handler(id,data);
                }
                p = p->nextsub;
            }
            signal(tmutex);
        }
        signal(bmutex);
        sleep(5);
    }
}


process A (void) {
    int32 issub[256] = {0};
    
    if (subscribe(10, &foo,2)) {
        issub[10] = 1;
        printf("Process A subscribes to topic 10 value with 2 and handler foo() \n");
    }
    else printf("Process A fail to subscribe. \n");
    sleep(10);
    if (unsubscribe(10)) {
        issub[10] = 0;
        printf("Process A unsubscribes to topic 10 \n");
    }
    else printf("Process A fail to unsubscribe. \n");
    sleep(20);
    int32 i;
    for (i = 0;i<256;i++) {
        if (issub[i] == 1) unsubscribe(i);
    }
    
}
process C (void) {
    if (publish(10,77,1) == OK) {
        printf("Process C publishes %d to topic %d %d. \n",77,10,1);
    }
    else printf("Process C fail to publish. \n");
    sleep(20);
    if (publish(10,42,3) == OK) {
        printf("Process C publishes %d to topic %d %d. \n",42,10,3);
    }
    else printf("Process C fail to publish. \n");
    
}

process B (void) {
    int32 issub[256] = {0};
    //int32 i = 10;
    if (subscribe(10, &bar,3)) {
        issub[10] = 1;
        printf("Process B subscribes to topic 10 with value 3 and handler bar() \n");
    }
    else printf("Process B fail to subscribe. \n");
    sleep(20);
    int32 i;
    for (i = 0;i<256;i++) {
        if (issub[i] == 1) unsubscribe(i);
    }
}


process	main(void)
{

	/* Run the Xinu shell */

	recvclr();
    //lock = semcreate(0);
    
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
