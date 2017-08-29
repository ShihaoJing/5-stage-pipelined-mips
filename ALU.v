//-----------------------------------------
//               ALU Module
//-----------------------------------------
module ALU( 
	
	//input definitions
	input clk,
	input enableComments_i,
	input tempBranch_i,
	input [4:0] shiftAmount_i,
	input [5:0] aluCtrl_i,
	input [31:0] a_i,
	input [31:0] readData2_i,
	input [31:0] signExtendedOutput_i,
	input aluSrc_i,
	
	//output definitions
	output [31:0] high_o,
	output [31:0] low_o,
	output zero_o,
	output [31:0] aluResult_o
	);

//interconnect definitions
wire [4:0] i;
wire [4:0] j;
wire [31:0] b_i;
wire [63:0] temp;
wire [4:0] shiftAmount_i;

//combinational logic
assign b_i = (aluSrc_i)?signExtendedOutput_i:readData2_i; /*readData2 from Reg or is a imm*/


/*on input change, carry out designated operation*/
always @ (*) 
begin
	zero_o = 0;
	case( aluCtrl_i ) 
		6'b000000,6'b000010,6'b000001,6'b110111,
		6'b110101:aluResult_o = a_i+b_i;//add,addi,addiu,addu,lwc
		6'b111101,6'b100001,6'b101010,6'b101011,6'b101100,6'b101101,
		6'b101110,6'b101111,6'b110000,6'b110001,6'b110010,6'b110011,
		6'b111001:aluResult_o = a_i+{{16{b_i[15]}},b_i[15:0]};//lw,lb,lbu,lh,lhu,lwl,lwr,sb,sh,sw,swl,swr,swc
		6'b000100:aluResult_o = a_i & b_i;//and
		6'b000101:begin //div
							if(b_i!=0)
									begin
										low_o[31] = a_i[31] | b_i[31];
										low_o[30:0] = a_i[30:0] / b_i[30:0];
										high_o[30:0] = a_i[30:0] % b_i[30:0];
									end
							end
		6'b000110:begin //divu
								if(b_i!=0)
									begin
										low_o = a_i / b_i;
										high_o = a_i % b_i;
									end
							end
		6'b001000:aluResult_o = {b_i[15:0],16'b0};//lui
		6'b001001:aluResult_o = high_o;//mfhi
		6'b001010:aluResult_o = low_o;//mflo
		6'b001011:high_o = a_i;//mthi
		6'b001100:low_o = a_i;//mtlo
		6'b001101:begin//mult//multu
								temp[63:0] = a_i * b_i;
								high_o = temp[63:32];
								low_o = temp[31:0];
							end
		//6'b001110:begin//j//jal  /*already handle jump instr outside ALU*/
								//aluResult_o = b_i << shiftAmount_i;
								//zero_o = 1;
								//end
		//6'b111110:begin//jr//jalr
								//aluResult_o = b_i << shiftAmount_i;
								//zero_o = 1;
								//end
		6'b001111:aluResult_o = ~(a_i | b_i);//nor
		6'b010000:aluResult_o = a_i | b_i;//or,ori
		6'b010011:aluResult_o = b_i << shiftAmount_i;//sll,LWC0
		6'b010100:aluResult_o = b_i << a_i;//sllv
		6'b010101:begin //slt
								if( a_i[31] < b_i[31] ) 
									aluResult_o = 0;
								else if( a_i[30:0] > b_i[30:0] ) 
									aluResult_o = 0;
								else if( a_i == b_i ) 
									aluResult_o = 0;
								else 
									aluResult_o = 1;
							end
		6'b111111:begin //sltu
								if( a_i[31:0] > b_i[31:0] )
									aluResult_o = 0;
								else if( a_i == b_i ) 
									aluResult_o = 0;
								else 
									aluResult_o = 1;
							end
		6'b011001:begin //sra
								temp[32]=b_i[31];
								temp[31:0] = {b_i[31:0] >> shiftAmount_i};
								temp[31]=temp[32];
								if(shiftAmount_i>=1)temp[30]=temp[32];
								if(shiftAmount_i>=2)temp[29]=temp[32];
								if(shiftAmount_i>=3)temp[28]=temp[32];
								if(shiftAmount_i>=4)temp[27]=temp[32];
								if(shiftAmount_i>=5)temp[26]=temp[32];
								if(shiftAmount_i>=6)temp[25]=temp[32];
								if(shiftAmount_i>=7)temp[24]=temp[32];
								if(shiftAmount_i>=8)temp[23]=temp[32];
								if(shiftAmount_i>=9)temp[22]=temp[32];
								if(shiftAmount_i>=10)temp[21]=temp[32];
								if(shiftAmount_i>=11)temp[20]=temp[32];
								if(shiftAmount_i>=12)temp[19]=temp[32];
								if(shiftAmount_i>=13)temp[18]=temp[32];
								if(shiftAmount_i>=14)temp[17]=temp[32];
								if(shiftAmount_i>=15)temp[16]=temp[32];
								if(shiftAmount_i>=16)temp[15]=temp[32];
								if(shiftAmount_i>=17)temp[14]=temp[32];
								if(shiftAmount_i>=18)temp[13]=temp[32];
								if(shiftAmount_i>=19)temp[12]=temp[32];
								if(shiftAmount_i>=20)temp[11]=temp[32];
								if(shiftAmount_i>=21)temp[10]=temp[32];
								if(shiftAmount_i>=22)temp[9]=temp[32];
								if(shiftAmount_i>=23)temp[8]=temp[32];
								if(shiftAmount_i>=24)temp[7]=temp[32];
								if(shiftAmount_i>=25)temp[6]=temp[32];
								if(shiftAmount_i>=26)temp[5]=temp[32];
								if(shiftAmount_i>=27)temp[4]=temp[32];
								if(shiftAmount_i>=28)temp[3]=temp[32];
								if(shiftAmount_i>=29)temp[2]=temp[32];
								if(shiftAmount_i>=30)temp[1]=temp[32];
								if(shiftAmount_i>=31)temp[0]=temp[32];
								aluResult_o = temp[31:0];
							end
		6'b011010:begin //srav
								temp[32]=b_i[31];
                temp[31:0] = {b_i[31:0] >> (a_i[4:0])};
                for(i=0;i<=a_i[4:0];i=i+1) 
                	temp[31-i] = temp[32];
                aluResult_o = temp[31:0];
              end
    6'b011011: aluResult_o = (b_i[31:0] >> shiftAmount_i);//srl
    6'b011100:begin //srlv
								temp[31:0] = {b_i[31:0] >> a_i[4:0]};
                for(i=0;i<=a_i[4:0];i=i+1) 
                	temp[31-i] = 0;
                aluResult_o = temp[31:0];
              end
    6'b011101,6'b011110:aluResult_o = a_i - b_i;//sub,subu
		6'b011111,6'b100000:aluResult_o = a_i ^ b_i;//xor,xori
		6'b100010:begin //beq
								if( (a_i == b_i) && (tempBranch_i) ) 
									begin 
                  	zero_o = 1; 
                  	if ( enableComments_i && !clk ) $display("\nTaken"); 
                  end 
                else if ( enableComments_i && !clk ) $display("\nNot Taken"); 
              end
    6'b100011:begin //bgez
                if( (a_i[31] != 1) && (tempBranch_i) ) 
                	begin 
                  	zero_o = 1; 
                    if ( enableComments_i && !clk ) $display("\nTaken"); 
                  end
                else if ( enableComments_i && !clk ) $display("\nNot Taken"); 
            	end
    6'b100101:begin //bgtz
              	if( (a_i > 0) && (tempBranch_i) ) 
              		begin 
                    zero_o = 1; 
                  	if ( enableComments_i && !clk ) $display("\nTaken"); 
                  end
                else if ( enableComments_i && !clk ) $display("\nNot Taken");  
              end
    6'b100110:begin //blez
                if(( (a_i[31] == 1)||(a_i == 0) )&& (tempBranch_i) ) 
                	begin 
                    zero_o = 1; 
                    if ( enableComments_i && !clk ) $display("\nTaken"); 
                  end
              	else if ( enableComments_i && !clk ) $display("\nNot Taken"); 
              end
		6'b100111:begin //bltz
								if( (a_i[31] == 1) && (tempBranch_i) )
									begin 
                    zero_o = 1; 
                    if ( enableComments_i && !clk ) $display("\nTaken"); 
                  end
                else if ( enableComments_i && !clk ) $display("\nNot Taken"); 
              end
    6'b101001:begin //bne
              	if( (a_i != b_i) && (tempBranch_i) ) 
              		begin 
                    zero_o = 1; 
                    if ( enableComments_i && !clk ) $display("\nTaken");     
                	end
                else if ( enableComments_i && !clk ) $display("\nNot Taken"); 
              end
    6'b110100,6'b111000:aluResult_o = b_i;//ctc,mtc
    default: aluResult_o = 0;
  endcase
end

endmodule