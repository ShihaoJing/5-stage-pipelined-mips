module ExecuteMem(
	
	input clk,
	input syscallFlag1_i,
	input [31:0] instrData1_i,
	input [31:0] dataAddr1_i,
	input [31:0] readData21_i,
	input [5:0] aluCtrl1_i,
	input [4:0] writeReg1_i,
	input memRead1_i,
	input memWrite1_i,
	input memtoReg1_i,
	input regWrite1_i,
	input link1_i, 
        input [1:0] store_type1_i,

	output reg syscallFlag2_o,
	output reg [31:0] instrData1_o,
	output reg [31:0] dataAddr2_o,
	output reg [31:0] readData22_o,
	output reg [5:0] aluCtrl2_o,
	output reg [4:0] writeReg2_o,
	output reg memRead2_o,
	output reg memWrite2_o,
	output reg memtoReg2_o,
	output reg regWrite2_o,
	output reg link2_o,
        output reg [1:0] store_type2_o
);


always @ (posedge clk)
begin 
	syscallFlag2_o <= syscallFlag1_i;
	instrData1_o <= instrData1_i;
	dataAddr2_o <= dataAddr1_i;
	readData22_o <= readData21_i;
	aluCtrl2_o <= aluCtrl1_i;
	writeReg2_o <= writeReg1_i;
	memRead2_o <= memRead1_i;
	memWrite2_o <= memWrite1_i;
	memtoReg2_o <= memtoReg1_i;
	regWrite2_o <= regWrite1_i;
	link2_o <= link1_i;
        store_type2_o <= store_type1_i;
end

endmodule