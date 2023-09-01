// This is a Proof-of-Concept (POC) project that demonstrates
// secure coding practices when programming binary
// serialization & de-serialization in C++.
//
// Copyright (c) 2023, by dennisbabkin.com
//
//
// This project is used in the following blog post:
//
//  "Secure Programming Practices - Serialization"
//  "Example of secure binary serialization and de-serialization in C++."
//
//   https://dennisbabkin.com/blog/?i=AAA12200
//


//Custom declarations
#pragma once

#include <intrin.h>

#include <cmath>




#define ALIGN_BY (sizeof(void*))    //Align by this number of bytes, or comment out to remove alignment
#define STR_CHAR char               //Type of characters in the STL strings


#define MIN_ALLOWED_AGE 10          //Inclusive
#define MAX_ALLOWED_AGE 200         //Inclusive


#define MIN_ALLOWED_YEAR 1000       //Inclusive
#define MAX_ALLOWED_YEAR 2100       //Inclusive

#define MAX_NAME_LEN_1 1024         //Maximum length in characters, inclusive
#define MAX_NAME_LEN_2 256          //Maximum length in characters, inclusive





/// <summary>
/// Template that returns an aligned size for 'n'
/// </summary>
/// <param name="n">Size to align</param>
/// <returns>Aligned size</returns>
template<class T>
inline T aligned(T n)
{
#ifdef ALIGN_BY
    //Use alignment

    static_assert(ALIGN_BY > 0, "Alignment must be positive");
    static_assert(std::popcount(ALIGN_BY) == 1, "Alignment must be done by a size the power of 2");

/*
                 AND(8-1)
0 => 0  => 000 => 0
1 => -1 => FFF => 7
2 => -2 => FFE => 6


*/

    return n + ((-(intptr_t)n) & (ALIGN_BY - 1));


    //return n + ((n % ALIGN_BY) != 0 ? ALIGN_BY - (n % ALIGN_BY) : 0);
#else
    //No alignment
    return n;
#endif
}



/// <summary>
/// Calculates size of STL string taking alignment into account
/// </summary>
/// <typeparam name="s">STL string</typeparam>
/// <returns>Aligned size</returns>
template<class T>
inline size_t aligned_sizeof_str(T& s)
{
    /*
    size_t length;
    char[] str;
    */

    return aligned(sizeof(size_t)) +
        aligned(s.size() * sizeof(STR_CHAR));
}



/// <summary>
/// Copy primitive type into a memory location
/// </summary>
/// <param name="p">Pointer to the memory location. It will be incremented by the sizeof 's'</param>
/// <param name="s">Primitive variable to copy</param>
template<class T>
inline void copy_aligned(uint8_t*& p, T s)
{
    *(T*)p = s;
    p += aligned(sizeof(s));

}



/// <summary>
/// Copy STL string into a memory location
/// </summary>
/// <param name="p">Pointer to the memory location. It will be incremented by the sizeof string</param>
/// <param name="s">STL string to copy</param>
template<class T>
inline void copy_aligned_str(uint8_t*& p, T& s)
{
    size_t szStr = s.size();

    *(size_t*)p = szStr;
    p += aligned(sizeof(szStr));

    memcpy(p, s.c_str(), szStr * sizeof(STR_CHAR));
    p += aligned(szStr * sizeof(STR_CHAR));

}





/// <summary>
/// Read primitive type from memory, by checking for overruns
/// </summary>
/// <param name="p">Pointer to byte array to read from</param>
/// <param name="pEnd">End of the byte array, exclusive</param>
/// <param name="s">Primite type to set</param>
/// <returns>true if success, false if failed</returns>
template<class T>
inline bool read_aligned(const uint8_t*& p, const uint8_t* pEnd, T& s)
{
    intptr_t szcb = aligned(sizeof(s));
    if(p + szcb > pEnd)
    {
        //Overrun
        return false;
    }

    s = *(T*)p;
    p += szcb;

    return true;
}



/// <summary>
/// Read a floating point type from memory, by checking for overruns
/// </summary>
/// <param name="p">Pointer to byte array to read from</param>
/// <param name="pEnd">End of the byte array, exclusive</param>
/// <param name="s">Floating point type to set</param>
/// <returns>true if success, false if failed</returns>
template<class T>
inline bool read_aligned_double(const uint8_t*& p, const uint8_t* pEnd, T& s)
{
    if(!read_aligned(p, pEnd, s))
    {
        return false;
    }

    //Check for infinities, NaN and such
    if(!std::isfinite(s))
    {
        return false;
    }

    return true;
}



/// <summary>
/// Read STL string from memory, by checking for overruns
/// </summary>
/// <param name="p">Pointer to byte array to read from</param>
/// <param name="pEnd">End of the byte array, exclusive</param>
/// <param name="s">STL string to set</param>
/// <param name="szchMaxLen">if not 0, maximum allowed length of 's' in characters</param>
/// <returns>true if success, false if failed</returns>
template<class T>
inline bool read_aligned_str(const uint8_t*& p, const uint8_t* pEnd, T& s, size_t szchMaxLen)
{
    /*
    size_t length;
    char[] str;
    */

    size_t sz;
    if(!read_aligned(p, pEnd, sz))
    {
        return false;
    }

    if((intptr_t)sz < 0 ||
        p + sz * sizeof(STR_CHAR) > pEnd)
    {
        //Overrun
        return false;
    }

    s.assign((const STR_CHAR*)p, sz);
    p += aligned(sz * sizeof(STR_CHAR));

    if(szchMaxLen > 0)
    {
        if(s.size() > szchMaxLen)
        {
            return false;
        }
    }

    return true;
}








