/*
 * soc
 */

`default_nettype none

module soc(
    input clk_24,
    input reset,
    // serial
    input RX,
    output TX,
    // SPI core 0
    inout spi0_mosi,
    inout spi0_miso,
    inout spi0_sclk,
    inout spi0_cs0,
    // SPI core 1
    inout spi1_mosi,
    inout spi1_miso,
    inout spi1_sclk,
    inout spi1_cs0,
    // I2C core 0
    inout i2c0_sda,		
    inout i2c0_scl,
    output reg [31:0] gp_out
);
    // CPU
    wire mem_valid;
    wire mem_instr;
    wire mem_ready;
    wire [31:0] mem_addr;
    reg [31:0] mem_rdata;
    wire [31:0] mem_wdata;
    wire [3:0] mem_wstrb;
    picorv32 #(
        .PROGADDR_RESET(32'h0000_0000),    // start or ROM
        .STACKADDR(32'h1002_0000),         // end of RAM
        .BARREL_SHIFTER(1),
        .COMPRESSED_ISA(0),
        .ENABLE_COUNTERS(0),
`ifdef RV32I
        .ENABLE_MUL(0),
        .ENABLE_DIV(0),
`else
        .ENABLE_MUL(1),
        .ENABLE_DIV(1),
`endif
        .ENABLE_IRQ(0),
        .ENABLE_IRQ_QREGS(0),
        .CATCH_MISALIGN(1),
        .CATCH_ILLINSN(0)
    ) cpu_inst (
        .clk(clk_24),
        .resetn(~reset),
        .mem_valid(mem_valid),
        .mem_instr(mem_instr),
        .mem_ready(mem_ready),
        .mem_addr(mem_addr),
        .mem_wdata(mem_wdata),
        .mem_wstrb(mem_wstrb),
        .mem_rdata(mem_rdata)
    );
    
    // address decode
    wire rom_sel = (mem_addr[31:28] == 4'h0) & mem_valid ? 1'b1 : 1'b0;
    wire ram_sel = (mem_addr[31:28] == 4'h1) & mem_valid ? 1'b1 : 1'b0;
    wire gpo_sel = (mem_addr[31:28] == 4'h2) & mem_valid ? 1'b1 : 1'b0;
    wire ser_sel = (mem_addr[31:28] == 4'h3) & mem_valid ? 1'b1 : 1'b0;
    wire wbb_sel = (mem_addr[31:28] == 4'h4) & mem_valid ? 1'b1 : 1'b0;
    wire cnt_sel = (mem_addr[31:28] == 4'h5) & mem_valid ? 1'b1 : 1'b0;
    
    // 2k x 32b ROM
    reg [31:0] rom[2047:0], rom_do;
    initial
        $readmemh("../out/placeholder_firmware.hex", rom);
    always @(posedge clk_24)
        rom_do <= rom[mem_addr[12:2]];
    
    // 128KB RAM, byte-addressable
    wire [31:0] ram_do;
    spram_32kx32 ram_inst(
        .clk(clk_24),
        .sel(ram_sel),
        .we(mem_wstrb),
        .addr(mem_addr[16:0]),
        .wdat(mem_wdata),
        .rdat(ram_do)
    );
    
    // GPIO
    always @(posedge clk_24)
        if(gpo_sel)
        begin
            if(mem_wstrb[0])
                gp_out[7:0] <= mem_wdata[7:0];
            if(mem_wstrb[1])
                gp_out[15:8] <= mem_wdata[15:8];
            if(mem_wstrb[2])
                gp_out[23:16] <= mem_wdata[23:16];
            if(mem_wstrb[3])
                gp_out[31:24] <= mem_wdata[31:24];
        end
    
    // serial
    wire [7:0] ser_do;
    acia acia_inst(
        .clk(clk_24),			// system clock
        .rst(reset),			// system reset
        .cs(ser_sel),			// chip select
        .we(mem_wstrb[0]),		// write enable
        .rs(mem_addr[2]),		// address
        .rx(RX),				// serial receive
        .din(mem_wdata[7:0]),	// data bus input
        .dout(ser_do),			// data bus output
        .tx(TX),				// serial transmit
        .irq()					// interrupt request
    );
    
    // 256B wishbone bus master and SB IP cores @ F100-F1FF
    wire [7:0] wbb_do;
    wire wbb_rdy;
    wb_bus wbb_inst(
        .clk(clk_24),			// system clock
        .rst(reset),			// system reset
        .cs(wbb_sel),			// chip select
        .we(mem_wstrb[0]),		// write enable
        .addr(mem_addr[9:2]),	// address
        .din(mem_wdata[7:0]),	// data bus input
        .dout(wbb_do),			// data bus output
        .rdy(wbb_rdy),			// bus ready
        .spi0_mosi(spi0_mosi),	// spi core 0 mosi
        .spi0_miso(spi0_miso),	// spi core 0 miso
        .spi0_sclk(spi0_sclk),	// spi core 0 sclk
        .spi0_cs0(spi0_cs0),	// spi core 0 cs
        .spi1_mosi(spi1_mosi),	// spi core 1 mosi
        .spi1_miso(spi1_miso),	// spi core 1 miso
        .spi1_sclk(spi1_sclk),	// spi core 1 sclk
        .spi1_cs0(spi1_cs0),	// spi core 1 cs
        .i2c0_sda(i2c0_sda),	// i2c core 0 data
        .i2c0_scl(i2c0_scl)		// i2c core 0 clk
    );
    
    // resettable clock counter
    reg [31:0] cnt;
    always @(posedge clk_24)
        if(cnt_sel & |mem_wstrb)
        begin
            if(mem_wstrb[0])
                cnt[7:0] <= mem_wdata[7:0];
            if(mem_wstrb[1])
                cnt[15:8] <= mem_wdata[15:8];
            if(mem_wstrb[2])
                cnt[23:16] <= mem_wdata[23:16];
            if(mem_wstrb[3])
                cnt[31:24] <= mem_wdata[31:24];
        end
        else
            cnt <= cnt + 32'd1;
    
    // read mux
    always @(*)
        casex({cnt_sel,wbb_sel,ser_sel,gpo_sel,ram_sel,rom_sel})
            6'b000001: mem_rdata = rom_do;
            6'b00001x: mem_rdata = ram_do;
            6'b0001xx: mem_rdata = gp_out;
            6'b001xxx: mem_rdata = {{24{1'b0}},ser_do};
            6'b01xxxx: mem_rdata = {{24{1'b0}},wbb_do};
            6'b1xxxxx: mem_rdata = cnt;
            default: mem_rdata = 32'd0;
        endcase
    
    // ready flag
    reg mem_rdy;
    always @(posedge clk_24)
        if(reset)
            mem_rdy <= 1'b0;
        else
            mem_rdy <= (cnt_sel|ser_sel|gpo_sel|ram_sel|rom_sel) & ~mem_rdy;
    assign mem_ready = wbb_rdy | mem_rdy;
endmodule
