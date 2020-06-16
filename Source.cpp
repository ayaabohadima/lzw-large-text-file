#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <chrono>
using namespace std;

///////////////////////////////////////////////////////
void convertTochar1(unsigned int num, char* codes, unsigned long & icode) {
	for (unsigned int i = 0; i < 3; i++) {
		codes[icode] = (num % 256);
		num = num / 256;
		icode++;
	}
}
///////////////////////////////////////////////////////
unsigned long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	unsigned int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}
/////////////////////////////////////////////////////////
//// main :
unsigned int main() {
	// to get millsecound  when start the code to calculate code time .
	unsigned long startMill = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//**to read file at once : 
	string fileName = "enwik8";
	ifstream ifs(fileName , ios::binary);
	string fileData;
	getline(ifs, fileData, (char)ifs.eof());
	// to get millsecound  when end read file and start encode function to calculate code time .
	unsigned long readMill = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Compression : read file :" << readMill - startMill <<endl;
	//**to start encode with lzw :
	//***fill base dectionary by all ascii code from 0 to 255 
	unordered_map<string, unsigned int> myDicEncode; // dec for encode 
	for (unsigned int i = 0; i<256; i++) {
		string s = "";
		s += char(i);
		myDicEncode[s] = i;
	}
	//***declaration variables :
	unsigned long fileLength = fileData.length(); // length of file "number of bytes "
	unsigned char  *codes = new unsigned char[fileLength * 3]; // indexes which will out when encode 
	unsigned long icode = 0; // last index in codes reached to  
	unsigned long newDec = 256; // the index in dectionary .
	string matchdata = ""; // matched string 
	unsigned int num; // to calculate code
	string oldMatch; // last match "less than matchdata by one char "
	//***encode algorihm :
	for (unsigned long i = 0; i < fileLength; i++) {
		oldMatch = matchdata; 
		matchdata.push_back(fileData[i]);
		if (myDicEncode.find(matchdata ) == myDicEncode.end()) {  // if not match 
		//****to write   integer in 3 byte :
		num = myDicEncode[oldMatch];
		codes[icode++] = (num % 256);
		num = num >> 8;
		codes[icode++] = (num % 256);
		num = num >> 8;
		codes[icode++] = (num % 265);
		//****add new index in dectionary
		myDicEncode[matchdata ] = newDec;
		newDec++; 
		matchdata = fileData[i];
	   }
	}
	//****the last code if note exist .
	if (matchdata != "") {
		convertTochar1(myDicEncode[matchdata], (char *) codes, icode);
	}
	//get current millseconds 
	unsigned long funMill = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Compression : Time :" << funMill - readMill << " msec " << endl;
	cout << "Compression : Time + Reading :" << funMill - startMill << " msec " << endl;
	//***to write file 
	fstream indexesFile;
	indexesFile.open("compressedFile.bin", ios::binary | ios::out | ios::trunc);
	indexesFile.write((char *) codes, icode);
	indexesFile.close();
	//get current millseconds 
	unsigned long writeMill = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Compression : Write file " << writeMill - funMill <<" msec "<< endl;
	cout << "Compression : Total: Write + Compression Time + Reading " << writeMill - startMill << " msec " << endl;
	////////////////////////////////////////////////
	//// to decode 
	//***read file :
	fileName = "compressedFile.bin";
	fstream indexfs;
	unsigned long fs = GetFileSize(fileName);
	indexfs.open(fileName, ios::binary | ios::in);
	unsigned char *codes21 = new unsigned char[fs];
	indexfs.read((char *) codes21, fs);
	indexfs.close();
	//get current millseconds :
	unsigned long readMillDEc = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Decompression : read file : " << readMillDEc - writeMill << " msec " << endl;
	//////////////////////////////////////////
	//***decode algorithim :
	//***fill base dectionary from 0 to 255 ascii codes :
	unordered_map< unsigned int, string > myDicDecode;
	for (unsigned int i = 0; i<256; i++) {
		string s = "";
		s += char(i);
		myDicDecode[i] = s;
	}
	//declaration some variables :
	unsigned long  currentDecNum = 256;
	unsigned char * mainFile = new unsigned char[3* fs];
	unsigned long currentI = 0;
	//***decode :
	for (unsigned int i = 0; i < fs; i += 3) {
		unsigned long decNumInDec = codes21[i] + 256*( codes21[i + 1]   + codes21[i + 2] * 256 );
		unsigned long nextNumDec = codes21[i + 3] + 256 * (codes21[i + 4]  + codes21[i + 5]  * 256);
		unsigned long km = myDicDecode[decNumInDec].length();
		for (unsigned int k = 0; k < km; k++) {
			mainFile[currentI] = myDicDecode[decNumInDec][k];
			currentI++;
		}
		if(nextNumDec== currentDecNum) myDicDecode[currentDecNum] = myDicDecode[decNumInDec] + myDicDecode[decNumInDec][0];
		else myDicDecode[currentDecNum] = myDicDecode[decNumInDec] + myDicDecode[nextNumDec][0];
		currentDecNum++;
	}
	//get current millseconds :
	unsigned long funMillDEc = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Decompression :  decompress time : " <<  funMillDEc - readMillDEc << " msec"  << endl;
	//***write file after decompress :
	fstream decodeFile;
	decodeFile.open("decodeFile", ios::binary | ios::out | ios::trunc);
	decodeFile.write((char *) mainFile, currentI);
	decodeFile.close();
	//get current millseconds :
	unsigned long writeMillDec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	cout << "Decompression :  write file  " << writeMillDec - funMillDEc << " msec"  << endl;
	cout << "Decompression : total time  :  " << writeMillDec - writeMill <<" msec" << endl;

	delete[] codes;
	delete[] mainFile;
	delete[] codes21;
	system("pause");
	return 0;
}