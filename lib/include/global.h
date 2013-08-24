/*--------------------------------------------------------------------------
   Author: Thomas Nowotny
  
   Institute: Center for Computational Neuroscience and Robotics
              University of Sussex
	      Falmer, Brighton BN1 9QJ, UK 
  
   email to:  T.Nowotny@sussex.ac.uk
  
   initial version: 2010-02-07
  
--------------------------------------------------------------------------*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_ //!< macro for avoiding multiple inclusion during compilation

//--------------------------------------------------------------------------
/*! \file global.h

\brief Global header file containing a few global variables. Part of the code generation section.

This global header file also takes care of including some generally used cuda support header files.
*/
//--------------------------------------------------------------------------


#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <cuda_runtime.h>

using namespace std; // replaced these two lines : problem with visual studio

cudaDeviceProp *deviceProp; //!< Global variable that contains the detected properties of all CUDA-enabled devices
int optimiseBlockSize = 0; //!< Flag that regulates whether BlockSize optimisation shall be attempted
int theDev; //!< Global variable that contains the id number of the chosen CUDA device
 
#endif  // _GLOBAL_H_
