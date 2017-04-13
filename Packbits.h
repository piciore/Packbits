#ifndef PACKBITS_H
#define PACKBITS_H

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

class Packbits {
protected:
	std::istream& is;
	std::ostream& os;
	char state = 'b';
	uint8_t runLength = 0;
	uint8_t L;
	std::vector<uint8_t> buffer;

	void writeRun(uint8_t val)
	{
		L = 257 - runLength;
		os.write(reinterpret_cast<char*>(&L), 1);
		os.write(reinterpret_cast<char*>(&val), 1);
		runLength = 0;
	}
	void writeBuffer()
	{
		L = buffer.size() - 1;
		os.write(reinterpret_cast<char*>(&L), 1);
		for (auto ch : buffer) {
			os.write(reinterpret_cast<char*>(&ch), 1);
		}
		buffer.clear();
	}
public:
	/*Constructor*/
	Packbits(std::istream& is_, std::ostream& os_) : is(is_), os(os_){}
	/*Compressione*/
	void compress() {
		uint8_t prec;
		uint8_t current=0;
		std::istream_iterator<uint8_t> init(is);
		std::istream_iterator<uint8_t> eos;

		while (init!= eos) {
			prec = current;
			current = *init;
			init++;
			switch (state) {
			case 'b':
				buffer.push_back(current);
				state = 'f';
				break;
			case 'f':
				if (current == prec) {
					runLength=2;
					buffer.clear();
					state = 'r';
				}
				else {
					buffer.push_back(current);
					state = 'c';
				}
				break;
			case 'r':
				if (current == prec) {
					runLength++;
					if (runLength == 128) {
						writeRun(current);
						current = 0;
						state = 'c';
					}
				}
				else {
					writeRun(prec);
					buffer.push_back(current);
					state = 'c';
				}
				break;
			
			case 'c':
				if (current == prec) {
					buffer.pop_back();
					runLength = 2;
					if (buffer.size() != 0) {
						writeBuffer();
					}
					state = 'r';
				}
				else {
					buffer.push_back(current);
					if (buffer.size() == 129) {
						buffer.pop_back();
						writeBuffer();
						buffer.push_back(current);
					}
				}
				break;
			}
		}

		switch (state) {
		case 'r':
			writeRun(prec);
			break;
		case 'f':
			writeBuffer();
			break;
		case 'c':
			writeBuffer();
			break;
		default: break;
		}

		/*End Of Data*/
		L = 128;
		os.write(reinterpret_cast<char*>(&L), 1);
	}
	/*Decompressione*/
	void decompress() {
		std::istream_iterator<uint8_t> init(is);
		std::istream_iterator<uint8_t> eos;
		uint8_t readChar;
		L = *init;
		uint8_t i=0;
		while (L!=128) {
			if (L >= 0 && L <= 127) {
				/*copia*/
				for (i = 0; i <= L; i++) {
					is.read(reinterpret_cast<char*>(&readChar), 1);
					os.write(reinterpret_cast<char*>(&readChar), 1);
				}
			}
			else {
				/*run*/
				is.read(reinterpret_cast<char*>(&readChar), 1);
				for (; L!=1; L++) {
					os.write(reinterpret_cast<char*>(&readChar), 1);
				}
			}
			init++;
			L = *init;
		}
	}
	~Packbits() {}
};

class SmartPackbits : public Packbits {
public:
	/*Constructor*/
	SmartPackbits(std::istream& is_, std::ostream& os_) : Packbits(is_, os_) {}
	/*Polymorphic compression*/
	void compress() {
		uint8_t prec;
		uint8_t current = 0;
		uint8_t suspendedChar;
		std::istream_iterator<uint8_t> init(is);
		std::istream_iterator<uint8_t> eos;

		while (init != eos) {
			prec = current;
			current = *init;
			init++;
			switch (state) {
			case 'b':
				buffer.push_back(current);
				state = 'f';
				break;
			case 'f':
				if (current == prec) {
					runLength = 2;
					buffer.clear();
					state = 'R';
				}
				else {
					buffer.push_back(current);
					state = 'c';
				}
				break;
			case 'r':
				if (current == prec) {
					runLength++;
					if (runLength == 128) {
						writeRun(current);
						current = 0;
						state = 'c';
					}
				}
				else {
					writeRun(prec);
					buffer.push_back(current);
					state = 'c';
				}
				break;
			case 'c':
				if (current == prec) {
					if (buffer.size() >= 1) {
						buffer.push_back(current);
						state = 'R';
						if (buffer.size() == 129) {
							buffer.pop_back();
							writeBuffer();
							buffer.push_back(current);
							state = 'c';
						}		
					}
				}
				else {
					buffer.push_back(current);
					int size = buffer.size();
					if (buffer.size() == 129) {
						buffer.pop_back();
						writeBuffer();
						buffer.push_back(current);
					}
				}
				break;
			case 'R':
				if (current == prec) {
					if (buffer.size() > 2) {
						buffer.pop_back();
						buffer.pop_back();
						writeBuffer();
					}
					buffer.clear();
					runLength = 3;
					state = 'r';
				}
				else {
					buffer.push_back(current);
					suspendedChar = prec;
					if (buffer.size() == 129) {
						buffer.pop_back();
						writeBuffer();
						buffer.push_back(current);
					}
					state = 'd';
				}
				break;
			case 'd':
				if (current == prec) {
					buffer.pop_back();
					buffer.pop_back();
					buffer.pop_back();
					writeBuffer();
					runLength = 2;
					writeRun(suspendedChar);
					runLength = 2;
					state = 'r';
				}
				else {
					buffer.push_back(current);
					state = 'c';
					if (buffer.size() == 129) {
						buffer.pop_back();
						writeBuffer();
						buffer.push_back(current);
					}
				}
				break;
			}
		}

		switch (state) {
		case 'r':
			writeRun(prec);
			break;
		case 'f':
			writeBuffer();
			break;
		case 'c':
			writeBuffer();
			break;
		case 'R':
			buffer.pop_back();
			buffer.pop_back();
			if (buffer.size() > 0)
				writeBuffer();
			runLength = 2;
			writeRun(current);
			break;
		case 'd':
			writeBuffer();
			break;
		default: break;
		}

		/*End Of Data*/
		L = 128;
		os.write(reinterpret_cast<char*>(&L), 1);
	}
};

#endif /*PACKBITS_H*/