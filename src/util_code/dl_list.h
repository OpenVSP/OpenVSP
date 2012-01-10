//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Double Linked List Class
//
//
//   J.R. Gloudemans - 8/3/93
//   Sterling Software
//
//   Adapted from:   OBJECT-ORIENTED PROGRAMMING
//                   Peter Coad / Jill Nicola
//                   Source Code Diskette
//
//******************************************************************************


#ifndef DL_LIST_H
#define DL_LIST_H

#include <iostream>
using namespace std;

//====== Forward reference ======//
template<class Item_type>class dl_list;


//====== Double_Linked_List_Element Class Declaration ======//
template<class Item_type>

class dl_elem
{
private:

        Item_type item;
	dl_elem<Item_type>* next_elem;
	dl_elem<Item_type>* prev_elem;

	dl_elem( const dl_elem& ); // copy constructor not provided

public:

	dl_elem()	{ next_elem = 0; prev_elem = 0; }
	dl_elem(const Item_type& new_item)
                        { next_elem = 0; prev_elem = 0; item = new_item; }
        ~dl_elem()	{ }

        friend class dl_list<Item_type>;
};


//====== Double_Linked_List Class Declaration ======//
template<class Item_type>

class dl_list
{
private:
        int num_elems;
        int end_flag;

	dl_elem<Item_type>* first_elem;
	dl_elem<Item_type>* curr_elem;
	dl_elem<Item_type>* last_elem;
public:

        dl_list()   { first_elem = curr_elem = last_elem = 0;
                      end_flag = 1; num_elems = 0; }
        dl_list(const dl_list<Item_type>& ); // copy constructor
        ~dl_list()  { this->clear(); }
        void clear();

        void insert_after(const Item_type& new_item);
        void insert_before(const Item_type& new_item);
        Item_type* insert_new();
        void remove_curr();

        void reset()   	    { curr_elem = first_elem;
                              if (num_elems > 0) end_flag = 0;
                              else               end_flag = 1; }

        void set_end()	    {  curr_elem = last_elem;/* end_flag = 1; */ }
        int end_of_list()   {  return(end_flag); }
        int start_of_list() {  return(end_flag); }

        int is_empty()	{ if ( num_elems <= 0 ) return(1); else return(0); }

        int num_objects()	{ return(num_elems); }
        int get_curr_num();

        void set_curr_ptr(Item_type* in_ptr)
                  { curr_elem = (dl_elem<Item_type>*) in_ptr; end_flag = 0; }

        Item_type* get_curr_ptr();
        Item_type& get_curr();
	Item_type& get_first()			{ return( first_elem->item ); }
	Item_type& get_last()			{ return( last_elem->item ); }

	void operator++ ();
	void inc();
	void operator-- ();
	void dec();

    dl_list<Item_type>& operator=(const dl_list<Item_type>&);
};


template<class Item_type>

//***** copy constructor *****//
dl_list<Item_type>::dl_list(const dl_list<Item_type>& dl) :
    first_elem(0), curr_elem(0), last_elem(0), end_flag(1), num_elems(0)
{
  dl_elem<Item_type>* dl_curr_elem(dl.first_elem);

  // cycle through all of the items to add
  while (dl_curr_elem!=0)
  {
    insert_after(dl_curr_elem->item);
    inc();
  }
}

template<class Item_type>

//***** assignment operator *****//
dl_list<Item_type>& dl_list<Item_type>::operator=(const dl_list<Item_type>& dl)
{
  // remove all current elements
  this->clean();

  // cycle through all of the items to add
  dl_elem<Item_type>* dl_curr_elem(dl.first_elem);
  while (dl_curr_elem!=0)
  {
    insert_after(dl_curr_elem->item);
    inc();
  }
}


template<class Item_type>

//***** Clear and Delete all Elements in List *****//
void dl_list<Item_type>::clear()
{
  if (num_elems <= 0) return;
  curr_elem = first_elem;
  while(curr_elem != last_elem)
    {
      curr_elem = first_elem->next_elem;
      delete first_elem;
      first_elem = curr_elem;
    }
  delete last_elem;

  first_elem = curr_elem = last_elem = 0;
  num_elems = 0;
  end_flag = 1;
}

template<class Item_type>

//***** Insert Element After Curr Elem and Return Pointer To New Elem *****//
void dl_list<Item_type>::insert_after(const Item_type& new_item)
{
  dl_elem<Item_type>* new_elem = new dl_elem<Item_type>(new_item);

  if (!first_elem)
    {
      first_elem = new_elem;
      curr_elem = new_elem;
      last_elem = new_elem;
    }
  else
    {
      dl_elem<Item_type>* curr_old_next_elem;

      curr_old_next_elem = curr_elem->next_elem;
      curr_elem->next_elem = new_elem;
      new_elem->next_elem = curr_old_next_elem;
      new_elem->prev_elem = curr_elem;

      if (curr_elem == last_elem)
        last_elem = new_elem;

      if (curr_old_next_elem)
        curr_old_next_elem->prev_elem = new_elem;
    }

  num_elems++;
  end_flag = 0;

}

template<class Item_type>

