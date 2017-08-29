// -----------------------------------------
//              Top Module
//-----------------------------------------
module MIPS( 
	
	//input definitions
	input clk,
	input reset,/*initial pc*/
	input [31:0] pcInit_i, /*initial pc*/
	input [31:0] instrInput_i, /*fetched instr*/
	input [31:0] dataInput_i, /*data get from mem*/
 	input [31:0] r2Input_i, /*syscall input*/
	input syscallFlag_i, /*syscall input*/

  //output definitions
        output branchStall,
        output loadHazard,
	output memRead2_o, /*signals for mem read&write*/
	output memWrite2_o,
	output [31:0] tempInstrInput_o,
	output [31:0] instrAddr_o,
	output [31:0] dataAddr_o,
	output [31:0] dataOutput_o,
	output writebackFlag_o,
	output writebackFlag1_o,
	/* verilator lint_off UNOPTFLAT */
	output [31:0] r2Output_o /*syscall output*/,
        output [1:0] store_type2
	);
  
  //interconnect definitions
	//ID stage
	wire [31:0] instrAddr /*verilator public*/;
	wire [31:0] instrData /*verilator public*/;
	
	wire branchStall /*verilator public*/;
	wire branchFlush /*verilator public*/;
	wire branchFlag /*verilator public*/;
	wire loadHazard /*verilator public*/;
	wire specialloadFlag /*verilator public*/;
	wire syscall /*verilator public*/;
	wire jump /*verilator public*/;
	wire jumpReg /*verilator public*/;
	wire [31:0] jumpAddr /*verilator public*/;
	wire link /*verilator public*/;
	wire signOrZero_Flag /*verilator public*/;
	wire [31:0] signExtendedOutput /*verilator public*/;
	wire aluSrc /*verilator public*/;
	wire regDst /*verilator public*/;
	wire branch /*verilator public*/;
	
	wire [31:0] readData1 /*verilator public*/;
	wire [31:0] readData2 /*verilator public*/;
	wire [4:0] writeReg /*verilator public*/;
	wire memRead /*verilator public*/;
	wire memWrite /*verilator public*/;
	wire memtoReg /*verilator public*/;
	wire regWrite /*verilator public*/; 
        wire [1:0] store_type /*verilator public*/;	

	//EX stage
	wire [31:0] bypassingResult1 /*verilator public*/;
	wire [31:0] bypassingResult2 /*verilator public*/;
	wire bypassingControl1 /*verilator public*/;
	wire bypassingControl2 /*verilator public*/;
	wire jumpReg1 /*verilator public*/;
	wire syscallFlag1 /*verilator public*/;
	wire taken /*verilator public*/;
	wire [31:0] readData11 /*verilator public*/;
	wire [31:0] readData21 /*verilator public*/;
	wire [31:0] instrData1 /*verilator public*/;
	wire [31:0] instrAddr1 /*verilator public*/;
	wire [31:0] shiftAddResult /*verilator public*/;
	wire [31:0] signExtendedOutput1 /*verilator public*/;
	wire aluSrc1 /*verilator public*/;
	wire branch1 /*verilator public*/;
	wire jump1 /*verilator public*/;

	wire [5:0] aluCtrl /*verilator public*/;
	wire [31:0] dataAddr /*verilator public*/;
	wire [4:0] writeReg1 /*verilator public*/;
	wire memRead1 /*verilator public*/;
	wire memWrite1 /*verilator public*/;
	wire memtoReg1 /*verilator public*/;
	wire regWrite1 /*verilator public*/;
	wire link1 /*verilator public*/;

	wire [31:0] high /*verilator public*/;
	wire [31:0] low /*verilator public*/;
	wire  enableComments /*verilator public*/;
        wire [1:0] store_type1 /*verilator public*/;

	//MEM stage
	wire syscallFlag2 /*verilator public*/;
	wire [5:0] aluCtrl2 /*verilator public*/;
	wire [4:0] writeReg2 /*verilator public*/;
	wire memWrite2 /*verilator public*/;
	wire memtoReg2 /*verilator public*/;
	wire regWrite2 /*verilator public*/;
	wire link2 /*verilator public*/;
        wire [1:0] store_type2 /*verilator public*/;

	//WB stage
	wire syscallFlag3 /*verilator public*/;
	wire [31:0] writeData /*verilator public*/;
	wire [31:0] r2Output_o /*verilator public*/;
	wire [31:0] aluData /*verilator public*/;
	wire [31:0] dataInput /*verilator public*/;
	wire [5:0] aluCtrl3 /*verilator public*/;
	wire [4:0] writeReg3 /*verilator public*/;
	wire memtoReg3 /*verilator public*/;
	wire regWrite3 /*verilator public*/;
	wire link3 /*verilator public*/;

