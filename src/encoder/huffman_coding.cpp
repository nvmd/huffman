#include <vector>
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include <map>
#include <iterator>
#include <intrin.h>
#include <tclap/CmdLine.h>

#include "huffman_encoder.hpp"

//#define INTERACTIVE_MODE
// try to avoid codes starting with 0
//#define TRY_TO_AVOID_ZERO_LEADING_BIT

/// Huffman tree node
struct huffman_node_t
{
	int   symbol;
	float probability;
	int   code;

	huffman_node_t *left_symbol;
	huffman_node_t *right_symbol;
};

struct huffman_node_comp_t
{
	bool operator()(const huffman_node_t *n1, const huffman_node_t *n2)
	{
		return n1->probability > n2->probability;
	}
};

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

/// Code nodes of Huffman tree
template <class T>
void huffman_coding(huffman_node_t *top, T &map)
{
	if (top->left_symbol)
	{
		top->left_symbol->code = (top->code << 1) + 1;
		huffman_coding(top->left_symbol, map);
	}
	
	if (top->right_symbol)
	{
		top->right_symbol->code = (top->code << 1) + 0;
		huffman_coding(top->right_symbol, map);
	}
	
	if (!top->left_symbol && !top->right_symbol)
	{
		map[top->symbol] = top->code;
		std::cout << top->symbol << " " << top->probability 
					<< " - " << convBase(top->code, 2) << std::endl;
	}
}

/// Convert symbols (probability vector) to Huffman tree nodes
template <class P, class Q>
void convert_prob_vect_to_huffman_nodes(const P &prob_vect, Q &queue)
{
	for (size_t i=0; i<prob_vect.size(); ++i)
	{
		huffman_node_t *new_node = new huffman_node_t;

		new_node->symbol = i+1;
		new_node->code = 0;
		new_node->probability = prob_vect[i];
		new_node->left_symbol = NULL;
		new_node->right_symbol = NULL;
		
		queue.push(new_node);
	}
}

/// Generate huffman tree
template <class Q>
void generate_huffman_tree(Q &queue)
{
	while (queue.size() > 1)
	{
		huffman_node_t *left = queue.top();
		queue.pop();
		huffman_node_t *right = queue.top();
		queue.pop();
		
		huffman_node_t *top = new huffman_node_t;

		top->symbol = 0;
		top->probability = left->probability + right->probability;
		top->code = 0;
		top->left_symbol = left;
		top->right_symbol = right;

		queue.push(top);
	}
}

template <class P, class C>
void do_huffman(const P &prob_vect, C &codes)
{
	// Convert symbols to huffman tree nodes
	std::priority_queue<huffman_node_t*, std::vector<huffman_node_t*>, huffman_node_comp_t> huffman_tree;
	convert_prob_vect_to_huffman_nodes(prob_vect, huffman_tree);

	// Generate huffman tree
	generate_huffman_tree(huffman_tree);
	
	std::map<int,int> code_map;	// symbol->code map
	huffman_node_t *top = huffman_tree.top();

#if !defined(TRY_TO_AVOID_ZERO_LEADING_BIT)
	top->code = 0;
	huffman_coding(top, code_map);
#else
	if (!top->left_symbol->left_symbol && !top->left_symbol->right_symbol)
	{
		top->left_symbol->code = 0;
		top->right_symbol->code = 1;
	}
	else
	{
		top->left_symbol->code = 1;
		top->right_symbol->code = 0;
	}

	huffman_coding(top->left_symbol, code_map);
	huffman_coding(top->right_symbol, code_map);
#endif

	for (std::map<int,int>::iterator i=code_map.begin(); i!=code_map.end(); ++i)
	{
		codes.at(i->first-1) = i->second;
	}
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
		std::cout << convBase(*i,2);
		T::const_iterator j = i;
		if (++j != vect.end())
		{
			stream << ", ";
		}
	}
	stream << ")" << std::endl;
}

template <class P, class C>
float coding_cost(const P &prob_vect, const C &codes)
{
	float cost = 0;
	C::const_iterator c = codes.begin();
	P::const_iterator p = prob_vect.begin();
	while (c != codes.end() && p != prob_vect.end())
	{
		unsigned long index = 0;
		if (_BitScanReverse(&index, *c) == 0)
		{
			index = 0;
		}
		++index;
		cost += *p * index;
		++c;
		++p;
	}
	return cost;
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
	std::cout << std::fixed;
	std::cout << "Exp.1: " << std::endl;
	output_vect(std::cout, "P", prob_vect);
	std::vector<int> codes(prob_vect.size());

	do_huffman(prob_vect, codes);

	output_vect_bin(std::cout, "V", codes);

	std::cout << "C = " << coding_cost(prob_vect, codes) <<std::endl;
	std::cout << std::endl;

	std::cout << "Exp.2: " << std::endl;
	output_vect(std::cout, "P", prob_vect);
	std::vector<float> prob_vect_merged;
	std::copy(prob_vect.begin(), prob_vect.end(), std::back_inserter(prob_vect_merged));
	
	float last = prob_vect_merged.back();
	prob_vect_merged.pop_back();
	prob_vect_merged.back() += last;

	output_vect(std::cout, "P'", prob_vect_merged);

	std::vector<int> codes_merged(prob_vect_merged.size());
	do_huffman(prob_vect_merged, codes_merged);

	output_vect_bin(std::cout, "V'", codes_merged);
	codes_merged.push_back(codes_merged.back());
	output_vect_bin(std::cout, "V", codes_merged);

	std::cout << "C' = " << coding_cost(prob_vect_merged, codes_merged) <<std::endl;

	// new encoder test
	std::cout << "NEW ENCODER" << std::endl;
	huffman_coding_ns::huffman_encoder_t encoder;
	std::cout << "Weighted path length: " << encoder(prob_vect, codes) << std::endl;
	
	
	return 0;
}
