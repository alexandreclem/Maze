## Distributed Systems Architecture: Client-Balancer-Server

### What does it do?
The program uses multiple **clients** that request - in this case a simple prime number verification - to **servers**. Every request is controlled by the **balancer** which is composed of a request handler, a requests queue, and a dispatcher that sends the requests to servers and handles the synchronization of the database (each server file). To allow multiple clients to make requests, sockets programming, multithreading and concurrent techniques were used.

### Architecture
<p align="center" width="100%">
    <img width="90%" src="https://raw.githubusercontent.com/alexandreclem/loadBalancer/master/images/img.png">    
</p>

### Description
#### Clients
- Three clients will be instantiated (*Client.java*) in the form of three different threads (*ClientInstance.java*).
- Each client maintains an active socket with the client's RequestHandler program.
- Each request consists of sending:
    - Customer ID (arbitrary)
    - Action (Read or Write)
        - The action will be defined by a function that uses probabilities (rational only, e.g. $3/4$) and selects which action to send to the RequestHandler.
    - Number
        - Integer selected from the Random function.
        
#### Balancer
- The *Balancer.java* program is in charge of instantiating (in an infinite loop) a thread (*RequestHandler.java*) for each new client. In this case, three RequestHandler were instantiated.
- The *Balancer.java* contains final information about the IP/Port of the servers, as well as the IP/Port of RequestHandler and DispatcherSynchronize.
- Each RequestHandler thread maintains an active connection to a specific client.
- The RequestHandler receives the requests (three pieces of information already mentioned).
- RequestHandler defines the server that will receive the request: The selection chosen was the circular type.
- All request information is encapsulated in an object of type Request.
- *Request.java* contains:
    - Customer ID
    - Action
    - Number
    - Client RequestHandler IP
    - Client RequestHandler PORT
    - Server IP
    - Server PORT
- After encapsulating, the RequestHandler places this request in the object that contains a static (global) queue, *RequestsQueue.java*.
- *RequestsQueue.java* contains:
    - Static ConcurrentLinkedQueue that stores objects of type Request.
    - After adding to the queue, the RequestHandler waits for a connection from the server to receive the response and send it to the Client.

- The *DispatcherSynchronize.java* program is only responsible for taking requests from the queue and directing them to the destination server.

- If the request is a **written** request:
    - A method (synchronize) is called and the request is sent to the main server of the  request where it is processed and the response is sent to dispatcherSynchronize, so this response can be forwarded to the other servers to maintain the data **synchronicity**.
    - This process does not interfere with requests being queued by RequestHandler.

#### Servers
- Three threads (*ServerInstance.java*) are instantiated in *Server.java*.
- Servers listen for DispatcherSynchronize requests.
- Receives and processes the requests.
- Sends the response to the request's RequestHandler.
- In the case of writing: They also write the answer in the respective file.
- If it is **read**, it sends the entire file information to the client's RequestHandler.

#### Data Files
- Three text files to store the writings made by the servers.
- Each file will only be changed by a specific server.


### The Program

#### Program Execution
<p align="center" width="100%">
    <img width="90%" src="https://raw.githubusercontent.com/alexandreclem/loadBalancer/master/images/img_2.png">    
</p>

#### Data Synchronized
<p align="center" width="100%">
    <img width="90%" src="https://raw.githubusercontent.com/alexandreclem/loadBalancer/master/images/img_3.png">    
</p>



### How to run?
- Clone the Repository
    ```bash
    $ git clone https://github.com/alexandreclem/loadBalancer.git
    ```
- Compile 
    
    - Within the **src** directory, run: 
        ```bash
        $ javac -d ../bin ./requests/*.java
        $ javac -cp ../bin -d ../bin ./clients/*.java 
        $ javac -cp ../bin -d ../bin ./servers/*.java
        $ javac -cp ../bin -d ../bin ./balancer/*.java         
        ```    
    > **NOTE**
    >
    > After that a **bin** directory will be created inside the project folder, in which all compiled code (.class) is stored.       
- Execute
    - Within the **src** directory, with 3 **separated** terminals and in this **order**: 
        ```bash    
        $ java -cp ../bin balancer.Balancer
        $ java -cp ../bin servers.Server
        $ java -cp ../bin clients.Client
         ```

    > **NOTE**
    >
    > You can verify the data in the **data** directory inside the project folder.    
    
    