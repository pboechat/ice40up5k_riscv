/*
 * 32k 32-bit word single-port RAM (128KB)
 */

`default_nettype none

module spram_32kx32(
    input clk,
    input sel,
    input [3:0] we,
    input [16:0] addr,
    input [31:0] wdat,
    output [31:0] rdat
);
    wire [14:0] word_addr = addr[16:2];
    wire bank_sel = word_addr[14];
    wire [13:0] bank_addr = word_addr[13:0];

    // ---------------
    // Bank 0 (64 KB)
    // ---------------
    wire [31:0] rdat_b0;
    wire sel_b0 = sel && (bank_sel == 1'b0);

    SB_SPRAM256KA mem0_lo(
        .ADDRESS(bank_addr),
        .DATAIN(wdat[15:0]),
        .MASKWREN({we[1],we[1],we[0],we[0]}),
        .WREN(|we),
        .CHIPSELECT(sel_b0),
        .CLOCK(clk),
        .STANDBY(1'b0),
        .SLEEP(1'b0),
        .POWEROFF(1'b1),
        .DATAOUT(rdat_b0[15:0])
    );

    SB_SPRAM256KA mem0_hi(
        .ADDRESS(bank_addr),
        .DATAIN(wdat[31:16]),
        .MASKWREN({we[3],we[3],we[2],we[2]}),
        .WREN(|we),
        .CHIPSELECT(sel_b0),
        .CLOCK(clk),
        .STANDBY(1'b0),
        .SLEEP(1'b0),
        .POWEROFF(1'b1),
        .DATAOUT(rdat_b0[31:16])
    );

    // ---------------
    // Bank 1 (64 KB)
    // ---------------
    wire [31:0] rdat_b1;
    wire sel_b1 = sel && (bank_sel == 1'b1);

    SB_SPRAM256KA mem1_lo(
        .ADDRESS(bank_addr),
        .DATAIN(wdat[15:0]),
        .MASKWREN({we[1],we[1],we[0],we[0]}),
        .WREN(|we),
        .CHIPSELECT(sel_b1),
        .CLOCK(clk),
        .STANDBY(1'b0),
        .SLEEP(1'b0),
        .POWEROFF(1'b1),
        .DATAOUT(rdat_b1[15:0])
    );

    SB_SPRAM256KA mem1_hi(
        .ADDRESS(bank_addr),
        .DATAIN(wdat[31:16]),
        .MASKWREN({we[3],we[3],we[2],we[2]}),
        .WREN(|we),
        .CHIPSELECT(sel_b1),
        .CLOCK(clk),
        .STANDBY(1'b0),
        .SLEEP(1'b0),
        .POWEROFF(1'b1),
        .DATAOUT(rdat_b1[31:16])
    );

    assign rdat = bank_sel ? rdat_b1 : rdat_b0;
endmodule
