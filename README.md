# CPP11SampleCodeQuestion
This README outlines the technical choices made by William Bittner while writing the multi-threaded reverse polish notation calclator as required by [Original Project](https://github.com/zedle/CPP11CodeSampleQuestion/blob/master/CPP-Threads.md)
## Threading
When designing the threading model of system, in my opinion, it is most important to first start with the threading model of the input and output of that system. When using a test framework such as this, where the input and output is really just stdin/stdout, you can ignore many things such as incomming threads/cache lines, etc.

We are assuming that this system is a high throughput, non-latency sensitive application. 

When deciding between using an Async Task Framework, and Locked Pipeline model, I decided to use the locked pipeline model to avoid the possibility of overloading task pools in high throughput situations that would cause an essential dead lock. Try spinning up 1,000 threads. Context Switches kill you.

Using condition variables to allow the threads to sleep until there is work you notify one, kill all open work, and go back to sleep, is what I call the non-latency sensitive pipleine model. This is because, in latency sensitive applications you can't afford to wait for the thread to wake up, you have to process the work immediately. That causes a lot of difficulty if you have many sections of code that need to be parallel as you need to marshall the threads around or use a global pool task/data enqueue/dequeue pool which is what I usually use.

For a more latency sensitive application, as well as one that I have more time to spend on, I would use a non-blocking , non-locking Multi Producer, Multi Consumer queue. This is a blocking and locking naive queue. If time permits I will include a non-locking concurrent queue without a reclaimation function (they tend to be closely held IP)

Steps for important functions:

### Enqueue:
  1. Lock Work Queue Mutex ( Or Block until can Aquire Lock)
  2. Enqueue Item
  3. Unlock
  4. Signal Condition Variable notify_one

### try_pop
   1. lock mutex
   2. check queue length
   3. if (length > 0) {
   	   retValue = queue.head;
   	   queue.pop;
   }
   4. unlock
   5. return true
   6. else false
   
### wait_for_pop
   1. lock mutex
   2. spin on while queue empty
   3. conditin variable wait ( which does the unlock, lock on wakeup for you, thanks cpp11)
   4. retvalue = queue.head;
   5. queue.pop;
   6. unlock mutex;
   

## Parseing
The strict requirements for syntax gave us the ability to write a very naive parser that if it encounters even the slightest wrong syntax it will throw exception. This helps greatly with debugging early on.

If not so luckly, I would use at the very least a Regex + Tokens model, or something like Boost Spirit Parser
## Changes for Production
1. Test coverage
2. Split commonly used multithreading functionality up to reusable components
3. Use Boost Spirit Parser and provider better error messages
4 Const Expr optimizations
5. Avoid copy objects and pass string more by reference, etc. Avoid copy constructor unless you have to. 

## Misc Notes
I always put non-essential (logging, error handling, etc) background work in seperate work queue that is multi producer single consumer with low priority. You have to do this or you will ( on most systems, though std::cout will serialize you code sometimes ) your std::cout will you like a bunch of garbled garbage. By using a MPSC queue, you can guarantee that only one thread is writing to std out.
## References
1. C++ Concurrency In Action - Anthony Williams 

## License
MIT
