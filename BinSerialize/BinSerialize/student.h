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


//Class that describes students for our fictional class
#pragma once

#include <string>
#include <assert.h>
#include <Windows.h>

#include "types.h"




enum class AttendanceType : unsigned int
{
    Unknown = 0,

    Enrolled,               //Student is currently enrolled
    Enrolling,              //Student is trying to enroll
    Graduated,              //Student has graduated
    Expelled,               //Student was expelled
    DroppedOut,             //Student voluntarily dropped out
    External,               //Person is not currently associated with the college

    MaxCount                //MUST BE LAST! Do not use!
};





struct Student
{
    //Age of the person, or 0 if not known
    //[MIN_ALLOWED_AGE - MAX_ALLOWED_AGE] acceptable range
    int nAge = 0;

    //Student's given name - must be provided
    //MAX_NAME_LEN_1 - max length
    std::string strGivenName;

    //Optional
    //MAX_NAME_LEN_1 - max length
    std::string strSecondName;

    //Optional
    //MAX_NAME_LEN_1 - max length
    std::string strThirdName;

    //Type of the person's attendance
    AttendanceType attendance = AttendanceType::Unknown;

    //true if the student is currently suspended
    bool bSuspended = false;

    //Student's performance score
    double fPerformanceScore = 0.0;

    //Internal notes about the student
    std::string strNotes;




    Student()
    {
    }

    Student(int age, 
        AttendanceType attend,
        const char* givenName,
        const char* secondName = nullptr,
        const char* thirdName = nullptr
        )
        : nAge(age)
        , attendance(attend)
        , strGivenName(givenName ? givenName : "")
        , strSecondName(secondName ? secondName : "")
        , strThirdName(thirdName ? thirdName : "")
    {
    }




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
/*

0   x    <- pS
1   y
2   z
3        <- pEnd

*/


            //Check 'nAge'
            if(!read_aligned(pS, pEnd, nAge))
                break;

            if(nAge != 0)
            {
                if(nAge < MIN_ALLOWED_AGE ||
                    nAge > MAX_ALLOWED_AGE)
                {
                    break;
                }
            }


            //Check 'strGivenName'
            if(!read_aligned_str(pS, pEnd, strGivenName, MAX_NAME_LEN_1))
                break;

            if(strGivenName.empty())
                break;


            //Check 'strSecondName'
            if(!read_aligned_str(pS, pEnd, strSecondName, MAX_NAME_LEN_1))
                break;


            //Check 'strThirdName'
            if(!read_aligned_str(pS, pEnd, strThirdName, MAX_NAME_LEN_1))
                break;


            //Check 'attendance'
            if(!read_aligned(pS, pEnd, attendance))
                break;

            if(attendance < AttendanceType::Unknown ||
                attendance >= AttendanceType::MaxCount)
                break;


            //Check 'bSuspended'
            if(!read_aligned(pS, pEnd, bSuspended))
                break;

            if(bSuspended != true &&
                bSuspended != false)
                break;


            //Check 'fPerformanceScore'
            if(!read_aligned_double(pS, pEnd, fPerformanceScore))
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
        *this = Student();

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
            aligned(sizeof(nAge)) +
            aligned_sizeof_str(strGivenName) +
            aligned_sizeof_str(strSecondName) +
            aligned_sizeof_str(strThirdName) +
            aligned(sizeof(attendance)) +
            aligned(sizeof(bSuspended)) +
            aligned(sizeof(fPerformanceScore)) +
            aligned_sizeof_str(strNotes);

        //Was the buffer provided?
        if(pBuff)
        {
            //Compare the size provided
            if(szcbBuff >= szcbData)
            {
                //Fill out the buffer
                uint8_t* pD = (uint8_t*)pBuff;

                //Clear provided buffer
                memset(pD, 0, szcbData);

                copy_aligned(pD, nAge);

                copy_aligned_str(pD, strGivenName);
                copy_aligned_str(pD, strSecondName);
                copy_aligned_str(pD, strThirdName);

                copy_aligned(pD, attendance);
                copy_aligned(pD, bSuspended);
                copy_aligned(pD, fPerformanceScore);

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


