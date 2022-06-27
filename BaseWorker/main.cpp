//
//  main.cpp
//  BaseWorker
//
//  Created by Watching Wang on 2022/6/27.
//

#include "BaseWorker.h"
#include <iostream>

int main(int argc, const char * argv[]) {
    BaseWorker worker;
    worker.Start();
    
    worker.AsyncCall([]{
        for (size_t i = 0; i<10000; ++i) {
            std::cout<<i<<' ';
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout<<-1;
    worker.Stop();

    return 0;
}
