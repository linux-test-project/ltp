.. SPDX-License-Identifier: GPL-2.0-or-later

.. Include headers in this file with:
.. .. kernel-doc:: ../../include/tst_test.h

Developing using KVM API
========================

Testing KVM is more complex than other Linux features. Some KVM bugs allow
userspace code running inside the virtual machine to bypass (emulated) hardware
access restrictions and elevate its privileges inside the guest operating
system. The worst types of KVM bugs may even allow the guest code to crash or
compromise the physical host. KVM tests therefore need to be split into two
components – a KVM controller program running on the physical host and a guest
payload program running inside the VM. The cooperation of these two components
allows testing even of bugs that somehow cross the virtualization boundary.

Basic KVM Test Structure
------------------------

KVM tests are simple C source files containing both the KVM controller code
and the guest payload code separated by ``#ifdef COMPILE_PAYLOAD`` preprocessor
condition. The file will be compiled twice: once to compile the payload part,
and once to compile the KVM controller part and embed the payload binary inside.
The result is a single self-contained binary that will execute the embedded
payload inside a KVM virtual machine and print results in the same format as
a normal LTP test.

A KVM test source should start with ``#include "kvm_test.h"`` instead of the
usual ``tst_test.h``. The ``kvm_test.h`` header file will include the other basic
headers appropriate for the current compilation pass. Everything else in the
source file should be enclosed in ``#ifdef COMPILE_PAYLOAD ... #else ... #endif``
condition, including any other header file includes. Note that the standard
LTP headers are not available in the payload compilation pass; only the KVM
guest library headers can be included.

.. _example-kvm-test:

.. rubric:: Example KVM Test

.. code-block:: c

    #include "kvm_test.h"

    #ifdef COMPILE_PAYLOAD

    /* Guest payload code */

    void main(void)
    {
        tst_res(TPASS, "Hello, world!");
    }

    #else /* COMPILE_PAYLOAD */

    /* KVM controller code */

    static struct tst_test test = {
        .test_all = tst_kvm_run,
        .setup = tst_kvm_setup,
        .cleanup = tst_kvm_cleanup,
    };

    #endif /* COMPILE_PAYLOAD */

The KVM controller code is a normal LTP test and needs to define an instance
of ``struct tst_test`` with metadata and the usual setup, cleanup, and test
functions. Basic implementation of all three functions is provided by the KVM
host library.

On the other hand, the payload is essentially a tiny kernel that will run
on bare virtual hardware. It cannot access any files, Linux syscalls, standard
library functions, etc., except for the small subset provided by the KVM guest
library. The payload code must define a ``void main(void)`` function which will
be the VM entry point of the test.

KVM Host Library
----------------

The KVM host library provides helper functions for creating and running
a minimal KVM virtual machine.

Data Structures
~~~~~~~~~~~~~~~

.. code-block:: c

    struct tst_kvm_instance {
        int vm_fd, vcpu_fd;
        struct kvm_run *vcpu_info;
        size_t vcpu_info_size;
        struct kvm_userspace_memory_region ram[MAX_KVM_MEMSLOTS];
        struct tst_kvm_result *result;
    };

``struct tst_kvm_instance`` holds the file descriptors and memory buffers
of a single KVM virtual machine:

* ``int vm_fd`` is the main VM file descriptor created by ``ioctl(KVM_CREATE_VM)``
* ``int vcpu_fd`` is the virtual CPU file descriptor created by
  ``ioctl(KVM_CREATE_VCPU)``
* ``struct kvm_run *vcpu_info`` is the VCPU state structure created by
  ``mmap(vcpu_fd)``
* ``size_t vcpu_info_size`` is the size of ``vcpu_info`` buffer
* ``struct kvm_userspace_memory_region ram[MAX_KVM_MEMSLOTS]`` is the list
  of memory slots defined in this VM. Unused memory slots have zero
  in the ``userspace_addr`` field.
* ``struct tst_kvm_result *result`` is a buffer for passing test result data
  from the VM to the controller program, mainly ``tst_res()``/``tst_brk()`` flags
  and messages.

.. code-block:: c

    struct tst_kvm_result {
        int32_t result;
        int32_t lineno;
        uint64_t file_addr;
        char message[0];
    };

