/**
 * sel4 test
 * @author Tobias Weber
 * @date apr-2021
 * @license GPLv3
 *
 * References:
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/mapping/mapping.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/untyped/untyped.md
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

typedef seL4_Uint8  u8;
typedef seL4_Int8   i8;
typedef seL4_Uint16 u16;
typedef seL4_Int16  i16;
typedef seL4_Uint32 u32;
typedef seL4_Int32  i32;
typedef seL4_Uint64 u64;
typedef seL4_Int64  i64;
typedef seL4_Word   word_t;

#define SCREEN_ROW_SIZE  25
#define SCREEN_COL_SIZE  80
#define SCREEN_SIZE      (SCREEN_ROW_SIZE * SCREEN_COL_SIZE)

#define PAGE_SIZE        4096
#define CHAROUT_PHYS     0x000b8000  /* see https://jbwyatt.com/253/emu/memory.html */

#define ATTR_BOLD        0b00001111
#define ATTR_INV         0b01110000
#define ATTR_NORM        0b00000111


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


void calc(u64 num_start, u64 num_end, i8 *charout)
{
	const u64 spacing = 16;

	my_memset(charout, 0, SCREEN_SIZE*2);
	write_str("                                 Long Mode Test                                 ", ATTR_INV, charout);

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
		//printf("Untyped slot %lx: size=%ld, physical address=0x%lx.\n",
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

		//printf("Untyped device slot %lx: physical addresses=[0x%lx..0x%lx[.\n",
		//	cur_slot, cur_descr->paddr, addr_end);
		if(addr >= addr_start && addr < addr_end)
			return cur_slot;
	}

	return 0;
}


i64 main()
{
	printf("--------------------------------------------------------------------------------\n");

	// initial thread and boot infos
	const seL4_SlotPos cnode = seL4_CapInitThreadCNode;
	const seL4_SlotPos vspace = seL4_CapInitThreadVSpace;
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
	// some (arbitrary) virtual address to map the video/character memory to
	word_t virt_addr = 0x8000000000;

	//seL4_SlotPos table_slot = 0x183;
	seL4_SlotPos table_slot = find_untyped(untyped_start, untyped_end,
		untyped_list, 4*1024*1024);
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
	seL4_X86_VMAttributes vmattr = seL4_X86_Default_VMAttributes;

	for(i16 level=0; level<sizeof(pagetable_objs)/sizeof(pagetable_objs[0]); ++level)
	{
		pagetable_slots[level] = cur_slot++;
		seL4_Untyped_Retype(table_slot, pagetable_objs[level], 0, cnode,
			0, 0, pagetable_slots[level], 1);
		if((*pagetable_map[level])(pagetable_slots[level],
			vspace, virt_addr, vmattr) != seL4_NoError)
		{
			printf("Error mapping page table level %d!\n", level);
			break;
		}
	}


	// find page whose frame contains the vga memory
	//seL4_SlotPos base_slot = 0x137;
	seL4_SlotPos base_slot = find_devicemem(untyped_start, untyped_end,
		untyped_list, (word_t)CHAROUT_PHYS);
	printf("Using device memory slot 0x%lx.\n", base_slot);

	seL4_SlotPos page_slot = 0;
	// iterate all page frames till we're at the correct one
	// TODO: find more efficient way to specify an offset
	for(word_t i=0; i<=CHAROUT_PHYS/PAGE_SIZE; ++i)
	{
		seL4_SlotPos frame_slot = cur_slot++;
		seL4_Untyped_Retype(base_slot, seL4_X86_4K, 0, cnode, 0, 0, frame_slot, 1);
		page_slot = frame_slot;
	}
	if(seL4_X86_Page_Map(page_slot, vspace, virt_addr, seL4_ReadWrite, vmattr)
		!= seL4_NoError)
	{
		printf("Error mapping Page!\n");
	}

	seL4_X86_Page_GetAddress_t phys_addr = seL4_X86_Page_GetAddress(page_slot);
	printf("Mapped virtual address: 0x%lx -> physical address: 0x%lx.\n",
		virt_addr, phys_addr.paddr);


	// run calculation
	//printf("fact = %ld\n", fact(5));
	calc(0, 12, (i8*)virt_addr);


	// end program
	//seL4_CNode_Revoke(cnode, page_slot, seL4_WordBits);
	seL4_CNode_Revoke(cnode, base_slot, seL4_WordBits);
	seL4_CNode_Revoke(cnode, table_slot, seL4_WordBits);

	printf("--------------------------------------------------------------------------------\n");
	while(1) seL4_Yield();
	return 0;
}
