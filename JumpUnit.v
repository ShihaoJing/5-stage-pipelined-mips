//-----------------------------------------
//           JumpUnit Module
//-----------------------------------------
module JumpUnit(
	//input definitions
        input clk,
	input jumpReg_i,
	input [25:0] target_i,
	input [31:0] readData1_i,
	input [31:0] pcOutput_i,
	//output definitions
	output[31:0] jumpAddr_o
	);

reg [31:0] temp;
reg [31:0] jumpAddr_o;
reg [31:0] pcplus4;
assign temp = 0;
assign temp[27:2] = target_i;
assign pcplus4 = pcOutput_i+4;

always @ (*)
begin
	
	//temp = 0;
	//temp[27:2] = target_i;
	
	if( jumpReg_i == 1 ) /*addr is read directly in the instr*/
		jumpAddr_o = {pcplus4[31:28],temp[27:0]};
	else 
		jumpAddr_o = readData1_i;/*addr is read from register*/

end

endmodule