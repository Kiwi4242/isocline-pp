/* ----------------------------------------------------------------------------
  Copyright (c) 2021, Daan Leijen
  Copyright (c) 2024, John Burnell
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/

// Modified to use c++ vectors
#include <vector>
#include <string>

#include <stdio.h>
#include <string.h>  
#include <sys/stat.h>


#include "../include/isocline.h"
#include "common.h"
#include "history.h"
#include "stringbuf.h"


// rewritten to use C++ containers
// also allow main program to use own history
typedef std::vector<std::string> StrVec;

struct history_s {
  ic_history_count_fun_t *count_fun;
  ic_history_fun_t *get_fun;  
  ic_history_index_sub_t *delete_fun;
  ic_history_st_sub_t *push_fun;
  void *historyData;
  bool useDefault;

  unsigned int  count;         // current number of entries in use
  ssize_t  len;                // size of elems 
  StrVec elems;                // history items

  std::string  fname;          // history file
  alloc_t* mem;
  bool     allow_duplicates;   // allow duplicate entries?
};


void history_set_custom(history_t* h, ic_history_count_fun_t *c, ic_history_fun_t *g, ic_history_index_sub_t *d,
                       ic_history_st_sub_t *p, void *data) {
  h->count_fun = c;
  h->get_fun = g;
  h->delete_fun = d;
  h->push_fun = p;
  h->historyData = data;
  h->useDefault = false;
}


ic_private history_t* history_new(alloc_t* mem) {
  history_t* h = mem_zalloc_tp(mem,history_t);
  h->mem = mem;
  h->useDefault = true;
  h->count = 0;
  return h;
}

ic_private void history_free(history_t* h) {
  if (h == NULL) return;
  history_clear(h);
  if (h->len > 0) {
    h->elems.clear();
  }
  mem_free(h->mem, h); // free ourselves
}

ic_private bool history_enable_duplicates( history_t* h, bool enable ) {
  bool prev = h->allow_duplicates;
  h->allow_duplicates = enable;
  return prev;
}

ic_private ssize_t  history_count(const history_t* h) {
  if (h->useDefault) {
    return h->count;
  }
  if (h->count_fun) {
    return h->count_fun(h->historyData);
  }
  return 0;
}

//-------------------------------------------------------------
// push/clear
//-------------------------------------------------------------

ic_private bool history_update( history_t* h, const char* entry ) {
  if (entry==NULL) return false;
  history_remove_last(h);
  history_push(h,entry);
  //debug_msg("history: update: with %s; now at %s\n", entry, history_get(h,0));
  return true;
}

static void history_delete_at( history_t* h, ssize_t idx ) {
  if (h->useDefault) {
    ssize_t count = history_count(h);
    if (idx < 0 || idx >= count) return;
    for(unsigned int i = idx+1; i < count; i++) {
      h->elems[i-1] = h->elems[i];
    }
    h->elems.erase(h->elems.begin()+count-1);
    h->count--;
  } {
    if (h->delete_fun) {
      h->delete_fun(h->historyData, idx, 1);
    }
  }
}

ic_private bool history_push( history_t* h, const char* entry ) {
  // if (h->len <= 0 || entry==NULL)  return false;
  if (entry==NULL)  return false;
  if (h->useDefault) {
    // remove any older duplicate
    if (!h->allow_duplicates) {
      for(unsigned int i = 0; i < h->count; i++) {
        if (h->elems[i] == entry) {
          history_delete_at(h,i);
        }
      }
    }
    h->elems.push_back(entry);   // mem_strdup(h->mem,entry);
    h->count++;
  } else {
    if (h->push_fun) {
      h->push_fun(h->historyData, entry);
    }
  }
  return true;
}


static void history_remove_last_n( history_t* h, ssize_t n ) {
  if (n <= 0) return;
  if (h->useDefault) {
    if (n > h->count) n = h->count;
    h->elems.erase(h->elems.begin()+h->count-n, h->elems.end());
    h->count -= n;
    assert(h->count >= 0);    
  } else {
    if (h->delete_fun) {
      h->delete_fun(h->historyData, 0, -n);
    }
  }
}

ic_private void history_remove_last(history_t* h) {
  history_remove_last_n(h,1);
}

ic_private void history_clear(history_t* h) {
  history_remove_last_n( h, history_count(h) );
}

ic_private const std::string history_get( const history_t* h, ssize_t n ) {
  // only called in history_search - has n in range
  if (h->useDefault) {
    return h->elems[h->count - n - 1];
  } else {
    if (h->get_fun) {
      return h->get_fun(h->historyData, n);
    }
  }
  return "";
}

ic_private bool history_search( const history_t* h, ssize_t from /*including*/, const char* search, bool backward, ssize_t* hidx, ssize_t* hpos ) {
  size_t pos = std::string::npos;
  size_t count = history_count(h);
  int ind = -1;
  if (backward) {
    for(size_t i = from; i < count; i++ ) {
      if ((pos = history_get(h,i).find(search)) != std::string::npos) {
        ind = i;
        break;
     }
    }
  }
  else {
    for(int i = from; i >= 0; i-- ) {
      if ((pos = history_get(h,i).find(search)) != std::string::npos) {
        ind = i;
        break;
     }
    }
  }
  if (pos == std::string::npos) return false;
  if (hidx != NULL) *hidx = ind;
  if (hpos != NULL) *hpos = pos;
  return true;
}

