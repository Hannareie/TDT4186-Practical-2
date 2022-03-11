"""
Implement a ring buffer in the module bbuffer (file bbuffer.c), which will be used for communicating between
a single producer and multiple consumer threads.

Use your semaphore implementation from subtask b to synchronize over- and underflow situations of the ring
buffer, so that producers block when trying to insert an element into a full buffer and receivers block when trying
to take an element from an empty buffer.

"""