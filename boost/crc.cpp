/**
 * crc tests
 * @author Tobias Weber
 * @date 7-may-20
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_73_0/doc/html/crc.html
 */

#include <iostream>
#include <cstdint>

#include <boost/crc.hpp>

/*
 * Polynomial mod division:
 x^7 + x^6 + x^5 + x^4 + x^3 +       x^1  :  x^3 + x^2  =  x^4 + x^2 + x^0
 x^7 + x^6
-----------------------------------------
             x^5 + x^4 + x^3 +       x^1
             x^5 + x^4
-----------------------------------------
                         x^3 +       x^1
                         x^3 + x^2
-----------------------------------------
remainder:                     x^2 + x^1  ->  0b110 = 6




* crctst1 & crctst2:

crc size 8 -> polynomial is 9 bits long, set msb (as boost.crc assumes a truncated polynomial)
add 8 zero bits to data

1111101000000000 : 100011100
100011100
-------------------
 111010000000000
 100011100
-------------------
  11001100000000
  100011100
-------------------
   1000010000000
   100011100
-------------------
       101000000
       100011100
-------------------
         1011100  ->  0x5c




* crctst3:

111110100000 : 11100
11100
------------
   110100000
   11100
------------
     1100000
     11100
------------
       10000
       11100
------------
        1100  ->  0x0c

*/


template<class T>
T mycrc(int poly_size, T poly, T data)
{
	int poly_start = poly_size-1;
	// find starting "1"
	for(; poly_start>=0; --poly_start)
	{
		if(poly & (1<<poly_start))
			break;
	}

	// no "1" in poly
	if(poly_start <= 0)
		return 0;

	data <<= poly_size-1;
	//std::cout << std::hex << data << std::endl;

	for(int i=sizeof(data)*8-1; i>=poly_size-1; --i)
	{
		if((data & (T{1}<<T(i))) == 0)
			continue;
		//std::cout << std::dec << "i = " << i << std::endl;

		data ^= (poly<<(i+1-poly_size));
	}

	return data;
}


int main()
{
	constexpr std::uint8_t poly = 0b0001'1100;
	constexpr std::uint8_t data = 0b1111'1010;
	constexpr std::uint8_t data_reversed = 0b0101'1111;

	std::cout << std::hex << "poly: " << (std::uint32_t)poly << "\n";
	std::cout << std::hex << "data: " << (std::uint32_t)data << "\n";
	std::cout << std::hex << "data % poly = " << (std::uint32_t)(data % poly) << "\n";
	std::cout << std::hex << "data ^ poly = " << (std::uint32_t)(data ^ poly) << "\n";
	std::cout << std::endl;

	{
		boost::crc_optimal<sizeof(poly)*8, poly, 0, 0, false, false> crctst1;
		crctst1.process_bytes(&data, sizeof(data));

		boost::crc_optimal<sizeof(poly)*8, poly, 0, 0, true, false> crctst2;
		crctst2.process_bytes(&data_reversed, sizeof(data_reversed));

		boost::crc_optimal<4, poly, 0, 0, false, false> crctst3;
		crctst3.process_bytes(&data, sizeof(data));

		std::cout << "trunc poly: " << std::hex
			<< (std::uint32_t)crctst1.get_truncated_polynominal() << std::endl;

		std::cout << "opt 1: " << std::hex << (std::uint32_t)crctst1.checksum() << std::endl;
		std::cout << "opt 2: " << std::hex << (std::uint32_t)crctst2.checksum() << std::endl;
		std::cout << "opt 3: " << std::hex << (std::uint32_t)crctst3.checksum() << std::endl;
	}


	{
		boost::crc_basic<sizeof(data)*8> crctst2{poly, 0, 0, false, false};
		crctst2.process_bytes(&data, sizeof(data));
		std::cout << "basic: " << std::hex << (std::uint32_t)crctst2.checksum() << std::endl;
	}


	{
		std::cout << "own 1: " << std::hex << (std::uint32_t)mycrc<unsigned>(5, poly, data) << std::endl;
		std::cout << "own 2: " << std::hex << (std::uint64_t)mycrc<unsigned>(5, poly, data) << std::endl;
	}

	return 0;
}
