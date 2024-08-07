// save-restore.h
// Facility for saving a value and restoring it upon return.

#ifndef SMBASE_SAVE_RESTORE_H
#define SMBASE_SAVE_RESTORE_H

#include "sm-pp-util.h"                // SM_PP_CAT


// Restore a variable's value when this object goes out of scope.
template <class T>
class SaveRestore {
public:      // data
  T &m_variable;
  T m_origValue;

public:      // methods
  SaveRestore(T &variable)
    : m_variable(variable),
      m_origValue(variable)
  {}

  ~SaveRestore()
  {
    m_variable = m_origValue;
  }
};


// SaveRestore with a uniquely-named restorer object and deduced type.
#define SAVE_RESTORE(variable) \
  SaveRestore<decltype(variable)> SM_PP_CAT(save_restore_,__LINE__) \
    (variable) /* user ; */


// Set a variable to a value, then restore when going out of scope.
template <class T>
class SetRestore : public SaveRestore<T> {
public:      // methods
  SetRestore(T &variable, T const &newValue)
    : SaveRestore<T>(variable)
  {
    this->m_variable = newValue;
  }
};


// SetRestore with a uniquely-named restorer object and deduced type.
#define SET_RESTORE(variable, value) \
  SetRestore<decltype(variable)> SM_PP_CAT(set_restore_,__LINE__) \
    (variable, (value)) /* user ; */

// Same, but with an additional disambiguator, which is needed if I want
// to set/restore multiple variables from the same macro.
#define SET_RESTORE_D(disambig, variable, value)                            \
  SetRestore<decltype(variable)> SM_PP_CAT(set_restore_##disambig,__LINE__) \
    (variable, (value)) /* user ; */


// SET_RESTORE to add an amount to a numeric value.
#define ADD_RESTORE(variable, amount) \
  SET_RESTORE(variable, (variable)+(amount)) /* user ; */


// ADD_RESTORE with 1.
#define INC_RESTORE(variable) \
  ADD_RESTORE(variable, 1) /* user ; */


#endif // SMBASE_SAVE_RESTORE_H
