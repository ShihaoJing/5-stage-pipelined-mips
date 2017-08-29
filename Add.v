//-----------------------------------------
//              Add Module
//-----------------------------------------
module Add(
  input [31:0] in1_i,
	input [31:0] in2_i,
	output[31:0] out_o
	);
 
assign	out_o=in1_i+in2_i;

endmodule