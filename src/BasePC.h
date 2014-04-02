#pragma once

#include "stdafx.h"

using namespace std;

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
		string dblspace = "  ";
		out << uppercase << setfill('0') << hex;
		out << setw(3) << hex << regs.ip << dblspace;
		out << setw(4) << regs.ir << dblspace;
		out << setw(3) << regs.ar << dblspace;
		out << setw(4) << regs.dr << dblspace;
		out << setw(4) << regs.accu << dblspace;
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
		bind_operations();
	}
	// copy vector
	BasePC(vector<unsigned short>& data, unsigned short start = 0) : BasePC() {
		assign(data, start);
	}
	// std::map or std::unordered_map
	template <class T>
	BasePC(T m, unsigned short start = 0) : BasePC() {
		assign(m, start);
	}
	// iterators
	template <class Iterator>
	BasePC(Iterator begin, Iterator end) : BasePC() {
		assign(begin, end);
	}
	// input file (txt or csv)
	explicit BasePC(const string& path) : BasePC() {
		parse_file(path);
	}

	void set_cell(unsigned short cell_number, unsigned short value) {
		memory.at(cell_number) = value;
	}
	void set_start(unsigned short cell_number) {
		ip = cell_number;
	}

	void configure() {
		bool char_instr = true; // char = false, instr = true
		unsigned short kr = 0; // key register
		while (true) {
			cout << "F3: enter instruction" << endl;
			cout << "F4: enter address" << endl;
			cout << "F5: record" << endl;
			cout << "F8: continue" << endl;
			char ch = _getch();
			switch (ch) {
				case 61: cout << "input the intruction/data: ";
						 cin >> hex >> kr;
						 break;
				case 62: ip = kr; break;
				case 63: memory[ip] = kr; break;
				case 66: cout << execute() << endl;
				default: break;
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

	void run(const string& html_path) {
		for (int attempt = 0; attempt < 5; ++attempt) {
			try {
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
					running = true;
					log << uppercase << setfill('0') << hex;

					while (running && ip < 2048) {
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
					return;
				}
			} catch (...) {
				cout << "Error with the output. Trying again..." << endl;
			}
		}
		cout << "Couldn't display the trace table." << endl;
	}

	//void run(const string& html_path) {
	//	running = true;
	//	list<Registers> trace_table;
	//	unordered_map<size_t, pair<unsigned short, unsigned short>> altered_cells;
	//	decltype(memory) current_memory = memory;
	//	unsigned short instruction = ip;
	//	while (running && ip < 2048) {
	//		trace_table.push_back(execute());
	//		for (size_t i = 0; i < 2048; ++i) {
	//			if (current_memory[i] != memory[i]) {
	//				altered_cells[trace_table.size() - 1] = { i, memory[i] };
	//				current_memory[i] = memory[i];
	//				break;
	//			}
	//		}
	//	}
	//	ofstream log(html_path);
	//	if (log.is_open()) {
	//		log << "<html>" << endl;
	//		log << "<head><style>td,th { border: 1px solid; text-align:center;  max-width: 100px; padding: 5px} th {font-family:\"Arial Narrow\";font-size:11px} td {font-family:\"courier new\"; font-size:13px;}</style></head>" << endl;
	//		log << "<body>" << endl;
	//		log << "<table style=\"border:1px solid;border-collapse:collapse;\">" << endl;
	//		log << "<tr><th colspan=2>Выполняемая команда</th>" << endl;
	//		log << "<th colspan=6>Содержимое регистров процессора после выполнения команды</th>" << endl;
	//		log << "<th colspan=2>Ячейка, содержимое кот-й изм-сь после вып-ния команды</th></tr>" << endl;
	//		log << "<tr><td>Адрес</td><td>Код</td><td>РК</td><td>РА</td><td>РД</td><td>А</td><td>С</td><td>СК</td><td>Адрес</td><td>Новое</td></tr>" << endl;
	//		log << uppercase << setfill('0') << hex;
	//		size_t instr_count = 0;
	//		for (auto &status : trace_table) {
	//			log << "<tr>";
	//			log << "<td>" << setw(3) << instruction << "</td>";
	//			log << "<td>" << setw(4) << current_memory[instruction] << "</td>";
	//			log << "<td>" << setw(4) << status.ir << "</td>";
	//			log << "<td>" << setw(3) << status.ar << "</td>";
	//			log << "<td>" << setw(4) << status.dr << "</td>";
	//			log << "<td>" << setw(4) << status.accu << "</td>";
	//			log << "<td>" << setw(1) << status.c << "</td>";
	//			log << "<td>" << setw(3) << status.ip << "</td>";
	//			if (altered_cells.count(instr_count)) {
	//				auto &altered_cell = altered_cells[instr_count];
	//				log << "<td>" << setw(3) << altered_cell.first << "</td>";
	//				log << "<td>" << setw(4) << altered_cell.second << "</td>";
	//			} else {
	//				log << "<td></td>";
	//				log << "<td></td>";
	//			}
	//			log << "</tr>" << endl;
	//			instruction = status.ip;
	//		}
	//		log << "</table>" << endl;
	//		log << "</body>" << endl;
	//		log << "</html>" << endl;
	//		log.close();
	//	} else {
	//		cout << "Error with the output" << endl;
	//		_getch();
	//	}
	//}

	void show_memory(unsigned short start = 0, unsigned short end = 2048) const {
		if (end > 2048) end = 2048;
		cout << "size: " << memory.size() << endl;
		cout << uppercase << setfill('0') << hex;
		for (unsigned short i = start; i < end; ++i) {
			cout << setw(3) << i << "  ";
			cout << setw(4) << memory[i] << endl;
		}
	}

	void show_memory_not_null(unsigned short stop_showing_after_n_zeros = 10) const {
		bool show = false;
		for (unsigned short i = 0; i < 2048; ++i) {
			if (!memory[i]) {
				if (show) {
					unsigned short j = i;
					for (; j < 2048; ++j) {
						if (memory[j]) break;
					}
					if (j == 2048) return;
				}
			} else if (!show) {
				show = true;
				cout << uppercase << setfill('0') << hex;
			}
			if (show) {
				cout << setw(3) << i << "  ";
				cout << setw(4) << memory[i] << endl;
			}
		}
	}

	void assign(vector<unsigned short>& data, unsigned short start = 0) {
		unsigned short i = 0;
		memory = data;
		ip = start;
	}

	template <class T>
	void assign(T m, unsigned short start = 0) {
		for (pair<const unsigned short, unsigned short> &a : m) {
			memory[a.first] = a.second;
		}
		ip = start;
	}

	template <class Iterator>
	void assign(Iterator begin, Iterator end) {
		auto mit = memory.begin();
		while (mit != memory.end()) {
			*mit = *begin;
			++begin;
			++mit;
		}
		if (it != end) {
			cout << "The input container has been truncated to 2048 elements" << endl;
		}
	}

	void parse_file(const string& path) {
		bool csv = (path.substr(path.length() - 4, 4) == ".csv" ? true : false);
		try {
			ifstream file(path.data());
			string line;
			bool find_start = true;
			bool plus_found = false;
			unsigned short memory_cell;
			if (file.is_open()) {
				while (getline(file, line)) {
					if (csv) line[line.find(';')] = ' ';
					if (find_start) {
						int pos = line.find('\+');
						if (pos > -1) {
							plus_found = true;
							line[pos] = ' ';
							if (pos && line[pos - 1] == '(' && line[pos + 1] == ')') {
								line[pos - 1] = line[pos + 1] = ' ';
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
			} else {
				throw runtime_error("Couldn't open file");
			}
		} catch (runtime_error e) {
			cout << e.what() << endl;
			_getch();
		} catch (...) {
			cout << "Couldn't parse file" << endl;
			_getch();
		}
	}

protected:

	unsigned short get_addr(unsigned short addr) const {
		if ((addr >> 11) & 1) {
			addr &= 0x7ff;
			return memory.at(addr);
		}
		return addr;
	}

	unsigned short fetch_addr(unsigned short instruction) const {
		return instruction & 0x7ff;
	}

	bool is_indirect() const {
		return (ir >> 11) & 1;
	}

	Registers execute() {
		ar = ip;
		dr = memory[ar];
		++ip;
		ir = dr;

		if (ir < 0xf000) {
			//if (is_indirect()) {
			//	ar = fetch_addr(ir);
			//	dr = memory[ar];
			//	// todo: finish (check for addition)
			//}
			unsigned char operation = ir >> 12;
			if (operation < 0x8) {
				ar = get_addr(ir & 0xfff); // ir or dr...
			}
			operations[operation]();
		} else {
			if (!(ir & 0xff)) {
				operations[ir >> 8]();
			}
		}
		return (Registers)*this;
	}

private:

	void and() {
		dr = memory[ar];
		accu &= dr;
	}
	void mov() {
		dr = accu;
		memory[ar] = dr;
		altered_cell = ar;
	}
	void add() {
		dr = memory[ar];
		if (accu < 0) {
			if (dr < 0 && (unsigned short)(-accu + -dr) > 32768) c = !c;
		} else {
			if (dr > 0 && (unsigned short)(accu + dr) >= 32768) c = !c;
		}
		//unsigned short prev_state = accu + 0x8000;
		accu += dr;
		//if ((unsigned short)(accu + 0x8000) < prev_state) c = !c;
	}
	void adc() {
		dr = memory[ar];
		accu += (dr + c);
	}
	void sub() {
		dr = memory[ar];
		if (accu < 0) {
			if (dr > 0 && accu - dr > 0) c = !c;
		} else {
			if (dr < 0 && accu - dr < 0) c = !c;
		}
		//unsigned short prev_state = accu + 0x8000;
		accu -= dr;
		//if ((unsigned short)(accu + 0x8000) > prev_state) c = !c;
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
		dr = memory[ar];
		++dr;
		memory[ar] = dr;
		if (dr >= 0) ++ip;
		altered_cell = ar;
	}
	void jsr() {
		// check this later, something's wrong
		//memory[ar] = ip;
		//ip = memory[ar] + 1;
		//altered_cell = ar;
		++ir;
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

	void bind_operations() {
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