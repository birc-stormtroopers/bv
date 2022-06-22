# Bit-vectors

There are times when we wish to store one bit of information about each index in a range `0, 1, ..., n-1`, for whatever reason, and for that we can use *bit vectors*.

Bit vectors are just arrays of bits, but to implement one, you often need a little bit more work than a normal array. The one exception I can think of is C++ where a `std::vector<bool>` is guaranteed by the standard to be a bit vector. Normally, though, an array of boolean values is not.

The thing is, most languages have a minimal size that an object can have, and that size is larger than a single bit. In C, for example, the smallest size an object can have is whatever the size a `char` has. The standard says `sizeof(char) == 1` and sizes can't be smaller than one. In practise, although the standard doesn't guarantee it, this is a byte, so eight bits. To store value for `n` bits, you then might try an array of `bool`

```c
#include <stdbool.h> // to get the bool type

...

bool my_array[n]; // <- an attempt at a bit vector
```

but the `my_array` is `n` bytes long, not `n` bits. If `n` is a billion, you are using a gigabyte memory where 125 megabytes would suffice for `n` boolean values.

In Python it is even worse. If you have a list of `n` `bool` values (`True` or `False`),

```python
my_list = [True] * n
```

the list really stores pointers to objects. If it only stores pointers to either the `True` or the `False` object, we can ignore the data the pointers point to, but the list will contain `n` pointers plus some overhead, and pointers on [most common architectures](https://en.wikipedia.org/wiki/X86-64) are 64 bits. So, for each bit you want to store, you are using 64 plus some change.

This doesn't matter much for small `n`; we have plenty of space to spare on modern computers, but if you are working with large data, such as full genomes, it does become noticable. Let's say we wanted to store one bit of information for each of the nucleotides in the human chromosome 1. There are about 250 billion nucleotides and we would use 250Mb (250 mega bytes) if we used a byte for each nucleotide, 16Gb (250 times 64 mega) if we used 64 bits / 8 bytes for each, and only 31Mb (250 / 8 when we pack 8 bits into each byte) if we used a bit to represent a bit.

You might have 16Gb on your computer, but programs usually need memory for more than holding a bit per nucleotide, so using 16Gb just for this bit vector might prevent you from doing whatever it is your program is supposed to do. If you only need 31Mb, however, you can usually squeeze that in. (Maybe not on the computers I grew up with, but today it shouldn't be a problem).

## What's the problem with bits?

The reason the computer doesn't readily lets you make array of bits is that the computer doesn't *work* with bits, contrary to what popular culture might suggest. A computer doesn't work with ones and zeros; it works with words and what it can address.

Let's look at what we can address first. If you have memory in RAM (and we only care about RAM right now), then you have addresses starting from zero and going up to as far as you have memory[^1]. These addresses do not point out individual bits. You can't ask the computer to give you bit 2315662 in RAM. The addresses are in some larger quantum, which on modern computers is bytes. You can address your memory in bytes, and then you get a whole byte back to work with.

It doesn't have to be bytes, you could have computers that would only give you 64-bit words, and then that would be all you can address, but bytes is what the x86_64 architectures give you, and that is the architecture you are probably using today.

Once you get data from RAM, you can work on it on the CPU, and here you also don't work on individual bits. The data you get goes into a register, and you generally work with whole registers at a time. I say generally, because it isn't always that easy for historical reasons, and you might work with sub-registers at times, but this is a mess that I don't want to get into. While it isn't entirely accurate, think of the data as going into a register, that lets you compute on the data there. The size of registers is what we sometimes call a computer word. These are typically larger than a byte, so you can have data from more than one address here, but if you want to fill a full register from RAM the CPU knows how to get the right bytes.

On the x86_64 architecture, most registers are 64 bits. (Most, because there are special registers for working with floats and vectors of floats, but the general purpose ones have 64 bits). That means that the minimal quantity of data we can work on is actually 64 bits. If we want to work on single bytes, we can still do that; we can put a single byte at the low 8 bits of a 64-bit register and put the remaining bits to zero. But we are always paying for 64-bit operations, even if we only care about eight of the bits.

This lowlevel stuff is not something we need to think about that often, and most of it we don't need to think about here either, but keep it in mind, because it affects how we can work with individual bits.

Consider the figure below, where at the top/right I have shown values we might have in RAM starting at some address `offset`. That means that the first hardware address we have at `offset` is `offset + 1`. The next is at `offset + 1` and is one byte or eight bits later. Then the next is `offset + 2` and so on. The computer doesn't let you access the individual bits, because we cannot address at fractions like `offset + 2/8`, but you can get a whole byte at a time.

The computer also works with larger word sizes than bytes. For an x86_64 architecture you can also work with 16, 32, and 64 bit words (and larger for some registers). As I mentioned above, you are actually always working with 64 bits on the CPU, it is just that you can use an eight, a quarter, half of, or a full register for your instructions. The larger word sizes span multiple bytes. A 16-bit word consists of two bytes, a 32-bit word of four bytes, and a 64-bit word of eight bytes. When you address them on the hardware, you use the byte addresses, though. So the first 16-bit word sits at `offset + 0` and the next at `offset + 2`.

You usually don't see it that way, because high-level programming languages understand that if you ask for the second value in an array, `A[1]`, you want the second value of that type. You don't, for example, want the second half of the first 16-bit word and the first half of the second 16-bit word. So programming languages work out how to map from an array address and an integral number of elements in. If we have a 16-bit word array that starts at `offset`, then the first would be at `offset + 0`, the second at `offset + 2`, the third at `offset + 4` and so on, but we would index them as `A[0]`, `A[1]`, `A[2]` and so on. The `((uint16_t*)A)[1]` bit in the figure is C syntax for saying "I want to consider `A` an array of 16-bit unsigned integers, and then I want the second element in it". It translates into finding the second 16-bit word (`offset + 2` byte) and fetching two bytes. Similar for the `((uint32_t*)A)[0]` where we want the first 32-bit integer (it sits at `offset + 0` and the next would sit at `offset + 4`) and we fetch that.

A slight aside here: in the figure, I have accessesd a 16-bit word that sits at offset 2, so a whole number of 16-bit words into the array, and I have accessed a 32-bit word at offset 0, a whole number of 32-bit words into the array (although the whole number is zero). Some architectures will only allow you to extract 16-, 32-, or 64-bit words if they sit at such offets, i.e. byte offsets that are multiples of the word sizes in bytes (2 for 16-bit words, 4 for 32-bit words, and 8 for 64-bit words). This is called [*alignment*](https://en.wikipedia.org/wiki/Data_structure_alignment). If you tried to get a word with the wrongly aligned address, the program would crash. Other architectures will freely let you access memory as multi-byte words at other offsets, x86_64 included, but there might be performance penalties to it.

Anyway, alignment is not important for the topic of bit-vectors; I just thought I would mention it. For arrays of more complex data types, it sometimes is important.

But back to bytes and words. As we see, we cannot address individual bits, but we can address words of various sizes, that are multiples of bytes.

![Addressing for different types.](figs/bv/addresses-and-interpretation.png)

If the words are composed of multiple bytes, we run into another issue. If we interpret, say, four bytes as a single 32-bit word, which byte should then contain the most significant bits and which should contain the least significant bits? It will not surprise you to learn that hardware people have gone with both putting the first byte first in the larger word and with putting it last. This is called [endianess](https://en.wikipedia.org/wiki/Endianness) (after Jonathan Swift's Gulliver's Travels, where there is a war over which end of an egg you should put on top when eating soft-boiled eggs). One choice isn't intrinsically better than another, and you usually don't have to worry about it--the hardware will take care of managing the bits so 2-, 4-, or 8-byte integers behave the way they should. If you need to manipulate multi-byte words as their individual bytes, however, you need to consider this. It can, for example, come up when you transfer data in binary between different computers, where the computers might interpret the integers differently. For our use today, though, we won't need to consider this, but it is part of the picture.

![Endianess.](figs/bv/endianess.png)

So, to boil all this down: if we want to manipulate data, we can get it in chunks of 1, 2, 4, or 8 (or maybe more) bytes, but not bits. If we use a `bool array[n]` array in C, we get one byte per value, and we thus use eight time as much memory was we need. For `[True] * n` in Python, we use 4 bytes, or 64 bits, instead of one bit, per entry. We are going to improve on that.




[^1]: But not more than $2^{64} \approx 1.8\times 10^{19}$ with 64-bit pointers, and actually less on modern 64-bit systems, but this limit is far higher than the limit set by your physical memory available.


