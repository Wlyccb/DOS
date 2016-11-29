# Lab 2: Extended Message Passing in Xinu

### Background - communicating by message passing
For message passing, Xinu provides a send/receive set of system calls as follows:
* send - send a message to a process
* receive – receive a message from some other process
* recvclr - check if a message has been received from another process and return it if it exists, 
            but do not wait if one does not exist. Recvclr removes (clears) the message from the process table if one exists.
* recvtime - same as receive, yet will only wait for a limited specified time for the message to be received.


### Extending send/receive system calls

#### A. System calls for sending/receiving multiple messages
A sender process may need to send multiple messages to a single process. For instance, different types of messages need to be sent at the same time. Or a message should be segmented in a fixed size and then sent to a receiver process. The messages should be received in the order in which they were sent. You may use a circular buffer for queuing (or dequeuing) the messages as you did in Lab 1. You will be implementing the following system calls:
* syscall sendMsg (pid32 pid,umsg32 msg): 
  Sending message (msg) to process (pid). In case a message or a number of messages are already waiting on pid to receive them, the new msg will be queued. It will return OK on success or SYSERR on error.
* umsg32 receiveMsg (void): 
  Receiving a message from any sender. It causes the calling process to wait for a message to be sent (by any process). When a message is sent, it is received and returned.
* uint32 sendMsgs (pid32 pid,umsg32* msgs,uint32 msg_count): S
  ending a group (msg_count) of messages (msgs) to process (pid). It will return the number of msgs successfully sent or SYSERR on error.
* syscall receiveMsgs(umsg32* msgs,uint32 msg_count):
  Receiving a group (msg_count) of messages (msgs). It causes the calling process to wait for msg_count messages to be sent. When all messages are in the queue, they are then all together immediately received.

#### B. System call for sending to multiple recipients
Sometimes a sender process is required to send a message to multiple processes. For instance, after a process changes data, it should update the change in other processes which use the data. You will be implementing the following system calls;
* uint32 sendnMsg (uint32 pid_count, pid32* pids, umsg32 msg): 
  Sending the message (msg) to the number (pid_count) of processes (pids). It will return the number of pids successfully sent to or SYSERR on error.