``struct tst_kvm_result`` is used to pass test results and synchronization data
between the KVM guest and the controller program. Most often, it is used
to pass ``tst_res()`` and ``tst_brk()`` messages from the VM, but special values
can also be used to send control flow requests both ways.

* ``int32_t result`` is the message type, either one of the ``TPASS``, ``TFAIL``,
  ``TWARN``, ``TBROK``, ``TINFO`` flags or a special control flow value. Errno flags
  are not supported.
* ``int32_t lineno`` is the line number for ``tst_res()``/``tst_brk()`` messages.
* ``uint64_t file_addr`` is the VM address of the filename string for
  ``tst_res()``/``tst_brk()`` messages.
* ``char message[0]`` is the buffer for arbitrary message data, most often used
  to pass ``tst_res()``/``tst_brk()`` message strings.

Working with Virtual Machines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The KVM host library provides default implementation of the setup, cleanup
and test functions for ``struct tst_test`` in cases where you do not need
to customize the VM configuration. You can either assign these functions
to the ``struct tst_test`` instance directly or call them from your own function
that does some additional steps. All three functions must be used together.

* ``void tst_kvm_setup(void)``
* ``void tst_kvm_run(void)``
* ``void tst_kvm_cleanup(void)``

.. note:: ``tst_kvm_run()`` calls ``tst_free_all()``. Calling it will free all
          previously allocated guarded buffers.

* ``void tst_kvm_validate_result(int value)`` – Validates whether the value
  returned in ``struct tst_kvm_result.result`` can be safely passed
  to ``tst_res()`` or ``tst_brk()``. If the value is not valid, the controller
  program will be terminated with an error.

* ``uint64_t tst_kvm_get_phys_address(const struct tst_kvm_instance *inst, uint64_t addr)`` –
  Converts pointer value (virtual address) from KVM virtual
  machine ``inst`` to the corresponding physical address. Returns 0 if
  the virtual address is not mapped to any physical address. If virtual memory
  mapping is not enabled in the VM or not available on the architecture at all, this
  function simply returns ``addr`` as is.

* ``int tst_kvm_find_phys_memslot(const struct tst_kvm_instance *inst, uint64_t paddr)`` –
  Returns index of the memory slot in KVM virtual machine
  ``inst`` which contains the physical address ``paddr``. If the address is not
  backed by a memory buffer, returns -1.

* ``int tst_kvm_find_memslot(const struct tst_kvm_instance *inst, uint64_t addr)`` –
  Returns index of the memory slot in KVM virtual machine
  ``inst`` which contains the virtual address ``addr``. If the virtual address
  is not mapped to a valid physical address backed by a memory buffer,
  returns -1.

* ``void *tst_kvm_get_memptr(const struct tst_kvm_instance *inst, uint64_t addr)`` –
  Converts pointer value (virtual address) from KVM virtual
  machine ``inst`` to host-side pointer.

* ``void *tst_kvm_alloc_memory(struct tst_kvm_instance *inst, unsigned int slot, uint64_t baseaddr, size_t size, unsigned int flags)`` –
  Allocates a guarded buffer of given ``size`` in bytes and installs it into specified memory ``slot``
  of the KVM virtual machine ``inst`` at base address ``baseaddr``. The buffer
  will be automatically page aligned at both ends. See the kernel
  documentation of ``KVM_SET_USER_MEMORY_REGION`` ioctl for a list of valid
  ``flags``. Returns pointer to page-aligned beginning of the allocated buffer.
  The actual requested ``baseaddr`` will be located at
  ``ret + baseaddr % pagesize``.

* ``struct kvm_cpuid2 *tst_kvm_get_cpuid(int sysfd)`` – Gets a list of supported
  virtual CPU features returned by ``ioctl(KVM_GET_SUPPORTED_CPUID)``.
  The argument must be an open file descriptor returned by ``open("/dev/kvm")``.

* ``void tst_kvm_create_instance(struct tst_kvm_instance *inst, size_t ram_size)`` –
  Creates and fully initializes a new KVM virtual machine
  with at least ``ram_size`` bytes of memory. The VM instance info will be
  stored in ``inst``.

