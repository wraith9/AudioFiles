

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


class Semaphore {
   public:
      explicit Semaphore(unsigned int initial_count);

   private:
      // The current semaphore count.
      unsigned int count_;

      //mutex_ protects count_.
      //Any code that reads or writes the count_ data must hold a lock on
      //the mutex.
      boost::mutex mutex_;

      //Code that increments count_ must notify the condition variable.
      boost::condition condition_;

   public:
      //unsigned int get_count() const; //for debugging/testing only
      void signal();
      void wait();
      bool timed_wait(const boost::system_time& sec);      
};
