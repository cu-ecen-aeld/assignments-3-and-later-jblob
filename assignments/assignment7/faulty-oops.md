# Faulty Driver Kernel Oops Analysis

## Observed Behavior

When writing to /dev/faulty, the system crashes and produces a kernel Oops.

# echo "Hello Juergen" >/dev/faulty
BUG: kernel NULL pointer dereference, address: 00000000
#PF: supervisor write access in kernel mode
#PF: error_code(0x0002) - not-present page
*pde = 00000000
Oops: 0002 [#1] PREEMPT SMP
CPU: 0 PID: 105 Comm: sh Tainted: G           O       6.1.44 #1
Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.15.0-1 04/01/2014
EIP: faulty_write+0x2/0x10 [faulty]
Code: Unable to access opcode bytes at 0xc9800fd8.
EAX: 00000000 EBX: c13efb40 ECX: 0000000e EDX: 00532eb0
ESI: c183bf64 EDI: c9801000 EBP: c183bf54 ESP: c183beec
DS: 007b ES: 007b FS: 00d8 GS: 0033 SS: 0068 EFLAGS: 00000246
CR0: 80050033 CR2: c9800fd8 CR3: 01847000 CR4: 00000690
Call Trace:
 ? show_regs.part.0+0x17/0x1a
 ? __die+0x48/0x80
 ? page_fault_oops+0x56/0x100
 ? kernelmode_fixup_or_oops.constprop.0+0x88/0xe0
 ? __bad_area_nosemaphore.constprop.0+0x11d/0x150
 ? find_vma+0x22/0x40
 ? bad_area_nosemaphore+0x12/0x20
 ? exc_page_fault+0xcb/0x440
 ? do_filp_open+0xa3/0x160
 ? paravirt_BUG+0x10/0x10
 ? handle_exception+0x133/0x133
 ? 0xc9801000
 ? paravirt_BUG+0x10/0x10
 ? faulty_write+0x2/0x10 [faulty]
 ? paravirt_BUG+0x10/0x10
 ? faulty_write+0x2/0x10 [faulty]
 ? vfs_write+0xbc/0x400
 ? next_uptodate_page+0x250/0x250
 ? mt_find+0xd1/0x1b0
 ksys_write+0x6c/0xf0
 __ia32_sys_write+0x10/0x20
 __do_fast_syscall_32+0x50/0xc0
 do_fast_syscall_32+0x32/0x70
 do_SYSENTER_32+0x15/0x20
 entry_SYSENTER_32+0x98/0xf1
EIP: 0xb7efa549
Code: b8 01 10 06 03 74 b4 01 10 07 03 74 b0 01 10 08 03 74 d8 01 00 00 00 00 00 00 00 00 00 00 00 00 00 51 52 55 89 e5 0f 34 cd 80 <5d> 5a 59 c3 90 90 90 90 8d 76 00 58 b8 76
EAX: ffffffda EBX: 00000001 ECX: 00532eb0 EDX: 0000000e
ESI: b7ed6e34 EDI: 00532eb0 EBP: 0000000e ESP: bf84eaf0
DS: 007b ES: 007b FS: 0000 GS: 0033 SS: 007b EFLAGS: 00000246
Modules linked in: hello(O) faulty(O) scull(O)
CR2: 0000000000000000
---[ end trace 0000000000000000 ]---
EIP: faulty_write+0x2/0x10 [faulty]
Code: Unable to access opcode bytes at 0xc9800fd8.
EAX: 00000000 EBX: c13efb40 ECX: 0000000e EDX: 00532eb0
ESI: c183bf64 EDI: c9801000 EBP: c183bf54 ESP: c183beec
DS: 007b ES: 007b FS: 00d8 GS: 0033 SS: 0068 EFLAGS: 00000246
CR0: 80050033 CR2: c9800fd8 CR3: 01847000 CR4: 00000690

Welcome to Buildroot
buildroot login:


the test with the yocto environment:


root@qemuarm64:~# echo "Hallo Juergen" >/dev/faulty
[  864.149814] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[  864.165711] Mem abort info:
[  864.167707]   ESR = 0x0000000096000045
[  864.167859]   EC = 0x25: DABT (current EL), IL = 32 bits
[  864.167993]   SET = 0, FnV = 0
[  864.168066]   EA = 0, S1PTW = 0
[  864.168144]   FSC = 0x05: level 1 translation fault
[  864.168264] Data abort info:
[  864.168391]   ISV = 0, ISS = 0x00000045
[  864.168508]   CM = 0, WnR = 1
[  864.168723] user pgtable: 4k pages, 39-bit VAs, pgdp=00000000436cc000
[  864.168942] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[  864.169460] Internal error: Oops: 0000000096000045 [#1] PREEMPT SMP
[  864.170095] Modules linked in: faulty(O) hello(O) scull(O)
[  864.170631] CPU: 3 PID: 418 Comm: sh Tainted: G           O      5.15.201-yocto-standard #1
[  864.170944] Hardware name: linux,dummy-virt (DT)
[  864.171365] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[  864.171573] pc : faulty_write+0x18/0x20 [faulty]
[  864.172118] lr : vfs_write+0xf8/0x2a0
[  864.172266] sp : ffffffc00b81bd80
[  864.172356] x29: ffffffc00b81bd80 x28: ffffff8003692940 x27: 0000000000000000
[  864.172606] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[  864.172791] x23: 0000000000000000 x22: ffffffc00b81bdc0 x21: 000000558d6eb9b0
[  864.172992] x20: ffffff800340b600 x19: 000000000000000e x18: 0000000000000000
[  864.173200] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[  864.173393] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[  864.173581] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffc008272378
[  864.194144] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[  864.196314] x5 : 0000000000000001 x4 : ffffffc000bb7000 x3 : ffffffc00b81bdc0
[  864.198578] x2 : 000000000000000e x1 : 0000000000000000 x0 : 0000000000000000
[  864.201808] Call trace:
[  864.202142]  faulty_write+0x18/0x20 [faulty]
[  864.202499]  ksys_write+0x74/0x110
[  864.202725]  __arm64_sys_write+0x24/0x30
[  864.203459]  invoke_syscall+0x5c/0x130
[  864.204302]  el0_svc_common.constprop.0+0x4c/0x100
[  864.206234]  do_el0_svc+0x4c/0xc0
[  864.206450]  el0_svc+0x28/0x80
[  864.206670]  el0t_64_sync_handler+0xa4/0x130
[  864.207120]  el0t_64_sync+0x1a0/0x1a4
[  864.208758] Code: d2800001 d2800000 d503233f d50323bf (b900003f)
[  864.209554] ---[ end trace 89a923017286ee99 ]---
Segmentation fault

Poky (Yocto Project Reference Distro) 4.0.35 qemuarm64 /dev/ttyAMA0

qemuarm64 login:
