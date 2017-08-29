//-----------------------------------------
//            Program Counter
//-----------------------------------------
module PC(	
	
	//input definitions
	input clk,
	input reset, /*PC initial*/
	input taken_i,
	input jump_i,
	input branchStall_i,
	input loadStall_i,
	input syscallFlag_i,

	input [31:0] pcInit_i,
	input [31:0] shiftAddResult_i, /*candidates for nextInstrAddr*/
	input [31:0] jumpAddr_i,	
	
	//output definitions
	output [31:0] pcOutput_o
	);

//interconnect definitions
wire[31:0] pcInput;
wire[31:0] nextInstrAddr;
reg [31:0] pcOutput;

//combinational logic: calculate nextInstrAddr
assign pcInput = (taken_i)?shiftAddResult_i:(pcOutput_o+4);
assign nextInstrAddr = (jump_i)?jumpAddr_i:pcInput;
assign pcOutput_o = pcOutput;

//sequential logic: update PC
always @ (posedge clk)
begin 
    
		if(reset) /*PC initial*/
			pcOutput <= pcInit_i;

		else /*conditions that need to stall fetch*/
		if(!branchStall_i && !loadStall_i && !syscallFlag_i)
      pcOutput <= nextInstrAddr;
end
endmodule