## Lab 3: Event Processing in Xinu

### Background
Modern operating systems often have a mechanism for sending events between processes. Events allow a process to send information to other processes in a system asynchronously. While events sound very similar to message passing, the two differ in how they are received. While a process can check for messages and read them explicitly once received, an event triggers a handler, or callback function, that had been previously specified by the target process. Events allow a process to receive a notification on a change without explicitly checking for that change.  

Event systems have proven especially useful in the field of the Internet of Things. One such system that has seen widespread use is MQTT. MQTT defines a “publisher/subscriber” model, where one or more subscribers, processes wanting to receive an event, can listen to a topic (a specific event). A publisher can then send data under a topic to be received by all subscribers.  

In this lab, you will implement an event system in the spirit of MQTT, allowing XINU processes to subscribe to and publish events.  

### Part I
The three main components of the XINU event system will be subscribe, unsubscribe, and publish. A process can subscribe to a topic by providing a topic ID and a callback function. Once subscribed, the specified callback will be triggered when another process publishes data to that topic. If a process unsubscribes, its callback will no longer be triggered when data is received for the specified topic. When a process publishes data to a topic, all processes that have subscribed to that topic will receive the data through their callback function.  

The above functions will have the following signatures:
* syscall subscribe(topic16 topic, void (*handler)(topic16, uint32));
* syscall unsubscribe(topic16 topic);
* syscall publish(topic16 topic, uint32 data);

### Part II
To allow more fine-grained control over published data, MQTT implements namespaces above topics. These namespaces look like file paths, where the directory part is the namespace, and the file part is the actual topic. For example, house/temperature and office/temperature both contain the same topic name, temperature, but are logically separated by the namespace. So, data published to house/temperature would not be received by subscribers to office/temperature.  

Another common feature of namespaces is the ability to use wildcards. A wildcard allows a publisher to specify a wider range of subscribers to send data to. Using the above example, if data was published to +/temperature, (where + is the wildcard symbol) it would be sent to both house/temperature and office/temperature.  

You will implement a simplified version of namespaces in your XINU event system. When subscribing, a process can specify a specific group (namespace) in addition to the topic; publishers should then be able to target an individual group when sending data. Additionally, one group will be reserved as a wildcard, sending to it will send to all groups.