* ``int tst_kvm_run_instance(struct tst_kvm_instance *inst, int exp_errno)`` –
  Executes the program installed in KVM virtual machine ``inst``. Any result
  messages returned by the VM will be automatically printed to the controller
  program output. Returns zero. If ``exp_errno`` is non-zero, the VM execution
  syscall is allowed to fail with the ``exp_errno`` error code and
  ``tst_kvm_run_instance()`` will return -1 instead of terminating the test.

* ``void tst_kvm_destroy_instance(struct tst_kvm_instance *inst)`` – Deletes
  the KVM virtual machine ``inst``. Note that the guarded buffers assigned
  to the VM by ``tst_kvm_create_instance()`` or ``tst_kvm_alloc_memory()`` will
  not be freed.

The KVM host library does not provide any way to reset a VM instance back
to its initial state. Running multiple iterations of the test requires destroying
the old VM instance and creating a new one. Otherwise, the VM will exit
without reporting any results on the second iteration, and the test will fail.
The ``tst_kvm_run()`` function handles this issue correctly.

KVM Guest Library
-----------------

The KVM guest library provides a minimal implementation of both the LTP
test library and the standard C library functions. Do not try to include
the usual LTP or C headers in guest payload code; it will not work.

Standard C Functions
~~~~~~~~~~~~~~~~~~~~

``#include "kvm_test.h"``

The functions listed below are implemented according to the C standard:

* ``void *memset(void *dest, int val, size_t size)``
* ``void *memzero(void *dest, size_t size)``
* ``void *memcpy(void *dest, const void *src, size_t size)``
* ``char *strcpy(char *dest, const char *src)``
* ``char *strcat(char *dest, const char *src)``
* ``size_t strlen(const char *str)``

LTP Library Functions
~~~~~~~~~~~~~~~~~~~~~

``#include "kvm_test.h"``

The KVM guest library currently provides the LTP functions for reporting test
results. All standard result flags except for ``T*ERRNO`` are supported
with the same rules as usual. However, printf-like formatting is not
implemented yet.

* ``void tst_res(int result, const char *message)``
* ``void tst_brk(int result, const char *message) __attribute__((noreturn))``

A handful of useful macros is also available:

* ``TST_TEST_TCONF(message)`` – Generates a test program that will simply print
  a ``TCONF`` message and exit. This is useful when the real test cannot be
  built due to missing dependencies or architecture limitations.

* ``ARRAY_SIZE(arr)`` – Returns the number of items in statically allocated
  array ``arr``.

* ``LTP_ALIGN(x, a)`` – Aligns integer ``x`` to be a multiple of ``a``, which
  must be a power of 2.

Architecture Independent Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``#include "kvm_test.h"``

Memory management in the KVM guest library currently uses only a primitive linear
buffer for memory allocation. There are no checks whether the VM can allocate
more memory, and the already allocated memory cannot be freed.

* ``void *tst_heap_alloc(size_t size)`` – Allocates a block of memory on the heap.

* ``void *tst_heap_alloc_aligned(size_t size, size_t align)`` – Allocates
  a block of memory on the heap with the starting address aligned to a given
  value.

x86 Specific Functions
~~~~~~~~~~~~~~~~~~~~~~

``#include "kvm_test.h"``
``#include "kvm_x86.h"``

* ``struct kvm_interrupt_frame`` – Opaque architecture-dependent structure which holds
  interrupt frame information. Use the functions below to get individual values:

* ``uintptr_t kvm_get_interrupt_ip(const struct kvm_interrupt_frame *ifrm)`` –
  Gets instruction pointer value from the interrupt frame structure. This may be
  the instruction which caused an interrupt or the one immediately after,
  depending on the interrupt vector semantics.

* ``int (*tst_interrupt_callback)(void *userdata, struct kvm_interrupt_frame *ifrm, unsigned long errcode)`` –
  Interrupt handler callback prototype. When an interrupt occurs, the assigned callback function
  will be passed the ``userdata`` pointer that was given
  to ``tst_set_interrupt_callback()``, interrupt frame ``ifrm`` and the error
  code ``errcode`` defined by the interrupt vector semantics. If the interrupt
  vector does not generate an error code, ``errcode`` will be set to zero.
  The callback function must return 0 if the interrupt was successfully
  handled and test execution should resume. A non-zero return value means that
  the interrupt could not be handled and the test will terminate with an error.

