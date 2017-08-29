module DecodeExecute(

		input clk,
		input loadFlush_i,
		input bypassingControl1_i,
		input bypassingControl2_i,
		input [31:0] bypassingResult1_i,
		input [31:0] bypassingResult2_i,
		input syscallFlag_i,
		input [31:0] instrAddr_i,
		input [31:0] signExtendedOutput_i,
		input [31:0] readData1_i,
		input [31:0] readData2_i,
		input branch_i,
		input jump_i,
		input aluSrc_i,
		input [31:0] instrData_i,
		input jumpReg_i,
		input [4:0] writeReg_i,
		input memRead_i,
		input memWrite_i,
		input memtoReg_i,
		input regWrite_i,
		input link_i,
                input [1:0] store_type_i,

		output reg syscallFlag1_o,
		output reg [31:0] instrAddr1_o,
		output reg [31:0] signExtendedOutput1_o,
		output reg [31:0] readData11_o,
		output reg [31:0] readData21_o,
		output reg branch1_o,
		output reg jump1_o,
		output reg aluSrc1_o,
		output reg [31:0] instrData1_o,
		output reg jumpReg_o,
		output reg [4:0] writeReg1_o,
		output reg memRead1_o,
		output reg memWrite1_o,
		output reg memtoReg1_o,
		output reg regWrite1_o,
		output reg link1_o,
                output reg [1:0] store_type1_o
);

always @ (posedge clk)
begin 
if(!loadFlush_i)
	begin
		syscallFlag1_o <= syscallFlag_i;
		instrAddr1_o <= instrAddr_i;
		signExtendedOutput1_o <= signExtendedOutput_i;
		branch1_o <= branch_i;
		jump1_o <= jump_i;
		aluSrc1_o <= aluSrc_i;
		instrData1_o <= instrData_i;
		jumpReg_o <= jumpReg_i;

		writeReg1_o <= writeReg_i;
		memRead1_o <= memRead_i;
  	        memWrite1_o <= memWrite_i;
  	        memtoReg1_o <= memtoReg_i;
		regWrite1_o <= regWrite_i;
		link1_o <= link_i;
                store_type1_o <= store_type_i;

		/*bypassing*/
		if(bypassingControl1_i)
			begin
			readData11_o <= bypassingResult1_i;
			end
		else
			readData11_o <= readData1_i;

		if(bypassingControl2_i)
			begin
			readData21_o <= bypassingResult2_i;
			end
		else
			readData21_o <= readData2_i;
	end
else
	begin
		syscallFlag1_o <= 0;
		instrAddr1_o <= 0;
		signExtendedOutput1_o <= 0;
		branch1_o <= 0;
		jump1_o <= 0;
		aluSrc1_o <= 0;
		instrData1_o <= 0;
		jumpReg_o <= 0;

		writeReg1_o <= 0;
		memRead1_o <= 0;
  	        memWrite1_o <= 0;
  	        memtoReg1_o <= 0;
		regWrite1_o <= 0;
		link1_o <= 0;
                store_type1_o <= 0;

	end
end

endmodule