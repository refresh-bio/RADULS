#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <sstream>
#include "defs.h"
template<typename T>
struct Opt
{
	T val;
	std::string str, meaning;
	Opt(const std::string& str, const std::string& meaning, T _default = T()) :
		val(_default), str(str), meaning(meaning)
	{ }
};

template<typename T>
struct SaveOpt
{
	static void Save(const std::string& in, Opt<T>& opt)
	{
		std::istringstream iss(in.substr(opt.str.length()));
		iss >> opt.val;
	}
};

template<>
struct SaveOpt<bool>
{
	static void Save(const std::string& in, Opt<bool>& opt)
	{
		opt.val = !opt.val;
	}
};

class Params
{
	bool is_opt(const std::string& in, const std::string& opt)
	{
		auto len = opt.length();
		if (opt.length() < len)
			return false;
		for (decltype(len) i = 0; i < len; ++i)
			if (in[i] != opt[i])
				return false;
		return true;
	}

	template<typename T>
	void set_option(const std::string& in, Opt<T>& opt)
	{
		if (!is_opt(in, opt.str))
			return;
		SaveOpt<T>::Save(in, opt);
	}

	template<typename T>
	void PrintOpt(const Opt<T>& opt)
	{
		std::cerr << "  " << std::setw(18) << std::left << opt.str << " - " << opt.val << "\n";
	}
	
	template<typename T>
	void HelpForOpt(const Opt<T>& opt)
	{
		std::cerr <<"  " << std::setw(18) <<std::left << opt.str << " - " << opt.meaning << "\n";
	}

public:
	Opt<uint64_t> n_recs = Opt<uint64_t>("-n_recs", "number of records to sort, may be ommited if -i specified");
	Opt<uint32_t> n_threads = Opt<uint32_t>("-n_threads", "number of threads");
	Opt<uint32_t> rec_size = Opt<uint32_t>("-rec_size", "rec size in bytes (must by a multiple of 8)");
	Opt<uint32_t> key_size = Opt<uint32_t>("-key_size", "size of a key part of a record in bytes");
	Opt<std::string> input = Opt<std::string>("-i", "input file, if not specified random input");
	Opt<std::string> output = Opt<std::string>("-o", "output file, if not specified output ignored");
	Opt<bool> full_validation = Opt<bool>("-full_validation", "performd full validation of result after sorting");	
	Opt<bool> clear_tmp = Opt<bool>("-c", "disable touching tmp array before sorting", true);
	Params(int argc, char **argv)
	{
		
		if (argc == 1 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-help")
		{
			Help(argv[0]);
			exit(1);
		}
		for (int i = 0; i < argc; ++i)
		{
			std::string in = argv[i];
			set_option(in, n_recs);
			set_option(in, n_threads);
			set_option(in, rec_size);
			set_option(in, key_size);
			set_option(in, full_validation);			
			set_option(in, clear_tmp);
			set_option(in, input);
			set_option(in, output);
		}

		bool err = false;
		if (n_recs.val == 0 && input.val == "")
		{
			std::cerr << "Error: n_recs or input must be set\n";
			err = true;
		}
		if (input.val != "")
		{
			auto f = fopen(input.val.c_str(), "rb");
			fseek(f, 0, SEEK_END);
			auto n_bytes = ftell(f);
			if (n_recs.val == 0)
				n_recs.val = n_bytes / rec_size.val;

			decltype(n_bytes) expected_n_bytes = n_recs.val * rec_size.val;

			if (expected_n_bytes != n_bytes)
			{
				std::cerr << "This input file do not contain specified number of records\n";
				err = true;
			}
			fclose(f);
		}
		if (n_threads.val == 0)
			n_threads.val = std::thread::hardware_concurrency();

		if (rec_size.val == 0)
		{
			std::cerr << "Error: rec_size must be set\n";
			err = true;
		}

		if (key_size.val == 0)
		{
			std::cerr << "Error: key_size must be set\n";
			err = true;
		}

		if (rec_size.val % 8)
		{
			std::cerr << "Error: rec_size must be a multiple of 8\n";
			err = true;
		}
		if (key_size.val > rec_size.val)
		{
			std::cerr << "Error: key_size must not be greater than rec_size\n";
			err = true;
		}

		if (err)
		{
			Help(argv[0]);
			exit(1);
		}
	}

	void Help(const std::string& exe)
	{
		std::cerr << "RADULS ver. " << RADULS_VER << " ("<< RADULS_DATE <<")\n";
		std::cerr << "Usage:\n";
		std::cerr << exe << " [options]\n";
		std::cerr << "Options:\n";
		std::cerr << "Mandatory:\n";
		HelpForOpt(n_recs);		
		HelpForOpt(rec_size);
		HelpForOpt(key_size);
		std::cerr << "Other:\n";
		HelpForOpt(full_validation);
		HelpForOpt(clear_tmp);
		HelpForOpt(n_threads);
		HelpForOpt(input);
		HelpForOpt(output);

		std::cerr << "Example:\n";
		std::cerr << exe << " -n_recs1000000000 -rec_size16 -key_size8 -n_threads12 -full_validation\n";
		
	}
	void Print()
	{
		PrintOpt(n_recs);
		PrintOpt(n_threads);
		PrintOpt(rec_size);
		PrintOpt(key_size);
		PrintOpt(full_validation);
		PrintOpt(clear_tmp);
		PrintOpt(input);
		PrintOpt(output);
	}
};

