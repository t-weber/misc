/**
 * sel4 test
 * @author Tobias Weber
 * @date apr-2021
 * @license GPLv3, see 'LICENSE.GPL' file
 *
 * References:
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/mapping/mapping.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/untyped/untyped.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/threads/threads.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/interrupts/interrupts.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/libsel4tutorials/src/alloc.c
 *   - https://docs.sel4.systems/projects/sel4/api-doc.html
 */

#define SERIAL_DEBUG 1

#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>

#if SERIAL_DEBUG !=0
	#include <stdio.h>
#else
	#define printf(...) {}
#endif

typedef seL4_Uint8       u8;
typedef seL4_Int8        i8;
typedef seL4_Uint16      u16;
typedef seL4_Int16       i16;
typedef seL4_Uint32      u32;
typedef seL4_Int32       i32;
typedef seL4_Uint64      u64;
typedef seL4_Int64       i64;
typedef seL4_Word        word_t;

#define SCREEN_ROW_SIZE  25
#define SCREEN_COL_SIZE  80
#define SCREEN_SIZE      (SCREEN_ROW_SIZE * SCREEN_COL_SIZE)

//#define PAGE_TYPE      seL4_X86_LargePageObject
#define PAGE_TYPE        seL4_X86_4K
#define PAGE_SIZE        4096
#define CHAROUT_PHYS     0x000b8000  // see https://jbwyatt.com/253/emu/memory.html

#define ATTR_BOLD        0b00001111
#define ATTR_INV         0b01110000
#define ATTR_NORM        0b00000111

#define KEYB_DATA_PORT   0x60        // see https://wiki.osdev.org/%228042%22_PS/2_Controller
#define KEYB_PIC         0           // on which PIC is the keyboard?
#define KEYB_IRQ         1           // on which IRQ pin of the PIC is the keyboard?
#define KEYB_INT         33          // cpu interrupt to map to


// ----------------------------------------------------------------------------
// string helpers
// ----------------------------------------------------------------------------
void reverse_str(char* buf, word_t len)
{
	for(word_t i=0; i<len/2; ++i)
	{
		word_t j = len-i-1;
		//if(j <= i) break;

		char c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}


void uint_to_str(word_t num, word_t base, char* buf)
{
	word_t idx = 0;
	while(1)
	{
		word_t mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';
			break;
		}

		if(mod <= 9)
			buf[idx] = (char)mod + '0';
		else
			buf[idx] = (char)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf, idx);
}


void int_to_str(i64 num, word_t base, char* buf)
{
	word_t idx = 0;
	word_t beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(1)
	{
		i64 mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';

			break;
		}

		if(mod <= 9)
			buf[idx] = (char)mod + '0';
		else
			buf[idx] = (char)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf+beg, idx-beg);
}


word_t my_strlen(char *str)
{
	word_t len = 0;

	while(str[len])
		++len;

	return len;
}


void my_memset(i8* mem, i8 val, word_t size)
{
	for(word_t i=0; i<size; ++i)
		mem[i] = val;
}


void write_str(char *str, u8 attrib, i8 *addr)
{
	word_t len = my_strlen(str);

	for(word_t i=0; i<len; ++i)
	{
		*addr++ = str[i];
		*addr++ = attrib;
	}
}
// ----------------------------------------------------------------------------


u64 fact(u64 num)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	return num * fact(num-1);
}


u64 fibo(u64 num)
{
	if(num < 2)
		return 1;

	return fibo(num-1) + fibo(num-2);
}


