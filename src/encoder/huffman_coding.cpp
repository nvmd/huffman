#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include <tclap/CmdLine.h>

#include "huffman_encoder.hpp"

//#define INTERACTIVE_MODE

std::string convBase(unsigned long v, long base)
{
	std::string digits = "0123456789abcdef";
	std::string result;
	if((base < 2) || (base > 16)) {
		result = "Error: base out of range.";
	}
	else {
		do {
			result = digits[v % base] + result;
			v /= base;
		}
		while(v);
	}
	return result;
}

template <typename C>
std::string code_to_str(C code)
{
	return convBase(code, 2).substr(1);
}

template <class S, class T>
void output_vect(S &stream, std::string name, const T &vect)
{
	stream << name << " = (";
	for (T::const_iterator i=vect.begin(); i!=vect.end(); ++i)
	{
		std::cout << *i;
		T::const_iterator j = i;
		if (++j != vect.end())
		{
			stream << ", ";
		}
	}
	stream << ")" << std::endl;
}

template <class S, class T>
void output_vect_bin(S &stream, std::string name, const T &vect)
{
	stream << name << " = (";
	for (T::const_iterator i=vect.begin(); i!=vect.end(); ++i)
	{
		std::cout << code_to_str(*i);
		T::const_iterator j = i;
		if (++j != vect.end())
		{
			stream << ", ";
		}
	}
	stream << ")" << std::endl;
}

/// Generate random probability vector
template <class P>
void generate_rand_prob_vect(P &prob_vect, size_t vect_size)
{
	int prob_sum = 0;
	srand(time(NULL));
	
	for (size_t i=0; i<vect_size; ++i)
	{
		int prob = rand()%101;
		prob_sum += prob;
		prob_vect[i] = prob;
	}

	for (size_t i=0; i<vect_size; ++i)
	{
		prob_vect[i] /= static_cast<float>(prob_sum);
	}
}

int main(int argc, char **argv)
{
	size_t symbol_count = 4;

#if defined(INTERACTIVE_MODE)
	do
	{
		std::cout << "Vector length: ";
		std::cin >> symbol_count;
		std::cout << std::endl;
	} while (symbol_count <= 0);
#else
	try
	{
		TCLAP::CmdLine cmd("Huffman coding", ' ', "0.0");

		TCLAP::ValueArg<size_t> symbol_count_arg("n", "symbol-count", 
										"Symbols count (probability vector length)", 
										true, 
										symbol_count, 
										"unsigned integer", 
										cmd);
		
		// parse command line
		cmd.parse(argc, argv);
		symbol_count = symbol_count_arg.getValue();
	}
	catch(TCLAP::ArgException &excp)
	{
		std::cerr << "TCLAP Error: " << excp.error();
		return 1;
	}
#endif

	std::vector<float> prob_vect(symbol_count);

#undef _DEBUG
#ifndef _DEBUG
	// Generate random probability vector
	generate_rand_prob_vect(prob_vect, symbol_count);
#else

#define ADD_PROB(symb,prob) \
	{\
		prob_vect.at(symb-1) = prob;\
	}\

	/*ADD_PROB(1,0.2);
	ADD_PROB(2,0.35);
	ADD_PROB(3,0.05);
	ADD_PROB(4,0.4);*/

	/*ADD_PROB(4,0.5);
	ADD_PROB(2,0.2);
	ADD_PROB(3,0.2);
	ADD_PROB(1,0.1);*/

	ADD_PROB(1,0.1);
	ADD_PROB(2,0.2);
	ADD_PROB(3,0.7);
	ADD_PROB(4,0.1);
#endif

	huffman_coding::huffman_encoder_t<uint8_t> encoder;
	float weighted_path_length = 0;

	std::cout << std::fixed;

	// Experiment #1
	std::cout << "Exp.1: " << std::endl;
	output_vect(std::cout, "P", prob_vect);
	std::vector<int> code(prob_vect.size());

	weighted_path_length = encoder(prob_vect, code);

	output_vect_bin(std::cout, "V", code);

	std::cout << "C = " << weighted_path_length <<std::endl;
	std::cout << std::endl;

	// Experiment #2
	std::cout << "Exp.2: " << std::endl;
	output_vect(std::cout, "P", prob_vect);
	std::vector<float> prob_vect_merged;
	prob_vect_merged.reserve(prob_vect.size());
	std::copy(prob_vect.begin(), prob_vect.end(), std::back_inserter(prob_vect_merged));
	
	float last = prob_vect_merged.back();
	prob_vect_merged.pop_back();
	prob_vect_merged.back() += last;

	output_vect(std::cout, "P'", prob_vect_merged);

	std::vector<int> code_merged(prob_vect_merged.size());
	
	weighted_path_length = encoder(prob_vect_merged, code_merged);

	output_vect_bin(std::cout, "V'", code_merged);
	code_merged.push_back(code_merged.back());
	output_vect_bin(std::cout, "V", code_merged);

	std::cout << "C' = " << weighted_path_length <<std::endl;

	typedef huffman_coding::huffman_encoder_t<uint32_t> int32_encoder_t;
	int32_encoder_t int32_encoder;
	std::map<int32_encoder_t::symbol_t,float> prob_map;
	prob_map[1] = 0.3;
	prob_map[400] = 0.1;
	prob_map[34] = 0.3;
	prob_map[42] = 0.1;
	prob_map[6543] = 0.2;
	std::map<int32_encoder_t::symbol_t,int32_encoder_t::symbol_code_t> code_map;
	weighted_path_length = int32_encoder(prob_map, code_map);

	return 0;
}
