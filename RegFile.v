//-----------------------------------------
//         Register File Module
//-----------------------------------------
module RegFile ( 
	
	//input definitions
	input clk,
	
	/*Decode stage input*/
	input syscall_i, /*instr is syscall*/
	input [31:0] r2Input_i, /*syscall*/
	input jump_i,
	input link_i, 
	input [31:0] linkAddr_i, /*current pc*/
	input [4:0] readReg1_i, /*read address1*/
	input [4:0] readReg2_i, /*read address2*/
	input regDst_i, /*write to rd or rt*/
	input [31:0] instrData_i,
	
	/*WriteBack stage input*/
	input link3_i,
	input [4:0] writeReg_i, /*write address*/
	input memToReg_i, /*indicate load or not*/
	input [31:0] dataInput_i, /*write data-load-directly input*/
	input [31:0] dataAddr_i, /*write data-no load-data resloved at EX stage*/
	input regWrite3_i, /*write enable*/
	input [5:0] aluCtrl_i,
	input [1:0] lwxCtrl_i, /*help to deal with load instruction write*/

	//output definitions
	output [4:0] writeReg_o,
	output [31:0] r2Output_o, /*syscall*/
	output [31:0] readData1_o, 
	output [31:0] readData2_o,
	output [31:0] writeData_o
	);

//interconnect definitions
/* verilator lint_off UNOPTFLAT */
reg [31:0] WriteData;
reg [31:0] Reg [0:31];

wire [31:0] templink; 
wire linkFlag;

//combinational logic 
assign writeReg_o = (regDst_i)?instrData_i[15:11]:instrData_i[20:16]; /*write to rd or rt*/
assign WriteData = (!memToReg_i)?dataAddr_i:dataInput_i; /*write data from alu or mem*/
assign writeData_o = WriteData;
assign r2Output_o = Reg[2];

/*Some syscalls require a return register*/
always @(*) 
begin
	if ((syscall_i) && (r2Output_o[4:0]!=14) && (r2Output_o[4:0]!=19)&& (r2Output_o!=1) 
			&& (r2Output_o!=2) && (r2Output_o!=5) && (r2Output_o!=6)) 
		Reg[2] = r2Input_i;
	else 
		r2Output_o = Reg[2];
end

/*read register when no data hazard exits*/
always @(*)
begin
	if((readReg1_i==5'b11111) && jump_i && linkFlag)
		readData1_o = templink;
	else
		readData1_o = Reg[readReg1_i];
		
		readData2_o = Reg[readReg2_i];
end

/*write register*/
always @(*)
begin
	case( aluCtrl_i )
		6'b101101:begin //LWX
								case( lwxCtrl_i )
									0:  Reg[writeReg_i] = WriteData;
									1:  Reg[writeReg_i][31:8] = WriteData[23:0];
									2:  Reg[writeReg_i][31:16] = WriteData[15:0];
									3:  Reg[writeReg_i][31:24] = WriteData[7:0];
								endcase
							end
		6'b101110:begin //LWX
								case( lwxCtrl_i )
									0:  Reg[writeReg_i][7:0] = WriteData[31:24];
									1:  Reg[writeReg_i][15:0] = WriteData[31:16];
									2:  Reg[writeReg_i][23:0] = WriteData[31:8];
									3:  Reg[writeReg_i] = WriteData;
								endcase
							end
	  6'b100001:begin //LB
								case( dataAddr_i[1:0] )
									3:Reg[writeReg_i] = {{24{WriteData[7]}},WriteData[7:0]};
									2:Reg[writeReg_i] = {{24{WriteData[15]}},WriteData[15:8]};
									1:Reg[writeReg_i] = {{24{WriteData[23]}},WriteData[23:16]};
									0:Reg[writeReg_i] = {{24{WriteData[31]}},WriteData[31:24]};
									default:begin end 
								endcase
							end
	 	6'b101011:begin //LH
								case( dataAddr_i[1:0] )
									0:Reg[writeReg_i] = {{16{WriteData[15]}},WriteData[15:0]};
									2:Reg[writeReg_i] = {{16{WriteData[31]}},WriteData[31:16]};
									default:begin end
								endcase
							end
		6'b101010:begin //LBU
								case( dataAddr_i[1:0] )
									0: Reg[writeReg_i] = {24'b0,WriteData[31:24]};
									1: Reg[writeReg_i] = {24'b0,WriteData[23:16]};
									2: Reg[writeReg_i] = {24'b0,WriteData[15:8]};
									3: Reg[writeReg_i] = {24'b0,WriteData[7:0]};
									default: begin end 
								endcase
							end
		6'b101100:begin //LHU
								case(dataAddr_i%4)
									0:Reg[writeReg_i] = {16'b0,WriteData[15:0]};
									2:Reg[writeReg_i] = {16'b0,WriteData[31:16]};
									default: begin end
								endcase
							end
	
		default:begin
							if ((regWrite3_i)&&(writeReg_i!=5'b00000)&&(aluCtrl_i!=6'b110100)) 
								begin 
									Reg[writeReg_i] = WriteData;
								end
						end
		endcase
end

/*put return address in reg31*/
always @ (*)
begin
	if (link_i) 
		begin	
			templink = linkAddr_i+4; /*record link address at decode stage*/		
			linkFlag = 1;
		end

	if(link3_i)
		begin
			Reg[31] = templink;						
			linkFlag = 0;
		end

end 		
		

endmodule