void calc(u64 num_end, i8 *charout, seL4_SlotPos notify)
{
	if(notify)
	{
		printf("Start of calc() thread.\n");
	}
	const u64 num_start = 0;
	const u64 spacing = 16;

	my_memset(charout, 0, SCREEN_SIZE*2);
	write_str("                                seL4 Kernel Test                                ", ATTR_INV, charout);

	write_str("Number", ATTR_BOLD, charout + (SCREEN_COL_SIZE*2) * 2);
	write_str("Factorial", ATTR_BOLD, charout + (SCREEN_COL_SIZE*2 + spacing) * 2);
	write_str("Fibonacci", ATTR_BOLD, charout + (SCREEN_COL_SIZE*2 + spacing*2) * 2);

	for(u64 num=num_start; num<=num_end; ++num)
	{
		u64 num_fact = fact(num);
		u64 num_fibo = fibo(num);

		char buf_num[16], buf_fact[16], buf_fibo[16];
		int_to_str(num, 10, buf_num);
		int_to_str(num_fact, 10, buf_fact);
		int_to_str(num_fibo, 10, buf_fibo);

		write_str(buf_num, ATTR_NORM, charout + (SCREEN_COL_SIZE*(num-num_start+3)) * 2);
		write_str(buf_fact, ATTR_NORM, charout + ((SCREEN_COL_SIZE*(num-num_start+3)) + spacing)*2);
		write_str(buf_fibo, ATTR_NORM, charout + ((SCREEN_COL_SIZE*(num-num_start+3)) + spacing*2)*2);
	}

	if(notify)
	{
		printf("End of calc() thread.\n");

		seL4_Signal(notify);
		while(1) seL4_Yield();
	}
}


/**
 * find a free untyped slot
 */
seL4_SlotPos find_untyped(seL4_SlotPos untyped_start, seL4_SlotPos untyped_end,
	const seL4_UntypedDesc* untyped_list, word_t needed_size)
{
	for(seL4_SlotPos cur_slot=untyped_start; cur_slot<untyped_end; ++cur_slot)
	{
		const seL4_UntypedDesc *cur_descr = untyped_list + (cur_slot-untyped_start);

		if(cur_descr->isDevice)
			continue;

		word_t cur_size = (1<< cur_descr->sizeBits);
		//printf("Untyped slot 0x%lx: size=%ld, physical address=0x%lx.\n",
		//	cur_slot, cur_size, cur_descr->paddr);
		if(cur_size < needed_size)
			continue;
		return cur_slot;
	}

	return 0;
}


/**
 * find device memory region which has the given address
 */
seL4_SlotPos find_devicemem(seL4_SlotPos untyped_start, seL4_SlotPos untyped_end,
	const seL4_UntypedDesc* untyped_list, word_t addr)
{
	for(seL4_SlotPos cur_slot=untyped_start; cur_slot<untyped_end; ++cur_slot)
	{
		const seL4_UntypedDesc *cur_descr = untyped_list + (cur_slot-untyped_start);

		if(!cur_descr->isDevice)
			continue;

		word_t size = (1 << cur_descr->sizeBits);
		word_t addr_start = cur_descr->paddr;
		word_t addr_end = addr_start + size;

		//printf("Untyped device slot 0x%lx: physical addresses=[0x%lx..0x%lx[.\n",
		//	cur_slot, cur_descr->paddr, addr_end);
		if(addr >= addr_start && addr < addr_end)
			return cur_slot;
	}

	return 0;
}


