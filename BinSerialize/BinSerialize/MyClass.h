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


//Fictional class to demo serialization and de-serialization
//
#pragma once

#include <string>
#include <vector>

#include "student.h"




struct MyClass
{
    //Year the class was established, 0 if unknown
    //[MIN_ALLOWED_YEAR - MAX_ALLOWED_YEAR] acceptable range
    int nYearEstablished = 0;

    //Name of the class, must be provided
    //MAX_NAME_LEN_2 - max length
    std::string strName;

    //People associated with a class
    std::vector<Student> students;

    //Internal notes about the class
    std::string strNotes;





    /// <summary>
    /// De-serializes byte array into this struct
    /// </summary>
    /// <param name="pData">Byte array to convert</param>
    /// <param name="szcbData">Size of 'pData' in bytes</param>
    /// <returns>[1 and up) if success, for amount of bytes used, 0 if error - in this case this struct will be reset</returns>
    size_t fromByteArray(const void* pData, size_t szcbData)
    {
        while(true)
        {
            //Do we have a pointer to data?
            if(!pData)
                break;

            //Check overall data size provided
            if((intptr_t)szcbData <= 0)
                break;

            const uint8_t* pS = (const uint8_t*)pData;
            const uint8_t* pEnd = pS + szcbData;
            assert(pEnd > pS);


            //Check 'nYearEstablished'
            if(!read_aligned(pS, pEnd, nYearEstablished))
                break;

            if(nYearEstablished != 0)
            {
                if(nYearEstablished < MIN_ALLOWED_YEAR ||
                    nYearEstablished > MAX_ALLOWED_YEAR)
                    break;
            }
            

            //Check 'strName'
            if(!read_aligned_str(pS, pEnd, strName, MAX_NAME_LEN_2))
                break;

            if(strName.empty())
                break;


            //Check 'students'
            size_t szCntStudents;
            if(!read_aligned(pS, pEnd, szCntStudents))
                break;

            if((intptr_t)szCntStudents < 0)
                break;

/*

size_t cnt_students
student_data[]
----
--------
--
---------------
------
*/

            //Get all students
            bool bReadStudentsOK = true;
            students.clear();

            Student st;

            for(size_t s = 0; s < szCntStudents; s++)
            {
                size_t szcb = st.fromByteArray(pS, pEnd - pS);
                if(!szcb)
                {
                    //Failed
                    bReadStudentsOK = false;

                    break;
                }

                //Add student to the list
                students.push_back(st);

                pS += szcb;
            }

            if(!bReadStudentsOK)
                break;


            //Check 'strNotes'
            if(!read_aligned_str(pS, pEnd, strNotes, 0))
                break;



            //Sanity check
            if(pS <= pEnd)
            {
                //Success!
                return pS - (const uint8_t*)pData;
            }
            else
            {
                //Overflow
                assert(false);

#ifdef _WIN32
                //Microsoft specific code
                __fastfail(FAST_FAIL_FATAL_APP_EXIT);
#else
                //General case
                abort(-1);
#endif
            }


            break;
        }

        //Failure to de-serialize

        //Reset this struct
        *this = MyClass();

        return 0;
    }






    /// <summary>
    /// Serializes this struct by converting it to a byte array
    /// </summary>
    /// <param name="pBuff">if not 0, pointer to the buffer to fill out</param>
    /// <param name="szcbBuff">Size of provided 'pBuff' in bytes</param>
    /// <returns>Size of the filled (or needed to fill) buffer in bytes, or 0 if error</returns>
    size_t toByteArray(void* pBuff = nullptr, size_t szcbBuff = 0) const
    {
        size_t szcbRet = 0;

        //Determine the size needed
        size_t szcbData = 
            aligned(sizeof(nYearEstablished)) +
            aligned_sizeof_str(strName) +
            aligned(sizeof(size_t)) +               //Count of elements in the 'students' array
            aligned_sizeof_str(strNotes);

        std::vector<Student>::const_iterator itr = students.begin();
        const std::vector<Student>::const_iterator itrEnd = students.end();

        for(; itr != itrEnd; itr++)
        {
            szcbData += itr->toByteArray();
        }

        //Was the buffer provided?
        if(pBuff)
        {
            //Compare the size provided
            if(szcbBuff >= szcbData)
            {
                //Fill out the buffer
                uint8_t* pD = (uint8_t*)pBuff;
                uint8_t* pEnd = pD + szcbData;

                //Clear provided buffer
                memset(pD, 0, szcbData);

                copy_aligned(pD, nYearEstablished);

                copy_aligned_str(pD, strName);

                //Students array
                size_t szCntStudents = students.size();
                copy_aligned(pD, szCntStudents);

                for(itr = students.begin(); itr != itrEnd; itr++)
                {
                    size_t szcb = itr->toByteArray(pD, pEnd - pD);
                    if(!szcb)
                    {
                        //Failed
                        assert(false);

#ifdef _WIN32
                        //Microsoft specific code
                        __fastfail(FAST_FAIL_FATAL_APP_EXIT);
#else
                        //General case
                        abort(-1);
#endif
                    }

                    pD += szcb;
                }
                
                //Add notes
                copy_aligned_str(pD, strNotes);


                //Sanity check
                if(pD - (uint8_t*)pBuff == szcbData)
                {
                    //All done!
                    szcbRet = szcbData;
                }
                else
                {
                    //Overflow
                    assert(false);

#ifdef _WIN32
                    //Microsoft specific code
                    __fastfail(FAST_FAIL_FATAL_APP_EXIT);
#else
                    //General case
                    abort(-1);
#endif
                }
            }
            else
                assert(false);
        }
        else
        {
            //Only needs the size
            szcbRet = szcbData;
        }

        return szcbRet;
    }


};

