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

struct symbol_t
{
	int symbol;
	float probability;
	int code;
};

struct huffman_node_t
{
	symbol_t symbol;
	huffman_node_t *left_symbol;
	huffman_node_t *right_symbol;
};

struct huffman_node_comp_t
{
	bool operator()(const huffman_node_t *n1, const huffman_node_t *n2)
	{
		return n1->symbol.probability > n2->symbol.probability;
	}
};

struct coded_symbol_t
{
	int symbol;
	int code;
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

template <class T>
void huffman_coding(huffman_node_t *top, T &map)
{
	if (top->left_symbol)
	{
		top->left_symbol->symbol.code = (top->symbol.code << 1) + 1;
		huffman_coding(top->left_symbol, map);
	}
	
	if (top->right_symbol)
	{
		top->right_symbol->symbol.code = (top->symbol.code << 1) + 0;
		huffman_coding(top->right_symbol, map);
	}
	
	if (!top->left_symbol && !top->right_symbol)
	{
		map[top->symbol.symbol] = top->symbol.code;
		std::cout << top->symbol.symbol << " " << top->symbol.probability 
					<< " - " << convBase(top->symbol.code,2) << std::endl;
	}
}

template <class P, class C>
void do_huffman(const P &prob_vect, C &codes)
{
	// Convert symbols to huffman tree nodes
	std::priority_queue<huffman_node_t*, std::vector<huffman_node_t*>, huffman_node_comp_t> huffman_tree;
	size_t symb_count = 0;
	for (std::vector<float>::const_iterator i=prob_vect.begin(); i!=prob_vect.end(); ++i)
	{
		huffman_node_t *new_node = new huffman_node_t;
		new_node->symbol.symbol = ++symb_count;
		new_node->symbol.code = 0;
		new_node->symbol.probability = *i;
		new_node->left_symbol = NULL;
		new_node->right_symbol = NULL;
		huffman_tree.push(new_node);
	}

	// Generate huffman tree
	while (huffman_tree.size() > 1)
	{
		huffman_node_t *left = huffman_tree.top();
		huffman_tree.pop();
		huffman_node_t *right = huffman_tree.top();
		huffman_tree.pop();
		
		huffman_node_t *top = new huffman_node_t;
		top->symbol.symbol = 0;
		top->symbol.probability = left->symbol.probability+right->symbol.probability;
		top->symbol.code = 0;
		top->left_symbol = left;
		top->right_symbol = right;
		huffman_tree.push(top);
	}
	
	std::map<int,int> code_map;
	huffman_node_t *top = huffman_tree.top();
	/*top->symbol.code = 0;
	huffman_coding(top, code_map);*/
	if (!top->left_symbol->left_symbol && !top->left_symbol->right_symbol)
	{
		top->left_symbol->symbol.code = 0;
		top->right_symbol->symbol.code = 1;
	}
	else
	{
		top->left_symbol->symbol.code = 1;
		top->right_symbol->symbol.code = 0;
	}

	huffman_coding(top->left_symbol, code_map);
	huffman_coding(top->right_symbol, code_map);

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

int main(int argc, char **argv)
{
	size_t symbol_count = 4;

	do
	{
		std::cout << "Vector length: ";
		std::cin >> symbol_count;
		std::cout << std::endl;
	} while (symbol_count <= 0);

	std::vector<float> prob_vect(symbol_count);

#undef _DEBUG
#ifndef _DEBUG
	int prob_sum = 0;
	srand(time(NULL));
	for (size_t i=0; i<symbol_count; ++i)
	{
		int prob = rand()%101;
		prob_sum += prob;
		prob_vect[i] = prob;
	}
	for (std::vector<float>::iterator i=prob_vect.begin(); i!=prob_vect.end(); ++i)
	{
		*i /= static_cast<float>(prob_sum);
	}
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
	
	return 0;
}
