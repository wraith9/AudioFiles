

#include "Semaphore.h"


Semaphore::Semaphore(unsigned int initial_count)
   : count_(initial_count),
   mutex_(),
   condition_()
{
}

/*unsigned int Semaphore::get_count() const //for debugging/testing only
{
   //The "lock" object locks the mutex when it's constructed,
   //and unlocks it when it's destroyed.
   boost::mutex::scoped_lock scoped_lock(mutex_);
   return count_;
}*/

void Semaphore::signal() //called "release" in Java
{
   boost::mutex::scoped_lock scoped_lock(mutex_);
   ++count_;
   
   //Wake up any waiting threads.
   //Always do this, even if count_ wasn't 0 on entry.
   //Otherwise, we might not wake up enough waiting threads if we
   //get a number of signal() calls in a row.
   condition_.notify_one();
}

void Semaphore::wait() //called "acquire" in Java
{
   boost::mutex::scoped_lock scoped_lock(mutex_);
   while (count_ == 0) {
      condition_.wait(scoped_lock);
   }
   --count_;
}

bool Semaphore::timed_wait(const boost::system_time& sec) {

   boost::mutex::scoped_lock scoped_lock(mutex_);
   while (count_ == 0) {
      if (!condition_.timed_wait(scoped_lock, sec)) 
         return false;
   }

   --count_;
   return true;
} 
