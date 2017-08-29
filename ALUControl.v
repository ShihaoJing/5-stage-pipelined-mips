//-----------------------------------------
//           ALUControl Module
//-----------------------------------------
module ALUControl( 
	input clk,
	input [31:0] instrInput_i,
	output [5:0] aluCtrl_o
	);

wire [5:0] opcode;
wire [5:0] funct;
wire [4:0] format;
wire [4:0] rt;
wire [4:0] fd;
reg [5:0] aluCtrl_o;

assign opcode = instrInput_i[31:26];
assign format = instrInput_i[25:21];
assign rt = instrInput_i[20:16];
assign funct = instrInput_i[5:0];
assign fd = instrInput_i[10:6];
        
always @ (*) 
begin // @ ( instrInput_i ) begin
	case ( opcode )
			6'b000000:begin //SPECIAL
									case ( funct )
										6'b000000:aluCtrl_o=6'b010011;//nop,sll
										6'b000010:aluCtrl_o=6'b011011;//SRL
										6'b000011:aluCtrl_o=6'b011001;//SRA                                                            
										6'b000100:aluCtrl_o=6'b010100;//SLLV
										6'b000110:aluCtrl_o=6'b011100;//SRLV
										6'b000111:aluCtrl_o=6'b011010;//SRAV
										6'b001000:aluCtrl_o=6'b111110;//JR
										6'b001001:aluCtrl_o=6'b111110;//JALR
										6'b001100:aluCtrl_o=6'b010011;//SYSCALL*
										6'b001101:aluCtrl_o=6'b010011;//BREAK*
										6'b010000:aluCtrl_o=6'b001001;//MFHI
										6'b010001:aluCtrl_o=6'b001011;//MTHI
										6'b010010:aluCtrl_o=6'b001010;//MFLO
										6'b010011:aluCtrl_o=6'b001100;//MTLO
										6'b011000:aluCtrl_o=6'b001101;//mult
										6'b011001:aluCtrl_o=6'b001101;//multu
										6'b011010:aluCtrl_o=6'b000101;//div
										6'b011011:aluCtrl_o=6'b000110;//divu
										6'b100000:aluCtrl_o=6'b000000;//add
										6'b100001:aluCtrl_o=6'b110111;//addu
										6'b100010:aluCtrl_o=6'b011101;//sub
										6'b100011:aluCtrl_o=6'b011110;//subu
										6'b100100:aluCtrl_o=6'b000100;//and
										6'b100101:aluCtrl_o=6'b010000;//or
										6'b100110:aluCtrl_o=6'b011111;//xor
										6'b100111:aluCtrl_o=6'b001111;//nor
										6'b101010:aluCtrl_o=6'b010101;//slt
										6'b101011:aluCtrl_o=6'b111111;//SLTU
										default:; //$display("Not an instrInput_i 0!");
									endcase // case ( funct )
								end // begin SPECIAL
			
			6'b000001:begin //REGIMM
									case ( rt )
										5'b00000:aluCtrl_o=6'b100111;//BLTZ
										5'b00001:aluCtrl_o=6'b100011;//BGEZ
										5'b10000:aluCtrl_o=6'b100111;//BLTZAL
										5'b10001:aluCtrl_o=6'b100011;//BGEZAL
									  default:; //$display("Not an instrInput_i 6!");
									endcase
		            end
			
			6'b000010:aluCtrl_o=6'b001110;//J
			6'b000011:aluCtrl_o=6'b001110;//JAL
			6'b000100:aluCtrl_o=6'b100010;//BEQ
			6'b000101:aluCtrl_o=6'b101001;//BNE
			6'b000110:aluCtrl_o=6'b100110;//BLEZ
			6'b000111:aluCtrl_o=6'b100101;//BGTZ
			6'b001000:aluCtrl_o=6'b000001;//ADDI 
			6'b001001:aluCtrl_o=6'b000010;//ADDIU
			6'b001010:aluCtrl_o=6'b010101;//SLTI
			6'b001011:aluCtrl_o=6'b010101;//SLTIU
			6'b001100:aluCtrl_o=6'b000100;//ANDI
			6'b001101:aluCtrl_o=6'b010000;//ORI
			6'b001110:aluCtrl_o=6'b100000;//XORI
			6'b001111:aluCtrl_o=6'b001000;//LUI
			
			6'b010001:begin //COP1
									case( format )
										5'b00000:aluCtrl_o=6'b011100;//MFC1
										5'b00010:aluCtrl_o=6'b011010;//CFC1
										5'b00100:aluCtrl_o=6'b111000;//MTC1
										5'b00110:aluCtrl_o=6'b110100;//CTC1
										5'b01000: begin
																case( instrInput_i[16] )
																	1'b1:aluCtrl_o=6'b011101;//BC1T
																	1'b0:aluCtrl_o=6'b001111;//BC1F
																	default: ;//$display("Not an instrInput_i 11!");
																endcase
														 	end
										5'b10000:	begin
																if( instrInput_i[7:4] == 4'b0011 ) 
														   	 	aluCtrl_o=6'b011111;//fp c.cond*
														 		else 
														  	begin
															 		case( funct )
																		6'b000000:aluCtrl_o=6'b011011;//fp add
																		6'b000001:aluCtrl_o=6'b000000;//fp sub
																		6'b000010:aluCtrl_o=6'b001101;//fp mul
																		6'b000011:aluCtrl_o=6'b000101;//fp div
																		6'b000101:aluCtrl_o=6'b110111;//fp abs
																		6'b000110:aluCtrl_o=6'b000100;//MOV.FMT
																		6'b000111:aluCtrl_o=6'b010000;//fp neg
																		default:; //$display("Not an instrInput_i 31!");
																	endcase // case ( funct )
																end
														 	end // case: 5'b10000
										5'b10001:aluCtrl_o=6'b001000;//fp CVT.s
														 default: ;//$display("Not an instrInput_i 41!");
									endcase // case ( format )
								end // begin COP1

			6'b100000:aluCtrl_o=6'b100001;//LB
			6'b100001:aluCtrl_o=6'b101011;//LH
			6'b100010:aluCtrl_o=6'b101101;//LWL
			6'b100011:aluCtrl_o=6'b111101;//LW
			6'b110000:aluCtrl_o=6'b111101;//LWC0
			6'b100100:aluCtrl_o=6'b101010;//LBU
			6'b100101:aluCtrl_o=6'b101100;//LHU
			6'b100110:aluCtrl_o=6'b101110;//LWR
			6'b101000:aluCtrl_o=6'b101111;//SB
			6'b101001:aluCtrl_o=6'b110000;//SH
			6'b101010:aluCtrl_o=6'b110010;//SWL 
			6'b101011:aluCtrl_o=6'b110001;//SW*
			6'b111000:aluCtrl_o=6'b110001;//SWC0
			6'b101110:aluCtrl_o=6'b110011;//SWR
			6'b110001:aluCtrl_o=6'b110101;//LWC1
			6'b111001:aluCtrl_o=6'b111001;//SWC1
			6'b010100:aluCtrl_o=6'b111010;//BEQL
			6'b010110:aluCtrl_o=6'b111011;//BLEZL
			6'b010101:aluCtrl_o=6'b111100;//BNEL
			default: ;//$display("Not an instrInput_i 5!");
	endcase //case ( opcode )
end //end always

endmodule