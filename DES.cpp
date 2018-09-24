#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>



std::vector<int> make_key(int num){
	std::vector<int>fin(10);
	for (int i = 0; i < 10; ++i){
		fin[i]=num%2;
		num/=2;
	}
	return fin;
}
std::vector<int> char_to_vector(char c){
	//convert a char to a vector representation
	std::vector<int> out(8);
	for (int i = 7; i >= 0; --i){
        out[i]=c&1;
        c=c>>1;
    }
   return out;
}
char vector_to_char(std::vector<int> vec){
	//convert a vector representation back to a char
	int val=0;
	for (int i = 0; i < 8; ++i){
		val+=pow(2,7-i)*vec[i];
	}
	return (char)  val;
}
std::vector<std::vector<int>> string_to_matrix(std::string str){
	//take a c++ string and convert it to a vector of vector binary representation
	std::vector<std::vector<int>>out;
	for (int i = 0; i < str.length(); ++i){
		out.push_back(char_to_vector(str[i]));
	}
	return out;
}
//THE DIFFERENT PERMUTATIONS
void ExP(std::vector<int>&in){
	in.push_back(in[1]);
	in.push_back(in[2]);
	in.push_back(in[3]);
	in.push_back(in[0]);

	in[0]=in[6];
	in[1]=in[7];
	in[2]=in[4];
	in[3]=in[5];
}
void P10(std::vector<int>&in){
	//2 4 1 6 3 9 0 8 7 5
	//0 1 2 3 4 5 6 7 8 9
	int temp=0;
	temp=in[0];
	in[0]=in[2];
	in[2]=in[1];
	in[1]=in[4];
	in[4]=in[3];
	in[3]=in[6];
	in[6]=temp;
	//5,7,8,9
	temp=in[5];
	in[5]=in[9];
	in[9]=temp;
	temp=in[7];
	in[7]=in[8];
	in[8]=in[7];
}
void P8(std::vector<int>&in){
	//5 2 6 3 7 4 9 8
	//0 1 2 3 4 5 6 7 8 9
	//NO 0 or 1
	int temp=0;
	in[0]=in[5];
	in[5]=in[4];
	in[4]=in[7];
	in[7]=in[8];
	in[1]=in[2];
	in[2]=in[6];
	in[6]=in[9];

	in.pop_back();
	in.pop_back();
}
void IP(std::vector<int>& in){
	//15203746
	//01234567
	int temp=0;

	//swap all the spots around in a set order

	temp=in[0];
	in[0]=in[1];
	in[1]=in[5];
	in[5]=in[7];
	in[7]=in[6];
	in[6]=in[4];
	in[4]=in[3];
	in[3]=temp;
}
void iIP(std::vector<int>& in){
	//30246175
	//01234567
	int temp=0;
	//swap all the spots back to where they were.
	temp=in[0];
	in[0]=in[3];
	in[3]=in[4];
	in[4]=in[6];
	in[6]=in[7];
	in[7]=in[5];
	in[5]=in[1];
	in[1]=temp;
}
void left_shift(std::vector<int>&in){
	int temp=in[0];
	for (int i = 0; i < in.size()-1; ++i){
		in[i]=in[i+1];
	}
	in[in.size()]=temp;
}
std::vector<int> convert_Sbox(std::vector<int>input, int S[][4]){
	//run through S-boxes
	int r=input[0]*2+input[3];
	int c=input[1]*2+input[2];

	int v= S[r][c];

	std::vector<int> out(2);
	out[0]=v%2;
	v/=2;
	out[1]=v%2;

	return out;
}
std::vector<int> Key_gen(std::vector<int> key,int k1_k2){
	//generate keys from start key

	//p10 of 10 bit key
	P10(key);
	//break into 2 5 bit pieces
	std::vector<int> left(5);
	std::vector<int> right(5);
	for (int i = 0; i < 5; ++i){
		left[i]=key[i];
		right[i]=key[i+5];
	}

	//Left shift
	left_shift(left);
	left_shift(right);

	//recombine for key 1.
	std::vector<int>k1(10);
	for (int i = 0; i < 5; ++i){		
		k1[i]=left[i];
		k1[i+5]=right[i];
	}

	//send into P8 for k1
	P8(k1);


	//left shift again and send into p8 for k2
	left_shift(left);
	left_shift(right);

	//recombine for key 2.
	std::vector<int>k2(10);
	for (int i = 0; i < 5; ++i){		
		k2[i]=left[i];
		k2[i+5]=right[i];
	}
	P8(k2);

	if(k1_k2==1){
		return k1;
	}
	else return k2;
}
std::vector<int> F_box(std::vector<int> input, std::vector<int> key, int S1[][4], int S2[][4]){

	//starts as 4 bit

	//expansion permutation to make into 8 bit
	ExP(input);
	//xor with 8 bit key
	std::vector<int> xor1(8);
	for (int i = 0; i < 8; ++i){
		if(input[i]==key[i]){
			xor1[i]=0;
		}
		else
			xor1[i]=1;
	}
	//break into 4 bit and 4 bit
	std::vector<int> left(4);
	std::vector<int> right(4);
	for (int i = 0; i < 4; ++i){
		left[i]=xor1[i];
		right[i]=xor1[i+4];
	}

	//send into S-box1 and S-box2
	std::vector<int> out1=convert_Sbox(left,S1);
	std::vector<int> out2=convert_Sbox(right,S2);

	//combine these 2 bit outputs using P4
	std::vector<int>final={left[1],right[1],left[0],right[0]};

	//return the 4 bit solution	
	return final;
}
std::vector<int> encrypt(std::vector<int> PT,std::vector<int> key,int S1[][4], int S2[][4],bool encrypt){
	//create the 2 keys
	std::vector<int>K1;
	std::vector<int>K2;

	if(encrypt==true){
		K1=Key_gen(key,1);
		K2=Key_gen(key,2);
	}
	else{
		K1=Key_gen(key,2);
		K2=Key_gen(key,1);
	}
	//run the Initial Permutation
	IP(PT);
	std::vector<int> Left(4);
	std::vector<int> Right(4);
	//split them
	for (int i = 0; i < 4; ++i){
		Left[i]=PT[i];
		Right[i]=PT[i+4];
	}
	//send throught he F-box
	std::vector<int> out1=F_box(Right,K1,S1,S2);
	std::vector<int>xor1(4);
	//xor the 2
	for (int i = 0; i < 4; ++i){
		if(Left[i]==out1[i]){
			xor1[i]=0;
		}
		else
			xor1[i]=1;
	}
	//run through second F-box
	std::vector<int> out2=F_box(xor1,K2,S1,S2);
	std::vector<int>xor2(4);
	//second xor
	for (int i = 0; i < 4; ++i){
		if(Right[i]==out2[i]){
			xor2[i]=0;
		}
		else
			xor2[i]=1;
	}

	//recombine to 8 bit
	std::vector<int>CT(8);
	for (int i = 0; i < 4; ++i){
		CT[i]=xor2[i];
		CT[i+4]=xor1[i];
	}
	//run through inverse Initial Permutation
	iIP(CT);
	return CT;
}
int main(int argc, char const *argv[]){
	//argv[prog_name,inputfile,Key,port,IP]
	std::vector<int> KEY=make_key(atoi(argv[2]));
	std::ifstream inFile(argv[1]);
	if(!inFile.good()){
		perror("Error");
		return -1;
	}

	//Making the Sboxes
	int S1[4][4]={{1,0,3,2},{3,2,1,0},{0,2,1,3},{3,1,3,2}};
	int S2[4][4]={{0,1,2,3},{2,0,1,3},{3,0,1,0},{2,1,0,3}};


	char pc;
	
	std::string fptxt;	//full plain txt
	std::string fctxt;	//full cypher txt
	while(inFile.get(pc)){
		//set the plain text
		std::string ptxt;
		ptxt.push_back(pc);
		std::vector<std::vector<int>> Pmat=string_to_matrix(ptxt);
		std::vector<std::vector<int>> Cmat;
		//run through DES
		for (int i = 0; i < Pmat.size(); ++i){
			Cmat.push_back(encrypt(Pmat[i],KEY,S1,S2,true));
		}
		std::string ctxt;
		for (int i = 0; i < Pmat.size(); ++i){
			ctxt+=vector_to_char(Cmat[i]);
		}
		
		fctxt+=ctxt;
		fptxt+=ptxt;


	}
	const char* Cctxt=fctxt.c_str();
	std::cout<<fctxt;
	//sockets and stuff

	//port
	short p= atoi(argv[3]);
	struct sockaddr_in mysock;
	bzero(&mysock, sizeof(mysock));
	mysock.sin_family= AF_INET;
	mysock.sin_port= htons(p);
	if(inet_pton(AF_INET,argv[4],&mysock.sin_addr)<=0){
		perror("Error");
		return -1;
	}
	int fd=socket(AF_INET,SOCK_STREAM,0);
	if(connect(fd,(struct sockaddr*) &mysock,sizeof(mysock))){
		perror("Error");
		return -1;
	}

	//send the whole message
	int w=write(fd, Cctxt,fptxt.length());
	if (w==0){
		std::cout<<"NOTHING WRITTEN"<<std::endl;
		return -1;
	}
	if (w==-1){
		perror("Error");
		return -1;
	}
	std::vector<std::vector<int>> Cmat=string_to_matrix(fctxt);
	//Decrypt
	std::vector<std::vector<int>> Fmat;
	for (int i = 0; i < Cmat.size(); ++i){
		Fmat.push_back(encrypt(Cmat[i],KEY,S1,S2,false));
	}
/*	//print the decoded binary.
	for (int i = 0; i < Fmat.size(); ++i){
		for (int j = 0; j < 8; ++j){
			std::cout<<Fmat[i][j];
		}
	}
*/
	std::string decrypted;
	std::vector<char> Fchar;
	for (int i = 0; i < Fmat.size(); ++i){
		Fchar.push_back(vector_to_char(Fmat[i]));
		decrypted+=Fchar[i];
	}

	//print the entire original Plain Text
	std::cout<<"PLAIN TEXT"<<std::endl;
	std::cout<<fptxt<<std::endl;
	//print the entire Cypher Text
	std::cout<<"CYPHER TEXT"<<std::endl;
	std::cout<<fctxt<<std::endl;
	//print the entire Decrypted Text
	std::cout<<"DECRYPTED TEXT"<<std::endl;
	std::cout<<decrypted<<std::endl;
	return 0;
}