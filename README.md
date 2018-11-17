# Random Partitioning using Fiduccia Mattheyses method

As a part of my Graduate course __EEDG 6375: Design Automation of Physical Design__, I have done this project with my project partner __Akshay Patil__([LinkedIN](https://www.linkedin.com/in/akshayxpatil/)). We did this course under Dr. Dinesh Bhatia at The University of Texas at Dallas.
*In the following text, the words 'Cell' and 'Node' as well as words 'Net' and 'Wire' might be used interchangeably.*

In the world of Design Automation, the circuit partitioning is the most fundamental step that is followed during the design of the backend of the EDA tools. With the help of partitioning, the circuit is divided in parts such a way that the cells which talks with each other frequently are brought close together. This helps in reducing the routing and the cost related to it.

For this project, our task is to use a well-known partitioning method called, __Fiduccia Mattheyses (FM) method__. This is very popular and widely used method used for partitioning task. This method is derived from the __Kernighan–Lin (KL) algorithm__.

Just like KL method, in FM method, the circuit is first randomly partitioned in two partitions (Say A and B). The difference that it carries is that, unlikem KL method where the nodes from two partitions are "swaped" with each other, the FM method aim to move one cell at a time.
One more difference in these two algorithms is that, in KL method the swap which increases the current cutset is rejected. But in the FM method, even if the move is increasing the cutset, that move is made in a hope that this move will lead to a better cutset in the future. (a.k.a. Hill climbing)

We have used C++ to design our source code in order to implement this algorithm. To test our code, we were given 4 real world example FPGA benchmarks circuits. These are ISPD (International Symposium on Physical Design) benchmarks. They are from [__"ISPD 2016 : Routability-Driven FPGA Placement Contest"__](http://www.ispd.cc/contests/16/ispd2016_contest.html)

Following gives the summary our code's result. __*NOTE THAT: These results are for the current code and we are working on it to improve in terms of execution time*__

|Circuit|Execution time (approx)|
|-------------|------------------|
|FPGA-example1|3 seconds|
|FPGA-example2|13 hours|
|FPGA-example3|7 hours 30 minutes|
|FPGA-example4|24 hours|

The percentage improvement that we are currently getting with this code is between 60% to 75%.

The source code for the given problem can be found [here](https://github.com/akash10295/Random-Partitioning-using-Fiduccia-Mattheyses-method/blob/master/fmpart.cpp). This is an alpha version of the code and there are many potential places in code where it can be optimized. Suggestions are welcomed. Contact at akash.t123@gmail.com
