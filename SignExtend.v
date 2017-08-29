//-----------------------------------------
//          Sign Extend Module
//-----------------------------------------
module SignExtend(
	input signOrZero_Flag_i,
	input [15:0] instrInput_i,
	output[31:0] signExtendedOutput_o
	);

reg [31:0] signExtendedOutput_o;

assign signExtendedOutput_o = (signOrZero_Flag_i)?{{16{instrInput_i[15]}},instrInput_i[15:0]}:{16'b0,instrInput_i[15:0]};

endmodule