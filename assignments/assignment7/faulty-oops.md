# Analysis - OOPS Message

## Command Run
```
echo "hello_world" > /dev/faulty
```
The above command raised the below OOPS kernel log.

## OOPS Log
```
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x96000045
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
  FSC = 0x05: level 1 translation fault
Data abort info:
  ISV = 0, ISS = 0x00000045
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042079000
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
Internal error: Oops: 96000045 [#1] SMP
Modules linked in: hello(O) scull(O) faulty(O) [last unloaded: hello]
CPU: 0 PID: 158 Comm: sh Tainted: G           O      5.15.18 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
pc : faulty_write+0x14/0x20 [faulty]
lr : vfs_write+0xa8/0x2b0
sp : ffffffc008d1bd80
x29: ffffffc008d1bd80 x28: ffffff80020ca640 x27: 0000000000000000
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
x23: 0000000040001000 x22: 000000000000000c x21: 000000558a712750
x20: 000000558a712750 x19: ffffff800209a800 x18: 0000000000000000
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
x5 : 0000000000000001 x4 : ffffffc0006f0000 x3 : ffffffc008d1bdf0
x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
Call trace:
faulty_write+0x14/0x20 [faulty]
ksys_write+0x68/0x100
__arm64_sys_write+0x20/0x30
invoke_syscall+0x54/0x130
el0_svc_common.constprop.0+0x44/0xf0
do_el0_svc+0x40/0xa0
el0_svc+0x20/0x60
el0t_64_sync_handler+0xe8/0xf0
el0t_64_sync+0x1a0/0x1a4
Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
---[ end trace 720d2026fc57bd96 ]---
```

## Message Analysis
The message `Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000` indicates that the kernel has encountered a NULL pointer dereference. The kernel has detected that the address `0000000000000000` is invalid and has raised an exception.

`ESR = 0x96000045` is the data abort exception. `96000045` is the exception syndrome register (ESR) value. `ESR` is a 32-bit register containining information about the exception raised.

`EC = 0x25: DABT (current EL), IL = 32 bits` indicates the exception class. `0x25` is the exception class. It is a 6-bit field that indicates the type of exception that is raised. `DABT` is the data abort exception. `current EL` is the exception raised in the current exception level. `IL = 32 bits` is the instruction length of 32 bits.

`SET = 0, FnV = 0` indicates the syndrome exception type as synchronous exception. `FnV = 0` indicates the fault not valid bit as not set. `FnV` is a 1-bit field that indicates whether the exception is caused by a fault or not. `EA = 0, S1PTW = 0` indicates that the exception is not caused by an external abort. `S1PTW = 0` indicates stage 1 page table walk is not used.

`FSC = 0x05: level 1 translation fault` indicates fault status code is of level 1 translation fault. `0x05` is the fault status code. `FSC` is a 6-bit field that indicates the type of fault that is raised. `level 1 translation fault` indicates fault of level 1 translation fault.

`ISV = 0, ISS = 0x00000045` indicates invalid instruction specific syndrome. `0x00000045` is the instruction specific syndrome. `ISV` is a 25-bit field containining additional information about the exception that is raised.

`CM = 0, WnR = 1` indicates invalid cache maintenance operation. `WnR = 1` indicates that the write not read bit is set. `WnR` is a 1-bit field indicating whether the exception was caused by a read or write operation.

`user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042079000` indicates user page table is using 4k pages and 39-bit virtual addresses. `pgdp=0000000042079000` indicates page global directory pointer is `0000000042079000`.

`[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000` indicates invalid page global directory (pgd), invalid page 4th level directory (p4d), and invalid page upper directory (pud).

`Internal error: Oops: 96000045 [#1] SMP` indicates kernel has raised an internal error. `96000045` is the ESR value. `#1` is the number of times the kernel has raised the internal error. `SMP` indicates kernel is running in a symmetric multiprocessing (SMP) configuration.

`Modules linked in: hello(O) scull(O) faulty(O) [last unloaded: hello]` indicates `hello`, `scull`, and `faulty` modules are loaded. `hello` module is the last module to be unloaded.

`CPU: 0 PID: 158 Comm: sh Tainted: G           O      5.15.18 #1` indicates CPU type is CPU 0, the process ID is 158, command is `sh`, and the kernel is tainted. `G` indicates kernel is running in a guest virtual machine. `O` indicates kernel is running in an out-of-tree configuration. `5.15.18` is the kernel version. `#1` is the kernel build number.

`Hardware name: linux,dummy-virt (DT)` indicates the hardware name is `linux,dummy-virt` and the device tree is used.

`pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)` indicates that the processor state is `80000005`. `Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--` is the processor state flags. 

`pc : faulty_write+0x14/0x20 [faulty]` indicates the program counter is `faulty_write+0x14/0x20 [faulty]`. `faulty_write` is the function that raised the exception. `0x14` is the offset from the start of the function. `0x20` is the size of the function. `faulty` is the module that contains the function.