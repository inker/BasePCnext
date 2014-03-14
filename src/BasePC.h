#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

struct Registers {
	int accu; // accumulator
	short dr; // data register
	unsigned short ar; // address register
	unsigned short ip; // instruction pointer
	unsigned short ir; // instruction register

	Registers() : accu(0), dr(0), ar(0), ip(0), ir(0) {}
	
	Registers(short ip, short ir, short ar, short dr, int accu) :
		ip(ip), ir(ir), ar(ar), dr(dr), accu(accu) {}

	friend ostream& operator <<(ostream& out, Registers &regs) {
		out << uppercase;
		out << setfill('0') << setw(3) << hex << regs.ip << "  ";
		out << setfill('0') << setw(4) << hex << regs.ir << "  ";
		out << setfill('0') << setw(3) << hex << regs.ar << "  ";
		out << setfill('0') << setw(4) << hex << regs.dr << "  ";
		out << setfill('0') << setw(4) << hex << short(regs.accu) << "  ";
		out << setfill('0') << setw(1) << hex << short(bool(regs.accu >> 16));
		return out;
	}
};

class BasePC : protected Registers {
	vector<unsigned short> memory;
	bool running;
public:
	BasePC() : Registers(), memory(2048, 0), running(false) {}
	// starting address
	explicit BasePC(short start) : BasePC() {
		ip = start;
	}
	BasePC(vector<unsigned short> memory, short start = 0) : BasePC() {
		for (size_t i = 0; i < memory.size(); ++i) {
			this->memory[i] = memory[i];
		}
		ip = start;
	}
	// input file
	explicit BasePC(string path) : BasePC() {
		ifstream file (path.data());
		string line;
		bool find_start = true;
		
		unsigned short memory_cell;

		while (getline(file, line)) {
			istringstream iss;
			iss.str(line);
			iss >> hex >> memory_cell;
			iss >> hex >> memory[memory_cell];
			if (find_start && (int)line.find('\+') > -1) {
				ip = memory_cell;
				find_start = false;
			}
		}
	}

	void run() {
		running = true;
		cout << "IP   IR    AR   DR    ACCU  C" << endl;
		while (running) {
			cout << execute() << endl;
		}
	}

	void run(string html_path) {
		ofstream log(html_path);
		if (log.is_open()) {
			log << "<html>" << endl;
			log << "<head></head>" << endl;
			log << "<body>" << endl;
			log << "<table>" << endl;
		}
		running = true;
		while (running) {
			Registers status = execute();
		}
		log.close();
	}

	void show_memory(unsigned short start = 0, unsigned short end = 2048) {
		cout << "size: " << memory.size() << endl;
		cout << uppercase;
		for (unsigned short i = start; i < end; ++i) {
			cout << setfill('0') << setw(3) << hex << i << "  ";
			cout << setfill('0') << setw(4) << hex << memory[i] << endl;
		}
	}
private:
	Registers execute() {
		unsigned short instruction = memory[ip];
		char operation = instruction >> 12;
		short addr = instruction & 0xfff;
		if (operation == 0xf) {
			if (!(instruction & 0xff)) {
				operation = (instruction >> 8) & 0xf;
				switch (operation) {
					case 0x0: hlt(); break;
					case 0x1: nop(); break;
					case 0x2: cla(); break;
					case 0x3: clc(); break;
					case 0x4: cma(); break;
					case 0x5: cmc(); break;
					case 0x6: rol(); break;
					case 0x7: ror(); break;
					case 0x8: inc(); break;
					case 0x9: dec(); break;
					case 0xa: ei(); break;
					case 0xb: di(); break;
					default: break;
				}
				Registers regs(ip, memory[ip], ip, instruction, accu);
				++ip;
				return regs;
			}
		}
		switch (operation) {
			case 0x0: isz(addr); break;
			case 0x1: and(addr); break;
			case 0x2: jsr(addr); break;
			case 0x3: mov(addr); break;
			case 0x4: add(addr); break;
			case 0x5: adc(addr); break;
			case 0x6: sub(addr); break;
				//case 0x7:
			case 0x8: bcs(addr); break;
			case 0x9: bpl(addr); break;
			case 0xa: bmi(addr); break;
			case 0xb: beq(addr); break;
			case 0xc: br(addr); break;
				//case 0xd: 
				//case 0xe: 
			default: break;
		}
		if (operation < 8) ++ip;
		return Registers( ip, memory[ip], get_addr(addr), get_value(addr), accu );
	}

	

	bool& carry() {
		return *((bool*)&accu + 2);
	}
	short& get_value(short addr) {
		cout << memory.size() << endl;
		if ((addr >> 11) & 1) {
			return (short&)memory.at(memory[addr]);
		}

		return (short&)memory[addr];
	}
	unsigned short& get_addr(unsigned short addr) {
		if ((addr >> 11) & 1) {
			return memory.at(addr);
		}
		return addr;
	}
	void and(short addr) {
		accu &= get_value(addr);
	}
	void mov(short addr) {
		get_value(addr) = short(accu);
	}
	void add(short addr) {
		accu += get_value(addr);
	}
	void adc(short addr) {
		accu += (get_value(addr) + (bool)carry());
	}
	void sub(short addr) {
		accu -= get_value(addr);
	}
	void bcs(short addr) {
		if (carry()) ip = get_addr(addr);
	}
	void bpl(short addr) {
		if (accu >= 0) ip = get_addr(addr);
	}
	void bmi(short addr) {
		if (accu < 0) ip = get_addr(addr);
	}
	void beq(short addr) {
		if (!accu) ip = get_addr(addr);
	}
	void br(short addr) {
		ip = get_addr(addr);
	}
	void isz(short addr) {
		++get_value(addr);
		if (get_value(addr) >= 0) ++ip;
	}
	void jsr(short addr) {
		// check this later, something's wrong
		get_value(addr) = ip;
		ip = get_addr(addr) + 1;
	}
	void cla() {
		(short&)accu = 0;
	}
	void clc() {
		carry() = 0;
	}
	void cma() {
		accu = ~accu;
	}
	void cmc() {
		carry() = !carry();
	}
	void rol() {
		bool temp = carry();
		carry() = accu >> 15;
		accu <<= 1;
		accu |= temp;
	}
	void ror() {
		short temp = carry();
		carry() = accu & 1;
		accu >>= 1;
		accu |= (temp << 15);
	}
	void inc() {
		++accu;
	}
	void dec() {
		--accu;
	}
	void hlt() {
		running = false;
		// include no more commands to follow flag
	}
	void nop() {
		// probably increment the ip
	}
	void ei() {
		// todo
	}
	void di() {
		// todo
	}
};