//-------------------------------------------------------------
// 
//-------------------------------------------------------------

ic_private void history_load_from(history_t* h, const char* fname, long max_entries ) {
  history_clear(h);
  h->fname = fname;
  if (max_entries == 0) {
    // assert(h->elems == NULL);
    return;
  }
  if (max_entries < 0) max_entries = 100;
  h->elems.resize(max_entries);
  history_load(h);
}




//-------------------------------------------------------------
// save/load history to file
//-------------------------------------------------------------

static char from_xdigit( int c ) {
  if (c >= '0' && c <= '9') return (char)(c - '0');
  if (c >= 'A' && c <= 'F') return (char)(10 + (c - 'A'));
  if (c >= 'a' && c <= 'f') return (char)(10 + (c - 'a'));
  return 0;
}

static char to_xdigit( uint8_t c ) {
  if (c <= 9) return ((char)c + '0');
  if (c >= 10 && c <= 15) return ((char)c - 10 + 'A');
  return '0';
}

static bool ic_isxdigit( int c ) {
  return ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'));
}

static bool history_read_entry( history_t* h, FILE* f, stringbuf_t* sbuf ) {
  sbuf_clear(sbuf);
  while( !feof(f)) {
    int c = fgetc(f);
    if (c == EOF || c == '\n') break;
    if (c == '\\') {
      c = fgetc(f);
      if (c == 'n')       { sbuf_append(sbuf,"\n"); }
      else if (c == 'r')  { /* ignore */ }  // sbuf_append(sbuf,"\r");
      else if (c == 't')  { sbuf_append(sbuf,"\t"); }
      else if (c == '\\') { sbuf_append(sbuf,"\\"); }
      else if (c == 'x') {
        int c1 = fgetc(f);         
        int c2 = fgetc(f);
        if (ic_isxdigit(c1) && ic_isxdigit(c2)) {
          char chr = from_xdigit(c1)*16 + from_xdigit(c2);
          sbuf_append_char(sbuf,chr);
        }
        else return false;
      }
      else return false;
    }
    else sbuf_append_char(sbuf,(char)c);
  }
  if (sbuf_len(sbuf)==0 || sbuf_string(sbuf)[0] == '#') return true;
  return history_push(h, sbuf_string(sbuf));
}

static bool history_write_entry( const char* entry, FILE* f, stringbuf_t* sbuf ) {
  sbuf_clear(sbuf);
  //debug_msg("history: write: %s\n", entry);
  while( entry != NULL && *entry != 0 ) {
    char c = *entry++;
    if (c == '\\')      { sbuf_append(sbuf,"\\\\"); }
    else if (c == '\n') { sbuf_append(sbuf,"\\n"); }
    else if (c == '\r') { /* ignore */ } // sbuf_append(sbuf,"\\r"); }
    else if (c == '\t') { sbuf_append(sbuf,"\\t"); }
    else if (c < ' ' || c > '~' || c == '#') {
      char c1 = to_xdigit( (uint8_t)c / 16 );
      char c2 = to_xdigit( (uint8_t)c % 16 );
      sbuf_append(sbuf,"\\x"); 
      sbuf_append_char(sbuf,c1); 
      sbuf_append_char(sbuf,c2);            
    }
    else sbuf_append_char(sbuf,c);
  }
  //debug_msg("history: write buf: %s\n", sbuf_string(sbuf));
  
  if (sbuf_len(sbuf) > 0) {
    sbuf_append(sbuf,"\n");
    fputs(sbuf_string(sbuf),f);
  }
  return true;
}

ic_private void history_load( history_t* h ) {
  if (h->fname.length() == 0) return;
  FILE* f = fopen(h->fname.c_str(), "r");
  if (f == NULL) return;
  stringbuf_t* sbuf = sbuf_new(h->mem);
  if (sbuf != NULL) {
    while (!feof(f)) {
      if (!history_read_entry(h,f,sbuf)) break; // error
    }
    sbuf_free(sbuf);
  }
  fclose(f);
}

ic_private void history_save( const history_t* h ) {
  if (h->fname.length() == 0) return;
  FILE* f = fopen(h->fname.c_str(), "w");
  if (f == NULL) return;
#ifndef _WIN32
  chmod(h->fname,S_IRUSR|S_IWUSR);
#endif
  stringbuf_t* sbuf = sbuf_new(h->mem);
  if (sbuf != NULL) {
    for( int i = 0; i < h->count; i++ )  {
      if (!history_write_entry(h->elems[i].c_str(),f,sbuf)) break;  // error
    }
    sbuf_free(sbuf);
  }
  fclose(f);  
}
