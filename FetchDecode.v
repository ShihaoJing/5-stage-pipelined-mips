module FetchDecode(	
	
	//input definitions
	input clk,
	input syscallFlag_i,
	input branchFlush_i,
	input loadStall_i,
	input [31:0] pcOut_i, /*instr addr*/
	input [31:0] instrIn_i, /*instr data*/

	//output definitions
	output reg [31:0] pcOut_o,
	output reg [31:0] instrIn_o
	);

always @(posedge clk)
begin
	
	if(branchFlush_i || syscallFlag_i)
		begin
			pcOut_o <= 0;
			instrIn_o <= 0;
		end
	
	else if(!syscallFlag_i && !loadStall_i)
		begin	
			pcOut_o <= pcOut_i;
			instrIn_o <= instrIn_i;
		end	

end

endmodule