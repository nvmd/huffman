
#ifndef HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_
#define HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_

#include <cassert>

namespace huffman_coding
{

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

protected:

	// outputs code C
	template <class W, class C>
	void encode(const W &weights, C &code)
	{
		
	}

	template <typename C>
	size_t code_length(C code)
	{
		unsigned long length = 0;

		if (_BitScanReverse(&length, code) == 0)
		{
			length = 0;
		}

		return ++length;
	}

	// returns L(C) - weighted path length of code C
	template <class W, class C>
	float weighted_path_length(const W &weights, const C &code)
	{
		float wlength = 0;

		assert(weights.size() == code.size());

		for (size_t i=0; i<weights.size(); ++i)
		{
			wlength += weights[i] * code_length(code[i]);
		}

		return wlength;
	}
};

};

#endif	// HUFFMAN_CODING_HUFFMAN_ENCODER_HPP_
