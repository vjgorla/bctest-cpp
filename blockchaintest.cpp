#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "blockchain.h"
#include "bigInt.h"

using std::string;
typedef BigInt::Rossi BigInteger;

AddBlockResult addBlock(string blockHash, string prevBlockHash, BigInteger nonce, long ts, bool mined, Blockchain& blockchain) {
    Block* block = new Block(prevBlockHash, nonce, ts, "x");
    block->blockHash = blockHash;
    return blockchain.addBlock(*block, mined);
}

void addBlock(string blockHash, string prevBlockHash, Blockchain& blockchain) {
    addBlock(blockHash, prevBlockHash, BigInteger(0), 1, true, blockchain);
}

void assertTopBlock(string topBlockHash, int topBlockNumber, Blockchain& blockchain) {
    REQUIRE(blockchain.topBlockHash == topBlockHash);
    REQUIRE(blockchain.topBlockNumber.toUnit() == topBlockNumber);
}

string getDescendantsAsStr(string hash, Blockchain& blockchain) {
    std::vector<Block*> descendants = blockchain.getDescendants(hash);
    string acc = "";
    for (std::vector<Block*>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
        if (acc.length() > 0) {
            acc.append(":");
        }
        acc.append((*it)->blockHash);
    }
    return acc;
}


TEST_CASE( "testIntialState", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    REQUIRE(blockchain.topBlockHash == Blockchain::ROOT_HASH);
    REQUIRE(blockchain.topBlockNumber.toUnit() == 0);
}

TEST_CASE( "testFirstBlock", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("a",Blockchain::ROOT_HASH,blockchain);
    assertTopBlock("a", 1, blockchain);
}

TEST_CASE( "testOrphan", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("a",Blockchain::ROOT_HASH,blockchain);
    addBlock("b","x",blockchain);
    assertTopBlock("a", 1, blockchain);
}

TEST_CASE( "testNoReorg", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("a",Blockchain::ROOT_HASH,blockchain);
    addBlock("b","a",blockchain);
    addBlock("c", "b",blockchain);
    addBlock("p",Blockchain::ROOT_HASH,blockchain);
    assertTopBlock("c", 3, blockchain);
}

TEST_CASE( "testReorgSuperficial", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("p",Blockchain::ROOT_HASH,blockchain);
    assertTopBlock("p", 1, blockchain);
    addBlock("a",Blockchain::ROOT_HASH,blockchain);
    assertTopBlock("p", 1, blockchain);
    addBlock("b","a",blockchain);
    assertTopBlock("b", 2, blockchain);
    addBlock("q","p",blockchain);
    assertTopBlock("b", 2, blockchain);
    addBlock("r","q",blockchain);
    assertTopBlock("r", 3, blockchain);
}

TEST_CASE( "testReorgDeep", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("x",Blockchain::ROOT_HASH,blockchain);
    addBlock("p","x",blockchain);
    addBlock("q","p",blockchain);
    addBlock("a","x",blockchain);
    addBlock("b", "a",blockchain);
    addBlock("c", "b",blockchain);
    assertTopBlock("c", 4, blockchain);
    addBlock("r","q",blockchain);
    assertTopBlock("c", 4, blockchain);
    addBlock("s","r",blockchain);
    assertTopBlock("s", 5, blockchain);
}

TEST_CASE( "testReorgVeryDeep", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("p",Blockchain::ROOT_HASH,blockchain);
    addBlock("q","p",blockchain);
    addBlock("a",Blockchain::ROOT_HASH,blockchain);
    addBlock("b", "a",blockchain);
    addBlock("c", "b",blockchain);
    assertTopBlock("c", 3, blockchain);
    addBlock("r","q",blockchain);
    assertTopBlock("c", 3, blockchain);
    addBlock("s","r",blockchain);
    assertTopBlock("s", 4, blockchain);
}

