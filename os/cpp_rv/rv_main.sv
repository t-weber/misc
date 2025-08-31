/**
 * cpu test
 * @author Tobias Weber
 * @date 31-aug-2025
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *   - https://github.com/YosysHQ/picorv32/tree/main/picosoc
 *   - https://github.com/grughuhler/picorv32_tang_nano_unified/tree/main
 */


module main
(
	// main clock
	input clk27,

	// keys
	input  [1 : 0] key,

	// leds
	output [5 : 0] led,
	output [7 : 0] ledg

);


// ----------------------------------------------------------------------------
// keys
// ----------------------------------------------------------------------------
logic rst;

debounce_switch debounce_key0(.in_clk(clk27), .in_rst(1'b0),
	.in_signal(~key[0]), .out_debounced(rst));
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// slow clock
// ----------------------------------------------------------------------------
localparam MAIN_CLK = 27_000_000;
localparam SLOW_CLK =        100;

logic clock;

clkgen #(.MAIN_CLK_HZ(MAIN_CLK), .CLK_HZ(SLOW_CLK))
clk_slow (.in_clk(clk27), .in_rst(1'b0), .out_clk(clock));
// ----------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// overall state
// ---------------------------------------------------------------------------
typedef enum bit [1 : 0]
{
	RESET_ALL, COPY_ROM, RUN_CPU
} t_state;

t_state state = RESET_ALL, next_state = RESET_ALL;

always_ff@(posedge clock) begin
	if(rst == 1'b1)
		state <= RESET_ALL;
	else
		state <= next_state;
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

wire reset = (state == RESET_ALL);
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// instantiate ram
// ---------------------------------------------------------------------------
localparam ADDR_BITS = 12;
localparam DATA_BITS = 32;

// ram port 1
wire write_enable_1;
logic [ADDR_BITS - 1 : 0] addr_1;
logic [DATA_BITS - 1 : 0] in_data_1, out_data_1;

// ram port 2
logic [ADDR_BITS - 1 : 0] addr_2;
logic [DATA_BITS - 1 : 0] out_data_2, next_out_data_2;


ram_2port #(.ADDR_BITS(ADDR_BITS), .WORD_BITS(DATA_BITS), .ALL_WRITE(1'b0))
	ram_mod(
		.in_rst(reset),

		// port 1 (reading and writing)
		.in_clk_1(clock),
		.in_read_ena_1(1'b1), .in_write_ena_1(write_enable_1),
		.in_addr_1(addr_1), .in_data_1(in_data_1), .out_data_1(out_data_1)

		// multiport not supported by hardware
		// port 2 (reading)
		//.in_clk_2(clock),
		//.in_read_ena_2(1'b1), .in_write_ena_2(1'b0)
		//.in_addr_2(addr_2), .in_data_2(32'b0), .out_data_2(out_data_2)
	);
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// instantiate rom
// ---------------------------------------------------------------------------
logic [DATA_BITS - 1 : 0] out_rom_data;
rom #()
	rom_mod(
		.in_addr(addr_1[5 : 0]),
		.out_data(out_rom_data)
	);
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// copy rom to ram
// ---------------------------------------------------------------------------
wire memcpy_finished;
logic [5 : 0] memcpy_addr;
logic [DATA_BITS - 1 : 0] memcpy_data;
logic memcpy_write_enable;

memcpy #(.ADDR_BITS(6), .NUM_WORDS(2**6), .WORD_BITS(DATA_BITS))
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
wire clock_cpu;

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
picorv32 #(.COMPRESSED_ISA(1'b0), .REGS_INIT_ZERO(1'b0),
	.ENABLE_MUL(1'b1), .ENABLE_DIV(1'b1), .BARREL_SHIFTER(1'b1),
	.PROGADDR_RESET(32'h0),                            // initial program counter
	.ENABLE_IRQ(1'b0), .PROGADDR_IRQ(32'h0020),        // see symbol table for _isr_entrypoint
	.ENABLE_IRQ_QREGS(1'b0), .ENABLE_IRQ_TIMER(1'b0),  // non-standard
	.STACKADDR({ (ADDR_BITS /*- 2*/){ 1'b1 } } - 4'hf) // initial stack pointer
)
cpu_mod(
	.resetn(~reset),
	.clk(clock_cpu),

	// interrupts
	.irq(cpu_irq),             // in
	.eoi(cpu_irq_ended),       // out,
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
	out_data_2 <= next_out_data_2;
end

always_comb begin
	next_state_memaccess = state_memaccess;
	next_write_data = write_data;
	cpu_mem_ready = 1'b0;
	cpu_write_enable = 1'b0;
	next_out_data_2 = out_data_2;

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
			if(addr_1 == addr_2)
				next_out_data_2 = write_data_sel;
		end

		CPU_WRITE: begin
			cpu_write_enable = 1'b1;
			next_state_memaccess = CPU_MEM_READY;
		end

		endcase
end
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// leds for debugging
// ---------------------------------------------------------------------------
assign led[0] = ~(state == COPY_ROM);
assign led[1] = ~(state == RUN_CPU);
/*assign led[2] = ~rst;
assign led[3] = ~reset;
assign led[4] = ~clock;
assign led[5] = ~clock_cpu;*/
assign led[2] = ~(state_memaccess == CPU_WAIT_MEM);
assign led[3] = ~(state_memaccess == CPU_MEM_READY);
assign led[4] = ~(state_memaccess == CPU_PREPARE_WRITE);
assign led[5] = ~(state_memaccess == CPU_WRITE);


// watch the 0x3f00 address for the memory test in main.cpp
assign addr_2 = (16'h3f00 >> 2'h2);
assign ledg[7:0] = out_data_2[7 : 0];
//assign ledg[7:0] = addr_1[7 : 0];
//assign ledg[7:0] = cpu_addr[7 : 0];
// ---------------------------------------------------------------------------


endmodule
