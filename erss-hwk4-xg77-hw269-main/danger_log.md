Problems have solved

1. **Problems**: At first, we didn't know to handle the database concurrency issue for the multi-threading version. 

   **Solution**: After comparing sever possible solutions, including using mutex lock, row-level locking, and several translation-isolation levels, we decided to use a serializable translation level that only allows concurrent transactions to commit if it can prove there is a serial order of execution that would produce the same effect.

2. **Problems**: Our database can process the transaction in the serializable version firstly but fails to match orders correctly in the multi-threading version.

   **Reason**: The reason is that, in a multi-threading environment, there exists a database concurrency issue. Since we have set the transaction isolation level to "serializable," the matchOrders() function will roll back, and the subtracting effect will be accumulative on the input argument "curr_order_amount," which violates our assertion that the "curr_order_amount" is always larger than zero.

   **Solution**: We created a new variable to store the original argument amount value, such that even if the function rolled back, it will start from the original value.

3. **Problems**: We first tried to check the format of the data extracted from XML and convert it into the corresponding type before inserting it into the database. But this format checking is not robust. 

   **Solution**: So we directly inserted the raw information into the database and let the database perform the data type checking for us. We just need to handle the exception and send back the error message to the client.

4. **Problem**: Our server cannot distinguish whether the client has sent the request but failure happens in receiving or the client doesn't send the request when we are calling the read().

   **Solution**: We used select() for monitoring ready incoming requests. Thus, read() is only called when there is a request ready in the monitoring file descriptor.

5. **Problem**: We tried to use one client to establish a socket connection with the server and use muti-threads to send multiple requests. But this would cause the buffer cannot contain all the requests, and it was difficult to concatenate each part of the request together.

   **Solution**: Finally, we made our client create a new connection in each thread. We can send multiple requests in each thread, and it would not lead to a problem since, in each thread, the request is sent serially.



Prolems to be solved in the future

1. **Problem**: We didn't check whether the request XML is valid before parsing, which could cause failure if the clients send malformed request information.
2. **Problem**: We tried to implement a thread pool to reduce the performance overhead of creating a new thread for each client connection. However, after adding the thread pool, our server still could not support more than 200 clients' concurrent requests, which violated our assumption that the thread pool can be used to protect our server from exceeding the limit of resource. 
3. **Problem**: We didn't set the time-out limit for receiving client requests. If the client didn't send the request but does not close the connection, the thread will get stuck. 