TEST_CASE( "testReorgMultipleBranches", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    addBlock("p",Blockchain::ROOT_HASH,blockchain);
    REQUIRE(getDescendantsAsStr(Blockchain::ROOT_HASH, blockchain) == "p");
    REQUIRE(getDescendantsAsStr("p", blockchain) == "");
    addBlock("q","p",blockchain);
    REQUIRE(getDescendantsAsStr(Blockchain::ROOT_HASH, blockchain) == "p:q");
    REQUIRE(getDescendantsAsStr("p", blockchain) == "q");
    addBlock("r","q",blockchain);
    assertTopBlock("r", 3, blockchain);
    addBlock("a","p",blockchain);
    assertTopBlock("r", 3, blockchain);
    addBlock("b", "a",blockchain);
    assertTopBlock("r", 3, blockchain);
    addBlock("c", "b",blockchain);
    REQUIRE(getDescendantsAsStr(Blockchain::ROOT_HASH, blockchain) == "p:a:b:c");
    REQUIRE(getDescendantsAsStr("p", blockchain) == "a:b:c");
    REQUIRE(getDescendantsAsStr("q", blockchain) == "");
    assertTopBlock("c", 4, blockchain);
    addBlock("x", "b",blockchain);
    assertTopBlock("c", 4, blockchain);
    addBlock("y", "x",blockchain);
    assertTopBlock("y", 5, blockchain);
    addBlock("d", "c",blockchain);
    assertTopBlock("y", 5, blockchain);
    addBlock("e", "d", blockchain);
    assertTopBlock("e", 6, blockchain);
    addBlock("s", "r", blockchain);
    assertTopBlock("e", 6, blockchain);
    addBlock("t", "s", blockchain);
    assertTopBlock("e", 6, blockchain);
    addBlock("u", "t", blockchain);
    assertTopBlock("e", 6, blockchain);
    addBlock("v", "u", blockchain);
    REQUIRE(getDescendantsAsStr(Blockchain::ROOT_HASH, blockchain) == "p:q:r:s:t:u:v");
    assertTopBlock("v", 7, blockchain);
}

TEST_CASE( "testInitialDifficulty", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    blockchain.retargetBlockInterval = 2;
    addBlock("a",Blockchain::ROOT_HASH, BigInteger(0), 0, true, blockchain);
    REQUIRE(blockchain.calculateDifficulty("a") == blockchain.INITIAL_DIFFICULTY);
    REQUIRE(blockchain.currentDifficulty == blockchain.INITIAL_DIFFICULTY);
    addBlock("b","a", BigInteger(0), 0, true, blockchain);
    REQUIRE(blockchain.calculateDifficulty("b") == blockchain.INITIAL_DIFFICULTY);
    REQUIRE(blockchain.currentDifficulty == blockchain.INITIAL_DIFFICULTY);
}

TEST_CASE( "testRetargetMainBranch", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    blockchain.retargetBlockInterval = 2;
    addBlock("a",Blockchain::ROOT_HASH, BigInteger(0), 0, true, blockchain);
    addBlock("b","a", BigInteger(0), 10, true, blockchain);
    addBlock("c","b", BigInteger(0), 2000000, true, blockchain);
    REQUIRE(blockchain.calculateDifficulty("c") == (blockchain.INITIAL_DIFFICULTY * BigInteger(2)));
    REQUIRE(blockchain.currentDifficulty == (blockchain.INITIAL_DIFFICULTY * BigInteger(2)));
    addBlock("d","c", BigInteger(0), 2000010, true, blockchain);
    addBlock("e","d", BigInteger(0), 3000000, true, blockchain);
    REQUIRE(blockchain.calculateDifficulty("e") == (blockchain.INITIAL_DIFFICULTY * BigInteger(2)));
    REQUIRE(blockchain.currentDifficulty == (blockchain.INITIAL_DIFFICULTY * BigInteger(2)));
    addBlock("f","e", BigInteger(0), 3000010, true, blockchain);
    addBlock("g","f", BigInteger(0), 3000010, true, blockchain);
    REQUIRE(blockchain.calculateDifficulty("g") == (blockchain.INITIAL_DIFFICULTY / BigInteger(50000)));
    REQUIRE(blockchain.currentDifficulty == (blockchain.INITIAL_DIFFICULTY / BigInteger(50000)));
}

