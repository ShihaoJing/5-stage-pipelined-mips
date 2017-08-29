//-----------------------------------------
//            Control Module
//-----------------------------------------
module Control(
  //input definitions
  input [31:0] instrInput_i,
	//output definitionsl
  output regDst_o,
  output jumpReg_o,
  output jump_o,
  output memRead_o,
	output memToReg_o,
	output memWrite_o,
	output branch_o,
	output aluSrc_o,
	output signOrZero_Flag_o,
	output regWrite_o,
	output syscall_o,
	output link_o,
	output loadStallFlag_o,
        output [1:0] store_type
	);

wire [5:0] opcode;
wire [5:0] funct;
wire [4:0] format;
wire [4:0] rt;
wire [4:0] fd;
wire [11:0] packet;
wire [1:0] store_type;

assign opcode = instrInput_i[31:26];
assign funct = instrInput_i[5:0];
assign format = instrInput_i[25:21];
assign rt = instrInput_i[20:16];
assign fd = instrInput_i[10:6];
assign {link_o,regDst_o,jump_o,branch_o,memRead_o,memToReg_o,memWrite_o,
				aluSrc_o,regWrite_o,jumpReg_o,signOrZero_Flag_o,syscall_o} = packet;

always @ (*) 
begin  
case ( opcode ) 
6'b000000:begin //SPECIAL
			case ( funct )
			6'b000000:packet = 12'b010000001000;//SLL,NOP
			6'b000010:packet = 12'b010000001000;//SRL
			6'b000011:packet = 12'b010000001000;//SRA  
			6'b000100:packet = 12'b010000001000;//SLLV  
			6'b000110:packet = 12'b010000001000;//SRLV 
			6'b000111:packet = 12'b010000001000;//SRAV  
			6'b001000:packet = 12'b001100000000;//JR  
			6'b001001:packet = 12'b101100000000;//JALR
			6'b001100:packet = 12'b010000011001;//syscall_o*   
			6'b001101:packet = 12'b010000011000;//BREAK* 
			6'b010000:packet = 12'b010000001000;//MFHI 
			6'b010001:packet = 12'b000000000000;//MTHI   
			6'b010010:packet = 12'b010000001000;//MFLO
			6'b010011:packet = 12'b000000000000;//MTLO   
			6'b011000:packet = 12'b000000000000;//mult
			6'b011001:packet = 12'b000000000000;//multu
			6'b011010:packet = 12'b000000000000;//div 
			6'b011011:packet = 12'b000000000000;//divu 
			6'b100000:packet = 12'b010000001100;//add  
			6'b100001:packet = 12'b010000001100;//addu  
			6'b100010:packet = 12'b010000001100;//sub 
			6'b100011:packet = 12'b010000001100;//subu  
			6'b100100:packet = 12'b010000001100;//and   
			6'b100101:packet = 12'b010000001100;//or
			6'b100110:packet = 12'b010000001100;//Xor   
			6'b100111:packet = 12'b010000001100;//nor 
			6'b101010:packet = 12'b010000001100;//slt  
			6'b101011:packet = 12'b010000001000;//SLTU
			default: begin end
			endcase //case ( funct )
		end
6'b000001:begin   
			case ( rt )
			5'b00000:packet = 12'b000100000010;//BLTZ  
			5'b00001:packet = 12'b000100000010;//BGEZ  
			5'b10000:packet = 12'b100100000010;//BLTZAL 
			5'b10001:packet = 12'b100100000010;//BGEZAL
			default: begin end
			endcase
		end
6'b000010:packet = 12'b001100000100;//J   
6'b000011:packet = 12'b101100000100;//JAL 
6'b000100:packet = 12'b000100000010;//BEQ  
6'b000101:packet = 12'b000100000010;//BNE 
6'b000110:packet = 12'b000100000010;//BLEZ   
6'b000111:packet = 12'b000100000010;//BGTZ 
6'b001000:packet = 12'b000000011010;//ADDI  
6'b001001:packet = 12'b000000011010;//ADDIU   
6'b001010:packet = 12'b000000011010;//SLTI   
6'b001011:packet = 12'b000000011010;//SLTIU   
6'b001100:packet = 12'b000000011000;//ANDI   
6'b001101:packet = 12'b000000011000;//ORI    
6'b001110:packet = 12'b000000011000;//XorI  
6'b001111:packet = 12'b000000011000;//LUI   
6'b010001:begin //COP1
			case( format )
				5'b00000:packet = 12'b000000001010;//MFC1
				5'b00010:packet = 12'b000000001010;//CFC1
				5'b00100:packet = 12'b010000000010;//MTC1
				5'b00110:packet = 12'b010000000010;//CTC1
				5'b01000:begin
							case( instrInput_i[16] )
							1'b1:packet = 12'b000100000010;//BC1T
							1'b0:packet = 12'b000100000010;//BC1F
							endcase
						end
				5'b10000:begin 
							if( instrInput_i[7:4] == 4'b0011 )
								packet = 12'b000000000000;//fp c.cond
							else begin  
									case( funct )
									6'b000000:packet = 12'b010000000000;//fp add
									6'b000001:packet = 12'b010000000000;//fp sub
									6'b000010:packet = 12'b010000000000;//fp mul
									6'b000011:packet = 12'b010000000000;//fp div
									6'b000101:packet = 12'b010000000000;//fp abs
									6'b000110:packet = 12'b010000000010;//MOV.FMT
									6'b000111:packet = 12'b010000000000;//fp neg
									default: begin end 
									endcase // case ( funct )
								end
						end
				5'b10001:packet = 12'b010000000010;//CVT.S.FMT
				default: begin end
			endcase // case ( format )
		end
6'b100000:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LB 
6'b100001:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LH   
6'b100010:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LWL  
6'b100011:packet = 12'b000011011100;//LW  
6'b110000:packet = 12'b000011011100;//LWC0
6'b100100:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LBU  
6'b100101:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LHU  
6'b100110:begin packet = 12'b000011011100; loadStallFlag_o=1; end//LWR  
6'b101000:begin packet = 12'b000000110100; store_type=2; end//SB   
6'b101001:begin packet = 12'b000000110100; store_type=1; end//SH 
6'b101010:begin packet = 12'b000000110100; store_type=0; end//SWL   
6'b101011:begin packet = 12'b000000110100; store_type=0; end//SW 
6'b111000:begin packet = 12'b000000110100; store_type=0; end//SWC0
6'b101110:begin packet = 12'b000000110100; store_type=0; end//SWR
6'b110001:packet = 12'b000011010010;//LWC1
6'b111001:begin packet = 12'b000000110010; store_type=0; end//SWC1  
6'b010100:packet = 12'b000100000010;//BEQL
6'b010110:packet = 12'b000100000010;//BLEZL 
6'b010101:packet = 12'b000100000010;//BNEL
default:  begin end
endcase
end

endmodule