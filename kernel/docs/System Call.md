# System Calls

| Name              | Code |
| ----------------- | ---- |
| [Malloc](#malloc) |  0   | 


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

## Get Environment
Gets the current process' environment variables.
#### Arguments:
- An array of a key value pair sequence. The end is marked with a `0`.

