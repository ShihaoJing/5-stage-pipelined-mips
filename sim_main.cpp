/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  
|        WRAPPER PROGRAM: Emulating OS for top->MIPS I Processor (MIPS32 ABI)                         |
|           This program automates the process of generating a binary                            |
|           and loading the hex dump of that program into a verilog processor                    |
|           memory map.  This wrapper also acts as the clock generator for                       |
|           the processor.                                                                       |
|        Written by Dan Snyder                                                                   |
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
  
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h> 
#include <sys/socket.h>   
#include <sys/time.h>
#include <sys/times.h>
#include <sys/mman.h>
#include <sys/resource.h> 
#include <sys/ioctl.h> 
#include <sys/uio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <ulimit.h>
#include <dirent.h>
#include <ulimit.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <cstdio> 
#include <cmath>
#include <iostream>
#include <fstream>
#include <istream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <map>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include <time.h>

#include "VMIPS.h"		//for access to verilog parent module
#include "VMIPS_MIPS.h"		//for access to verilog submodules
#include <verilated.h>		//verilator lib

using namespace std;

unsigned int main_time = 0;
typedef map<int,int> int_int_Map;
typedef map<int,string> int_string_Map;
int_int_Map MainMemory;
int_string_Map HEXMainMemory;
int_string_Map offset;
float IPC;
int iCount,blockbase,heapStart,blockNum,load_memory,start_reg,number_of_regs,RF_FPRF_BOTH,enable_syscall_comments;
int enable_timing,enable_single_step,filecounter,PC_start,hexCtr,flag3,clockCounter,enable_memory_bandaid,functionFlag,jumpTemp;
string fileARG,libc_openAddress,libc_readAddress,exitAddress,munmapAddress,geteuidAddress,getuidAddress,unameAddress,getpidAddress,getgidAddress,getegidAddress,libc_mallocAddress,cfreeAddress,fxstat64Address,mmapAddress,libc_writeAddress;
vector<int>argumentV;
int_int_Map HeapStatus;