PC programCounter(
	.clk(clk),
	.reset(reset),
	.taken_i(taken),
	.jump_i(jump1),
	.branchStall_i(branchStall),
	.loadStall_i(loadHazard  && !branchFlag),
	.syscallFlag_i(syscallFlag_i),
	.pcInit_i(pcInit_i),
	.shiftAddResult_i(shiftAddResult),
	.jumpAddr_i(jumpAddr),
	.pcOutput_o(instrAddr_o)
	);


FetchDecode fetchDecode(
	.clk(clk),

	/*control signal*/
	.syscallFlag_i(syscallFlag_i),
	.branchFlush_i(branchFlush),
	.loadStall_i(loadHazard),

	/*pipeline register*/
	.pcOut_i(instrAddr_o),
	.instrIn_i(instrInput_i),
	
	.pcOut_o(instrAddr),
	.instrIn_o(instrData)
);

HandleStall handleStall(
	.clk(clk),
	.branch_i(branch),
	.memRead_i(memRead),
	.syscallFlag3_i(syscallFlag3),
	.loadHazard_i(loadHazard),
	.branchStall_o(branchStall),
	.branchFlush_o(branchFlush),
	.writebackFlag_o(writebackFlag_o),
	.branchFlag_o(branchFlag)
);

Control control(
	.instrInput_i(instrData),
	.regDst_o(regDst),
	.jumpReg_o(jumpReg),
	.jump_o(jump),
	.memRead_o(memRead),
	.memToReg_o(memtoReg),
	.memWrite_o(memWrite),
	.branch_o(branch),
	.aluSrc_o(aluSrc),
	.signOrZero_Flag_o(signOrZero_Flag),
	.regWrite_o(regWrite),
	.syscall_o(syscall),
	.link_o(link),
	.loadStallFlag_o(specialloadFlag),
        .store_type(store_type)
	);

RegFile RegisterFile(
	.clk(clk), /*just for debug*/ 

	/*Decode stage input*/
	.syscall_i(syscall),
	.r2Input_i(r2Input_i),
	.jump_i(jump),
	.link_i(link), 
	.linkAddr_i(instrAddr_o),
	.readReg1_i(instrData[25:21]),
	.readReg2_i(instrData[20:16]),
	.regDst_i(regDst), /*write to rt or rd*/
	.instrData_i(instrData),
	
	/*WriteBack stage input*/
	.link3_i(link3),
	.writeReg_i(writeReg3), /*write to rd or rt*/
	.memToReg_i(memtoReg3), /*write data from mem or alu*/
	.dataInput_i(dataInput), /*data from mem*/
	.dataAddr_i(aluData), /*data from alu*/
	.regWrite3_i(regWrite3),/*en*/
	.aluCtrl_i(aluCtrl3), 
	.lwxCtrl_i(aluData[1:0]),
	
	.writeReg_o(writeReg),
	.r2Output_o(r2Output_o),
	.readData1_o(readData1),
	.readData2_o(readData2),	
	.writeData_o(writeData)
	);

SignExtend signExtend(
	.signOrZero_Flag_i(signOrZero_Flag),
	.instrInput_i(instrData[15:0]),
	.signExtendedOutput_o(signExtendedOutput)
	);

DecodeExecute decodeExecute(
	.clk(clk),
	
	/*control signal*/
	.loadFlush_i(loadHazard),
	.bypassingControl1_i(bypassingControl1),
	.bypassingControl2_i(bypassingControl2),
	.bypassingResult1_i(bypassingResult1),
	.bypassingResult2_i(bypassingResult2),

	/*pipeline register*/
	.syscallFlag_i(syscallFlag_i),
	.instrAddr_i(instrAddr),
	.signExtendedOutput_i(signExtendedOutput),
	.readData1_i(readData1),
	.readData2_i(readData2),
	.branch_i(branch),
	.jump_i(jump),
	.aluSrc_i(aluSrc),
	.instrData_i(instrData),
	.jumpReg_i(jumpReg),

	.writeReg_i(writeReg),
	.memRead_i(memRead),
	.memWrite_i(memWrite),
	.memtoReg_i(memtoReg),
	.regWrite_i(regWrite),
	.link_i(link),
        .store_type_i(store_type),

	.syscallFlag1_o(syscallFlag1),
	.instrAddr1_o(instrAddr1),
	.signExtendedOutput1_o(signExtendedOutput1),
	.readData11_o(readData11),
	.readData21_o(readData21),
	.branch1_o(branch1),
	.jump1_o(jump1),
	.aluSrc1_o(aluSrc1),
	.instrData1_o(instrData1),
	.jumpReg_o(jumpReg1),

	.writeReg1_o(writeReg1),
	.memRead1_o(memRead1),
	.memWrite1_o(memWrite1),
	.memtoReg1_o(memtoReg1),
	.regWrite1_o(regWrite1),
	.link1_o(link1),
        .store_type1_o(store_type1)
);

