# dining-philosophers

Dining philosophers problem using pthreads library and semaphores/conditional variables, returns every philosophers statistics of eating. Every philosopher and waiter is a thread. Rules that I considered while coding:
- Waiter cannot serve while there is rice on the table.
- Philosophers can wait for the rice while holding forks.
