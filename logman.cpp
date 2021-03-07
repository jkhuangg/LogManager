#include <iostream>
#include <algorithm>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <deque>
#include <fstream>
#include <set>

using namespace std;

// Create strcasecmp() ONLY for people using Visual Studio. 
// Unix, Linux (CAEN) and MacOS already have it. 
#ifdef _MSC_VER 

inline int strcasecmp(const char *s1, const char *s2) {     
	return _stricmp(s1, s2); 
} 

#endif // _MSC_VER 

//helper func
int64_t ts_conv(string ts) {
	int64_t num = ts[ts.length() - 1] - '0';
	int64_t zrs = 10;
	for (size_t i = 1; i < ts.length(); ++i) {
		if (!isdigit(ts[ts.length() - 1 - i])) continue;
		num += (ts[ts.length() - 1 - i] - '0') * zrs;
		zrs *= 10;
	}
	return num;
}

void parse_str(string str, std::vector<string> &kw_vec) {
	kw_vec.clear();
	string kw;

	for (size_t i = 0; i < str.length(); ++i) {
		kw.clear();
		while (isalnum(str[i])) {
			kw.push_back(str[i]);
			++i;
		}

		if (!kw.empty()) kw_vec.push_back(kw);
	}
}

class LogEnt {
public: 
	LogEnt(string ts, string cat, string msg, int32_t entrid)
		: time_stamps{ ts }, category{ cat }, 
		  message{ msg }, entID{ entrid } {}
	
	string rtn_ts() {
		return time_stamps;
	}

	string rtn_cat() {
		return category;
	}

	string rtn_msg() {
		return message;
	}

	int32_t rtn_entID() {
		return entID;
	}
	
	int64_t rtn_ts_int() {
		return ts_int;
	}

	friend class cmpEnt;
	class cmpEnt {
	public:
		bool operator () (LogEnt *a, LogEnt *b) {
			if (a->ts_int == b->ts_int) {
				if (strcasecmp(a->category.c_str(), b->category.c_str()) == 0)
					return a->entID < b->entID;
				return strcasecmp(a->category.c_str(), b->category.c_str()) < 0;
			}
			return a->ts_int < b->ts_int;
		}
	};
	
private:
	string time_stamps;
	string category;
	string message;

	int64_t ts_int = ts_conv(time_stamps);
	int32_t entID;
};

struct cmpTimeStamp {
	bool operator() (LogEnt* a, LogEnt*b) {
		return a->rtn_ts_int() < b->rtn_ts_int();
	}
	bool operator()(LogEnt *a, int64_t num) {
		return a->rtn_ts_int() < num;
	}
	bool operator()(int64_t num, LogEnt *a) {
		return a->rtn_ts_int() > num;
	}
};


void print_LogEnt(LogEnt *a) {
	std::cout << a->rtn_entID() << "|"
		<< a->rtn_ts() << "|"
		<< a->rtn_cat() << "|"
		<< a->rtn_msg() << "\n";
}

struct LastResult {