//***** Insert Element Before Curr Elem and Return Pointer To New Elem *****//
void dl_list<Item_type>::insert_before(const Item_type& new_item)
{
  dl_elem<Item_type>* new_elem = new dl_elem<Item_type>(new_item);

  if (!first_elem)
    {
      first_elem = new_elem;
      curr_elem = new_elem;
      last_elem = new_elem;
    }
  else
    {
      dl_elem<Item_type>* curr_old_prev_elem;

      curr_old_prev_elem = curr_elem->prev_elem;
      curr_elem->prev_elem = new_elem;
      new_elem->next_elem = curr_elem;
      new_elem->prev_elem = curr_old_prev_elem;

      if (curr_elem == first_elem)
        first_elem = new_elem;

      if (curr_old_prev_elem)
        curr_old_prev_elem->next_elem = new_elem;
    }

  num_elems++;
  end_flag = 0;

}

template<class Item_type>

//***** Insert Element After Curr Elem and Return Pointer To New Elem *****//
Item_type* dl_list<Item_type>::insert_new()
{
  dl_elem<Item_type>* new_elem = new dl_elem<Item_type>();

  if (!first_elem)
    {
      first_elem = new_elem;
      curr_elem = new_elem;
      last_elem = new_elem;
    }
  else
    {
      dl_elem<Item_type>* curr_old_next_elem;

      curr_old_next_elem = curr_elem->next_elem;
      curr_elem->next_elem = new_elem;
      new_elem->next_elem = curr_old_next_elem;
      new_elem->prev_elem = curr_elem;

      if (curr_elem == last_elem)
        last_elem = new_elem;

      if (curr_old_next_elem)
        curr_old_next_elem->prev_elem = new_elem;
    }

  num_elems++;
  end_flag = 0;

  return(Item_type*)new_elem;

}

template<class Item_type>

//***** Insert Element After Curr Elem and Return Pointer To New Elem *****//
void dl_list<Item_type>::remove_curr()
{
  if (!curr_elem)
    {
      cout << "ERROR - NULL curr_elem - dl_list.remove_curr" << endl;
    }
  else if ( num_elems == 1 )
    {
      delete curr_elem;
      first_elem = curr_elem = last_elem = 0;
      num_elems = 0;
      end_flag = 1;
    }
  else if (curr_elem == first_elem)
    {
      first_elem = curr_elem->next_elem;
      first_elem->prev_elem = 0;
      delete curr_elem;
      num_elems--;
      curr_elem = first_elem;
    }
  else if (curr_elem == last_elem)
    {
      last_elem = curr_elem->prev_elem;
      last_elem->next_elem = 0;
      delete curr_elem;
      num_elems--;
      curr_elem = last_elem;
//      end_flag = 1;
    }
  else
    {
      dl_elem<Item_type>* temp = curr_elem->next_elem;
      curr_elem->next_elem->prev_elem = curr_elem->prev_elem;
      curr_elem->prev_elem->next_elem = curr_elem->next_elem;
      delete curr_elem;
      num_elems--;
      curr_elem = temp;
    }
}


template<class Item_type>

//***** Get Current Item *****//
Item_type& dl_list<Item_type>::get_curr()
{
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem - dl_list.get_curr" << endl;
    }

  return curr_elem->item;
}

template<class Item_type>

//***** Get Pointer to Current Item *****//
Item_type* dl_list<Item_type>::get_curr_ptr()
{
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem ptr - dl_list.get_curr" << endl;
    }

  return (Item_type*)curr_elem;

}

template<class Item_type>

//***** Increment Curr List Ptr *****//
void dl_list<Item_type>::operator++()
{
/****************
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem ptr - dl_list++" << endl;
    }
**************/
  if (curr_elem != last_elem)
    curr_elem = curr_elem->next_elem;
  else
    end_flag = 1;
}

template<class Item_type>

//***** Increment Curr List Ptr *****//
void dl_list<Item_type>::inc()
{
/****************
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem ptr - dl_list++" << endl;
    }
**************/
  if (curr_elem != last_elem)
    curr_elem = curr_elem->next_elem;
  else
    end_flag = 1;
}


template<class Item_type>

//***** Decrement Curr List Ptr *****//
void dl_list<Item_type>::operator--()
{
/****************
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem ptr - dl_list--" << endl;
    }
*************/
  if (curr_elem != first_elem)
    curr_elem = curr_elem->prev_elem;
  else
    end_flag = 1;
}

template<class Item_type>
//***** Decrement Curr List Ptr *****//
void dl_list<Item_type>::dec()
{
/****************
  if ( !curr_elem)
    {
      cout << "ERROR - NULL curr_elem ptr - dl_list--" << endl;
    }
*************/
  if (curr_elem != first_elem)
    curr_elem = curr_elem->prev_elem;
  else
    end_flag = 1;
}

template<class Item_type>

//***** Decrement Curr List Ptr *****//
int dl_list<Item_type>::get_curr_num()
{
  if ( curr_elem == first_elem ) return(0);

  if ( curr_elem == last_elem  ) return(num_elems-1);

  dl_elem<Item_type>* temp_elem = first_elem;

  int elem_num = 0;
  while (temp_elem != curr_elem)
    {
      temp_elem = temp_elem->next_elem;
      elem_num++;
    }
  return(elem_num);

}

#endif
