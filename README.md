# ThreadPool-C-Implementation
This project is a lightweight C++ thread pool implementation using standard C++11 libraries. It demonstrates how to manage a pool of worker threads that execute user-submitted tasks concurrently, with proper synchronization and future-based result retrieval.


Features

Fixed-size thread pool
Task submission using execute_task with perfect forwarding
Futures to get task results
Graceful shutdown of worker threads
Thread-safe task queue using mutex and condition variable
