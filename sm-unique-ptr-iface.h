// sm-unique-ptr-iface.h
// Interface for `sm-unique-ptr.h`.  See that file for overview, etc.

// This file is in the public domain.

#ifndef SMBASE_SM_UNIQUE_PTR_IFACE_H
#define SMBASE_SM_UNIQUE_PTR_IFACE_H

#include "sm-unique-ptr-fwd.h"         // forwards for this module

#include "sm-macros.h"                 // OPEN_NAMESPACE, NULLABLE, NO_OBJECT_COPIES

#include <cstddef>                     // std::nullptr_t


OPEN_NAMESPACE(smbase)


// Owning pointer to a single optional object (not an array).
//
// Like `std::unique_ptr`, a `UniquePtr const` cannot be reassigned to
// own a different object, but it can be used to modify the owned
// object.  However, `UniquePtr<T const>` prevents modification of the
// owned object.
//
template <typename T>
class UniquePtr {
  NO_OBJECT_COPIES(UniquePtr);

private:     // data
  // Pointer to the owned object, or `nullptr` if it is empty.
  T * NULLABLE m_ptr;

public:      // types
  using pointer = T*;
  using element_type = T;

public:      // methods
  // -------------------------- Constructors ---------------------------
  // Create an empty pointer.
  inline constexpr UniquePtr() noexcept;

  // Create an empty pointer.
  inline constexpr UniquePtr(std::nullptr_t) noexcept;

  // Take ownership of the object pointed to by `p`.  If `p` is
  // `nullptr` then the resulting `UniquePtr` is empty.
  inline constexpr explicit UniquePtr(T * NULLABLE p) noexcept;

  // Take ownership of the object owned by `obj`, if any.
  inline constexpr UniquePtr(UniquePtr &&obj) noexcept;

  // Take ownership of the object owned by `obj`, if any.  The type `U*`
  // must be implicitly convertible to the type `T*`.
  template <typename U>
  inline constexpr UniquePtr(UniquePtr<U> &&obj) noexcept;

  // --------------------------- Destructor ----------------------------
  // Deallocate the owned object, if any.
  //
  // The C++ draft I'm looking at has `constexpr` but clang -std=c++17
  // says that is not allowed.
  //
  inline ~UniquePtr() noexcept;

  // --------------------------- Assignment ----------------------------
  // Take ownership of the object owned by `obj`, if any.  The currently
  // owned object, if any, is deallocated first.
  inline constexpr UniquePtr &operator=(UniquePtr &&obj) noexcept;

  // Take ownership of the object owned by `obj`, if any.  The type `U*`
  // must be implicitly convertible to the type `T*`.  The currently
  // owned object, if any, is deallocated first.
  template <typename U>
  inline constexpr UniquePtr &operator=(UniquePtr<U> &&obj) noexcept;

  // Deallocate the owned object, if any, so that `*this` is empty.
  inline constexpr UniquePtr &operator=(std::nullptr_t) noexcept;

  // ---------------------------- Observers ----------------------------
  // Access the owned object.  If `*this` is empty, then this throws
  // `XAssert`, declared in `exc.h`.
  inline constexpr T &operator*() const;

  // Get a pointer to the owned object.  If `*this` is empty, then this
  // throws `XAssert`, declared in `exc.h`.
  inline constexpr T *operator->() const;

  // Get the owned object pointer, or `nullptr` if `*this` is empty.
  inline constexpr T *get() const noexcept;

  // Equivalent to `get() != nullptr`.
  inline constexpr explicit operator bool() const noexcept;

  // ---------------------------- Modifiers ----------------------------
  // Return a pointer to the owned object, if any, and clear the stored
  // pointer.
  inline constexpr T *release() noexcept;

  /* Acquire ownership of `p`.

     More precisely:

     1. Let `old_p` be the current stored pointer.

     2. Change the stored pointer to equal `p`.

     3. If `old_p != nullptr` then `delete old_p`.

     Among other things, that implies the precondition that `p` must not
     equal `get()` unless both are `nullptr`.
  */
  inline constexpr void reset(T *p = nullptr) noexcept;

  // Swap the stored pointer with that of `obj`.
  inline constexpr void swap(UniquePtr &obj) noexcept;
};


CLOSE_NAMESPACE(smbase)


#endif // SMBASE_SM_UNIQUE_PTR_IFACE_H
