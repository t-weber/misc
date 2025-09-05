/**
 * cpu testbench
 * @author Tobias Weber
 * @date 24-aug-2025
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *   - https://github.com/YosysHQ/picorv32/tree/main/picosoc
 *   - https://github.com/grughuhler/picorv32_tang_nano_unified/tree/main
 */


`timescale 1ns / 1ps
`default_nettype /*wire*/ none

//`define RAM_DISABLE_PORT2
//`define WRITE_DUMP
`ifndef USE_INTERRUPTS
	`define USE_INTERRUPTS 1'b0
`endif


module rv_tb();
	// ---------------------------------------------------------------------------
	// overall state
	// ---------------------------------------------------------------------------
	typedef enum bit [1 : 0]
	{
		RESET_ALL, COPY_ROM, RUN_CPU
	} t_state;

	t_state state = RESET_ALL, next_state = RESET_ALL;

	always_ff@(posedge clock) begin
		state <= next_state;
		//$display("state=%s", state.name());
	end

	always_comb begin
		next_state = state;

		case(state)
			RESET_ALL: begin
				next_state = COPY_ROM;
			end

			COPY_ROM: begin
				if(memcpy_finished == 1'b1)
					next_state = RUN_CPU;
			end

			RUN_CPU: begin
			end
		endcase
	end
	// ---------------------------------------------------------------------------


	wire reset = (state == RESET_ALL);
	logic clock = 1'b0;


	// ---------------------------------------------------------------------------
	// instantiate ram
	// ---------------------------------------------------------------------------
	localparam ADDR_BITS = 14; //12;
	localparam DATA_BITS = 32;

	// ram port 1
	wire write_enable_1;
	logic [ADDR_BITS - 1 : 0] addr_1;
	logic [DATA_BITS - 1 : 0] in_data_1, out_data_1;

	// ram port 2
	logic [ADDR_BITS - 1 : 0] addr_2;
	logic [DATA_BITS - 1 : 0] out_data_2;
`ifdef RAM_DISABLE_PORT2
	logic [DATA_BITS - 1 : 0] next_out_data_2;
`endif

	ram_2port #(.ADDR_BITS(ADDR_BITS), .WORD_BITS(DATA_BITS), .ALL_WRITE(1'b0))
		ram_mod(
			.in_rst(reset),

			// port 1 (reading and writing)
			.in_clk_1(clock),
			.in_read_ena_1(1'b1), .in_write_ena_1(write_enable_1),
			.in_addr_1(addr_1), .in_data_1(in_data_1), .out_data_1(out_data_1)

`ifndef RAM_DISABLE_PORT2
			,
			// port 2 (reading)
			.in_clk_2(clock),
			.in_read_ena_2(1'b1), .in_write_ena_2(1'b0),
			.in_addr_2(addr_2), .in_data_2(), .out_data_2(out_data_2)
`endif
		);
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// instantiate rom
	// ---------------------------------------------------------------------------
	logic [DATA_BITS - 1 : 0] out_rom_data;
	rom #()
		rom_mod(
			.in_addr(addr_1[7 : 0]),
			.out_data(out_rom_data)
		);
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// copy rom to ram
	// ---------------------------------------------------------------------------
	wire memcpy_finished;
	logic [7 : 0] memcpy_addr;
	logic [DATA_BITS - 1 : 0] memcpy_data;
	logic memcpy_write_enable;

	memcpy #(.ADDR_BITS(8), .NUM_WORDS(2**8), .WORD_BITS(DATA_BITS))
		memcpy_mod(
			.in_clk(clock),
			.in_rst(reset),

			.in_word(out_rom_data),
			.out_word(memcpy_data),
			.out_addr(memcpy_addr),

			.out_read_enable(),
			.out_write_enable(memcpy_write_enable),
			.in_read_finished(1'b1),
			.in_write_finished(1'b1),

			.out_finished(memcpy_finished)
		);
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// cpu
	// ---------------------------------------------------------------------------
	logic clock_cpu;

	logic [DATA_BITS - 1 : 0] cpu_irq = 4'b0000;
	wire [DATA_BITS - 1 : 0] cpu_irq_ended;
	wire cpu_trap;

	logic cpu_mem_valid;
	logic cpu_mem_ready = 1'b0;
	wire [31 : 0] cpu_addr;
	logic [3 : 0] cpu_bytesel;  // selects byte to write to, 0: read
	logic cpu_write_enable;

	logic [DATA_BITS - 1 : 0] cpu_data;
	logic [DATA_BITS - 1 : 0] write_data, next_write_data;

	// instantiate cpu
	picorv32 #(.COMPRESSED_ISA(1'b0), .REGS_INIT_ZERO(1'b1),
		.ENABLE_MUL(1'b1), .ENABLE_DIV(1'b1), .BARREL_SHIFTER(1'b1),
		.PROGADDR_RESET(32'h0),                                // initial program counter
		.ENABLE_IRQ(USE_INTERRUPTS), .PROGADDR_IRQ(32'h0040),  // see symbol table for _isr_entrypoint
		.ENABLE_IRQ_QREGS(1'b0), .ENABLE_IRQ_TIMER(1'b0),      // non-standard
		.STACKADDR({ (ADDR_BITS /*- 2*/){ 1'b1 } } - 4'hf)     // initial stack pointer
	)
	cpu_mod(
		.resetn(~reset),
		.clk(clock_cpu),

		// interrupts
		.irq(cpu_irq),             // in
		.eoi(cpu_irq_ended),       // out
		.trap(cpu_trap),           // out,

		// memory interface
		.mem_ready(cpu_mem_ready), // in
		.mem_valid(cpu_mem_valid), // out
		.mem_addr(cpu_addr),       // out
		.mem_wstrb(cpu_bytesel),   // out
		.mem_wdata(cpu_data),      // out
		.mem_rdata(out_data_1),    // in

		.pcpi_rd(32'b0),
		.pcpi_wr(1'b0),
		.pcpi_wait(1'b0),
		.pcpi_ready(1'b0)
	);


	// switch between cpu and memcpy
	assign clock_cpu = ((state == RUN_CPU || state == RESET_ALL) ? clock : 1'b0);
	assign addr_1 = (state == RUN_CPU ? cpu_addr[ADDR_BITS + 2 - 1 : 2] : memcpy_addr);
	assign in_data_1 = (state == RUN_CPU ? write_data : memcpy_data);
	assign write_enable_1 = (state == RUN_CPU ? cpu_write_enable : memcpy_write_enable);
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// manage memory read and write access by cpu
	// ---------------------------------------------------------------------------
	logic [DATA_BITS - 1 : 0] write_data_sel;

	memsel #(.WORD_BITS(DATA_BITS), .BYTE_BITS(8))
		memsel_mod(
			.in_word_1(cpu_data),
			.in_word_2(out_data_1),
			.in_sel(cpu_bytesel),
			.out_word(write_data_sel)
		);


	typedef enum bit [3 : 0]
	{
		CPU_WAIT_MEM, CPU_MEM_READY,
		CPU_PREPARE_WRITE, CPU_WRITE
	} t_state_memaccess;

	t_state_memaccess state_memaccess = CPU_WAIT_MEM, next_state_memaccess = CPU_WAIT_MEM;

	always_ff@(posedge clock) begin
		state_memaccess <= next_state_memaccess;
		write_data <= next_write_data;
`ifdef RAM_DISABLE_PORT2
		out_data_2 <= next_out_data_2;
