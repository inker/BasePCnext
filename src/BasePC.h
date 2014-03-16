#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <functional>

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
		out << uppercase << setfill('0') << hex;
		out << setw(3) << hex << regs.ip << "  ";
		out << setw(4) << regs.ir << "  ";
		out << setw(3) << regs.ar << "  ";
		out << setw(4) << regs.dr << "  ";
		out << setw(4) << regs.accu << "  ";
		out << setw(1) << regs.c;
		return out;
	}
};

class BasePC : protected Registers {
	unordered_map<unsigned char, function<void()> > operations;
	vector<unsigned short> memory;
	bool running;
	short altered_cell;

public:
	BasePC() : Registers(), memory(2048, 0), running(false), altered_cell(-1) {
		bind_functions();
	}
	// starting address
	explicit BasePC(short start) : BasePC() {
		ip = start;
	}
	BasePC(vector<unsigned short> memory, unsigned short start = 0) : BasePC() {
		for (size_t i = 0; i < memory.size(); ++i) {
			this->memory[i] = memory[i];
		}
		ip = start;
	}
	BasePC(map<unsigned short, unsigned short> m, unsigned short start = 0) : BasePC() {
		for (auto &a : m) {
			memory[a.first] = a.second;
		}
		ip = start;
	}
	// input file (txt or csv)
	explicit BasePC(string path) : BasePC() {
		bool csv = false;
		if (path.substr(path.length() - 4, 4) == ".csv") {
			csv = true;
		}
		ifstream file (path.data());
		string line;
		bool find_start = true;
		bool plus_found = false;
		unsigned short memory_cell;

		while (getline(file, line)) {
			if (csv) line.replace(line.find(';'), 1, " ");
			if (find_start) {
				int pos = line.find('\+');
				if (pos > -1) {
					plus_found = true;
					if (pos && line[pos - 1] == '(' && line[pos + 1] == ')') {
						line[pos - 1] = line[pos] = line[pos + 1] = ' ';
					} else {
						line[pos] = ' ';
					}
				}
			}
			istringstream iss;
			iss.str(line);
			iss >> hex >> memory_cell;
			iss >> hex >> memory[memory_cell];
			if (find_start && plus_found) {
				ip = memory_cell;
				find_start = false;
			}
		}
		file.close();
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
		log << uppercase << setfill('0') << hex;
		while (running) {
			log << "<tr>";
			log << "<td>" << setw(3) << ip << "</td>";
			log << "<td>" << setw(4) << memory[ip] << "</td>";
			Registers status = execute();
			log << "<td>" << setw(4) << status.ir << "</td>";
			log << "<td>" << setw(3) << status.ar << "</td>";
			log << "<td>" << setw(4) << status.dr << "</td>";
			log << "<td>" << setw(4) << status.accu << "</td>";
			log << "<td>" << setw(1) << status.c << "</td>";
			log << "<td>" << setw(3) << status.ip << "</td>";
			if (altered_cell > -1) {
				log << "<td>" << setw(3) << altered_cell << "</td>";
				log << "<td>" << setw(4) << memory[altered_cell] << "</td>";
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
		cout << uppercase << setfill('0') << hex;
		for (unsigned short i = start; i < end; ++i) {
			cout << setw(3) << i << "  ";
			cout << setw(4) << memory[i] << endl;
		}
	}

protected:

	unsigned short& get_addr(unsigned short addr) {
		if ((addr >> 11) & 1) {
			addr &= 0x7ff;
			return memory.at(addr);
		}
		return addr;
	}

	unsigned short fetch_addr(unsigned short instruction) {
		return instruction & 0x7ff;
	}

	Registers execute() {
		ar = ip;
		dr = memory[ar];
		++ip;
		ir = dr;

		unsigned char operation = ir >> 8;
		
		if (operation < 0xf0) {
			operation >>= 4;
			bool is_arithm = (operation < 8 && operation > 2 || operation == 1);
			if (is_arithm) {
				ar = get_addr(ir & 0xfff); // ir or dr...
			}
			operations[operation]();
		} else {
			if (!(ir & 0xff)) {
				operations[operation]();
			}
		}
		return (Registers)*this;
	}

private:

	void and() {
		accu &= memory[ar];
	}
	void mov() {
		dr = accu;
		memory[ar] = dr;
		altered_cell = ar;
	}
	void add() {
		dr = memory[ar];
		unsigned short prev_state = accu + 0x8000;
		accu += dr;
		if ((unsigned short)accu + 0x8000 < prev_state) c = !c;
	}
	void adc() {
		dr = memory[ar];
		accu += (dr + c);
	}
	void sub() {
		dr = memory[ar];
		unsigned short prev_state = accu + 0x8000;
		accu -= dr;
		if ((unsigned short)accu + 0x8000 > prev_state) c = !c;
	}

	void bcs() {
		if (c) ip = fetch_addr(dr);
	}
	void bpl() {
		if (accu >= 0) ip = fetch_addr(dr);
	}
	void bmi() {
		if (accu < 0) ip = fetch_addr(dr);
	}
	void beq() {
		if (!accu) ip = fetch_addr(dr);
	}
	void br() {
		//ip = ar;
		ip = fetch_addr(dr);
	}
	void isz() {
		++memory[ar];
		if (memory[ar] >= 0) ++ip;
		altered_cell = ar;
	}
	void jsr() {
		// check this later, something's wrong
		memory[ar] = ip;
		ip = memory[ar] + 1;
		altered_cell = ar;
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

	void bind_functions() {
		//operations[0x4] = [this]() {
		//	auto val = memory[ar];
		//	unsigned short prev_state = accu + 0x8000;
		//	accu += val;
		//	if ((unsigned short)accu + 0x8000 < prev_state) c = !c;
		//};
		operations[0x0] = bind(&BasePC::isz, this);
		operations[0x1] = bind(&BasePC::and, this);
		operations[0x2] = bind(&BasePC::jsr, this);
		operations[0x3] = bind(&BasePC::mov, this);
		operations[0x4] = bind(&BasePC::add, this);
		operations[0x5] = bind(&BasePC::adc, this);
		operations[0x6] = bind(&BasePC::sub, this);
		operations[0x8] = bind(&BasePC::bcs, this);
		operations[0x9] = bind(&BasePC::bpl, this);
		operations[0xa] = bind(&BasePC::bmi, this);
		operations[0xb] = bind(&BasePC::beq, this);
		operations[0xc] = bind(&BasePC::br, this);
		operations[0xf0] = bind(&BasePC::hlt, this);
		operations[0xf1] = bind(&BasePC::nop, this);
		operations[0xf2] = bind(&BasePC::cla, this);
		operations[0xf3] = bind(&BasePC::clc, this);
		operations[0xf4] = bind(&BasePC::cma, this);
		operations[0xf5] = bind(&BasePC::cmc, this);
		operations[0xf6] = bind(&BasePC::rol, this);
		operations[0xf7] = bind(&BasePC::ror, this);
		operations[0xf8] = bind(&BasePC::inc, this);
		operations[0xf9] = bind(&BasePC::dec, this);
		operations[0xfa] = bind(&BasePC::ei, this);
		operations[0xfb] = bind(&BasePC::di, this);
	}
};