Bypassing bypassing(
	.clk(clk),
	.loadStallFlag_i(specialloadFlag),
	.regWriteEX_i(regWrite1),
	.regWriteMEM_i(regWrite2),
	.memReadEX_i(memRead1),
	.memReadMEM_i(memRead2_o),
	.readRegRsID_i(instrData[25:21]),
	.readRegRtID_i(instrData[20:16]),
	.writeRegEX_i(writeReg1),
	.writeRegMEM_i(writeReg2),
	.aluResultEX_i(dataAddr),
	.aluResultMEM_i(dataAddr_o),
	.dataInputMEM_i(dataInput_i),
	.bypassingControl1_o(bypassingControl1),
	.bypassingControl2_o(bypassingControl2),
	.bypassingResult1_o(bypassingResult1),
	.bypassingResult2_o(bypassingResult2),
	.loadHazard_o(loadHazard)
);

JumpUnit jumpUnit(
        .clk(clk),
	.jumpReg_i(jumpReg1),
	.target_i(instrData1[25:0]),
	.readData1_i(readData11),
	.pcOutput_i(instrAddr1),
	.jumpAddr_o(jumpAddr)
	);

Add shiftAdder(
	.in1_i(instrAddr1+4),
	.in2_i((signExtendedOutput1<<2)), 
	.out_o(shiftAddResult)
	);

ALUControl aluControl(
	.clk(clk), /*just for debug*/
	.instrInput_i(instrData1),
	.aluCtrl_o(aluCtrl)
);

ALU alu(
	.clk(clk), /*just for debug*/
	.enableComments_i(enableComments),
	.tempBranch_i(branch1),
	.shiftAmount_i(instrData1[10:6]),
	.aluCtrl_i(aluCtrl),
	.a_i(readData11),
	.readData2_i(readData21),
	.signExtendedOutput_i(signExtendedOutput1),
	.aluSrc_i(aluSrc1),
	.high_o(high),
	.low_o(low),
	.zero_o(taken),
	.aluResult_o(dataAddr)
);

ExecuteMem executeMem(
	.clk(clk),
	.syscallFlag1_i(syscallFlag1),
	.instrData1_i(instrData1),
	.dataAddr1_i(dataAddr),
	.readData21_i(readData21),
	.aluCtrl1_i(aluCtrl),
	.writeReg1_i(writeReg1),
	.memRead1_i(memRead1),
	.memWrite1_i(memWrite1),
	.memtoReg1_i(memtoReg1),
	.regWrite1_i(regWrite1),	
	.link1_i(link1),
        .store_type1_i(store_type1),

	.syscallFlag2_o(syscallFlag2),
	.instrData1_o(tempInstrInput_o), /*for memWrite in c++*/
	.dataAddr2_o(dataAddr_o), /*mem addr*/
	.readData22_o(dataOutput_o),/*data write to mem*/
	.aluCtrl2_o(aluCtrl2),
	.writeReg2_o(writeReg2),
	.memRead2_o(memRead2_o), /*en*/
	.memWrite2_o(memWrite2_o), /*en*/
	.memtoReg2_o(memtoReg2),
	.regWrite2_o(regWrite2),
	.link2_o(link2),
        .store_type2_o(store_type2)
);

MemWriteback memWriteback(
	.clk(clk),
	.syscallFlag2_i(syscallFlag2),
	.dataInput_i(dataInput_i), /*data read from mem*/
	.dataOutput_i(dataAddr_o), /*data from alu result*/
	.aluCtrl2_i(aluCtrl2),
	.link2_i(link2),
	.writeReg2_i(writeReg2),
	.memtoReg2_i(memtoReg2),
	.regWrite2_i(regWrite2),
        
	
	.syscallFlag3_o(syscallFlag3),
	.dataInput1_o(dataInput),
	.dataOutput1_o(aluData),
	.aluCtrl3_o(aluCtrl3),
	.link3_o(link3),
	.writeReg3_o(writeReg3),
	.memtoReg3_o(memtoReg3),
	.regWrite3_o(regWrite3)
);


endmodule