`endif
	end

	always_comb begin
		next_state_memaccess = state_memaccess;
		next_write_data = write_data;
		cpu_mem_ready = 1'b0;
		cpu_write_enable = 1'b0;
`ifdef RAM_DISABLE_PORT2
		next_out_data_2 = out_data_2;
`endif

		case(state_memaccess)
			CPU_WAIT_MEM: begin
				if(cpu_mem_valid == 1'b1) begin
					if(cpu_bytesel == 1'b0)
						next_state_memaccess = CPU_MEM_READY;
					else
						next_state_memaccess = CPU_PREPARE_WRITE;
				end
			end

			CPU_MEM_READY: begin
				// skip one cycle to fetch the cpu's requested data from the memory
				cpu_mem_ready = 1'b1;
				next_state_memaccess = CPU_WAIT_MEM;
			end

			CPU_PREPARE_WRITE: begin
				next_write_data = write_data_sel;
				next_state_memaccess = CPU_WRITE;
`ifdef RAM_DISABLE_PORT2
				// if the cpu is writing to the address being watched,
				// copy the data
				if(addr_1 == addr_2)
					next_out_data_2 = write_data_sel;
`endif
			end

			CPU_WRITE: begin
				cpu_write_enable = 1'b1;
				next_state_memaccess = CPU_MEM_READY;
			end
			endcase
	end
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// create clock
	// ---------------------------------------------------------------------------
	integer iter;
	integer maxiter;

	initial begin
`ifdef RAM_DISABLE_PORT2
		$display("Using one-port RAM.");
`endif

		if($value$plusargs("iter=%d", maxiter)) begin
			$display("Maximum number of clock cycles: %d", maxiter);
		end else begin
			maxiter = 3000;
			$display("Maximum number of clock cycles: %d. Set using +iter=<num> argument.", maxiter);
		end

`ifdef WRITE_DUMP
		$dumpfile("rv_tb.vcd");
		$dumpvars(0, rv_tb);
`endif

		// read address for 2nd memory port
		addr_2 <= ({ADDR_BITS{16'h3f00}} >> 2'h2);  // watch the 0xff00 address for the memory test in main.cpp

		for(iter = 0; iter < maxiter; ++iter) begin
			clock <= ~clock;
			#1;

`ifdef USE_INTERRUPTS
			// generate a test interrupt (when enabled)
			if(iter >= 1650 && iter < 1660)
				cpu_irq <= 4'b1000;
			else
				cpu_irq <= 4'b0000;
`endif
		end

`ifdef WRITE_DUMP
		$dumpflush();
`endif
	end
	// ---------------------------------------------------------------------------


	// ---------------------------------------------------------------------------
	// debug output
	// ---------------------------------------------------------------------------
`ifdef DEBUG
	always@(posedge clock) begin
		if(state == RUN_CPU) begin
			$display("clk=%b, rst=%b, ", clock, reset, /*"cyle=%4d, ", iter,*/
				/*"irq=%h, ", cpu_irq, "irq_end=%h, ", cpu_irq_ended,*/
				"addr=%h, ", addr_1, /*"fulladdr=%h, ", cpu_addr,*/ "byte=%h, ", cpu_bytesel,
				"mem->cpu=%h, ", out_data_1, "cpu->mem=%h, ", in_data_1,
				"valid=%b, ", cpu_mem_valid,
				"addr_2=%h, data_2=%h.", addr_2, out_data_2);
			end
	end
`endif
	// ---------------------------------------------------------------------------

endmodule
