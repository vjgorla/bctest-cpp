#include <iostream>
#include "sha256.h"
#include "bigInt.h"
#include "blockchain.h"
#include <ctime>
#include <chrono>

using std::string;
using std::cout;
using std::endl;
using namespace std::chrono;

typedef BigInt::Rossi BigInteger;


int main(int argc, char *argv[])
{
//	string prevBlockHash = "x";
//	string text = "text";
//	BigInteger nonce = BigInteger("5206793040080000452345425425414265445009812", BigInt::DEC_DIGIT);
//	cout << nonce.toStrDec() << endl;
//	Block block = Block(prevBlockHash, nonce, 42534, text);
//	cout << block.blockContentsString() << endl;
//	nonce++;
//	text.append("text1");
//	cout << block.blockContentsString() << endl;
//	cout << nonce.toStrDec()  << endl;
//	cout << text  << endl;
	Blockchain bc = Blockchain();
//	cout << bc.INITIAL_DIFFICULTY.toStrDec() << endl;

	Blockchain blockchain = Blockchain();
	unsigned long long nonce = 0;
	while (true) {
		nonce = nonce + 1;
		unsigned long long ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		Block* block = new Block(blockchain.topBlockHash, nonce, ms, "text");
//		sha256_binary(block->blockContentsString());
		string blockHash = sha256(block->blockContentsString());
		if (blockHash != "") {
			BigInteger blockHashInt = BigInteger(blockHash, BigInt::HEX_DIGIT);
			if (blockHashInt <= blockchain.currentDifficulty) {
				block->blockHash = blockHash;
				blockchain.addBlock(*block, true);
//				cout << *blockHashBinary << *(blockHashBinary + 1);
			}
		}
	}
//	int x = 0;
//	for (int i = 0; i < 1000000; i++) {
//		x++;
//		string input = "grape";
//		string output1 = sha256(input);
//	}
//
//    cout << "sha256(' " << x;
//	string input = "grape13124345";
//	string output1 = sha256(input);
//    cout << "sha256('"<< input << "'):" << output1 << endl;
//    unsigned char uc = 'c';
//    cout << (unsigned int)uc;
    return 0;
}

/*
int main(int argc, char *argv[])
{
	string prevBlockHash = "00002007422630a845f547083b9bad6ffd06bb4653cfc575f0f6bbf7038bf4bc";
	string blockHash = sha256(prevBlockHash + ":45245:750010:x");
	BigInteger arg1 = BigInteger(prevBlockHash, BigInt::HEX_DIGIT);
	BigInteger arg3 = BigInteger(blockHash, BigInt::HEX_DIGIT);
	cout << arg1.toStrDec() << endl;
	cout << blockHash << endl;
	cout << arg3.toStrDec() << endl;
    return 0;
}
*/
/*
int main(int argc, char *argv[])
{
	BigInteger INITIAL_DIFFICULTY = BigInteger::pow(BigInteger(2), BigInteger(256)) / BigInteger(100000);
	unsigned long long nonce = 0;
	clock_t begin = clock();
	for (int i = 1; i <= 1000000; i++) {
        nonce = nonce + 1;
//        Block block = Block("0", nonce, 0, "x");
        sha256_binary("0:" + std::to_string(nonce) + ":0:x");
//        if (blockHash.substr(0, 3) == "000") {
//        	BigInteger blockHashInt = BigInteger(blockHash, BigInt::HEX_DIGIT);
//        	if (blockHashInt <= INITIAL_DIFFICULTY) {
//        		cout << blockHash << ":0:" + std::to_string(nonce) + ":0:x" << endl;
//        	}
//        }
    }
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "Elapsed : " << elapsed_secs << endl;
}
*/
