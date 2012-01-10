//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   State Flag Class
//   
//   J.R. Gloudemans - 11/23/93
//******************************************************************************

#ifndef STATE_H
#define STATE_H

#define ST_OFF '0'
#define ST_ON  '1'

class state
{
     int num_vars;
     char* states;

public:
     state()  			{ num_vars = 0; states = 0; }
     state(int in_num)		{ this->init(in_num); }			

private:
    //Explicitly disallow use of implicity generated methods 
	//Not implemented because not needed
	state(const state&);
	state& operator= (const state*);

public:
    ~state()  			{ this->clear(); }
     void clear()		{ if (states) delete [] states; }
     void init(int in_num)   	{ if (in_num > 0) { this->clear();
                                                    num_vars = in_num; 
                              			    states = new char[num_vars];
                             			    this->turn_all_off(); } }      

     void turn_all_off()        { for (int i = 0 ; i < num_vars ; i++) 
                                     states[i] = ST_OFF; }
     void turn_all_on()         { for (int i = 0 ; i < num_vars ; i++) 
                                     states[i] = ST_ON; }
     void turn_off(int index)	{ if ( index >= 0 && index < num_vars)
                                     states[index] = ST_OFF; }
     void turn_on(int index)	{ if ( index >= 0 && index < num_vars)
                                     states[index] = ST_ON; }
     int is_on(int index) 	{ if ( (index >= 0) && (index < num_vars) &&
                                       (states[index] == ST_ON)) return(1); 
                                  else return(0); }
     int is_off(int index) 	{ if ( (index >= 0) && (index < num_vars) &&
                                       (states[index] == ST_OFF)) return(1); 
                                  else return(0); }
}; 
                                  
#endif


		