//DISPLAYS HEAP CONTENTS
void heapDump(){
	int temp = heapStart+HeapStatus.size();					//shift pointer to end of the allocated heap
	while ((unsigned)temp%4!=0) temp++;					//align pointer
	for(int i=temp; i>=heapStart; i-=4) {					//start from top down, print heap contents
		if (((MainMemory[i]+MainMemory[i+1]+MainMemory[i+2]+MainMemory[i+3])!=0)) {
			printf("Heap:  0x%x",i);
			printf("(+%*u): ",4,i-heapStart);
			printf("0x%s ",((HEXMainMemory[i+3])+(HEXMainMemory[i+2])+(HEXMainMemory[i+1])+(HEXMainMemory[i+0])).c_str());
			cout << HeapStatus[i];
			cout << HeapStatus[i+1];
			cout << HeapStatus[i+2];
			cout << HeapStatus[i+3] << endl;
		}
	}
}
//ALLOCATES HEAP BLOCKS WHEN MALLOC IS CALLED
void allocateHeapBlock(int addr, int size){

	while ((unsigned)addr%4!=0) addr++;					//align block
	blockbase=addr;								//return aligned block start address
	for(int i=addr; i<=addr+size-1; i++)HeapStatus[i] = blockNum;		//set heap word state variable

}
//CLEAR HEAP BLOCKS WHEN MALLOC IS CALLED
void clearHeapBlock(int addr){
	int num = HeapStatus[addr];						//store the block # to be cleared
	for(int i=addr; i<=heapStart+HeapStatus.size(); i++) {			//iterate through memory resetting any states that match the block number
		if (HeapStatus[i]==num)HeapStatus[i] = 0;				//reset state
		else break;								//if the end of block is found break
	}
}
//DETERMINES ELF FILE SEGMENT OFFSETS BY READING ELF HEADER FILE
void getSegmentOffsets(string str, string str2){
  ifstream readFile1( str.c_str() );
  string word;
  int flag = 0;
  vector<string> words;
  if ( !readFile1 ) cerr << "File couldn't be opened" << endl;
  if (readFile1.is_open()) 
  {
	  while (!readFile1.eof() ) 
	  {
		  getline (readFile1,word);
		  if( word.find("[19]")!=string::npos){offset[19] = word.substr(52,4);}
		  else if( word.find("[20]")!=string::npos){offset[20] = word.substr(52,4);}
		  else if( word.find("[21]")!=string::npos){offset[21] = word.substr(52,4);}
		  else if( word.find("[22]")!=string::npos){offset[22] = word.substr(52,4);}
		  else if( word.find("[23]")!=string::npos){offset[23] = word.substr(52,4);}
		  else if( word.find("[24]")!=string::npos){offset[24] = word.substr(52,4);}
		  else if( word.find("[25]")!=string::npos){offset[25] = word.substr(52,4);}
		  else if( word.find("[26]")!=string::npos){offset[26] = word.substr(52,4);}
		  else if( word.find("[27]")!=string::npos){offset[27] = word.substr(52,4);}
		  else if( word.find("[29]")!=string::npos){offset[29] = word.substr(52,4);}
		  else if( word.find("[30]")!=string::npos){offset[30] = word.substr(52,4);}
		  else if( word.find("[31]")!=string::npos){offset[31] = word.substr(52,4);}
		  else if( word.find("[32]")!=string::npos){offset[32] = word.substr(52,4);}
		  else if( word.find("[33]")!=string::npos){offset[33] = word.substr(52,4);}
	  }
  }	
  readFile1.close();
  int counter=0;
  for(int i=0; i<=33; i++)
    {
      stringstream s;
      s<<i;
      string a = str2.substr(0,6)+ "_" + s.str() + ".txt";
      string newline = "\n";
      ifstream readFile2(a.c_str() );
      if (readFile2.is_open()) 
	{
	  while (!readFile2.eof() ) 
	    {
	      getline (readFile2,word);
	      if ( word.find("has no data to dump.")!=string::npos ) remove(a.c_str());
	      else if(word.find("Hex dump of section")==string::npos)
		{
		  if (counter == 1)
		    {
		      if( i>=19 && i!=28 && i!=29 )
			{
			  word.erase(0,12);
			  word.insert(0,"  0x0000"+offset[i]);
			}
		    }
		  words.push_back(word);
		  counter++;
		}
	    }
	  counter=0;
	  readFile2.close();
	  remove(a.c_str());
	  ofstream writeFile1(a.c_str());
	  for(int j=0; j<words.size(); j++)
	    {
	      writeFile1.write(words[j].c_str(),words[j].size());
	      writeFile1.write(newline.c_str(),newline.size());
	    }
	  writeFile1.close();
	  words.clear();
	}
    }
}
//BYPASSES NATIVE FUNCTIONS BY DETERMINING MEMORY LOCATION IN OBJDUMP OF EACH FUNCTION CALL
void functionBypass(string str){
	ifstream readFile1( str.c_str() );					//open the elf header file
	string word;								//variable for streaming file
	int flag = 0;								//prevents multiple tag detection
	vector<string> words;							
	if ( !readFile1 ) cerr << "File couldn't be opened" << endl;		//error if the file doesn't exist
	if (readFile1.is_open()) { 						//traverse file
		while (!readFile1.eof() ) { 						//until the end of the file is reached...
			getline (readFile1,word);						//look for tag names and store them if found
			if ( flag == 1 ) flag = 0;
			if( word.find("getgid")!=string::npos){ getgidAddress=word.substr(0,8); flag = 1;}
			else if( word.find("getegid")!=string::npos){ getegidAddress=word.substr(0,8); flag = 1;}
			else if( word.find("getpid")!=string::npos){ getpidAddress=word.substr(0,8); flag = 1;}
			else if( word.find("geteuid")!=string::npos){ geteuidAddress=word.substr(0,8); flag = 1;}
			else if( word.find("uname")!=string::npos){ unameAddress=word.substr(0,8); flag = 1;}
			else if( word.find("getuid")!=string::npos){ getuidAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<__libc_malloc>")!=string::npos){ libc_mallocAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<__cfree>")!=string::npos){ cfreeAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<___fxstat64>")!=string::npos){ fxstat64Address=word.substr(0,8); flag = 1;}
			else if( word.find("<__mmap>")!=string::npos){ mmapAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<__libc_write>")!=string::npos){ libc_writeAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<__munmap>")!=string::npos){ munmapAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<_exit>")!=string::npos){ exitAddress=word.substr(0,8); flag = 1;}
			else if( word.find("<__libc_read>")!=string::npos){ libc_readAddress=word.substr(0,8); flag = 1;}	
			else if( word.find("<__libc_open>")!=string::npos){ libc_openAddress=word.substr(0,8); flag = 1;}	
		}
    	}
	readFile1.close();
}
//CONVERTS INTEGER TO BINARY CHARACTERS
char *itob(int x){
  static char buff[sizeof(int) * CHAR_BIT + 1];
  int i;
  int j = sizeof(int) * CHAR_BIT - 1;
  buff[j] = 0;
  for(i=0;i<sizeof(int) * CHAR_BIT; i++)
    {
      if(x & (1 << i)) buff[j] = '1';
      else buff[j] = '0';
      j--;
    }
  return buff;
}
//CONVERTS HEX CHARACTERS TO INTEGERS
static inline int hexCharValue(char ch){
  if (ch>='0' && ch<='9')return ch-'0';
  if (ch>='a' && ch<='f')return ch-'a'+10;
  return 0;
}
//STORES VALUE IN MEMORY (STRING ARGUMENT)
void loadSingleHEX(string newValue, int location, int comment, int bh_word){
	
	switch (bh_word) {
		case 0:{												//store word	
			HEXMainMemory[location+0] = newValue.substr(0,2);							//msb
			HEXMainMemory[location+1] = newValue.substr(2,2);
			HEXMainMemory[location+2] = newValue.substr(4,2);
			HEXMainMemory[location+3] = newValue.substr(6,2);							//lsb
			MainMemory[location+0] = ((hexCharValue(newValue[1])) + (hexCharValue(newValue[0])<<4));		//msb
			MainMemory[location+1] = ((hexCharValue(newValue[3])) + (hexCharValue(newValue[2])<<4));
			MainMemory[location+2] = ((hexCharValue(newValue[5])) + (hexCharValue(newValue[4])<<4));
			MainMemory[location+3] = ((hexCharValue(newValue[7])) + (hexCharValue(newValue[6])<<4));		//lsb
			break;}
		case 1:{												//store byte
			HEXMainMemory[location] = newValue.substr(0,2);
			MainMemory[location] = ((hexCharValue(newValue[1])) + (hexCharValue(newValue[0])<<4));
			break;}
		case 2:{												//store halfword
			HEXMainMemory[location]   = newValue.substr(0,2);							//msB
			HEXMainMemory[location+1] = newValue.substr(2,2);							//lsB
			MainMemory[location]   = (hexCharValue(newValue[0]) + hexCharValue(newValue[1])<<4);			//msB
			MainMemory[location+1] = (hexCharValue(newValue[3]) + hexCharValue(newValue[2])<<4);			//lsB
			break;}
		default:{break;}
	}
} 
//STORES VALUE IN MEMORY (INTEGER ARGUMENT)
void loadSingleHEX(int newValue, int location, int comment, int bh_word){

	string newBinValue = itob(newValue);										//convert integer value to its (string) binary equivalent
	stringstream s;													//for binary conversion
	switch (bh_word) {
		case 0:	{												//store word	
			stringstream temp;
			string binary_str(newBinValue.substr(0,32));								//convert binary string to bitset
			bitset<32> set(binary_str);										
			temp << hex << set.to_ulong();
			while((temp.str()).size() < 8) {									//every byte, dump contents into stream
				s << "0";
				temp << "0";
			}
			s << hex << set.to_ulong();										//convert set 
			HEXMainMemory[ location + 3 ] = s.str().substr(6,2);							//lsb
			HEXMainMemory[ location + 2 ] = s.str().substr(4,2);
			HEXMainMemory[ location + 1 ] = s.str().substr(2,2);
			HEXMainMemory[ location + 0 ] = s.str().substr(0,2);							//msb
			MainMemory[    location + 3 ] = (hexCharValue((s.str().substr(6,2))[0])<<4) + (hexCharValue((s.str().substr(6,2))[1]));//lsb
			MainMemory[    location + 2 ] = (hexCharValue((s.str().substr(4,2))[0])<<4) + (hexCharValue((s.str().substr(4,2))[1]));
			MainMemory[    location + 1 ] = (hexCharValue((s.str().substr(2,2))[0])<<4) + (hexCharValue((s.str().substr(2,2))[1]));
			MainMemory[    location + 0 ] = (hexCharValue((s.str().substr(0,2))[0])<<4) + (hexCharValue((s.str().substr(0,2))[1]));//msb
			break;}
		case 1: {												//store byte
			stringstream temp;
			string binary_str(newBinValue.substr(24,8));
			bitset<8> set(binary_str);
			temp << hex << set.to_ulong();
			while((temp.str()).size() < 2) {
				s << "0";
				temp << "0";
			}
			s << hex << set.to_ulong();
			HEXMainMemory[location] = s.str();
			MainMemory[location] = (hexCharValue((s.str())[0])<<4) + (hexCharValue((s.str())[1]));
			break;
		}
		case 2: {												//store halfword
			stringstream temp;
			string binary_str(newBinValue.substr(16,16));
			bitset<16> set(binary_str);
			temp << hex << set.to_ulong();
			while((temp.str()).size() < 4) {
				s << "0";
				temp << "0";
			}
			s << hex << set.to_ulong();
			HEXMainMemory[location+1] = s.str().substr(0,2);							//msB
			HEXMainMemory[location] = s.str().substr(2,2);								//lsB
			MainMemory[location+1] = (hexCharValue((s.str())[0])<<4) + (hexCharValue((s.str())[1]));		//msb
			MainMemory[location] = (hexCharValue((s.str())[2])<<4) + (hexCharValue((s.str())[3]));			//lsb
			break;
		}
		default:{break;}
	}
	s.str("");
}
//ELF LOADER
void LoadMemory(string str){
	filecounter++;
	vector<int> V;										//temperary vector
	vector<string> tempV;									//temperary vector
	vector<string> tempVect;
	vector<string> words;
	string word;
	int offset=0;
	ifstream getFile( str.c_str(),ios::in ); 						//open the file and cut out anything unwanted if neccessary
		if (getFile.is_open()) 
		{
			while (!getFile.eof() ) 
			{
				getline (getFile,word);
				if(word.find("Hex")==string::npos) tempVect.push_back(word.substr(0,48));
			}
		}
		getFile.close();
	ofstream putFile( str.c_str(),ios::trunc );						//reopen the file to be written to (truncating old contents)
		for(int f=0; f<tempVect.size(); f++) putFile << tempVect[f] << endl;
		putFile.close();
	
	//open the file to be read into memory
	ifstream inClientFile( str.c_str(),ios::in );
		if ( !inClientFile ) cerr << "File couldn't be opened" << endl;			//test if instruction file can be opened
		while (inClientFile >> word){words.push_back(word);}				//capture raw code from file
		const int wordCount=words.size();						//determine most efficient sizing for vectors
		tempV.reserve(wordCount);							//size vector
		for(int i=0; i<wordCount; i++) {	
			if (i==0 && words[i].length()==10){ tempV.push_back(words[i]);}		//include first word to obtain data offset (memory insertion point)
			if (words[i].length()==8 && words[i].find(".")==string::npos && words[i].find(".")==string::npos ){ tempV.push_back(words[i]);}//cut out undesired strings from vector
		}
		for( int y=2; y<10; y++) offset+=hexCharValue(tempV[0][y])<<(4*(9-y));		//convert offset from hex to decimal
		tempV.erase(tempV.begin());							//delete offset from vector
		V.resize(tempV.size());								//resize vector
		for( int j=0; j<tempV.size(); j++ ) {						//convert string hex to numerical decimal
			for( int y=0; y<8; y++) V[j]+=hexCharValue(tempV[j][y])<<(4*(7-y)); 	//convert hex into int
			if (load_memory) loadSingleHEX(tempV[j],4*j+offset,0,0); 		//insert element into memory
		}
		if( filecounter == 1 ) PC_start = offset-4;
}
void uname(){
	/*insert into stack...
		"SescLinux"
		"sesc"
		"2.4.18"
		"#1 SMP Tue Jun 4 16:05:29 CDT 2002"
		"mips"*/
	loadSingleHEX("6d697073",4127448472 +348,0,0);
	loadSingleHEX("32000000",4127448472 +316,0,0);
	loadSingleHEX("20323030",4127448472 +312,0,0);
	loadSingleHEX("20434454",4127448472 +308,0,0);
	loadSingleHEX("353a3239",4127448472 +304,0,0);
	loadSingleHEX("31363a30",4127448472 +300,0,0);
	loadSingleHEX("6e203420",4127448472 +296,0,0);
	loadSingleHEX("65204a75",4127448472 +292,0,0);
	loadSingleHEX("50205475",4127448472 +288,0,0);
	loadSingleHEX("3120534d",4127448472 +284,0,0);
	loadSingleHEX("00000023",4127448472 +280,0,0);
	loadSingleHEX("342e3138",4127448472 +220,0,0);
	loadSingleHEX("0000322e",4127448472 +216,0,0);
	loadSingleHEX("63000000",4127448472 +156,0,0);
	loadSingleHEX("00736573",4127448472 +152,0,0);
	loadSingleHEX("78000000",4127448472 +96,0,0);
	loadSingleHEX("4c696e75",4127448472 +92,0,0);
	loadSingleHEX("53657363",4127448472 +88,0,0);
}
void getArguments(string str){
	argumentV.push_back(1);
	argumentV.push_back(0);
	argumentV.push_back(4127449044);
	argumentV.push_back(0);
	argumentV.push_back(0);	
	string temp = str.substr(0,str.find(".txt"));
	vector<string> argumentv;
	int flag=0;
	while((temp.size() >= 4)&&(flag!=1)) {
		argumentv.push_back(temp.substr(0,4));
		temp.erase(0,4);
		if (temp.size()<4)
			argumentv.push_back(temp);
	}
	for(int i=0; i<argumentv.size(); i++) {
		//convert first 4 characters = 4 bytes or one word
		int tempInt=0;
		//convert word to integer
		for(int j=0; j<argumentv[i].size(); j++) {
			string c = argumentv[i].substr(j,1);
			char *cs = new char[c.size() + 1];
			std::strcpy ( cs, c.c_str() );
			char a = *cs; 
			int as = a;
			tempInt = (tempInt + (as<<(24-j*8)));
		}
		argumentV.push_back(tempInt);
	}
	for(int i=0; i<=argumentV.size()-1; i++) loadSingleHEX(argumentV[i],-167518272+(i*4),0,0);
}
void fxstat64(int sp)
{
	loadSingleHEX("00000009",sp +32,0,0);
	loadSingleHEX("00000000",sp +48,0,0);
	loadSingleHEX("00000002",sp +52,0,0);
	loadSingleHEX("00002190",sp +56,0,0);
	loadSingleHEX("00000001",sp +60,0,0);
	loadSingleHEX("00001fb3",sp +64,0,0);
	loadSingleHEX("00000005",sp +68,0,0);
	loadSingleHEX("00008800",sp +72,0,0);
	loadSingleHEX("00000000",sp +88,0,0);
	loadSingleHEX("00000000",sp +92,0,0);
	loadSingleHEX("00000400",sp +120,0,0);
	loadSingleHEX("00000000",sp +128,0,0);
	loadSingleHEX("00000000",sp +132,0,0);	
}
/************************************/
/*********** MAIN PROGRAM ***********/
/************************************/
int main(int argc, char **argv)
{	
	time_t seconds;
	int duration = 1000000000;

	if(argc>3 || argc <2) { 
		cout << "USAGE: VMIPS [APP_NAME] <DURATION>" << endl; 
		exit(1);
	}

	fileARG = argv[1];

	if(argc==3) {
		sscanf(argv[2], "%d", &duration);
	}

	duration--;
	cout << "		*** ELF LOADING, PLEASE WAIT ***\n";	
	ofstream memWrite ("memoryWrites.txt");
	Verilated::commandArgs(argc, argv);
	VMIPS *top = new VMIPS;
	vector<string> FDT_filename;
	vector<int> FDT_state;//1 = open, 0 = closed       
	long spFlag=0;
	int FileDescriptorIndex = 3;//start of non-reserved indexes
	int temp1,temp2,number_of_cycles;
	int PrevInstrAddr;

	top->clk = 0;

	//first 3 positions reserved for stdin, stdout, and stderr    
	FDT_filename.push_back("stdin");FDT_state.push_back(0);		//reserve fildes 0 for stdin
	FDT_filename.push_back("stdout");FDT_state.push_back(0);	//reserve fildes 1 for stdout
	FDT_filename.push_back("stderr");FDT_state.push_back(0);	//reserve fildes 2 for stderr
	/*------------------------------------------------------------------------------------------------
	|	OPTIONS											 |
	|        -load_memory enables memory loading from external binary file                           |
	|        -enableComments allows for debugging comments to be visible at runtime per instruction |
	|        -number_of_regs allows both regfiles to be displayed up to specified number             |
	|        ---->number_of_regs [0 - 31]                                                            |
	|        -RF_FPRF_BOTH allows for RF(0) FPRF(1) or BOTH(2) to be displayed ( none (3+) )         |
	|        -bin_dec displays in binary (1) or decimal (0)                                          |
	|        -enable_syscall_comments turns off all user in/out for timing analysis                  |
	|        ---->note that exit syscall is disabled when (0)                                        |
	|        ---->return flagged by manual debugging section (default cycle # = 1M)                  |
	|        -debug allows for the debug function to compare the operation with SESC		 |
    	------------------------------------------------------------------------------------------------*/ 
	top->instrInput_i = 0;
	load_memory = 1;
	top->MIPS->enableComments = 1;
	RF_FPRF_BOTH = 2;
	start_reg = 0;
	number_of_regs = 32;
	enable_syscall_comments = 1;
	enable_single_step = 1;
	enable_timing = 1;
	
	top->reset = 1; //pc initial


	/*------------------------------------------------------------------------------------------------  
	|                                   LOAD MEMORY MAP FROM ELF FILE                                |
	|        -Offset of instruction is accounted for in load function                                |
	|        -Exclude any empty files                                                                |
	|        -Use readelf on compiled binary to extract sections 0-30 into appropriate file          |
	|        ---->use objdump to dissassemble contents                                               |
	|        ---->readelf -S [segent#] [binary name or object file] > "[segment#].txt]               |
	|        -Place for_loop_testfiles into program test folder                                      |
	|        -Note that text files are assumed to be in a specific format                            |
	------------------------------------------------------------------------------------------------*/ 
	if ( load_memory ) {
		std::string appObjDir ("/proj/ece401-spring2017/ClassShare/app_obj/");
		getArguments(appObjDir+fileARG+".txt");								//captures arguments for initializing stack
		getSegmentOffsets(appObjDir+"readelf"+fileARG+".txt",fileARG+".txt");				//determines offset of elf segments
		functionBypass(appObjDir+fileARG+".txt");								//captures function addressing
		LoadMemory(appObjDir+fileARG+"_1.txt");								//reginfo
		LoadMemory(appObjDir+fileARG+"_2.txt");								//init
		LoadMemory(appObjDir+fileARG+"_3.txt");								//text
		LoadMemory(appObjDir+fileARG+"_4.txt");								//__libc_freeres_fn
		LoadMemory(appObjDir+fileARG+"_5.txt");								//fini
		LoadMemory(appObjDir+fileARG+"_6.txt");								//rodata
		LoadMemory(appObjDir+fileARG+"_7.txt");								//data
		LoadMemory(appObjDir+fileARG+"_8.txt");								//__libc_subfreeres
		LoadMemory(appObjDir+fileARG+"_9.txt");								//__libc_atexit
		LoadMemory(appObjDir+fileARG+"_10.txt");								//eh_frame
		LoadMemory(appObjDir+fileARG+"_11.txt");								//gcc_except_table
		LoadMemory(appObjDir+fileARG+"_12.txt");								//ctors
		LoadMemory(appObjDir+fileARG+"_13.txt");								//dtors
		LoadMemory(appObjDir+fileARG+"_14.txt");								//jcr
		if (fileARG.find("noio")==string::npos)LoadMemory(appObjDir+fileARG+"_15.txt");		//got
		LoadMemory(appObjDir+fileARG+"_19.txt");								//comment
		LoadMemory(appObjDir+fileARG+"_20.txt");								//debug_aranges
		LoadMemory(appObjDir+fileARG+"_21.txt");								//debug_pubnames
		LoadMemory(appObjDir+fileARG+"_22.txt");								//debug_info
		LoadMemory(appObjDir+fileARG+"_23.txt");								//debug_abbrev
		LoadMemory(appObjDir+fileARG+"_24.txt");								//debug_line
		if (fileARG.find("noio")==string::npos)LoadMemory(appObjDir+fileARG+"_25.txt");		//debug_frame
		LoadMemory(appObjDir+fileARG+"_26.txt");								//debug_str
		LoadMemory(appObjDir+fileARG+"_27.txt");								//pdr
		LoadMemory(appObjDir+fileARG+"_28.txt");								//note.ABI-tag
		if (fileARG.find("noio")==string::npos)LoadMemory(appObjDir+fileARG+"_30.txt");		//debug_ranges
		if (fileARG.find("noio")==string::npos)LoadMemory(appObjDir+fileARG+"_31.txt");		//shstrtab
		if (fileARG.find("noio")==string::npos)LoadMemory(appObjDir+fileARG+"_32.txt");		//symtab
		top->MIPS->__PVT__RegisterFile__DOT__Reg[29] = -167518272;					//stack pointer
		top->MIPS->__PVT__RegisterFile__DOT__Reg[31] = 269446516;					//return address
		PC_start = 268435636-4;									//entry point
		top->pcInit_i = PC_start;								//determines pc startup offset
		heapStart = 3993198592;									//heap starting point
	}
	
	/*------------------------------------------------------------------------------------------------  
	|        This section contains									 |
	|	       MIPS object and all of the run-time displays			   		 |
	|           -Syscall interface									 |
	|           -Low level test interface (load instructions manually)				 |
	------------------------------------------------------------------------------------------------*/ 
	cout << "  	               *** PROGRAM EXECUTING ***\n";
	seconds  = time (NULL);
	
	while (!Verilated::gotFinish()){
		
		top->clk=!(top->clk);									//generate a clock that pulses on eval()	

		main_time++;										//increment time

		top->MIPS->enableComments = 0;								//disable comments for low clock
		stringstream s;										//for instruction processing

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(top->memRead2_o) {									//read from memory
			int tempAddress = top->dataAddr_o;						//ready variable for word alignment
			while ((unsigned)tempAddress%4!=0)tempAddress--;				//align address
			top->dataInput_i = ((MainMemory[tempAddress+0]<<24)+(MainMemory[tempAddress+1]<<16)+(MainMemory[tempAddress+2]<<8)+(MainMemory[tempAddress+3]<<0));						
		}
		 
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		
		/*tempInstrInput_o is correspond to instruction that write the memory*/
		if(top->memWrite2_o) {									//write to memory
			if (top->store_type2 == 2) 
			loadSingleHEX(top->dataOutput_o,top->dataAddr_o,0,1);	//sb
			
			else if (top->store_type2 == 1) 
			loadSingleHEX(top->dataOutput_o,top->dataAddr_o,0,2);	 //sh
			
			else loadSingleHEX(top->dataOutput_o,top->dataAddr_o,0,0); //sw
			
			memWrite << clockCounter << endl;		        //store memory access in file
			memWrite << top->dataOutput_o << " " << top->dataAddr_o << endl;				//store memory access in file
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		s << hex << top->instrAddr_o << endl;							//gets the next instruction ready for processing
		
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(main_time%2==0) {									//when the clock is positive do the following
			
			top->reset = 0; //pc initial
                        //if(!(top->branchStall || top->loadHazard || top->syscallFlag_i)){
			clockCounter ++;
                        //}			
		
			/*------------------------------------------------------------------------------------------------  
			 |				       DISPLAYS		  				          |
			 ------------------------------------------------------------------------------------------------*/ 
			
			if(clockCounter>=duration) {
				
				printf("#####################################################\nsp = 0x%x\n",top->MIPS->__PVT__RegisterFile__DOT__Reg[29]);
				
				/*
				cout << "-------------------------------------" << endl;
				
				for (int i=top->MIPS->__PVT__RegisterFile__DOT__Reg[29]+32768; i>=(top->MIPS->__PVT__RegisterFile__DOT__Reg[29]); i-=4) {
	 				int temp = (MainMemory[i+0]<<0) + (MainMemory[i+1]<<8) + (MainMemory[i+2]<<16) + (MainMemory[i+3]<<24);
					if (temp!=0) printf("Stack: 0x%x (+%u): 0x%x\n", i,i-(top->MIPS->__PVT__RegisterFile__DOT__Reg[29]),temp);
				}
				
				cout << "-------------------------------------" << endl;
				heapDump();
				cout << "-------------------------------------";
				
				cout << HEXMainMemory[4127448456]<<HEXMainMemory[4127448456+1]<<HEXMainMemory[4127448456+2]<<HEXMainMemory[4127448456+3] << endl;
				printf("MemoryAddress:%x MemoryElement:%x",top->instrAddr_o,(MainMemory[top->instrAddr_o+0]<<24) + (MainMemory[top->instrAddr_o+1]<<16) + (MainMemory[top->instrAddr_o+2]<<8) + (MainMemory[top->instrAddr_o+3]));
				*/
				
				for (int j=start_reg; j < number_of_regs; j++) {
					cout<<endl;
					if ( ( top->MIPS->__PVT__RegisterFile__DOT__Reg[j] ) == 3735928559 ) 
						printf("REG[%*d]:%*x   |   ",2,j,8,top->MIPS->__PVT__RegisterFile__DOT__Reg[j]);
					else if ( ( RF_FPRF_BOTH == 0 ) | ( RF_FPRF_BOTH == 2 ) ) 
						printf("REG[%*d]:%*u %*x   |   ",2,j,10,top->MIPS->__PVT__RegisterFile__DOT__Reg[j],8,top->MIPS->__PVT__RegisterFile__DOT__Reg[j]);
				}
					printf("Cycle:%d\n",clockCounter); 
			}
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			//function substitutions
			
			
			/*when fuction detect, set top->syscallFlag_i to 1*/
			if (((s.str().find(unameAddress)!=string::npos)||(s.str().find(geteuidAddress)!=string::npos)||
					(s.str().find(getgidAddress)!=string::npos)||(s.str().find(getegidAddress)!=string::npos)||
					(s.str().find(getpidAddress)!=string::npos)||(s.str().find(getuidAddress)!=string::npos)||
					(s.str().find(libc_mallocAddress)!=string::npos)||(s.str().find(mmapAddress)!=string::npos)||
					(s.str().find(cfreeAddress)!=string::npos)||(s.str().find(fxstat64Address)!=string::npos)||
					(s.str().find(libc_writeAddress)!=string::npos)||(s.str().find(munmapAddress)!=string::npos)||
					(s.str().find(exitAddress)!=string::npos)||(s.str().find(libc_readAddress)!=string::npos)||
					(s.str().find(libc_openAddress)!=string::npos)))
					top->syscallFlag_i = 1;
			
			/*this section only enter after all previous instruction WB*/
			if(top->writebackFlag_o){
			if (s.str().find(unameAddress)!=string::npos) {					//uname
				if(enable_syscall_comments) {
					cout << "************>> uname (out) ";					
					cout << clockCounter << " " << hex << top->instrAddr_o << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				uname();		

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = 0;	
				functionFlag=1;
			}
			else if (s.str().find(geteuidAddress)!=string::npos) { 				//geteuid
				if(enable_syscall_comments) {
					cout << "************>> geteuid (out) ";
					cout << clockCounter << " " << hex << top->instrAddr_o << endl;
					printf("Cycle:%d\n",clockCounter); 
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_geteuid);
				functionFlag=1;
			}
			else if (s.str().find(getgidAddress)!=string::npos) {				//getgid
				if(enable_syscall_comments) {
					cout << "************>> getgid ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				functionFlag=1;
	 		}
			else if (s.str().find(getegidAddress)!=string::npos) {				//getegid
				if(enable_syscall_comments) {
					cout << "************>> getegid ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getegid);
				functionFlag=1;
			}
			else if (s.str().find(getpidAddress)!=string::npos) {				//getpid
				if(enable_syscall_comments) {
					cout << "************>> getpid ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				functionFlag=1;
			}
			else if (s.str().find(getuidAddress)!=string::npos) {				//getuid
				if(enable_syscall_comments) {
					cout << "************>> getuid ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				functionFlag=1;
			}
			else if (s.str().find(libc_mallocAddress)!=string::npos) {			//libc_malloc
				if(enable_syscall_comments) {
					cout << "************>> libc_malloc (" << libc_mallocAddress << ") ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				
				blockNum++;
				int blockCounter=0;
				int blockStart=0;
				int size = top->MIPS->__PVT__RegisterFile__DOT__Reg[4];

				if(size < 32)size = 32;
					for(int i=heapStart; i<=heapStart+HeapStatus.size()+size; i++) {
						if (HeapStatus[i]==0) blockCounter++;
						else blockCounter=0;
						if (blockCounter==size) {
							allocateHeapBlock(i-size+1,size);
							break;
						}
					}
				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = blockbase;
				functionFlag=1;
			}
			else if (s.str().find(mmapAddress)!=string::npos) {				//mmap
				if(enable_syscall_comments) {
					cout << "************>> mmap (" << mmapAddress << ") ";
					cout << clockCounter << endl; 
					printf("Cycle:%d\n",clockCounter);
				}
				blockNum++;
				int blockCounter=0;
				int blockStart=0;
				int size = top->MIPS->__PVT__RegisterFile__DOT__Reg[5]*(1+top->MIPS->__PVT__RegisterFile__DOT__Reg[4]);
				
				if(size < 32)size = 32;
				for(int i=heapStart; i<=heapStart+HeapStatus.size()+size; i++) {
					if (HeapStatus[i]==0) blockCounter++;
					else blockCounter=0;
					if (blockCounter==size) {
						allocateHeapBlock(i-size+1,size);
						break;
					}
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = blockbase;
				functionFlag=1;
			}
			
			else if (s.str().find(cfreeAddress)!=string::npos) {				//cfree
				if(enable_syscall_comments) {
					cout << "************>> cfree ";
					cout << clockCounter << " " << top->instrAddr_o << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				
				clearHeapBlock(top->MIPS->__PVT__RegisterFile__DOT__Reg[4]);				
				functionFlag=1;
				
			}
			
			else if (s.str().find(fxstat64Address)!=string::npos) {				//fxstat64
				if(enable_syscall_comments) {
					cout << "************>> fxstat64 ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
			} 
				top->MIPS->__PVT__RegisterFile__DOT__Reg[4] = top->MIPS->__PVT__RegisterFile__DOT__Reg[5];
				top->MIPS->__PVT__RegisterFile__DOT__Reg[5] = top->MIPS->__PVT__RegisterFile__DOT__Reg[6];
				fxstat64(top->MIPS->__PVT__RegisterFile__DOT__Reg[29]);
				functionFlag=1;
				
			}
			
			else if (s.str().find(libc_writeAddress)!=string::npos) {			//libc_write
				if(enable_syscall_comments) {
					cout << "************>> libc_write ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = 4004;
				functionFlag=3;
				
			}
			
			else if (s.str().find(munmapAddress)!=string::npos) {				//munmap
				if(enable_syscall_comments) {
					cout << "************>> munmap ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}

				clearHeapBlock(top->MIPS->__PVT__RegisterFile__DOT__Reg[4]);
				functionFlag=1;
			}
			
			else if (s.str().find(exitAddress)!=string::npos) {				//exit
				if(enable_syscall_comments) {
					cout << "************>> exit ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}
				
				top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=4001;
				functionFlag=3;
				
			}
			
			else if (s.str().find(libc_readAddress)!=string::npos) {			//libc_read
				if(enable_syscall_comments) {
					cout << "************>> libc_read ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=4003;
				functionFlag=3;
			}
			
			else if (s.str().find(libc_openAddress)!=string::npos) {			//libc_open
				if(enable_syscall_comments) {
					cout << "************>> libc_open ";
					cout << clockCounter << endl;
					printf("Cycle:%d\n",clockCounter);
				}

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=4005;
				functionFlag=3;
			}	
			top->syscallFlag_i = 0;
			}


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			//function sub FSM
			int syscallJump;

			if (functionFlag == 1){ 							//jump to return address
				
				if(main_time%2==0) clockCounter--;

				top->instrInput_i = 65011720;						//JR instruction
				functionFlag = 2;							//goto nop
				
				syscallJump = 1; 
                                iCount++;
			}
			else if (functionFlag == 2){

					top->instrInput_i = 0;							//NOP instrucation
					functionFlag = 0;							//exit fsm
					iCount++;
			
			}
			else if (functionFlag == 3) {							//syscall
				
				if(main_time%2==0) clockCounter--;

					top->instrInput_i = 12;							//SYSCALL instruction
					functionFlag = 1;
					iCount++;
			}
			else if((HEXMainMemory[top->instrAddr_o+3] == "00")&&(HEXMainMemory[top->instrAddr_o+2] == "00")&&(HEXMainMemory[top->instrAddr_o+1] == "82")&&(HEXMainMemory[top->instrAddr_o+0] == "c0")) {
				
				int instruction = (
									 (MainMemory[top->instrAddr_o+3]) + 
								   (MainMemory[top->instrAddr_o+2]<<8) + 
								   (MainMemory[top->instrAddr_o+1]<<16) + 
								   (MainMemory[top->instrAddr_o+0]<<24));
				
				int source,immediate,base;
				source = (instruction << 12)>>28;
				immediate = (instruction << 16)>>16;
				base = (instruction << 6)>>26;

				top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = (
															(MainMemory[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate+3])+
															(MainMemory[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate+2]<<8)+
															(MainMemory[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate+1]<<16)+
															(MainMemory[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate+0]<<24));
				top->instrInput_i = 0;
				iCount++;

			}
			else if((HEXMainMemory[top->instrAddr_o+3] == "00")&&(HEXMainMemory[top->instrAddr_o+2] == "00")&&((HEXMainMemory[top->instrAddr_o+1] == "83")||(HEXMainMemory[top->instrAddr_o+1] == "82"))&&(HEXMainMemory[top->instrAddr_o+0] == "e0")) {
				
				
				/*when enter this part, set top->syscallFlag_i to 1*/
				top->syscallFlag_i = 1;
				

				/*this section only enter after all previous instruction WB*/
				if(top->writebackFlag_o == 1){

				int instruction = (
									 (MainMemory[top->instrAddr_o+3]) + 
								   (MainMemory[top->instrAddr_o+2]<<8) + 
								   (MainMemory[top->instrAddr_o+1]<<16) + 
								   (MainMemory[top->instrAddr_o+0]<<24));
				int rt=0,base,immediate;
				base = (instruction << 6)>>26;
				immediate = (instruction << 16)>>16;
				
				if(HEXMainMemory[top->instrAddr_o+1] == "83")rt = 3;
				else  if(HEXMainMemory[top->instrAddr_o+1] == "82")rt = 2;
				
				loadSingleHEX(top->MIPS->__PVT__RegisterFile__DOT__Reg[rt],top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate,0,0);

				memWrite << clockCounter << endl;
				memWrite << top->MIPS->__PVT__RegisterFile__DOT__Reg[rt] << " " << top->MIPS->__PVT__RegisterFile__DOT__Reg[4]+immediate << endl;
				top->instrInput_i = 0;

				if(HEXMainMemory[top->instrAddr_o+1] == "82")
					top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=1;
				else if(HEXMainMemory[top->instrAddr_o+1] == "83")
					top->MIPS->__PVT__RegisterFile__DOT__Reg[3]=1;

				top->syscallFlag_i = 0;		
				iCount++;
			
			}
			}

			//normal instruction supply (no function call or special instruction call)
			else {
				
				if(!syscallJump){
					top->instrInput_i = (MainMemory[top->instrAddr_o+3]) + (MainMemory[top->instrAddr_o+2]<<8) + (MainMemory[top->instrAddr_o+1]<<16) + (MainMemory[top->instrAddr_o+0]<<24);			
				
					if(PrevInstrAddr!=top->instrAddr_o) //when stall happens, do not count instruction
			                  
                                           iCount++;
				}
				
				syscallJump = 0;
			}
			
			PrevInstrAddr=top->instrAddr_o;  //for countering stall

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////			
			

			/*------------------------------------------------------------------------------------------------  
			|						SYSCALLS													 |
			------------------------------------------------------------------------------------------------*/
			int syscallIndex = top->MIPS->__PVT__RegisterFile__DOT__Reg[2];//get syscall number from register 2
			
			
			/*when syscall detect, set top->syscallFlag_i to 1*/
			if (top->instrInput_i==12) {  // if a syscall is detected,only enter syscall after WB
				top->syscallFlag_i = 1;

			/*this section only enter after all previous instruction WB*/
			if(top->writebackFlag_o){
				if(enable_syscall_comments)
				{cout << "syscall:" << syscallIndex << endl;
				 printf("Cycle:%d\n",clockCounter);
				}


				switch (syscallIndex) {
					case 4001:{											//exit
						if(enable_timing){
							seconds  = time(NULL) - seconds;
							cout << "*********************************" << endl;
							printf("Simulation time : %d sec\n",seconds);
							printf("Total cycles : %d \n",clockCounter);
							printf("Total instructions : %d \n",iCount);
							IPC = (float)iCount/((float)clockCounter);
							printf("IPC: %.2f\n",IPC);
						}
						 
						syscall(SYS_exit);
					break;}								
					case 4003:{											//read
						string input1;
						string input;
						int addr,i;
						addr = top->MIPS->__PVT__RegisterFile__DOT__Reg[5];						//memory entry pointed to by argument
						if(top->MIPS->__PVT__RegisterFile__DOT__Reg[4]==0) cin >> input;					//if STDIN use stdio					
						else {												//otherwise must be a file
							ifstream indata(FDT_filename[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]].c_str());	//stream in contents of file
							while(!indata.eof()){									//until eof
								getline (indata,input1);						
								input = input + input1;								//accumulate string
							}
						}
						if (input.size()>70)input.insert(70,"\n");							//syscall reads 70 chars at a time
						for (i=addr;i<=addr+input.size();i++) loadSingleHEX(input[i-addr],i,0,1);			//load content to memory
						loadSingleHEX("0a",i-1,0,1);									//end block with "0a"
						if (top->MIPS->__PVT__RegisterFile__DOT__Reg[4]==0) {						//if STDIN && open
							if (FDT_state[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]]!=0){					//close file when done
								top->r2Input_i = i-addr;									//return number of chars read
								FDT_state[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]]=0;					//set state bit
							}else top->r2Input_i = i-addr;									//if STDIN && closed
						}
						else {												//if fildes > 2 ( !(STD(IN,OUT,ERR) )
							if (FDT_state[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]]!=0){					//close file when done
								top->r2Input_i = i-addr;									//return number of chars read
								FDT_state[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]]=0;					//set state bit
							}else top->r2Input_i = 0;									//if fildes > 2 && closed
						}						
						break;}
					case 4004:{											//write
						int convert;											//accumulator for filename char convert
						int flag = 0;											//loop break flag
						int byte_offset;
						unsigned int k=top->MIPS->__PVT__RegisterFile__DOT__Reg[5];					//start at specified element
						unsigned int length=top->MIPS->__PVT__RegisterFile__DOT__Reg[6];
						int i = k;
						if (top->MIPS->__PVT__RegisterFile__DOT__Reg[4]!=1) {
							ofstream _file(FDT_filename[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]].c_str());
							while (MainMemory[i]!=00) {
								length--; _file << (char)MainMemory[i];
								i++; if(length == 0)break;
							}
							_file.close();
						}
						else {
							while (MainMemory[i]!=00) {
								length--; cout<<(char)MainMemory[i];
								i++; if(length == 0)break;
							}
						}							
						i++;
						top->r2Input_i = i-k-1;
						break;}
					case 4005:{		 									//open file
						string filename;
						int k=(top->MIPS->__PVT__RegisterFile__DOT__Reg[4]);
						while ( MainMemory[k]!=0 ) { filename = filename + (char)MainMemory[k]; k++; }
					 	FDT_filename.push_back(filename);			        					//add new filename to newest location
						FDT_state.push_back(1);										//add new open indicator to newest location
						top->r2Input_i = FileDescriptorIndex;								//place file descriptor into register
						FileDescriptorIndex++;										//ready the next file descriptor
						break;}
					case 4006:{FDT_state[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]]=0;break;}			//close file
					case 4018:{											//stat
						top->MIPS->__PVT__RegisterFile__DOT__Reg[4] = top->MIPS->__PVT__RegisterFile__DOT__Reg[5];
						top->MIPS->__PVT__RegisterFile__DOT__Reg[5] = top->MIPS->__PVT__RegisterFile__DOT__Reg[6];
						struct stat buf;
						top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=stat(FDT_filename[top->MIPS->__PVT__RegisterFile__DOT__Reg[4]].c_str(),&buf);
						fxstat64(top->MIPS->__PVT__RegisterFile__DOT__Reg[29]);
						break;}
					case 4020:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getpid);break;}		//getpid
					case 4024:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getuid);break;}		//getuid
					case 4028:{											//fstat
						top->MIPS->__PVT__RegisterFile__DOT__Reg[4] = top->MIPS->__PVT__RegisterFile__DOT__Reg[5];
						top->MIPS->__PVT__RegisterFile__DOT__Reg[5] = top->MIPS->__PVT__RegisterFile__DOT__Reg[6];
						struct stat buf;
						top->MIPS->__PVT__RegisterFile__DOT__Reg[2]=fstat(top->MIPS->__PVT__RegisterFile__DOT__Reg[4],&buf);
						fxstat64(top->MIPS->__PVT__RegisterFile__DOT__Reg[29]);
						break;}
					case 4037:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_kill);break;}			//kill
					case 4047:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getgid);break;}		//getgid
					case 4049:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_geteuid);clockCounter-=3;break;}
					case 4050:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getegid);break;}		//getegid
					case 4064:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getppid);break;}		//getppid
					case 4065:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getpgrp);break;}		//getpgrp
					case 4076:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getrlimit);break;}		//getrlimit
					case 4077:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getrusage);break;}		//getrusage
					case 4078:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_gettimeofday);break;}		//gettimeofday
					case 4091:{clearHeapBlock(top->MIPS->__PVT__RegisterFile__DOT__Reg[4]);}				//munmap
					case 4122:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = 0;uname();clockCounter -= 8;break;}	//uname
					case 4132:{top->MIPS->__PVT__RegisterFile__DOT__Reg[2] = syscall(SYS_getpgid);break;}		//getpgid
					case 4246:{syscall(SYS_exit);break;}								//exit
					default: { cout << "Sorry, syscall " << syscallIndex << " has not been implemented. Process terminated..." << endl; return 0; }
					printf("Time:%d\n",clockCounter);
				}
				top->syscallFlag_i = 0;
				}
			} //syscall		
			
			

			/*------------------------------------------------------------------------------------------------  
			 |								MANUAL TEST SECTION               |
	    		 ------------------------------------------------------------------------------------------------*/         
			
			if(clockCounter >= duration) cin.get(); 		//prevents next instruction traversal until user input (any key pressed)
				//	iCount += top->InstructionCompletionFlag;			
				//	iCount++;
		
		} //if(onStall:"<<functionStall<<endl;in_time%2==0)
		
		top->eval();							//assert c++ to verilog modules
	
	}  while (!Verilated::gotFinish())
	
	memWrite.close();							//closes memory tracking file

} // end of main 