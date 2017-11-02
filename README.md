# RADULS
RADULS is a fast multithreaded MSD radix sorter designed mainly to sort unsigned integers. This repository contains an example application using RADULS. 

# Requirements
RADULS may be compiled with GCC (6 or higher) or visual C++ (Visual Studio 2015 or higher). 
Additional memory of size of array to sort is required (RADULS is not in-place algorithm). 

# Usage
To use raduls one should include ```raduls.h``` file. All definitions are in ```raduls``` namespace. Currently there are two function available:

 * ```void raduls::CleanTmpArray(uint8_t* tmp, uint64_t n_recs, uint32_t rec_size, uint32_t n_threads);```
 
This function "touches" every memory page starting on address tmp of size n_recs\*rec_size bytes using n_threads. The rationale behind this function is that allocating a memory with ```new[]``` or ```malloc``` does not really givie the memory to the process (it is postponed by the operating system to the first use). This function should be used only if ```tmp``` memory was not yet touched by a program. See also the "On sorting time measurements" section for more explanation.

 * ```void raduls::RadixSortMSD(uint8_t* input, uint8_t* tmp, uint64_t n_recs, uint32_t rec_size, uint32_t key_size, uint32_t n_threads);```
 
This is the main sorting procedure. It uses ```n_threads``` threads. ```input``` points to an array of ```n_recs``` records of size ```rec_size``` bytes to sort. ```tmp``` points to an auxiliary array of the same size array pointed by ```input```. ```rec_size``` must be a multiple of 8. ```key_size``` is a number of bytes of a record that forms a key (i.e. the part of a record that is used while sorting). It must be lower or equal to ```rec_size```. In the simplest case a ```rec_size``` and ```key_size``` are equal and whole record forms a key. If they are not equal the first ```key_size``` bytes form a key, and the last ```floor((rec_size - key_size)/8)*8``` bytes form a data field. **The remaining bytes in the middle must be filled with zeros**. For example if ```rec_size``` is 16, and a key_size is 6 the record may be presented as follows:

```
-------------------------------------------------------------------------------------------------
|     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
|  K0 |  K1 |  K2 |  K3 |  K4 |  K5 |  0  |  0  |  D  |  D  |  D  |  D  |  D  |  D  |  D  |  D  |
|     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
-------------------------------------------------------------------------------------------------
```
where:
 - K0 -- least significant byte of a key, it is also a start address of a record (in case of first record in the array it is ```input``` address)
 - K5 -- most significant byte of a key
 - D -- byte belonging to data field
 
 The result of sorting may be in array pointed by ```input``` or ```tmp``` depending on the ```key_size```. If it is even the result will be in the array pointed by ```input```, in the oposite case (odd ```key_size```) the result will be in the array pointed by ```tmp```.
 
```input``` and ```tmp``` addresses must be divisible by 256.
## Simple Example

```
#include <iostream>
#include <chrono>
#include <random>
#include "raduls.h"

int main()
{
	uint64_t n_recs = 100000000;	
	uint32_t key_size = 8;
	std::mt19937_64 gen;
	std::uniform_int_distribution<uint64_t> dis(0, ~0ull);	
	auto _raw_input = new uint8_t[n_recs * sizeof(uint64_t) + raduls::ALIGNMENT];
	auto _raw_tmp = new uint8_t[n_recs * sizeof(uint64_t) + raduls::ALIGNMENT];
	auto input = reinterpret_cast<uint64_t*>(_raw_input);
	auto tmp = reinterpret_cast<uint64_t*>(_raw_tmp);
	while (reinterpret_cast<uintptr_t>(input) % raduls::ALIGNMENT) ++input;
	while (reinterpret_cast<uintptr_t>(tmp) % raduls::ALIGNMENT) ++tmp;
	for (uint64_t i = 0; i < n_recs; ++i)
	{		
		input[i] = dis(gen) & (~0ull >> ((8 - key_size)*8));
		//std::cerr << input[i] << " ";
	}
	auto n_threads = std::thread::hardware_concurrency();
	raduls::CleanTmpArray(reinterpret_cast<uint8_t*>(tmp), n_recs, sizeof(uint64_t), n_threads);
	auto start = std::chrono::high_resolution_clock::now();
	raduls::RadixSortMSD(reinterpret_cast<uint8_t*>(input), reinterpret_cast<uint8_t*>(tmp), n_recs, sizeof(uint64_t), key_size, n_threads);
	std::cerr << "\nTime: " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() << "\n";
	auto result = key_size % 2 ? tmp : input;
	//for (uint64_t i = 0; i < n_recs; ++i)
	//	std::cerr << result[i] << " ";
	delete[] _raw_input; delete[] _raw_tmp;
}
```

# On sorting time measurements
It is always an important issue how to measure the running time of functions that need some external memory. This is especially important in case of sorting of huge arrays using non-in-place algorithms that require a lot of additional memory (likely the same size as the input data). Operating systems vary in how they initialize huge memory blocks just after allocation (this also depends on the hardware). Sometimes the initialization is so slow that the time of first accesses to the memory cells can be comparable with the sorting time. From the other side, in typical scenarios, the memory allocated by some program will be used many times, so the additional cost of "touching" the memory will be paid only once. Thus, in our opinion, this is not a part of the sorting procedure and should not be included in the sorting time.

Therefore, in the experiments in which we evaluated sorting algorithms the time of "touching" the memory (with a use of ```CleanTmpArray```) are not taken into account.

# Exceptions
RADULS may throw following exceptions:
 * ```InputNotAlignedException``` - if ```input``` is not properly aligned
 * ```TempNotAlignedException``` - if ```tmp``` is not properly aligned
 * ```RecSizeNotMultipleOf8Exception``` - if ```rec_size``` is not multiple of 8
 * ```KeySizeGreaterThanRecSizeException``` - if ```key_size``` is greater than ```rec_size```
 * ```UsupportedRecSizeException``` - if ```rec_size``` is to big, currently it is set to 32, it may be extended changing the ```MAX_REC_SIZE_IN_BYTES``` in ```raduls.h``` file (it will lead to longer compilation time)

## Important remark
One may note that sorting_network.cpp is compiled with -O1 flag instead of higher optimization level. The reason is that GCC tends to produce branches in sorting network code when higher optimization level is specified, which leads to slower executable.

## Citation
If you found RADULS useful please cite us:

1. Kokot M., Deorowicz S., Debudaj-Grabysz A. (2017) Sorting Data on Ultra-Large Scale with RADULS. In: Kozielski S., Mrozek D., Kasprowski P., Małysiak-Mrozek B., Kostrzewa D. (eds) Beyond Databases, Architectures and Structures. Towards Efficient Solutions for Data Analysis and Knowledge Representation. BDAS 2017. Communications in Computer and Information Science, vol 716. Springer, Cham

2. Kokot M., Deorowicz S., Długosz M. (2018) Even Faster Sorting of (Not Only) Integers. In: Gruca A., Czachórski T., Harezlak K., Kozielski S., Piotrowska A. (eds) Man-Machine Interactions 5. ICMMI 2017. Advances in Intelligent Systems and Computing, vol 659. Springer, Cham