* ``void tst_set_interrupt_callback(unsigned int vector, tst_interrupt_callback func, void *userdata)`` –
  Registers a new interrupt handler callback function ``func`` for interrupt ``vector``. The ``userdata``
  argument is an arbitrary pointer that will be passed to ``func()`` every time
  it gets called. The previous interrupt handler callback will be removed.
  Setting ``func`` to ``NULL`` will remove any existing interrupt handler
  from ``vector``, and the interrupt will become a fatal error.

.. code-block:: c

    struct page_table_entry_pae {
        unsigned int present: 1;
        unsigned int writable: 1;
        unsigned int user_access: 1;
        unsigned int write_through: 1;
        unsigned int disable_cache: 1;
        unsigned int accessed: 1;
        unsigned int dirty: 1;
        unsigned int page_type: 1;
        unsigned int global: 1;
        unsigned int padding: 3;
        uint64_t address: 40;
        unsigned int padding2: 7;
        unsigned int prot_key: 4;
        unsigned int noexec: 1;
    } __attribute__((__packed__));

    struct kvm_cpuid {
        unsigned int eax, ebx, ecx, edx;
    };

    struct kvm_cregs {
        unsigned long cr0, cr2, cr3, cr4;
    };

    struct kvm_sregs {
        uint16_t cs, ds, es, fs, gs, ss;
    };

``struct page_table_entry_pae`` is the page table entry structure for PAE and
64-bit paging modes. See Intel® 64 and IA-32 Architectures Software
Developer's Manual, Volume 3, Chapter 4 for an explanation of the fields.

* ``uintptr_t kvm_get_page_address_pae(const struct page_table_entry_pae *entry)``
  – Returns the physical address of the memory page referenced by the given
  page table ``entry``. Depending on memory mapping changes done by the test,
  the physical address may not be a valid pointer. The caller must determine
  whether the address points to another page table entry or a data page, using
  the known position in page table hierarchy and ``entry->page_type``. Returns
  zero if the ``entry`` does not reference any memory page.

* ``void kvm_set_segment_descriptor(struct segment_descriptor *dst, uint64_t baseaddr, uint32_t limit, unsigned int flags)`` -
  Fills the ``dst`` segment descriptor with given values. The maximum value
  of ``limit`` is ``0xfffff`` (inclusive) regardless of ``flags``.

* ``void kvm_parse_segment_descriptor(struct segment_descriptor *src, uint64_t *baseaddr, uint32_t *limit, unsigned int *flags)`` -
  Parses data in the ``src`` segment descriptor and copies them to variables
  pointed to by the other arguments. Any parameter except the first one can
  be ``NULL``.

* ``int kvm_find_free_descriptor(const struct segment_descriptor *table, size_t size)`` -
  Finds the first segment descriptor in ``table`` which does not have
  the ``SEGFLAG_PRESENT`` bit set. The function handles double-size descriptors
  correctly. Returns the index of the first available descriptor or -1 if all
  ``size`` descriptors are taken.

* ``unsigned int kvm_create_stack_descriptor(struct segment_descriptor *table, size_t tabsize, void *stack_base)`` -
  A convenience function for registering a stack segment descriptor. It will
  automatically find a free slot in ``table`` and fill the necessary flags.
  The ``stack_base`` pointer must point to the bottom of the stack.

* ``void kvm_get_cpuid(unsigned int eax, unsigned int ecx, struct kvm_cpuid *buf)`` –
  Executes the CPUID instruction with the given
  ``eax`` and ``ecx`` arguments and stores the results in ``buf``.

* ``void kvm_read_cregs(struct kvm_cregs *buf)`` – Copies the current values
  of control registers to ``buf``.

* ``void kvm_read_sregs(struct kvm_sregs *buf)`` - Copies the current values
  of segment registers to ``buf``.

* ``uint64_t kvm_rdmsr(unsigned int msr)`` – Returns the current value
  of model-specific register ``msr``.

* ``void kvm_wrmsr(unsigned int msr, uint64_t value)`` – Stores ``value``
  into model-specific register ``msr``.

* ``void kvm_exit(void) __attribute__((noreturn))`` – Terminates the test.
  Similar to calling ``exit(0)`` in a regular LTP test, although ``kvm_exit()``
  will terminate only one iteration of the test, not the whole host process.

