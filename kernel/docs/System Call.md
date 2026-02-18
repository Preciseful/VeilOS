# System Calls

| Name                              | Code |
| --------------------------------- | ---- |
| [Malloc](#malloc)                 |  0   |
| [Free](#free)                     |  1   | 
| [Memory size](#memory-size)       |  1   | 
| [Exit Process](#exit-process)     |  1   | 


## Malloc
Allocate a region of memory.
#### Arguments:
- Size in bytes.
#### Return value:
- The address of the allocated memory region.

## Free
Frees up a region of memory.
#### Arguments:
- The address of the region of memory.

## Memory Size
Gets the size of the allocated memory region.
#### Arguments:
- The address of the region of memory.

## Exit Process
Marks the current process as to be killed in the next scheduler run.

