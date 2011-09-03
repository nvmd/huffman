
#ifndef HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_
#define HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_

#include <cassert>
#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <algorithm>

#include <intrin.h>	// for _BitScanReverse

namespace huffman_coding
{

template <typename S = uint8_t, typename C = uint32_t>
class huffman_encoder_t
{
public:
	huffman_encoder_t()
	{}

	// outputs code C
	// returns weighted path length of code C
	template <class W, class C>
	float operator()(const W &weights, C &code)
	{
		encode(weights, code);
		return weighted_path_length(weights, code);
	}

	typedef S symbol_t;
	typedef C symbol_code_t;

protected:

	/// Huffman tree node
	struct huffman_node_t
	{
		symbol_t   symbol;
		float weight;
		symbol_code_t   code;

		huffman_node_t *left_symbol;
		huffman_node_t *right_symbol;
	};

	struct huffman_node_comp_t
	{
		bool operator()(const huffman_node_t *n1, const huffman_node_t *n2)
		{
			return n1->weight > n2->weight;
		}
	};

	/// Code nodes of Huffman tree
	template <class C>
	void huffman_coding(huffman_node_t *top, C &code)
	{
		if (top->left_symbol)
		{
			top->left_symbol->code = (top->code << 1) + 1;
			huffman_coding(top->left_symbol, code);
		}
	
		if (top->right_symbol)
		{
			top->right_symbol->code = (top->code << 1) + 0;
			huffman_coding(top->right_symbol, code);
		}
	
		if (!top->left_symbol && !top->right_symbol)
		{
			code[top->symbol] = top->code;
//#ifdef _DEBUG
			std::cout //<< "Encoded symbol: " 
				<< top->symbol << " " << top->weight 
				<< " - " << convBase(top->code, 2) //including leading "1"
				<< " - " << code_to_str(top->code) //only code itself
				<< " [" << code_length(top->code) 
				<< "]" << std::endl;
//#endif
		}
	}

	/// Convert symbols (probability vector) to Huffman tree nodes
	template <class W, class Q>
	void weights_to_huffman_nodes(const W &weights, Q &queue)
	{
		for (size_t i=0; i<weights.size(); ++i)
		{
			huffman_node_t *new_node = new huffman_node_t;

			new_node->symbol = i;
			new_node->code = 0;
			new_node->weight = weights[i];
			new_node->left_symbol = NULL;
			new_node->right_symbol = NULL;
		
			queue.push(new_node);
		}
	}

	template <class Q>
	void weights_to_huffman_nodes(const std::map<symbol_t,float> &weights, Q &queue)
	{
		std::for_each(weights.begin(), weights.end(), 
			[&queue](const std::pair<symbol_t,float> &symbol_weight){
			huffman_node_t *new_node = new huffman_node_t;

			new_node->symbol = symbol_weight.first;
			new_node->code = 0;
			new_node->weight = symbol_weight.second;
			new_node->left_symbol = NULL;
			new_node->right_symbol = NULL;
		
			queue.push(new_node);
		});
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
			top->weight = left->weight + right->weight;
			top->code = 0;
			top->left_symbol = left;
			top->right_symbol = right;

			queue.push(top);
		}
	}

	// outputs code C
	template <class W, class C>
	void encode(const W &weights, C &code)
	{
		// Convert symbols to huffman tree nodes
		std::priority_queue<huffman_node_t*, std::vector<huffman_node_t*>, huffman_node_comp_t> huffman_tree;
		weights_to_huffman_nodes(weights, huffman_tree);

		// Generate huffman tree
		generate_huffman_tree(huffman_tree);
	
		// Code huffman tree nodes
		huffman_node_t *top = huffman_tree.top();
		top->code = 1;	// add leading "1" to the code
		huffman_coding(top, code);
	}

	template <typename C>
	size_t code_length(C code)
	{
		// code can't be zero as actual code has a non-zero leading bit
		assert(code != 0);

		unsigned long length = 0;
		
		// symbol code always has one leading non-zero bit
		// (not part of the code itself)
#if defined(_M_X64)
		if (_BitScanReverse64(&length, code) == 0)
#else
		if (_BitScanReverse(&length, code) == 0)
#endif
		{
			length = 0;
		}

		return length;
	}

	// returns L(C) - weighted path length of code C
	template <class W, class C>
	float weighted_path_length(const W &weights, const C &code)
	{
		assert(weights.size() == code.size());

		float wlength = 0;

		for (size_t i=0; i<weights.size(); ++i)
		{
			wlength += weights[i] * code_length(code[i]);
		}

		return wlength;
	}

	float weighted_path_length(const std::map<symbol_t,float> &weights, 
								const std::map<symbol_t,symbol_code_t> &code)
	{
		assert(weights.size() == code.size());

		float wlength = 0;
		
		std::for_each(weights.begin(), weights.end(), 
			[&wlength, &code, this](const std::pair<symbol_t,float> &symbol_weight){
			// cant' write just
			// wlength += symbol_weight.second * code_length(code[symbol_weight.first]);
			// because std::map::operator[] is not const, while we want code to be const in this method
			wlength += symbol_weight.second * code_length(code.find(symbol_weight.first)->second);
		});

		return wlength;
	}
};

typedef huffman_encoder_t<uint8_t> int8_encoder_t;
typedef huffman_encoder_t<uint32_t> int32_encoder_t;

}

#endif	// HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_
