/**
 * a calculator system program running on sel4
 * @author Tobias Weber
 * @date apr-2021
 * @license GPLv3, see 'LICENSE.GPL' file
 *
 * References:
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/mapping/mapping.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/untyped/untyped.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/threads/threads.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/interrupts/interrupts.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/dynamic-2/dynamic-2.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/libsel4tutorials/src/alloc.c
 *   - https://docs.sel4.systems/projects/sel4/api-doc.html
 */

#include "defines.h"
#include "string.h"
#include "expr_parser.h"

#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>


struct Keyboard
{
	seL4_SlotPos keyb_slot;
	seL4_SlotPos irq_slot;
	seL4_SlotPos irq_notify;
};


// some (arbitrary) badge number for the thread endpoint
#define CALCTHREAD_BADGE 1234


void calc(seL4_SlotPos start_notify, i8 *charout, seL4_SlotPos endpoint/*, seL4_SlotPos keyb_notify*/)
{
	printf("Start of calculator thread, endpoint: %ld.\n", endpoint);
	seL4_Signal(start_notify);

	i32 x=0, y=1;
	i32 x_prev=x, y_prev=y;

	// init parser
	init_symbols();

	my_memset(charout, 0, SCREEN_SIZE*2);
	write_str("                                seL4 Calculator                                 ",
		ATTR_INV, charout);

	while(1)
	{
		// cursor
		charout[(y_prev*SCREEN_COL_SIZE + x_prev)*2 + 1] = ATTR_NORM;
		charout[(y*SCREEN_COL_SIZE + x)*2 + 1] = ATTR_INV;

		x_prev = x;
		y_prev = y;

		//seL4_MessageInfo_t msg = seL4_Recv(keyb_notify, 0);
		word_t badge = 0;
		//seL4_Wait(keyb_notify, &badge);
		seL4_MessageInfo_t msg = seL4_Recv(endpoint, &badge);

		// get the key code from the message register
		i16 key = seL4_GetMR(0);
		seL4_Reply(msg);

		if(key == 0x1c)	// enter
		{
			// scroll
			if(y >= SCREEN_ROW_SIZE - 2)
			{
				// reset cursor
				charout[(y_prev*SCREEN_COL_SIZE + x_prev)*2 + 1] = ATTR_NORM;

				for(u32 scr=0; scr<2; ++scr)
				{
					for(u32 _y = 2; _y<SCREEN_ROW_SIZE; ++_y)
					{
						my_memcpy(
							charout+SCREEN_COL_SIZE*(_y-1)*2,
							charout+SCREEN_COL_SIZE*_y*2,
							SCREEN_COL_SIZE*2);
						//my_memset_interleaved(
						//	charout+SCREEN_COL_SIZE*(_y-1)*2 + 1,
						//	ATTR_NORM,
						//	SCREEN_COL_SIZE*2, 2);
					}

					y -= 1;
				}
			}

			// read current line
			i8 line[SCREEN_COL_SIZE+1];
			read_str(line, charout+y*SCREEN_COL_SIZE*2, SCREEN_COL_SIZE);
			t_value val = parse(line);

			i8 numbuf[32];
			int_to_str(val, 10, numbuf);
			write_str(numbuf, ATTR_BOLD, charout + (y+1)*SCREEN_COL_SIZE*2);

			print_symbols();

			// new line
			y += 2;
			x = 0;
		}
		else if(key == 0x0e && x >= 1)	// backspace
		{
			--x;
			write_char(' ', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
		}
		else if(x < SCREEN_COL_SIZE)
		{
			// digit
			u64 num = key - 0x01;
			if(num == 10)
				num = 0;
			if(num >= 0 && num <= 9)
				write_char((i8)(num+0x30), ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x39)	// space
				write_char(' ', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x27 || key == 0x0d)	// +
				write_char('+', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x28 || key == 0x0c)	// -
				write_char('-', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x34)	// *
				write_char('*', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x28 || key == 0x35)	// /
				write_char('/', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x29)	// ^
				write_char('^', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x1a)	// (
				write_char('(', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x1b)	// )
				write_char(')', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x2b || key == 0x33)	// =
				write_char('=', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x10)	// q
				write_char('q', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x11)	// w
				write_char('w', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x12)	// e
				write_char('e', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x13)	// r
				write_char('r', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x14)	// t
				write_char('t', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x15)	// y
				write_char('y', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x16)	// u
				write_char('u', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x17)	// i
				write_char('i', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x18)	// o
				write_char('o', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x19)	// p
				write_char('p', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x1e)	// a
				write_char('a', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x1f)	// s
				write_char('s', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x20)	// d
				write_char('d', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x21)	// f
				write_char('f', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x22)	// g
				write_char('g', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x23)	// h
				write_char('h', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x24)	// j
				write_char('j', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x25)	// k
				write_char('k', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x26)	// l
				write_char('l', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x2c)	// z
				write_char('z', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x2d)	// x
				write_char('x', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x2e)	// c
				write_char('c', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x2f)	// v
				write_char('v', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x30)	// b
				write_char('b', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x31)	// n
				write_char('n', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else if(key == 0x32)	// m
				write_char('m', ATTR_NORM, charout + y*SCREEN_COL_SIZE*2 + x*2);
			else
				continue;
			++x;
		}
	}

	// deinit parser
	deinit_symbols();

	printf("End of calculator thread.\n");
	while(1) seL4_Yield();
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

	// ------------------------------------------------------------------------
	// initial thread and boot infos
	const seL4_SlotPos this_cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos this_vspace = seL4_CapInitThreadVSpace;
	const seL4_SlotPos this_tcb = seL4_CapInitThreadTCB;
	const seL4_SlotPos this_ipcbuf = seL4_CapInitThreadIPCBuffer;
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
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// (arbitrary) virtual addresses to map page tables, video ram and the TCB stack into
	word_t virt_addr_tables = 0x8000000000;
	word_t virt_addr_char = 0x8000001000;
	word_t virt_addr_tcb_stack = 0x8000002000;
	word_t virt_addr_tcb_tls = 0x8000003000;
	word_t virt_addr_tcb_ipcbuf = 0x8000004000;
	word_t virt_addr_tcb_tlsipc = virt_addr_tcb_tls + 0x10;

	// map the page tables
	map_pagetables(untyped_start, untyped_end, untyped_list, &cur_slot, virt_addr_tables);

	// find page whose frame contains the vga memory
	seL4_SlotPos page_slot = map_page_phys(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_char, CHAROUT_PHYS);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// keyboard interrupt service routine
	struct Keyboard keyb;

	// keyboard interrupt
	keyb.keyb_slot = cur_slot++;
	if(seL4_X86_IOPortControl_Issue(this_ioctrl, KEYB_DATA_PORT, KEYB_DATA_PORT,
		this_cnode, keyb.keyb_slot, seL4_WordBits) != seL4_NoError)
		printf("Error getting keyboard IO control!\n");

	keyb.irq_slot = cur_slot++;
	//seL4_IRQControl_Get(this_irqctrl, KEYB_IRQ, this_cnode, keyb.irq_slot, seL4_WordBits);
	if(seL4_IRQControl_GetIOAPIC(this_irqctrl, this_cnode, keyb.irq_slot,
		seL4_WordBits, KEYB_PIC, KEYB_IRQ, 0, 1, KEYB_INT) != seL4_NoError)
		printf("Error getting keyboard interrupt control!\n");

	keyb.irq_notify = get_slot(seL4_NotificationObject, 1<<seL4_NotificationBits,
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);
	if(seL4_IRQHandler_SetNotification(keyb.irq_slot, keyb.irq_notify) != seL4_NoError)
		printf("Error setting keyboard interrupt notification!\n");
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// create a page frame for the thread's stack
	seL4_SlotPos page_slot_tcb_stack = map_page(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_tcb_stack);

	seL4_SlotPos page_slot_tcb_tls = map_page(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_tcb_tls);

	seL4_SlotPos page_slot_tcb_ipcbuf = map_page(untyped_start, untyped_end,
		untyped_list, &cur_slot, virt_addr_tcb_ipcbuf);

	seL4_SlotPos tcb = get_slot(seL4_TCBObject, 1<<seL4_TCBBits,
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);

	// the child thread uses the main thread's cnode and vspace
	if(seL4_TCB_SetSpace(tcb, 0, this_cnode, 0, this_vspace, 0) != seL4_NoError)
		printf("Error: Cannot set TCB space!\n");

	// set up thread local storage
	if(seL4_TCB_SetTLSBase(tcb, virt_addr_tcb_tlsipc) != seL4_NoError)
		printf("Error: Cannot set TCB IPC buffer!\n");

	// set up the ipc buffer
	if(seL4_TCB_SetIPCBuffer(tcb, virt_addr_tcb_ipcbuf, page_slot_tcb_ipcbuf) != seL4_NoError)
		printf("Error: Cannot set TCB IPC buffer!\n");

	*(seL4_IPCBuffer**)virt_addr_tcb_tls = __sel4_ipc_buffer;

	// doesn't seem to get scheduled otherwise...
	if(seL4_TCB_SetPriority(tcb, this_tcb, seL4_MaxPrio) != seL4_NoError)
		printf("Error: Cannot set TCB priority!\n");

	// create semaphores for thread signalling
	seL4_SlotPos tcb_startnotify = get_slot(seL4_NotificationObject, 1<<seL4_NotificationBits,
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);
	//seL4_SlotPos keyb_notify = get_slot(seL4_NotificationObject, 1<<seL4_NotificationBits,
	//	untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);
	seL4_SlotPos tcb_endpoint = get_slot(seL4_EndpointObject, 1<<seL4_EndpointBits,
		untyped_start, untyped_end, untyped_list, &cur_slot, this_cnode);
	seL4_TCB_BindNotification(this_tcb, tcb_startnotify);
	//seL4_TCB_BindNotification(tcb, keyb_notify);

	// arbitrary badge number
	word_t tcb_badge = CALCTHREAD_BADGE;
	seL4_SlotPos tcb_startnotify2 = cur_slot++;
	if(seL4_CNode_Mint(this_cnode, tcb_startnotify2, seL4_WordBits, this_cnode,
		tcb_startnotify, seL4_WordBits, seL4_AllRights, tcb_badge) != seL4_NoError)
		printf("Error: Minting of start notifier failed.");
	//seL4_SlotPos keyb_notify2 = cur_slot++;
	//if(seL4_CNode_Mint(this_cnode, keyb_notify2, seL4_WordBits, this_cnode,
	//	keyb_notify, seL4_WordBits, seL4_AllRights, tcb_badge) != seL4_NoError)
	//	printf("Error: Minting of keyboard notifier failed.");
	seL4_SlotPos tcb_endpoint2 = cur_slot++;
	if(seL4_CNode_Mint(this_cnode, tcb_endpoint2, seL4_WordBits, this_cnode,
		tcb_endpoint, seL4_WordBits, seL4_AllRights, tcb_badge) != seL4_NoError)
		printf("Error: Minting of thread endpoint failed.");

	seL4_UserContext tcb_context;
	i32 num_regs = sizeof(tcb_context)/sizeof(tcb_context.rax);
	seL4_TCB_ReadRegisters(tcb, 0, 0, num_regs, &tcb_context);

	// pass instruction pointer, stack pointer and arguments in registers
	// according to sysv calling convention
	tcb_context.rip = (word_t)&calc;          // entry point
	tcb_context.rsp = (word_t)(virt_addr_tcb_stack + PAGE_SIZE);  // stack
	tcb_context.rbp = (word_t)(virt_addr_tcb_stack + PAGE_SIZE);  // stack
	tcb_context.rdi = (word_t)tcb_startnotify2; // arg 1: start notification
	tcb_context.rsi = (word_t)virt_addr_char;   // arg 2: vga ram
	tcb_context.rdx = (word_t)tcb_endpoint;     // arg 3: ipc endpoint
	//tcb_context.rcx = (word_t)keyb_notify;    // arg 4: keyboard notification
	//tcb_context.r8 = (word_t)this_cnode;      // arg 5: cnode

	printf("rip = 0x%lx, rsp = 0x%lx, rflags = 0x%lx, rdi = 0x%lx, rsi = 0x%lx, rdx = 0x%lx.\n",
		tcb_context.rip, tcb_context.rsp, tcb_context.rflags,
		tcb_context.rdi, tcb_context.rsi, tcb_context.rdx);

	// write registers and start thread
	if(seL4_TCB_WriteRegisters(tcb, 1, 0, num_regs, &tcb_context) != seL4_NoError)
		printf("Error writing TCB registers!\n");
	// ------------------------------------------------------------------------

	printf("Waiting for thread to start...\n");
	word_t start_badge;
	seL4_Wait(tcb_startnotify, &start_badge);
	printf("Thread started, badge: %ld.\n", start_badge);

	// keyboard isr
	while(1)
	{
		seL4_Wait(keyb.irq_notify, 0);
		seL4_X86_IOPort_In8_t key = seL4_X86_IOPort_In8(keyb.keyb_slot, 0x60);

		if(key.error != seL4_NoError)
		{
			printf("Error reading keyboard port!\n");
		}
		else
		{
			printf("Key code: 0x%x.\n", key.result);
			seL4_IRQHandler_Ack(keyb.irq_slot);
			//seL4_Signal(keyb_notify2);

			// save the key code in the message register
			seL4_SetMR(0, key.result);
			// send the keycode to the thread
			seL4_Call(tcb_endpoint2, seL4_MessageInfo_new(0,0,0,1));
		}
	}


	// ------------------------------------------------------------------------
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