See Intel® 64 and IA-32 Architectures Software Developer's Manual
for documentation of standard and model-specific x86 registers.

AMD SVM Helper Functions
~~~~~~~~~~~~~~~~~~~~~~~~

``#include "kvm_test.h"``
``#include "kvm_x86.h"``
``#include "kvm_x86_svm.h"``

The KVM guest library provides basic helper functions for creating and running
nested virtual machines using the AMD SVM technology.

.. _example-code-to-execute-nested-vm:

.. rubric:: Example Code to Execute Nested VM

.. code-block:: c

    int guest_main(void)
    {
        ...
        return 0;
    }

    void main(void)
    {
        struct kvm_svm_vcpu *vm;

        kvm_init_svm();
        vm = kvm_create_svm_vcpu(guest_main, 1);
        kvm_svm_vmrun(vm);
    }

* ``int kvm_is_svm_supported(void)`` - Returns a non-zero value if the CPU
  supports AMD SVM; otherwise, returns 0.

* ``int kvm_get_svm_state(void)`` - Returns a non-zero value if SVM is currently
  enabled; otherwise, returns 0.

* ``void kvm_set_svm_state(int enabled)`` - Enables or disables SVM according
  to the argument. If SVM is disabled by the host or not supported, the test will exit
  with ``TCONF``.

* ``void kvm_init_svm(void)`` - Enables and fully initializes SVM, including
  allocating and setting up the host save area VMCB. If SVM is disabled by the host or
  not supported, the test will exit with ``TCONF``.

* ``struct kvm_vmcb *kvm_alloc_vmcb(void)`` - Allocates a new VMCB structure
  with correct memory alignment and fills it with zeroes.

* ``void kvm_vmcb_set_intercept(struct kvm_vmcb *vmcb, unsigned int id, unsigned int state)`` -
  Sets SVM intercept bit ``id`` to the given ``state``.

* ``void kvm_init_guest_vmcb(struct kvm_vmcb *vmcb, uint32_t asid, uint16_t ss, void *rsp, int (*guest_main)(void))`` -
  Initializes a new SVM virtual machine. The ``asid`` parameter is the nested
  page table ID. The ``ss`` and ``rsp`` parameters set the stack segment and stack
  pointer values, respectively. The ``guest_main`` parameter sets the code entry
  point of the virtual machine. All control registers, segment registers
  (except stack segment register), GDTR and IDTR will be copied
  from the current CPU state.

* ``struct kvm_svm_vcpu *kvm_create_svm_vcpu(int (*guest_main)(void), int alloc_stack)`` -
  A convenience function for allocating and initializing a new SVM virtual CPU.
  The ``guest_main`` parameter is passed to ``kvm_init_guest_vmcb()``;
  the ``alloc_stack`` parameter controls whether a new 8KB stack will be
  allocated and registered in GDT. Interception will be enabled for ``VMSAVE``
  and ``HLT`` instructions. If you set ``alloc_stack`` to zero, you must configure
  the stack segment register and stack pointer manually.

* ``void kvm_svm_vmrun(struct kvm_svm_vcpu *cpu)`` - Starts or continues execution
  of a nested virtual machine. Be aware that FPU state is not saved. Do not use
  floating point types or values in nested guest code. Also, do not use
  ``tst_res()`` or ``tst_brk()`` functions in nested guest code.

See AMD64 Architecture Programmer's Manual Volume 2 for documentation
of the Secure Virtual Machine (SVM) technology.

KVM Guest Environment
---------------------

KVM guest payload execution begins with bootstrap code which will perform
the minimal guest environment setup required for running C code:

* Activates the appropriate CPU execution mode (IA-32 protected mode
  on 32-bit x86 or the 64-bit mode on x86_64).
* Creates an identity mapping (virtual address = physical address) of the lower
  2GB memory region, even if parts of the region are not backed by any host
  memory buffers. The memory region above the 2GB threshold is left unmapped
  except for one memory page reserved for the ``struct tst_kvm_result`` buffer.
* Initializes an 8KB stack.
* Installs default interrupt handlers for standard CPU exception vectors.

When the environment setup is complete, bootstrap will call the ``void main(void)``
function implemented by the test program. To finish execution of the guest payload,
the test can either return from the ``main()`` function or call ``kvm_exit()``
at any point.