void map_pagetables(seL4_SlotPos untyped_start, seL4_SlotPos untyped_end,
	const seL4_UntypedDesc* untyped_list, seL4_SlotPos* cur_slot,
	word_t virt_addr)
{
	const seL4_SlotPos cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos vspace = seL4_CapInitThreadVSpace;
	seL4_X86_VMAttributes vmattr = seL4_X86_Default_VMAttributes;

	seL4_SlotPos table_slot = find_untyped(untyped_start, untyped_end,
		untyped_list, PAGE_SIZE*1024);
	if(table_slot < untyped_start)
		printf("Error: No large enough untyped slot found!\n");
	printf("Loading tables into untyped slot 0x%lx.\n", table_slot);

	// load three levels of page tables
	const seL4_ArchObjectType pagetable_objs[] =
	{
		seL4_X86_PDPTObject,
		seL4_X86_PageDirectoryObject,
		seL4_X86_PageTableObject
	};

	seL4_Error (*pagetable_map[])(word_t, seL4_CPtr, word_t, seL4_X86_VMAttributes) =
	{
		&seL4_X86_PDPT_Map,
		&seL4_X86_PageDirectory_Map,
		&seL4_X86_PageTable_Map
	};

	seL4_SlotPos pagetable_slots[] = { 0, 0, 0 };

	for(i16 level=0; level<sizeof(pagetable_objs)/sizeof(pagetable_objs[0]); ++level)
	{
		pagetable_slots[level] = (*cur_slot)++;
		seL4_Untyped_Retype(table_slot, pagetable_objs[level], 0, cnode,
			0, 0, pagetable_slots[level], 1);
		if((*pagetable_map[level])(pagetable_slots[level],
			vspace, virt_addr, vmattr) != seL4_NoError)
		{
			printf("Error mapping page table level %d!\n", level);
			break;
		}
	}
}


seL4_SlotPos map_page(seL4_SlotPos untyped_start, seL4_SlotPos untyped_end,
	const seL4_UntypedDesc* untyped_list, seL4_SlotPos* cur_slot,
	word_t virt_addr)
{
	const seL4_SlotPos cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos vspace = seL4_CapInitThreadVSpace;
	seL4_X86_VMAttributes vmattr = seL4_X86_Default_VMAttributes;

	seL4_SlotPos base_slot = find_untyped(untyped_start, untyped_end, untyped_list, PAGE_SIZE);
	printf("Using device memory slot 0x%lx.\n", base_slot);

	seL4_SlotPos page_slot = (*cur_slot)++;
	seL4_Untyped_Retype(base_slot, PAGE_TYPE, 0, cnode, 0, 0, page_slot, 1);

	if(seL4_X86_Page_Map(page_slot, vspace, virt_addr, seL4_AllRights, vmattr)
		!= seL4_NoError)
	{
		printf("Error mapping page!\n");
	}

	seL4_X86_Page_GetAddress_t addr_info = seL4_X86_Page_GetAddress(page_slot);
	printf("Mapped virtual address: 0x%lx -> physical address: 0x%lx.\n",
		virt_addr, addr_info.paddr);

	return page_slot;
}


seL4_SlotPos map_page_phys(seL4_SlotPos untyped_start, seL4_SlotPos untyped_end,
	const seL4_UntypedDesc* untyped_list, seL4_SlotPos* cur_slot,
	word_t virt_addr, word_t phys_addr)
{
	const seL4_SlotPos cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos vspace = seL4_CapInitThreadVSpace;
	seL4_X86_VMAttributes vmattr = seL4_X86_Default_VMAttributes;

	seL4_SlotPos base_slot = find_devicemem(untyped_start, untyped_end,
		untyped_list, (word_t)phys_addr);
	printf("Using device memory slot 0x%lx.\n", base_slot);

	seL4_SlotPos page_slot = 0;
	// iterate all page frames till we're at the correct one
	// TODO: find more efficient way to specify an offset
	for(word_t i=0; i<=phys_addr/PAGE_SIZE; ++i)
	{
		seL4_SlotPos frame_slot = (*cur_slot)++;
		seL4_Untyped_Retype(base_slot, PAGE_TYPE, 0, cnode, 0, 0, frame_slot, 1);
		page_slot = frame_slot;
	}
	//seL4_SlotPos frame_slot = (*cur_slot)++;
	//seL4_Untyped_Retype(base_slot, seL4_X86_LargePageObject, 0, cnode, 0, 0, frame_slot, 1);

	if(seL4_X86_Page_Map(page_slot, vspace, virt_addr, seL4_ReadWrite, vmattr)
		!= seL4_NoError)
	{
		printf("Error mapping page!\n");
	}

	seL4_X86_Page_GetAddress_t addr_info = seL4_X86_Page_GetAddress(page_slot);
	printf("Mapped virtual address: 0x%lx -> physical address: 0x%lx.\n",
		virt_addr, addr_info.paddr);

	return page_slot;
}