	// store last search
	char last_s;
	// c
	string last_c;
	bool found_c;
	// t / m
	std::vector<LogEnt*>::iterator itr_b;
	std::vector<LogEnt*>::iterator itr_e;
	// k
	std::vector<int> last_k;
};

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "./logman <LOGFILE.txt>\n";
		exit(1);
	}

	if (!strcmp(argv[1], "h") || !strcmp(argv[1], "help")) {
		std::cout << "./logman <LOGFILE.txt>\n";
		exit(0);
	}

	ios_base::sync_with_stdio(false);
	std::vector<LogEnt*> master_log;
	std::ifstream infile(argv[1]);
	string ts, cat, msg;
	int entid = 0;
	while (std::getline(infile, ts, '|')) {
		std::getline(infile, cat, '|');
		std::getline(infile, msg);

		LogEnt *entr = new LogEnt(ts, cat, msg, entid);
		master_log.push_back(entr);
		++entid;
	}// finished puttting in master log

	// sort master log
	std::sort(master_log.begin(), master_log.end(), LogEnt::cmpEnt());

	// put in relevant details (in sorted order)
	std::unordered_map<string, vector<int32_t>> kw_log;
	std::unordered_map<string, vector<int32_t>> ct_log;
	std::vector<string> kw_dq;

	std::vector<int32_t> entID_vc;
	entID_vc.resize(master_log.size());
	for (int i = 0; size_t(i) < master_log.size(); ++i) {

		// entID index in masterlog
		entID_vc[master_log[i]->rtn_entID()] = i;

		// categories (SORTED ml order)
		string cat = master_log[i]->rtn_cat();
		string msg = master_log[i]->rtn_msg();
		std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
		std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
		
		// add in category
		ct_log[cat].push_back(i);

		// add keywords in SORTED ml order
		// keyword == categories
		parse_str(cat, kw_dq);
		while (!kw_dq.empty()) {
			if (kw_log[kw_dq.back()].empty() ||
				(!kw_log[kw_dq.back()].empty() &&
				kw_log[kw_dq.back()].back() != i))
				kw_log[kw_dq.back()].push_back(i);
			kw_dq.pop_back();
		}

		// keyword == messages
		parse_str(msg, kw_dq);
		while (!kw_dq.empty()) {
			if (kw_log[kw_dq.back()].empty() ||
				(!kw_log[kw_dq.back()].empty() &&
				kw_log[kw_dq.back()].back() != i))
				kw_log[kw_dq.back()].push_back(i);
			kw_dq.pop_back();
		}
	} // finished putting in relevant containers in SORTED ml order
	std::cout << master_log.size() << " entries read\n";

	std::vector<int32_t> expt_log;
	LastResult last_search;
	last_search.last_s = 'n';

	char cmd;
	do {
		std::cout << "% ";
		std::cin >> cmd;

		if (cmd == 'a') {
			size_t entid;
			std::cin >> entid;
			if (entid >= master_log.size()) {
				std::cerr << "not valid entry ID\n";
				continue;
			}

			expt_log.push_back(entID_vc[entid]);
			std::cout << "log entry " << entid << " appended\n";
		}
		else if (cmd == 't') {
			last_search.last_s = cmd;
			string ts1, ts2;
			std::getline(cin >> std::ws, ts1, '|');
			std::getline(cin >> std::ws, ts2);

			int64_t num1 = ts_conv(ts1);
			int64_t num2 = ts_conv(ts2);

			last_search.itr_b = std::lower_bound(master_log.begin(), master_log.end(), num1, cmpTimeStamp());
			last_search.itr_e = std::upper_bound(master_log.begin(), master_log.end(), num2, cmpTimeStamp());

			std::cout << "Timestamps search: " << (last_search.itr_e - last_search.itr_b) << " entries found\n";
		}
		else if (cmd == 'm') {
			last_search.last_s = cmd;
			string ts;
			std::cin >> ts;

			int64_t num = ts_conv(ts);
			last_search.itr_b = std::lower_bound(master_log.begin(), master_log.end(), num, cmpTimeStamp());
			last_search.itr_e = std::upper_bound(master_log.begin(), master_log.end(), num, cmpTimeStamp());

			std::cout << "Timestamp search: " << (last_search.itr_e - last_search.itr_b) << " entries found\n";
		}
		else if (cmd == 'c') {
			last_search.last_s = cmd;
			string cat;
			std::getline(cin >> std::ws, cat);
			std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
			
			if (ct_log.find(cat) == ct_log.end()) {
				last_search.found_c = false;
				std::cout << "Category search: 0 entries found\n";
				continue;
			}

			last_search.found_c = true;
			last_search.last_c = cat;
			std::cout << "Category search: " << ct_log[cat].size() << " entries found\n";
		}
		else if (cmd == 'p') {
			// print nothing if no search
			if (expt_log.empty()) continue;

			for (size_t i = 0; i < expt_log.size(); ++i) {
				std::cout << i << "|";
				print_LogEnt(master_log[expt_log[i]]);
			}
		}
		else if (cmd == 'g') {
			// print nothing if no search
			if (last_search.last_s == 'n') continue;

			// for t, m, c, k
			//std::vector<int32_t> rec_vc;
			if (last_search.last_s == 't' || last_search.last_s == 'm') {
				auto beg = last_search.itr_b;
				auto end = last_search.itr_e;

				int ml_b = int(beg - master_log.begin());
				int ml_e = int(end - master_log.begin());

				for (int i = ml_b; i < ml_e; ++i)
					print_LogEnt(master_log[i]);
			}
			else if (last_search.last_s == 'c') { 
				if (!last_search.found_c) continue;
				for (auto x : ct_log[last_search.last_c])
					print_LogEnt(master_log[x]);
			}
			else if (last_search.last_s == 'k') { 
				for (auto x : last_search.last_k)
					print_LogEnt(master_log[x]);
			}
		}
		else if (cmd == 'r') {
			// check for virginity
			if (last_search.last_s == 'n') continue;

			// for t, m, c, k
			if (last_search.last_s == 't' || last_search.last_s == 'm') {
				auto beg = last_search.itr_b;
				auto end = last_search.itr_e;

				int ml_b = int(beg - master_log.begin());
				int ml_e = int(end - master_log.begin());

				for (int i = ml_b; i < ml_e; ++i)
					expt_log.push_back(i);
				std::cout << (ml_e - ml_b) << " log entries appended\n";
			}
			else if (last_search.last_s == 'c') {
				if (!last_search.found_c) {
					std::cout << "0 log entries appended\n";
					continue;
				}
				for (auto x : ct_log[last_search.last_c])
					expt_log.push_back(x);
				std::cout << ct_log[last_search.last_c].size() << " log entries appended\n";
			}
			else if (last_search.last_s == 'k') {
				for (auto x : last_search.last_k)
					expt_log.push_back(x);
				std::cout << last_search.last_k.size() << " log entries appended\n";
			}
		}
		else if (cmd == 'd') {
			size_t pos_delete;
			std::cin >> pos_delete;
			if (pos_delete >= expt_log.size()) continue;

			expt_log.erase(expt_log.begin() + pos_delete);
			std::cout << "Deleted excerpt list entry " << pos_delete << "\n";
		}
		else if (cmd == 'b') {
			size_t pos_beg;
			std::cin >> pos_beg;
			if (pos_beg >= expt_log.size()) continue;

			int tmp = expt_log[pos_beg];
			expt_log.erase(expt_log.begin() + pos_beg);
			expt_log.insert(expt_log.begin(), tmp);
			std::cout << "Moved excerpt list entry " << pos_beg << "\n";
		}
		else if (cmd == 'e') {
			size_t pos_end;
			std::cin >> pos_end;
			if (pos_end >= expt_log.size()) continue;

			int tmp = expt_log[pos_end];
			expt_log.erase(expt_log.begin() + pos_end);
			expt_log.push_back(tmp);
			std::cout << "Moved excerpt list entry " << pos_end << "\n";
		}
		else if (cmd == 's') {
			std::cout << "excerpt list sorted\n";
			if (expt_log.empty()) {
				std::cout << "(previously empty)\n";
				continue;
			}

			// prev ordering
			std::cout << "previous ordering:\n";
			std::cout << "0|";
			print_LogEnt(master_log[expt_log[0]]);		
			std::cout << "...\n";		
			std::cout << expt_log.size() - 1 << "|";
			print_LogEnt(master_log[expt_log[expt_log.size() - 1]]);

			// sort
			std::sort(expt_log.begin(), expt_log.end());

			// new ordering
			std::cout << "new ordering:\n";
			std::cout << "0|";
			print_LogEnt(master_log[expt_log[0]]);
			std::cout << "...\n";
			std::cout << expt_log.size() - 1 << "|";
			print_LogEnt(master_log[expt_log[expt_log.size() - 1]]);
		}
		else if (cmd == 'l') {
			std::cout << "excerpt list cleared\n";
			if (expt_log.empty()) {
				std::cout << "(previously empty)\n";
				continue;
			}

			// prev contents
			std::cout << "previous contents:\n";
			std::cout << "0|";
			print_LogEnt(master_log[expt_log[0]]);
			std::cout << "...\n";
			std::cout << expt_log.size() - 1 << "|";
			print_LogEnt(master_log[expt_log[expt_log.size() - 1]]);

			//clear excerpt deque
			expt_log.clear();
		}
		else if (cmd == 'k') {
			last_search.last_s = cmd;
			string keyword;
			std::getline(cin >> std::ws, keyword);
			std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
			std::vector<string> key_vec;
			parse_str(keyword, key_vec);

			// first check if value is valid
			if (key_vec.empty()) {
				last_search.last_k.clear();
				std::cout << "Keyword search: 0 entries found\n";
				continue;
			}
			
			// then see if its in there
			if (kw_log.find(key_vec[0]) == kw_log.end()) {
				last_search.last_k.clear();
				std::cout << "Keyword search: 0 entries found\n";
				continue;
			}

			last_search.last_k = kw_log[key_vec[0]];
			std::vector<int32_t> tmp;
			for (size_t i = 1; i < key_vec.size(); ++i) {
				if (kw_log.find(key_vec[i]) == kw_log.end()) {
					last_search.last_k.clear();
					break;
				}

				tmp = last_search.last_k;
				last_search.last_k.clear();
				std::set_intersection(tmp.begin(), tmp.end(),
					kw_log[key_vec[i]].begin(), kw_log[key_vec[i]].end(),
					std::back_inserter(last_search.last_k));
			}

			std::cout << "Keyword search: " << last_search.last_k.size() << " entries found\n";
		}
		else if (cmd == '#') {
			string junk;
			getline(cin, junk);
		}	
	} while (cmd != 'q');

	
	for (auto x : master_log) delete x;
	return 0;
}
