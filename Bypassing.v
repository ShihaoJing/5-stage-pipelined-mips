module Bypassing(

	//input definitions
	input clk,
	input loadStallFlag_i,
	input regWriteEX_i,
	input regWriteMEM_i,
	input memReadEX_i,
	input memReadMEM_i,
	input [4:0] readRegRsID_i,
	input [4:0] readRegRtID_i,
	input [4:0] writeRegEX_i,
	input [4:0] writeRegMEM_i,
	input [31:0] aluResultEX_i,/*candidates for bypassing*/
	input [31:0] aluResultMEM_i,
	input [31:0] dataInputMEM_i,

	//output definitions
	output bypassingControl1_o, 
	output bypassingControl2_o,
	output [31:0] bypassingResult1_o,
	output [31:0] bypassingResult2_o,
	output loadHazard_o
	
);

always @ (*)
begin
//bypassing for EX hazard
if((readRegRsID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0)  
	begin
		bypassingControl1_o = 1; /*use bypassingResult instead of readData1*/
		bypassingResult1_o = aluResultEX_i;
	end
else 
        begin
        bypassingControl1_o = 0;
        end

if((readRegRtID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0)
	begin
		bypassingControl2_o = 1; /*use bypassingResult instead of readData2*/
		bypassingResult2_o = aluResultEX_i;
	end
else 
        begin
        bypassingControl2_o = 0;
        end

//bypassing for MEM hazard
if((readRegRsID_i==writeRegMEM_i) && regWriteMEM_i && writeRegMEM_i!=0 && !loadStallFlag_i)
	if(!((readRegRsID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0))/*if not EX hazard*/
	begin
		bypassingControl1_o = 1;
	if(memReadMEM_i) /*if a load instruction does not WB*/
		bypassingResult1_o = dataInputMEM_i;
	else 
		bypassingResult1_o = aluResultMEM_i;
	end
else 
        begin
        bypassingControl1_o = 1;
        end

if((readRegRtID_i==writeRegMEM_i) && regWriteMEM_i && writeRegMEM_i!=0 && !loadStallFlag_i)
	if(!((readRegRtID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0))/*if not EX hazard*/
	begin
		bypassingControl2_o = 1;
	if(memReadMEM_i)
		bypassingResult2_o = dataInputMEM_i;
	else
		bypassingResult2_o = aluResultMEM_i;
	end
else 
        begin
        bypassingControl2_o = 0;
        end



//some special load instruction need another one cycle stall
if((((readRegRsID_i==writeRegMEM_i) && regWriteMEM_i && writeRegMEM_i!=0 && loadStallFlag_i) && (!((readRegRsID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0)))||
	(((readRegRtID_i==writeRegMEM_i) && regWriteMEM_i && writeRegMEM_i!=0 && loadStallFlag_i) && (!((readRegRtID_i==writeRegEX_i) && regWriteEX_i && writeRegEX_i!=0))))
	loadHazard_o = 1;
else
	loadHazard_o = 0;

end

endmodule
