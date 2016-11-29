# Lab1 Process Coordination using Sempahores & Mutexes

### The Producer&Consumer Problem
Two processes are sharing a circular buffer (queue), one produces at the tail of the circular queue (the Producer) and the other consumes from the head (the Consumer).

### Implement a mutex in Xinu
You are asked to implement a mutex in Xinu, using only Xinu semaphores (the semaphore’s count must always be either 0 or 1 to be a proper mutex). A functional mutex consists of two methods: acquire and release. A mutex is acquired to start a critical section; this is when no other processes can run code that also requires that mutex – they will wait instead. Once the critical section ends, the mutex is released, and other processes can continue if they were waiting to acquire the mutex themselves.

### Implement the producer consumer problem using mutex
Using the mutex functionality you just created, implement the producer consumer problem correctly (meaning, no problems or faulty situations). You are to use the provided main.c to define and allocate a shared circular buffer, and then create both the producer and the consumer processes. Note that more than one mutex may be required. You should program both processes with “simulation code” to actually produce or consume. Both processes should output to the console what they are producing or consuming.

### Implement the producer and consumer problem using Xinu semaphores
Use Xinu semaphores properly (counting up and down higher than 1) to implement the producer and consumer problem correctly and effectively. You are to use a copy of your main() form part (c), replacing the producer and consumer functions. Note that a mutex may still be required along with the proper use of semaphores. Be sure that your implementation isn’t faulty to critical case scenarios (locking and the like). You should program both processes with “simulation code” to actually produce or consume as in part (c). Both processes should output to the console what they are producing or consuming. 


