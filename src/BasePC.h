#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

//class Cell {
//	unsigned short cell;
//	static vector<Cell>& memory;
//public:
//	Cell() {}
//	Cell(unsigned short val) : cell(val) {}
//	operator unsigned short() {
//		return cell;
//	}
//	unsigned char operation() {
//		return cell >> 12;
//	}
//	// get full address (3 bits)
//	unsigned char operand() {
//		return cell & 0xfff;
//	}
//	// get final address
//	unsigned char address() {
//		unsigned char fa = operand();
//		if (fa >> 11) {
//			fa &= 0xf7ff;
//			return memory.at(memory[fa]);
//		}
//		return fa;
//	}
//	Cell& operator *() {
//		return memory[cell];
//	}
//};

struct Registers {
	unsigned short ip;	// instruction pointer
	unsigned short ir;	// instruction register
	unsigned short ar;	// address register
	short dr;			// data register
	short accu;// accumulator
	bool c;				// carry

	Registers() : accu(0), dr(0), ar(0), ip(0), ir(0) {}
	
	Registers(short ip, short ir, short ar, short dr, short accu, bool c) :
		ip(ip), ir(ir), ar(ar), dr(dr), accu(accu), c(c) {}

	friend ostream& operator <<(ostream& out, Registers &regs) {
		out << uppercase;
		out << setfill('0') << setw(3) << hex << regs.ip << "  ";
		out << setfill('0') << setw(4) << hex << regs.ir << "  ";
		out << setfill('0') << setw(3) << hex << regs.ar << "  ";
		out << setfill('0') << setw(4) << hex << regs.dr << "  ";
		out << setfill('0') << setw(4) << hex << regs.accu << "  ";
		out << setfill('0') << setw(1) << hex << regs.c;
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
		cout << "after IP   IR    AR   DR    ACCU  C" << endl;
		while (running) {
			cout << setfill('0') << setw(3) << hex << ip << "   ";
			cout << execute() << endl;
		}
	}

	void run(string html_path) {
		ofstream log(html_path);
		if (log.is_open()) {
			log << "<html>" << endl;
			log << "<head><style>td,th { border: 1px solid; text-align:center;  max-width: 100px; padding: 5px} th {font-family:\"Arial Narrow\";font-size:11px} td {font-family:\"courier new\"; font-size:13px;}</style></head>" << endl;
			log << "<body>" << endl;
			log << "<table style=\"border:1px solid;border-collapse:collapse;\">" << endl;
			log << "<tr><th colspan=2>Выполняемая команда</th>" << endl;
			log << "<th colspan=6>Содержимое регистров процессора после выполнения команды</th>" << endl;
			log << "<th colspan=2>Ячейка, содержимое кот-й изм-сь после вып-ния команды</th></tr>" << endl;
			log << "<tr><td>Адрес</td><td>Код</td><td>РК</td><td>РА</td><td>РД</td><td>А</td><td>С</td><td>СК</td><td>Адрес</td><td>Новое</td></tr>" << endl;
		}
		running = true;
		while (running) {

			log << uppercase;
			log << "<tr>";
			log << "<td>" << setfill('0') << setw(3) << hex << ip << "</td>";
			log << "<td>" << setfill('0') << setw(4) << hex << memory[ip] << "</td>";
			decltype(memory) old (memory.begin(), memory.end());
			Registers status = execute();
			log << "<td>" << setfill('0') << setw(4) << hex << status.ir << "</td>";
			log << "<td>" << setfill('0') << setw(3) << hex << status.ar << "</td>";
			log << "<td>" << setfill('0') << setw(4) << hex << status.dr << "</td>";
			log << "<td>" << setfill('0') << setw(4) << hex << status.accu << "</td>";
			log << "<td>" << setfill('0') << setw(1) << hex << status.c << "</td>";
			log << "<td>" << setfill('0') << setw(3) << hex << status.ip << "</td>";
			bool no_change = true;
			for (unsigned short i = 0; i < memory.size(); ++i) {
				if (memory[i] != old[i]) {
					log << "<td>" << setfill('0') << setw(3) << hex << i << "</td>";
					log << "<td>" << setfill('0') << setw(4) << hex << memory[i] << "</td>";
					no_change = false;
					break;
				}
			}
			if (no_change) {
				log << "<td></td>";
				log << "<td></td>";
			}
			log << "</tr>" << endl;
		}
		log << "</table>" << endl;
		log << "</body>" << endl;
		log << "</html>" << endl;
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
		unsigned char operation = instruction >> 12;
		pair<unsigned short, unsigned short> mem(ip, memory[ip]);
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
				++ip;
				return Registers (ip, mem.second, mem.first, instruction, accu, c);
			}
		}
		unsigned short addr = instruction & 0xfff;
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
		return Registers (ip, mem.second, get_addr(addr), get_value(addr), accu, c);
	}

	unsigned short& get_addr(unsigned short addr) {
		if ((addr >> 11) & 1) {
			addr &= 0xf7ff;
			return memory.at(addr);
		}
		return addr;
	}

	unsigned short& get_value(short addr) {
		return memory[get_addr(addr)];
	}

	void and(unsigned short addr) {
		accu &= get_value(addr);
	}
	void mov(unsigned short addr) {
		get_value(addr) = accu;
	}
	void add(unsigned short addr) {
		auto val = get_value(addr);
		auto prev_state = accu;
		accu += val;
		if (accu < prev_state) c = !c;
	}
	void adc(unsigned short addr) {
		accu += (get_value(addr) + c);
	}
	void sub(unsigned short addr) {
		auto val = get_value(addr);
		auto prev_state = accu;
		accu -= val;
		if (accu > prev_state) c = !c;
	}
	void bcs(unsigned short addr) {
		if (c) ip = get_addr(addr);
	}
	void bpl(unsigned short addr) {
		if (accu >= 0) ip = get_addr(addr);
	}
	void bmi(unsigned short addr) {
		if (accu < 0) ip = get_addr(addr);
	}
	void beq(unsigned short addr) {
		if (!accu) ip = get_addr(addr);
	}
	void br(unsigned short addr) {
		ip = get_addr(addr);
	}
	void isz(unsigned short addr) {
		++get_value(addr);
		if (get_value(addr) >= 0) ++ip;
	}
	void jsr(unsigned short addr) {
		// check this later, something's wrong
		get_value(addr) = ip;
		ip = get_addr(addr) + 1;
	}
	void cla() {
		(short&)accu = 0;
	}
	void clc() {
		c = false;
	}
	void cma() {
		accu = ~accu;
	}
	void cmc() {
		c = !c;
	}
	void rol() {
		bool temp = c;
		c = accu >> 15;
		accu <<= 1;
		accu |= temp;
	}
	void ror() {
		unsigned short temp = c;
		c = accu & 1;
		accu >>= 1;
		(unsigned short&)accu |= (temp << 15);
	}
	void inc() {
		++accu;
		if (!accu) c = !c;
	}
	void dec() {
		if (!accu) c = !c;
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
