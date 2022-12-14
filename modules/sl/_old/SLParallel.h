//#############################################################################
//  File:      Globals/sl/SLParallel.h
//  Purpose:   Concurrent C++ Template using C++ 11 Treads
//  Authors:   Marcus Hudritsch / Anthony Wiliams
//  Source:    Most algorithms are originated from Anthony Wiliams book
//             "C++ Concurrency in Action", ISBN: 9781933988771
//  Date:      February 2013
//  Copyright (c): Anthony Wiliams. 
//             Boost Software License - Version 1.0 - August 17th, 2003:
//
//  Permission is hereby granted, free of charge, to any person or organization
//  obtaining a copy of the software and accompanying documentation covered by
//  this license (the "Software") to use, reproduce, display, distribute,
//  execute, and transmit the Software, and to prepare derivative works of the
//  Software, and to permit third-parties to whom the Software is furnished to
//  do so, all subject to the following:
//  
//  The copyright notices in the Software and this entire statement, including
//  the above license grant, this restriction and the following disclaimer,
//  must be included in all copies of the Software, in whole or in part, and
//  all derivative works of the Software, unless such copies or derivative
//  works are solely in the form of machine-executable object code generated by
//  a source language processor.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
//  FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//#############################################################################

#ifndef SLPARALLEL_H
#define SLPARALLEL_H

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

//-----------------------------------------------------------------------------
class join_threads
{
private:
   vector<std::thread>& threads;
   
public:
   explicit join_threads(vector<std::thread>& threads_) : threads(threads_)
   {}
   ~join_threads()
   {
      for(unsigned long i=0;i<threads.size();++i)
      {
         if(threads[i].joinable())
            threads[i].join();
      }
   }
};
//-----------------------------------------------------------------------------
template<typename Iterator,typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
    unsigned long const length=std::distance(first,last);

    if(!length)
        return;

    unsigned long const min_per_thread=25;
    unsigned long const max_threads=
        (length+min_per_thread-1)/min_per_thread;

    unsigned long const hardware_threads=
        std::thread::hardware_concurrency();

    unsigned long const num_threads=
        std::min(hardware_threads!=0?hardware_threads:2,max_threads);

    unsigned long const block_size=length/num_threads;

    vector<std::future<void> > futures(num_threads-1);
    vector<std::thread> threads(num_threads-1);
    join_threads joiner(threads);

    Iterator block_start=first;
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        std::packaged_task<void(void)> task(
            [=]()
            {
                std::for_each(block_start,block_end,f);
            });
        futures[i]=task.get_future();
        threads[i]=std::thread(std::move(task));
        block_start=block_end;
    }
    std::for_each(block_start,last,f);
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        futures[i].get();
    }
}

//-----------------------------------------------------------------------------

#endif
