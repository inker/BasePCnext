#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

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
	unsigned short ip;	// instruction pointer	(11 bits)
	unsigned short ir;	// instruction register (16 bits)
	unsigned short ar;	// address register		(11 bits)
	short dr;			// data register		(16 bits)
	short accu;			// accumulator			(16 bits)
	bool c;				// carry				(1 bit)

	Registers() : accu(0), dr(0), ar(0), ip(0), ir(0) {}
	
	Registers(unsigned short ip, unsigned short ir, unsigned short ar, short dr, short accu, bool c) :
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
	short altered_cell;

public:
	BasePC() : Registers(), memory(2048, 0), running(false), altered_cell(-1) {}
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
	BasePC(map<unsigned short, unsigned short> m, short start = 0) : BasePC() {
		for (auto &a : m) {
			memory[a.first] = a.second;
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
			Registers status = execute();
			log << "<td>" << setfill('0') << setw(4) << hex << status.ir << "</td>";
			log << "<td>" << setfill('0') << setw(3) << hex << status.ar << "</td>";
			log << "<td>" << setfill('0') << setw(4) << hex << status.dr << "</td>";
			log << "<td>" << setfill('0') << setw(4) << hex << status.accu << "</td>";
			log << "<td>" << setfill('0') << setw(1) << hex << status.c << "</td>";
			log << "<td>" << setfill('0') << setw(3) << hex << status.ip << "</td>";
			if (altered_cell > -1) {
				log << "<td>" << setfill('0') << setw(3) << hex << altered_cell << "</td>";
				log << "<td>" << setfill('0') << setw(4) << hex << memory[altered_cell] << "</td>";
				altered_cell = -1;
			} else {
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

protected:

	unsigned short& get_addr(unsigned short addr) {
		if ((addr >> 11) & 1) {
			addr &= 0xf7ff;
			return memory.at(addr);
		}
		return addr;
	}

	//unsigned short& get_value(short addr) {
	//	return memory[get_addr(addr)];
	//}

	Registers execute() {
		ir = memory[ip]; // may need replacement to get_value(ip)
		unsigned char operation = ir >> 12;
		//pair<unsigned short, unsigned short> mem(ip, ir); // changing back to get_value(ip) may be needed
		auto ip_prev = ip;
		if (operation == 0xf) {
			ar = ip_prev;
			if (!(ir & 0xff)) {
				operation = (ir >> 8) & 0xf;
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
			}
			++ip;
			dr = ir;
		} else {
			ar = get_addr(ir & 0xfff);
			switch (operation) {
				case 0x0: isz(ar); break;
				case 0x1: and(ar); break;
				case 0x2: jsr(ar); break;
				case 0x3: mov(ar); break;
				case 0x4: add(ar); break;
				case 0x5: adc(ar); break;
				case 0x6: sub(ar); break;
					//case 0x7: doesn't even exist
				case 0x8: bcs(ar); break;
				case 0x9: bpl(ar); break;
				case 0xa: bmi(ar); break;
				case 0xb: beq(ar); break;
				case 0xc: br(ar); break;
					//case 0xd: 
					//case 0xe: 
				default: break;
			}
			if (operation < 8 && operation > 2 || operation == 1) {
				dr = memory[ar];
				++ip;
			} else {
				dr = ir;
			}
		}
		return (Registers)*this;
	}

private:

	void and(unsigned short addr) {
		accu &= memory[addr];
	}
	void mov(unsigned short addr) {
		memory[addr] = accu;
		altered_cell = addr;
	}
	void add(unsigned short addr) {
		auto val = memory[addr];
		unsigned short prev_state = accu + 0x8000;
		accu += val;
		if ((unsigned short)accu + 0x8000 < prev_state) c = !c;
	}
	void adc(unsigned short addr) {
		accu += (memory[addr] + c);
	}
	void sub(unsigned short addr) {
		auto val = memory[addr];
		unsigned short prev_state = accu + 0x8000;
		accu -= val;
		if ((unsigned short)accu + 0x8000 > prev_state) c = !c;
	}

	void bcs(unsigned short addr) {
		if (c) ip = addr;
	}
	void bpl(unsigned short addr) {
		if (accu >= 0) ip = addr;
	}
	void bmi(unsigned short addr) {
		if (accu < 0) ip = addr;
	}
	void beq(unsigned short addr) {
		if (!accu) ip = addr;
	}
	void br(unsigned short addr) {
		ip = addr;
	}
	void isz(unsigned short addr) {
		++memory[addr];
		if (memory[addr] >= 0) ++ip;
		altered_cell = addr;
	}
	void jsr(unsigned short addr) {
		// check this later, something's wrong
		memory[addr] = ip;
		ip = memory[addr] + 1;
		altered_cell = addr;
	}
	void cla() {
		accu = 0;
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
		if (!(accu + 0x8000)) c = !c;
	}
	void dec() {
		if (!(accu + 0x8000)) c = !c;
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