TEST_CASE( "testRetargetOtherBranches", "[blockchain]" ) {
    Blockchain blockchain = Blockchain();
    blockchain.retargetBlockInterval = 2;
    addBlock("a",Blockchain::ROOT_HASH, BigInteger(0), 0, true, blockchain);
    addBlock("b","a", BigInteger(0), 10, true, blockchain);
    addBlock("c","b", BigInteger(0), 1000000, true, blockchain);
    addBlock("d","c", BigInteger(0), 1000010, true, blockchain);
    addBlock("e","d", BigInteger(0), 2000000, true, blockchain);
    assertTopBlock("e", 5, blockchain);

    AddBlockResult result = addBlock("0000775a57e301f40a233af9e38a5b4b88252990e54ff6d5cdd90193bae574ab",
            Blockchain::ROOT_HASH, BigInteger(104566), 0, false, blockchain);
    REQUIRE(result.valid);

    result = addBlock("0000946104f7553285a916a1b4fcfa5daa90d3bfdba15f06f5f48f85dc8f508c",
            "0000775a57e301f40a233af9e38a5b4b88252990e54ff6d5cdd90193bae574ab", BigInteger(4666), 10, false, blockchain);
    REQUIRE(result.valid);

    result = addBlock("00002210f887de78b5697862f9b04a9ad880afc81a23a012b75b000a2c1305cd",
            "0000946104f7553285a916a1b4fcfa5daa90d3bfdba15f06f5f48f85dc8f508c", BigInteger(24846), 2000000, false, blockchain);
    REQUIRE(result.valid);

    REQUIRE(blockchain.calculateDifficulty("00002210f887de78b5697862f9b04a9ad880afc81a23a012b75b000a2c1305cd") == (blockchain.INITIAL_DIFFICULTY * BigInteger(2)));
    REQUIRE(blockchain.currentDifficulty == blockchain.INITIAL_DIFFICULTY);
    assertTopBlock("e", 5, blockchain);

    result = addBlock("00012eb9a9a568a13af7bad824ecf9719238aa45093d991941ed64a35e12fdd7",
            "00002210f887de78b5697862f9b04a9ad880afc81a23a012b75b000a2c1305cd", BigInteger(57500), 2000010, false, blockchain);
    REQUIRE(result.valid);

    result = addBlock("000093b3cea80d91e5046abe34a750c690891afa47ac83edba838e52190ec312",
            "0000946104f7553285a916a1b4fcfa5daa90d3bfdba15f06f5f48f85dc8f508c", BigInteger(57504), 500000, false, blockchain);
    REQUIRE(result.valid);

    REQUIRE(blockchain.calculateDifficulty("000093b3cea80d91e5046abe34a750c690891afa47ac83edba838e52190ec312") == (blockchain.INITIAL_DIFFICULTY / BigInteger(2)));
    REQUIRE(blockchain.currentDifficulty == blockchain.INITIAL_DIFFICULTY);
    assertTopBlock("e", 5, blockchain);

    result = addBlock("00009a0db70081b2091775b23c4f2829b5d9515c798d77533c99aef599565995",
            "000093b3cea80d91e5046abe34a750c690891afa47ac83edba838e52190ec312", BigInteger(147774), 500010, false, blockchain);
    REQUIRE(!result.valid);

    result = addBlock("000048021b2f9e47e0c81783ce2940cc318f31d909bf537526e73fb54a9fb013",
            "000093b3cea80d91e5046abe34a750c690891afa47ac83edba838e52190ec312", BigInteger(211319), 500010, false, blockchain);
    REQUIRE(result.valid);

    result = addBlock("00002007422630a845f547083b9bad6ffd06bb4653cfc575f0f6bbf7038bf4bc",
            "000048021b2f9e47e0c81783ce2940cc318f31d909bf537526e73fb54a9fb013", BigInteger(526528), 750000, false, blockchain);
    REQUIRE(result.valid);

    REQUIRE(blockchain.calculateDifficulty("00002007422630a845f547083b9bad6ffd06bb4653cfc575f0f6bbf7038bf4bc") == (blockchain.INITIAL_DIFFICULTY / BigInteger(8)));
    REQUIRE(blockchain.currentDifficulty == blockchain.INITIAL_DIFFICULTY);
    assertTopBlock("e", 5, blockchain);

    result = addBlock("00002d44ede421cb9287e478999fd875ea84247b430d140d680d203a99854897",
            "00002007422630a845f547083b9bad6ffd06bb4653cfc575f0f6bbf7038bf4bc", BigInteger(201327), 750010, false, blockchain);
    REQUIRE(!result.valid);
    assertTopBlock("e", 5, blockchain);

    result = addBlock("000005cdad940869376620dd4174419cb50ff6b342494ed59a7f45c5648f58b5",
            "00002007422630a845f547083b9bad6ffd06bb4653cfc575f0f6bbf7038bf4bc", BigInteger(243011), 750010, false, blockchain);
    REQUIRE(result.valid);

    REQUIRE(blockchain.currentDifficulty == (blockchain.INITIAL_DIFFICULTY / BigInteger(8)));
    assertTopBlock("000005cdad940869376620dd4174419cb50ff6b342494ed59a7f45c5648f58b5", 6, blockchain);
}
