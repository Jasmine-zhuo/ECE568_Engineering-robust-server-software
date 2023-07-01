After we finished the implementation of our code, we did the following things in order to test the functionality as well as performance of our server code.

1. Functionality test

  First, at server side, run ./server 

  At client side,   ./client  vcm-25961.vm.duke.edu pdf_create.xml 1
    [Explanation]:   deal with a <create> request succeed.              
    
  Then, run:       ./client vcm-25961.vm.duke.edu pdf_transaction1.xml 1
    [Explanation]:   deal with some <buy> request succeed.

  Last, run:     ./client vcm-25961.vm.duke.edu pdf_transaction2.xml 1
    [Explanation]:   matching orders/query/cancel succeed.



2. Scalability test

 First, at server side, run ./server

 At client side, ./client vcm-25961.vm.duke.edu  pdf_create.xml 5
   
   We run this multiple times and calculate the following parameters:

      Latency: [(time of last receive) - (time of first send)]/5
      
      Throuput: 1000/latency
