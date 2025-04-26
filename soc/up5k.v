/*
 * up5k top level
 */

`default_nettype none

module up5k(
    // 12MHz clock osc
    input clk_12,
    input resetn_sw,
    // serial
    inout RX, 
    inout TX,
    // SPI0 port hooked to cfg flash
    inout spi0_mosi,
    inout spi0_miso,
    inout spi0_sclk,
    inout spi0_cs0,
    // SPI1 port on PMOD
    inout spi1_mosi,
    inout spi1_miso,
    inout spi1_sclk,
    inout spi1_cs0,
    // I2C0 port on PMOD
    inout i2c0_sda,
    inout i2c0_scl,
    // GP out for LCD
    output lcd_nrst, 
    output lcd_dc,
    // diagnostic
    output  d1, d2,
    // LED - via drivers
    output  RGB0, RGB1, RGB2
);
    // fin=12, fout=24 (12*(4/2))
    wire clk, pll_lock;
    SB_PLL40_PAD #(
        .DIVR(4'b0000),
        .DIVF(7'b0000011),
        .DIVQ(3'b001),
        .FILTER_RANGE(3'b001),
        .FEEDBACK_PATH("SIMPLE"),
        .DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
        .FDA_FEEDBACK(4'b0000),
        .DELAY_ADJUSTMENT_MODE_RELATIVE("FIXED"),
        .FDA_RELATIVE(4'b0000),
        .SHIFTREG_DIV_MODE(2'b00),
        .PLLOUT_SELECT("GENCLK"),
        .ENABLE_ICEGATE(1'b0)
    ) pll_inst(
        .PACKAGEPIN(clk_12),
        .PLLOUTCORE(clk),
        .PLLOUTGLOBAL(),
        .EXTFEEDBACK(),
        .DYNAMICDELAY(8'h00),
        .RESETB(1'b1),
        .BYPASS(1'b0),
        .LATCHINPUTVALUE(),
        .LOCK(pll_lock),
        .SDI(),
        .SDO(),
        .SCLK()
    );
    
    // reset generator waits > 10us afer PLL lock
    reg [7:0] pll_reset_cnt;
    reg pll_reset;    
    always @(posedge clk)
    begin
        if(!pll_lock)
        begin
            pll_reset_cnt <= 8'h00;
            pll_reset <= 1'b1;
        end
        else
        begin
            if(pll_reset_cnt != 8'hff)
            begin
                pll_reset_cnt <= pll_reset_cnt + 8'h01;
                pll_reset <= 1'b1;
            end
            else
                pll_reset <= 1'b0;
        end
    end

    wire reset;

    assign reset = ~resetn_sw | pll_reset;
    
    // soc
    wire [3:0] tst;
    wire [31:0] gpio_o;
    wire raw_rx, raw_tx;
    soc soc_inst(
        .clk_24(clk),
        .reset(reset),
        .RX(raw_rx),
        .TX(raw_tx),
        .spi0_mosi(spi0_mosi),
        .spi0_miso(spi0_miso),
        .spi0_sclk(spi0_sclk),
        .spi0_cs0(spi0_cs0),
        .spi1_mosi(spi1_mosi),
        .spi1_miso(spi1_miso),
        .spi1_sclk(spi1_sclk),
        .spi1_cs0(spi1_cs0),
        .i2c0_sda(i2c0_sda),
        .i2c0_scl(i2c0_scl),
        .gp_out(gpio_o)
    );
    
    // serial I/O w/ pullup on RX
    SB_IO #(
        .PIN_TYPE(6'b101001),
        .PULLUP(1'b1),
        .NEG_TRIGGER(1'b0),
        .IO_STANDARD("SB_LVCMOS")
    ) rx_io(
        .PACKAGE_PIN(RX),
        .LATCH_INPUT_VALUE(1'b0),
        .CLOCK_ENABLE(1'b0),
        .INPUT_CLK(1'b0),
        .OUTPUT_CLK(1'b0),
        .OUTPUT_ENABLE(1'b0),
        .D_OUT_0(1'b0),
        .D_OUT_1(1'b0),
        .D_IN_0(raw_rx),
        .D_IN_1()
    );
    SB_IO #(
        .PIN_TYPE(6'b101001),
        .PULLUP(1'b0),
        .NEG_TRIGGER(1'b0),
        .IO_STANDARD("SB_LVCMOS")
    ) tx_io(
        .PACKAGE_PIN(TX),
        .LATCH_INPUT_VALUE(1'b0),
        .CLOCK_ENABLE(1'b0),
        .INPUT_CLK(1'b0),
        .OUTPUT_CLK(1'b0),
        .OUTPUT_ENABLE(1'b1),
        .D_OUT_0(raw_tx),
        .D_OUT_1(1'b0),
        .D_IN_0(),
        .D_IN_1()
    );
    
    assign d1 = gpio_o[0];
    assign d2 = raw_tx;
    
    // RGB LED driver IP core
    SB_RGBA_DRV #(
        .CURRENT_MODE("0b1"),
        .RGB0_CURRENT("0b000001"),
        .RGB1_CURRENT("0b000001"),
        .RGB2_CURRENT("0b000011")
    ) rgba_drv_inst(
        .CURREN(1'b1),
        .RGBLEDEN(1'b1),
        .RGB0PWM(gpio_o[17]),
        .RGB1PWM(gpio_o[18]),
        .RGB2PWM(gpio_o[19]),
        .RGB0(RGB0),
        .RGB1(RGB1),
        .RGB2(RGB2)
    );
    
    // LCD control lines
    assign lcd_nrst = gpio_o[31];
    assign lcd_dc = gpio_o[30];
endmodule
