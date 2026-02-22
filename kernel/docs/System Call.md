# System Calls

| Name                              | Code |
| --------------------------------- | ---- |
| [Malloc](#malloc)                 |  0   |
| [Free](#free)                     |  1   | 
| [Memory size](#memory-size)       |  2   | 
| [Exit Process](#exit-process)     |  3   | 
| [Own Device](#own-device)         |  4   |


## Malloc
Allocate a region of memory.
#### Arguments:
- UINT32 : Size in bytes.
#### Return value:
- UINT64 : The address of the allocated memory region.

## Free
Frees up a region of memory.
#### Arguments:
- UINT64 : The address of the region of memory.
#### Return value:
- UINT64 : The amount of memory freed.

## Memory Size
Gets the size of the allocated memory region.
#### Arguments:
- UINT64 : The address of the region of memory.
#### Return value:
- UINT64 : The amount of memory allocated.

## Exit Process
Marks the current process as to be killed in the next scheduler run.

## Own Device
Attempts to mark an IO device as owned by the current process.
#### Arguments:
- UINT64 : Category of the IO device requested.
- UINT32 : Code of the IO device requested.
- UINT64 : Permissions required
    - 0x1 : READ
    - 0x2 : WRITE
    - 0x4: REQUEST
#### Return value:
- INT32 : The token that will be used to access it later.
    - The result may be -1, the owning process has been denied.
