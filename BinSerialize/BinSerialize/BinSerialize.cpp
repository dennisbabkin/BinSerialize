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

#include <iostream>
#include <random>
#include "MyClass.h"



void fuzzer();



int main()
{
    //fuzzer();         //If we need to run a fuzzer


    //Create some data to work with
    MyClass myClass;

    myClass.nYearEstablished = 2023;
    myClass.strName = "Class of 2023";
    myClass.strNotes = "My super fictional class.";

    myClass.students.push_back(Student(21, AttendanceType::Enrolled, "John", "Doe"));
    myClass.students.back().fPerformanceScore = 12.5;
    myClass.students.back().strNotes = "Best student";

    myClass.students.push_back(Student(19, AttendanceType::Enrolling, "Mary", "Smith"));
    myClass.students.back().fPerformanceScore = 13.75;
    myClass.students.back().strNotes = "Will be attending in September";

    myClass.students.push_back(Student(76, AttendanceType::Graduated, "Kareem", "Abdul", "Jabbar"));
    myClass.students.back().fPerformanceScore = 125.44;

    myClass.students.push_back(Student(35, AttendanceType::External, "Rihanna"));
    myClass.students.back().strNotes = "Celebrity endorsement";

    myClass.students.push_back(Student(62, AttendanceType::DroppedOut, "Unruly Kid"));
    myClass.students.back().fPerformanceScore = -5.0;
    myClass.students.back().strNotes = "Never enroll him again!";
    myClass.students.back().bSuspended = true;



    //ActivateActCtx(0, 0);         //Beacon API call to locate this spot in the optimized Release build


    //Test serialization
    size_t szcbSize = myClass.toByteArray();
    if(szcbSize != 0)
    {
        //Reserve mem
        uint8_t* pMem = new uint8_t[szcbSize];

        if(myClass.toByteArray(pMem, szcbSize) == szcbSize)
        {
            //We have serialized our class
            std::cout << "Serialized OK, length of data: " << szcbSize << std::endl;


            //De-serialize what we've got
            MyClass myClass2;

            if(myClass2.fromByteArray(pMem, szcbSize) == szcbSize)
            {
                //All done
                std::cout << "De-serialized OK!" << std::endl;

            }
            else
                assert(false);
        }
        else
            assert(false);

        //Free mem
        delete[] pMem;
        pMem = nullptr;

    }
    else
        assert(false);



    //Wait before closing the console window
    std::cin.get();
    return 0;
}




#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")


/// <summary>
/// Function that performs test fuzzing of the de-serialization logic
/// </summary>
void fuzzer()
{
    std::random_device dev;
    std::mt19937 rng(dev());        //32-bit random number generator - state size of 19937 bits

    std::uniform_int_distribution<std::mt19937::result_type> randByte(0, 255);
    std::uniform_int_distribution<std::mt19937::result_type> randMemSize(1, 10000);

    MyClass myClass2;


    for(;;)
    {
        size_t szcbSize = randMemSize(rng);

        uint8_t* pMem = new uint8_t[szcbSize];

        std::cout << "Size: " << szcbSize << ": ";


        //Fill out byte array with random bytes

#ifdef _WIN32

        //Microsoft only
        NTSTATUS status = BCryptGenRandom(NULL, 
            pMem, 
            (ULONG)szcbSize, 
            BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        assert(status == 0);
#else
        //Other OS
        for(size_t i = 0; i < szcbSize; i++)
        {
            pMem[i] = (uint8_t)randByte(rng);
        }
#endif



        //De-serialize what we've got
        if(myClass2.fromByteArray(pMem, szcbSize) == szcbSize)
        {
            //Success! - very rarely called
            std::cout << "SUCCESS!" << std::endl;
        }
        else
        {
            //Failure - is most probable --- 99.99999999999%
            std::cout << "FAILED" << std::endl;
        }

        delete[] pMem;
        pMem = NULL;
    }


}




