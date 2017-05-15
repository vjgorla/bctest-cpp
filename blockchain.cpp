#include "bigInt.h"
#include "blockchain.h"
#include "sha256.h"

using std::string;
using std::cout;
using std::endl;
typedef BigInt::Rossi BigInteger;

const string Blockchain::ROOT_HASH = "0000000000000000000000000000000000000000000000000000000000000000";
const int Blockchain::RETARGET_BLOCK_INTERVAL = 100;
const BigInteger Blockchain::BASELINE_TS_INTERVAL = BigInteger(1000000);
const BigInteger Blockchain::DIFFICULTY_PRECISION = BigInteger(1000000);


Block::Block(string _prevBlockHash, unsigned long long _nonce, unsigned long long _ts, string _text) {
	prevBlockHash = _prevBlockHash;
	nonce = _nonce;
	ts = _ts;
	text = _text;
}

string Block::blockContentsString() {
	return prevBlockHash + ":" + std::to_string(nonce) + ":" + std::to_string(ts) + ":" + text;
}

string Block::blockString() {
	return blockHash + ":" + blockContentsString();
}

Blockchain::Blockchain() {
	INITIAL_DIFFICULTY = BigInteger::pow(BigInteger(2), BigInteger(256)) / BigInteger(100000);
    currentDifficulty = INITIAL_DIFFICULTY;
    retargetBlockInterval = Blockchain::RETARGET_BLOCK_INTERVAL;
    topBlockNumber = BigInteger(0);
    topBlockHash = Blockchain::ROOT_HASH;
}

AddBlockResult& Blockchain::addBlock(Block& block, bool mined) {
	AddBlockResult* result = new AddBlockResult();
    if (map.count(block.blockHash)) {
    	result->alreadyExists = true;
    	return *result;
    }
    if (block.prevBlockHash != ROOT_HASH && !map.count(block.prevBlockHash)) {
        cout << "Orphan - " << block.prevBlockHash << " does not exist" << endl;
    	result->isOrphan = true;
    	return *result;
    }
    if (!mined) {
        string blockContentsStr = block.blockContentsString();
        string blockHash = sha256(blockContentsStr);
        if (block.blockHash != blockHash) {
        	cout << "Invalid block hash " << block.blockString() << endl;
        	result->valid = false;
        	return *result;
        }
        BigInteger hashBigInt = BigInteger(block.blockHash, BigInt::HEX_DIGIT);;
        BigInteger targetDifficulty = calculateDifficulty(block.prevBlockHash);
        if (hashBigInt > targetDifficulty) {
        	cout << "Invalid difficulty" << endl;
        	result->valid = false;
        	return *result;
        }
    }
    map[block.blockHash] = &block;
    if (!descendantsMap.count(block.prevBlockHash)) {
    	descendantsMap[block.prevBlockHash] = new std::vector<Block*>();
    }
    std::vector<Block*>* descendants = descendantsMap.find(block.prevBlockHash)->second;
    descendants->push_back(&block);

    if (block.prevBlockHash == topBlockHash) {
        topBlockNumber = topBlockNumber + 1;
        cout << topBlockNumber.toStrDec() << (mined ? " > " : " < ") << block.blockString() << endl;
        topBlockHash = block.blockHash;
        setDifficulty(block);
    	result->valid = true;
    	return *result;
    } else {
        string ihash = block.prevBlockHash;
        int blockDepth = 0;
        while(true) {
            blockDepth++;
            int distance = findDistance(ihash, topBlockHash, 0);
            if (distance != -1) {
                if (blockDepth > distance) {
                    topBlockNumber = topBlockNumber - BigInteger(distance) + BigInteger(blockDepth);
                    cout << "...(" << blockDepth << ") " + topBlockNumber.toStrDec() << (mined ? " > " : " < ") << block.blockString() << endl;
                    topBlockHash = block.blockHash;
                    setDifficulty(block);
                } else {
                    cout << (mined ? " > " : " < ") << block.blockString() << endl;
                }
            	result->valid = true;
            	return *result;
            }
            ASSERTION(ihash != ROOT_HASH);
            ihash = map.find(ihash)->second->prevBlockHash;
        }
    }
    return *result;
}

BigInteger Blockchain::calculateDifficulty(string hash) {
    int height = calculateHeight(hash);
    if (height <= retargetBlockInterval) {
        return INITIAL_DIFFICULTY;
    }
    int fromHeight = height - ((height - 1) % retargetBlockInterval);
    int toHeight = fromHeight - retargetBlockInterval;
    long fromTs = 0;
    while (true) {
        Block block = *(map.find(hash)->second);
        if (height == fromHeight) {
            fromTs = block.ts;
        }
        if (height == toHeight) {
            long interval = fromTs - block.ts;
            return difficulty(interval, calculateDifficulty(block.blockHash));
        }
        hash = block.prevBlockHash;
        height--;
    }
}

void Blockchain::setDifficulty(Block& block) {
    BigInteger newDifficulty = calculateDifficulty(block.blockHash);
    if (newDifficulty != currentDifficulty) {
//        BigInteger oldD = _difficultyToDisplay(currentDifficulty);
//        BigInteger newD = _difficultyToDisplay(newDifficulty);
//        BigDecimal diff = new BigDecimal(newD.subtract(oldD).multiply(new BigInteger("100")))
//            .divide(new BigDecimal(oldD), 2, RoundingMode.HALF_UP);
//        System.out.println("Difficulty " + diff + "% ... " + oldD.toString() + " > " + newD.toString());
    	cout << "Difficulty ... " <<  currentDifficulty.toStrDec() << " > " << newDifficulty.toStrDec() << endl;
    }
    currentDifficulty = newDifficulty;
}

int Blockchain::findDistance(string fromHash, string toHash, int currentDepth) {
    if (fromHash == toHash) {
        return currentDepth;
    }
    if (!descendantsMap.count(fromHash)) {
        return -1;
    }
    std::vector<Block*>* descendants = descendantsMap.find(fromHash)->second;
    currentDepth++;
    for (std::vector<Block*>::iterator it = descendants->begin(); it != descendants->end(); ++it) {
        if ((*it)->blockHash == toHash) {
            return currentDepth;
        } else {
            int distance = findDistance((*it)->blockHash, toHash, currentDepth);
            if (distance != -1) {
                return distance;
            }
        }
    }
    return -1;
}

int Blockchain::calculateHeight(string hash) {
    int height = 0;
    while (hash != ROOT_HASH) {
    	Block block = *(map.find(hash)->second);
        height++;
        hash = block.prevBlockHash;
    }
    return height;
}

BigInteger Blockchain::difficulty(long interval, BigInteger prevDifficulty) {
    return (((BigInteger(interval) * DIFFICULTY_PRECISION) / BASELINE_TS_INTERVAL) * prevDifficulty) / DIFFICULTY_PRECISION;
}

std::vector<Block*>& Blockchain::getDescendants(string blockHash) {
	std::vector<Block*>* result = new std::vector<Block*>();
    getDescendants(blockHash, *result);
    return *result;
}

void Blockchain::getDescendants(string blockHash, std::vector<Block*>& result) {
    if (descendantsMap.count(blockHash)) {
        std::vector<Block*>* descendants = descendantsMap.find(blockHash)->second;
        for (std::vector<Block*>::iterator it = descendants->begin(); it != descendants->end(); ++it) {
			int distance = findDistance((*it)->blockHash, topBlockHash, 0);
			if (distance != -1) {
				result.push_back(&(**it));
				getDescendants((*it)->blockHash, result);
			}
        }
    }
}



