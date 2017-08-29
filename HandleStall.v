module HandleStall(
	
	//input definations
	input clk,
	input branch_i,
	input memRead_i,
	input syscallFlag3_i,
	input loadHazard_i,

	//output definations
	output branchStall_o,
	output branchFlush_o,
	output writebackFlag_o,
	output branchFlag_o
);

//combinational logic
/*stall one cycle when it's branch instruction*/
assign branchStall_o = branch_i;

/*flush IF/ID when it's branch instruction*/
assign branchFlush_o = (branchFlag_o && !loadHazard_i)? 1:0;

//sequential logic
always @ (posedge clk)
begin
	
	if(branchStall_o)
                        begin
			branchFlag_o <= 1;
                        end
	if(branchFlag_o)
                        begin
			branchFlag_o <= 0;
                        end

/*indicate WB, feedback signal to c++*/
	if(syscallFlag3_i)
		writebackFlag_o <= 1;
	else
		writebackFlag_o <= 0;
end

endmodule