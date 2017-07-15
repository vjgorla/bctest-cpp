#ifndef BLOCKCHAIN_H_
#define BLOCKCHAIN_H_
#include <string>
#include "bigInt.h"

using std::string;
typedef BigInt::Rossi BigInteger;

class Block {
    public:
        string blockHash;
        string prevBlockHash;
        unsigned long long nonce;
        unsigned long long ts;
        string text;
        Block(string prevBlockHash, unsigned long long nonce, unsigned long long ts, string text);
        string blockContentsString();
        string blockString();
};

class AddBlockResult {
    public:
        bool alreadyExists;
        bool isOrphan;
        bool valid;
};

class Blockchain {
    public:
        static const string ROOT_HASH;
        static const int RETARGET_BLOCK_INTERVAL;
        BigInteger INITIAL_DIFFICULTY;
        static const BigInteger BASELINE_TS_INTERVAL;
        static const BigInteger DIFFICULTY_PRECISION;
        BigInteger currentDifficulty;
        int retargetBlockInterval;
        BigInteger topBlockNumber;
        string topBlockHash;
        explicit Blockchain();
        AddBlockResult& addBlock(Block& block, bool mined);
        std::vector<Block*>& getDescendants(string blockHash);
        BigInteger calculateDifficulty(string hash);
    private:
        std::map<string, Block*> map;
        std::map<string, std::vector<Block*>*> descendantsMap;
        void setDifficulty(Block& block);
        int findDistance(string fromHash, string toHash, int currentDepth);
        int calculateHeight(string hash);
        BigInteger difficulty(long interval, BigInteger prevDifficulty);
        void getDescendants(string blockHash, std::vector<Block*>& result);
};

#endif