seL4_SlotPos get_slot(word_t obj, word_t obj_size, 
	seL4_SlotPos untyped_start, seL4_SlotPos untyped_end, const seL4_UntypedDesc* untyped_list, 
	seL4_SlotPos* cur_slot, seL4_SlotPos cnode)
{
	seL4_SlotPos slot = find_untyped(untyped_start, untyped_end, untyped_list, obj_size);
	seL4_SlotPos offs = (*cur_slot)++;
	seL4_Untyped_Retype(slot, obj, 0, cnode, 0, 0, offs, 1);

	return offs;
}


i64 main()
{
	printf("--------------------------------------------------------------------------------\n");

	// initial thread and boot infos
	const seL4_SlotPos this_cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos this_vspace = seL4_CapInitThreadVSpace;
	const seL4_SlotPos this_tcb = seL4_CapInitThreadTCB;
	const seL4_SlotPos this_irqctrl = seL4_CapIRQControl;
	const seL4_SlotPos this_ioctrl = seL4_CapIOPortControl;
	const seL4_BootInfo *bootinfo = platsupport_get_bootinfo();

	const seL4_SlotRegion *empty_region = &bootinfo->empty;
	const seL4_SlotPos empty_start = empty_region->start;
	const seL4_SlotPos empty_end = empty_region->end;
	printf("Empty CNodes in region: [%ld .. %ld[.\n", empty_start, empty_end);

	const seL4_UntypedDesc* untyped_list = bootinfo->untypedList;
	const seL4_SlotRegion *untyped_region = &bootinfo->untyped;
	const seL4_SlotPos untyped_start = untyped_region->start;
	const seL4_SlotPos untyped_end = untyped_region->end;
	printf("Untyped CNodes in region: [%ld .. %ld[.\n", untyped_start, untyped_end);

	seL4_SlotPos cur_slot = empty_start;
	seL4_SlotPos cur_untyped_slot = untyped_start;

	// (arbitrary) virtual addresses to map page tables, video ram and the TCB stack into
	word_t virt_addr_tables = 0x8000000000;
	word_t virt_addr_char = 0x8000001000;
	word_t virt_addr_tcb_stack = 0x8000002000;

	// map the page tables
	map_pagetables(untyped_start, untyped_end, untyped_list, &cur_slot, virt_addr_tables);

	// find page whose frame contains the vga memory
	seL4_SlotPos page_slot = map_page_phys(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_char, CHAROUT_PHYS);

	// run calculation in another thread
	// create a page frame for the thread's stack
	seL4_SlotPos page_slot_tcb_stack = map_page(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_tcb_stack);

	seL4_SlotPos tcb = get_slot(seL4_TCBObject, 1<<seL4_TCBBits, 
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);

	// the child thread uses the main thread's cnode and vspace
	if(seL4_TCB_SetSpace(tcb, 0, this_cnode, 0, this_vspace, 0) != seL4_NoError)
		printf("Error: Cannot set TCB space!\n");

	// doesn't seem to get scheduled otherwise...
	if(seL4_TCB_SetPriority(tcb, this_tcb, seL4_MaxPrio) != seL4_NoError)
		printf("Error: Cannot set TCB priority!\n");

	// create semaphore for thread signalling
	seL4_SlotPos notify = get_slot(seL4_NotificationObject, 1<<seL4_NotificationBits, 
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);

	// copy semaphore
	seL4_SlotPos notify2 = cur_slot++;
	if(seL4_CNode_Mint(this_cnode, notify2, seL4_WordBits, this_cnode, 
		notify, seL4_WordBits, seL4_AllRights, 0) != seL4_NoError)
		printf("Error: Mint failed.");

	seL4_UserContext tcb_context;
	i32 num_regs = sizeof(tcb_context)/sizeof(tcb_context.rax);
	seL4_TCB_ReadRegisters(tcb, 0, 0, num_regs, &tcb_context);

	// pass instruction pointer, stack pointer and arguments in registers
	// according to sysv calling convention
	tcb_context.rip = (word_t)&calc;          // entry point
	tcb_context.rsp = (word_t)(virt_addr_tcb_stack + PAGE_SIZE);  // stack
	tcb_context.rbp = (word_t)(virt_addr_tcb_stack + PAGE_SIZE);  // stack
	tcb_context.rdi = 12;                     // arg 1: number of calculations
	tcb_context.rsi = (word_t)virt_addr_char; // arg 2: vga ram
	tcb_context.rdx = (word_t)notify;         // arg 3: semaphore

	printf("rip = 0x%lx, rsp = 0x%lx, rflags = 0x%lx, rdi = 0x%lx, rsi = 0x%lx, rdx = 0x%lx.\n",
		tcb_context.rip, tcb_context.rsp, tcb_context.rflags,
		tcb_context.rdi, tcb_context.rsi, tcb_context.rdx);

	// write registers and start thread
	if(seL4_TCB_WriteRegisters(tcb, 1, 0, num_regs, &tcb_context) != seL4_NoError)
		printf("Error writing TCB registers!\n");

	// keyboard interrupt
	seL4_SlotPos keyb_slot = cur_slot++;
	if(seL4_X86_IOPortControl_Issue(this_ioctrl, KEYB_DATA_PORT, KEYB_DATA_PORT, 
		this_cnode, keyb_slot, seL4_WordBits) != seL4_NoError)
		printf("Error getting keyboard IO control!\n");

	seL4_SlotPos irq_slot = cur_slot++;
	//seL4_IRQControl_Get(this_irqctrl, KEYB_IRQ, this_cnode, irq_slot, seL4_WordBits);
	if(seL4_IRQControl_GetIOAPIC(this_irqctrl, this_cnode, irq_slot, 
		seL4_WordBits, KEYB_PIC, KEYB_IRQ, 0, 1, KEYB_INT) != seL4_NoError)
		printf("Error getting keyboard interrupt control!\n");

	seL4_SlotPos irq_notify = get_slot(seL4_NotificationObject, 1<<seL4_NotificationBits, 
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);
	if(seL4_IRQHandler_SetNotification(irq_slot, irq_notify) != seL4_NoError)
		printf("Error setting keyboard interrupt notification!\n");

	while(1)
	{
		seL4_Wait(irq_notify, 0);
		seL4_X86_IOPort_In8_t key = seL4_X86_IOPort_In8(keyb_slot, 0x60);
		if(key.error != seL4_NoError)
		{
			printf("Error reading keyboard port!\n");
		}
		else
		{
			printf("Key pressed: 0x%x.\n", key.result);
			seL4_IRQHandler_Ack(irq_slot);

			u64 num = key.result - 0x01;
			if(num >= 1 && num <= 10)
				calc(num, (i8*)virt_addr_char, 0);

			// esc
			if(key.result == 0x01)
				break;
		}
	}

	printf("Waiting for thread to end.");
	seL4_Wait(notify2, 0);
	printf("Thread ended.\n");

	// end program
	seL4_TCB_Suspend(tcb);
	seL4_CNode_Revoke(this_cnode, page_slot_tcb_stack, seL4_WordBits);
	seL4_CNode_Revoke(this_cnode, page_slot, seL4_WordBits);
	//seL4_CNode_Revoke(this_cnode, base_slot, seL4_WordBits);
	//seL4_CNode_Revoke(this_cnode, table_slot, seL4_WordBits);

	printf("--------------------------------------------------------------------------------\n");
	while(1) seL4_Yield();
	return 0;
}

