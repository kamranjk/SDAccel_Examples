/*******************************************************************************
Copyright (c) 2016, Xilinx, Inc.
All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
 
 
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
 
 
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/*******************************************************************************
Description: 
    HLS Example using AXI4-master interface for burst read and write 
*******************************************************************************/
				      
//Includes 
#include <stdio.h>
#include <string.h>

//define internal buffer max size
#define BURSTBUFFERSIZE 2048

extern "C" {
void vadd(int *a, int size, int inc_value){
    // Map pointer a to AXI4-master interface for global memory access
    #pragma HLS INTERFACE m_axi port=a  offset=slave bundle=gmem
    // We also need to map a and return to a bundled axilite slave interface
    #pragma HLS INTERFACE s_axilite port=a bundle=control
    #pragma HLS INTERFACE s_axilite port=size bundle=control
    #pragma HLS INTERFACE s_axilite port=inc_value bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control
      
    int burstbuffer[BURSTBUFFERSIZE];

    //Per iteration of this loop perform BURSTBUFFERSIZE vector addition
    for(int i=0; i < size;  i+=BURSTBUFFERSIZE)
    {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=64
        int chunk_size = BURSTBUFFERSIZE;
        //boundary checks
        if ((i + BURSTBUFFERSIZE) > size) 
            chunk_size = size - i;
        
        //memcpy creates a burst access to memory
        //multiple calls of memcpy cannot be pipelined and will be scheduled sequentially
        //memcpy requires a local buffer to store the results of the memory transaction
        memcpy(burstbuffer,&a[i],chunk_size * sizeof (int));
        //for(int k=0; k < chunk_size; k++){
        //#pragma HLS LOOP_TRIPCOUNT min=256 max=2048
        //    burstbuffer[k] = a[i+k];
        //}
        
        //calculate and write results to global memory, the sequential write in a for loop can be inferred to a memory burst access automatically
        calc_write: for(int j=0; j < chunk_size; j++){
        #pragma HLS LOOP_TRIPCOUNT min=256 max=2048
        #pragma HLS PIPELINE
            burstbuffer[j] = burstbuffer[j] + inc_value;
            a[i+j] = burstbuffer[j];
        }
    }
}
}
