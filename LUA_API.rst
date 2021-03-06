##############
DFHack Lua API
##############

.. contents::

====================
DF structure wrapper
====================

DF structures described by the xml files in library/xml are exported
to lua code as a tree of objects and functions under the ``df`` global,
which broadly maps to the ``df`` namespace in C++.

**WARNING**: The wrapper provides almost raw access to the memory
of the game, so mistakes in manipulating objects are as likely to
crash the game as equivalent plain C++ code would be. E.g. NULL
pointer access is safely detected, but dangling pointers aren't.

Objects managed by the wrapper can be broadly classified into the following groups:

1. Typed object pointers (references).

   References represent objects in DF memory with a known type.

   In addition to fields and methods defined by the wrapped type,
   every reference has some built-in properties and methods.

2. Untyped pointers

   Represented as lightuserdata.

   In assignment to a pointer NULL can be represented either as
   ``nil``, or a NULL lightuserdata; reading a NULL pointer field
   returns ``nil``.

3. Named types

   Objects in the ``df`` tree that represent identity of struct, class,
   enum and bitfield types. They host nested named types, static
   methods, builtin properties & methods, and, for enums and bitfields,
   the bi-directional mapping between key names and values.

4. The ``global`` object

   ``df.global`` corresponds to the ``df::global`` namespace, and
   behaves as a mix between a named type and a reference, containing
   both nested types and fields corresponding to global symbols.

In addition to the ``global`` object and top-level types the ``df``
global also contains a few global builtin utility functions.

Typed object references
=======================

The underlying primitive lua object is userdata with a metatable.
Every structured field access produces a new userdata instance.

All typed objects have the following built-in features:

* ``ref1 == ref2``, ``tostring(ref)``

  References implement equality by type & pointer value, and string conversion.

* ``pairs(ref)``

  Returns an iterator for the sequence of actual C++ field names
  and values. Fields are enumerated in memory order. Methods and
  lua wrapper properties are not included in the iteration.

* ``ref._kind``

  Returns one of: ``primitive``, ``struct``, ``container``,
  or ``bitfield``, as appropriate for the referenced object.

* ``ref._type``

  Returns the named type object or a string that represents
  the referenced object type.

* ``ref:sizeof()``

  Returns *size, address*

* ``ref:new()``

  Allocates a new instance of the same type, and copies data
  from the current object.

* ``ref:delete()``

  Destroys the object with the C++ ``delete`` operator.
  If destructor is not available, returns *false*.

  **WARNING**: the lua reference object remains as a dangling
  pointer, like a raw C++ pointer would.

* ``ref:assign(object)``

  Assigns data from object to ref. Object must either be another
  ref of a compatible type, or a lua table; in the latter case
  special recursive assignment rules are applied.

* ``ref:_displace(index[,step])``

  Returns a new reference with the pointer adjusted by index*step.
  Step defaults to the natural object size.

Primitive references
--------------------

References of the *_kind* ``'primitive'`` are used for objects
that don't fit any of the other reference types. Such
references can only appear as a value of a pointer field,
or as a result of calling the ``_field()`` method.

They behave as structs with one field ``value`` of the right type.

Struct references
-----------------

Struct references are used for class and struct objects.

They implement the following features:

* ``ref.field``, ``ref.field = value``

  Valid fields of the structure may be accessed by subscript.

  Primitive typed fields, i.e. numbers & strings, are converted
  to/from matching lua values. The value of a pointer is a reference
  to the target, or nil/NULL. Complex types are represented by
  a reference to the field within the structure; unless recursive
  lua table assignment is used, such fields can only be read.

  **NOTE:** In case of inheritance, *superclass* fields have precedence
  over the subclass, but fields shadowed in this way can still
  be accessed as ``ref['subclasstype.field']``.
  This shadowing order is necessary because vtable-based classes
  are automatically exposed in their exact type, and the reverse
  rule would make access to superclass fields unreliable.

* ``ref._field(field)``

  Returns a reference to a valid field. That is, unlike regular
  subscript, it returns a reference to the field within the structure
  even for primitive typed fields and pointers.

* ``ref:vmethod(args...)``

  Named virtual methods are also exposed, subject to the same
  shadowing rules.

* ``pairs(ref)``

  Enumerates all real fields (but not methods) in memory
  (= declaration) order.

Container references
--------------------

Containers represent vectors and arrays, possibly resizable.

A container field can associate an enum to the container
reference, which allows accessing elements using string keys
instead of numerical indices.

Implemented features:

* ``ref._enum``

  If the container has an associated enum, returns the matching
  named type object.

* ``#ref``

  Returns the *length* of the container.

* ``ref[index]``

  Accesses the container element, using either a *0-based* numerical
  index, or, if an enum is associated, a valid enum key string.

  Accessing an invalid index is an error, but some container types
  may return a default value, or auto-resize instead for convenience.
  Currently this relaxed mode is implemented by df-flagarray aka BitArray.

* ``ref._field(index)``

  Like with structs, returns a pointer to the array element, if possible.
  Flag and bit arrays cannot return such pointer, so it fails with an error.

* ``pairs(ref)``, ``ipairs(ref)``

  If the container has no associated enum, both behave identically,
  iterating over numerical indices in order. Otherwise, ipairs still
  uses numbers, while pairs tries to substitute enum keys whenever
  possible.

* ``ref:resize(new_size)``

  Resizes the container if supported, or fails with an error.

* ``ref:insert(index,item)``

  Inserts a new item at the specified index. To add at the end,
  use ``#ref`` as index.

* ``ref:erase(index)``

  Removes the element at the given valid index.

Bitfield references
-------------------

Bitfields behave like special fixed-size containers.
The ``_enum`` property points to the bitfield type.

Numerical indices correspond to the shift value,
and if a subfield occupies multiple bits, the
``ipairs`` order would have a gap.

Named types
===========

Named types are exposed in the ``df`` tree with names identical
to the C++ version, except for the ``::`` vs ``.`` difference.

All types and the global object have the following features:

* ``type._kind``

  Evaluates to one of ``struct-type``, ``class-type``, ``enum-type``,
  ``bitfield-type`` or ``global``.

* ``type._identity``

  Contains a lightuserdata pointing to the underlying
  DFHack::type_instance object.

Types excluding the global object also support:

* ``type:sizeof()``

  Returns the size of an object of the type.

* ``type:new()``

  Creates a new instance of an object of the type.

* ``type:is_instance(object)``

  Returns true if object is same or subclass type, or a reference
  to an object of same or subclass type. It is permissible to pass
  nil, NULL or non-wrapper value as object; in this case the
  method returns nil.

In addition to this, enum and bitfield types contain a
bi-directional mapping between key strings and values, and
also map ``_first_item`` and ``_last_item`` to the min and
max values.

Struct and class types with instance-vector attribute in the
xml have a ``type.find(key)`` function that wraps the find
method provided in C++.

Global functions
================

The ``df`` table itself contains the following functions and values:

* ``NULL``, ``df.NULL``

  Contains the NULL lightuserdata.

* ``df.isnull(obj)``

  Evaluates to true if obj is nil or NULL; false otherwise.

* ``df.isvalid(obj[,allow_null])``

  For supported objects returns one of ``type``, ``voidptr``, ``ref``.

  If *allow_null* is true, and obj is nil or NULL, returns ``null``.

  Otherwise returns *nil*.

* ``df.sizeof(obj)``

  For types and refs identical to ``obj:sizeof()``.
  For lightuserdata returns *nil, address*

* ``df.new(obj)``, ``df.delete(obj)``, ``df.assign(obj, obj2)``

  Equivalent to using the matching methods of obj.

* ``df._displace(obj,index[,step])``

  For refs equivalent to the method, but also works with
  lightuserdata (step is mandatory then).

* ``df.is_instance(type,obj)``

  Equivalent to the method, but also allows a reference as proxy for its type.

Recursive table assignment
==========================

Recursive assignment is invoked when a lua table is assigned
to a C++ object or field, i.e. one of:

* ``ref:assign{...}``
* ``ref.field = {...}``

The general mode of operation is that all fields of the table
are assigned to the fields of the target structure, roughly
emulating the following code::

    function rec_assign(ref,table)
        for key,value in pairs(table) do
            ref[key] = value
        end
    end

Since assigning a table to a field using = invokes the same
process, it is recursive.

There are however some variations to this process depending
on the type of the field being assigned to:

1. If the table contains an ``assign`` field, it is
   applied first, using the ``ref:assign(value)`` method.
   It is never assigned as a usual field.

2. When a table is assigned to a non-NULL pointer field
   using the ``ref.field = {...}`` syntax, it is applied
   to the target of the pointer instead.

   If the pointer is NULL, the table is checked for a ``new`` field:

   a. If it is *nil* or *false*, assignment fails with an error.

   b. If it is *true*, the pointer is initialized with a newly
      allocated object of the declared target type of the pointer.

   c. Otherwise, ``table.new`` must be a named type, or an
      object of a type compatible with the pointer. The pointer
      is initialized with the result of calling ``table.new:new()``.

   After this auto-vivification process, assignment proceeds
   as if the pointer wasn't NULL.

   Obviously, the ``new`` field inside the table is always skipped
   during the actual per-field assignment processing.

3. If the target of the assignment is a container, a separate
   rule set is used:

   a. If the table contains neither ``assign`` nor ``resize``
      fields, it is interpreted as an ordinary *1-based* lua
      array. The container is resized to the #-size of the
      table, and elements are assigned in numeric order::

        ref:resize(#table);
        for i=1,#table do ref[i-1] = table[i] end

   b. Otherwise, ``resize`` must be *true*, *false*, or
      an explicit number. If it is not false, the container
      is resized. After that the usual struct-like 'pairs'
      assignment is performed.

      In case ``resize`` is *true*, the size is computed
      by scanning the table for the largest numeric key.

   This means that in order to reassign only one element of
   a container using this system, it is necessary to use::

      { resize=false, [idx]=value }

Since nil inside a table is indistinguishable from missing key,
it is necessary to use ``df.NULL`` as a null pointer value.

This system is intended as a way to define a nested object
tree using pure lua data structures, and then materialize it in
C++ memory in one go. Note that if pointer auto-vivification
is used, an error in the middle of the recursive walk would
not destroy any objects allocated in this way, so the user
should be prepared to catch the error and do the necessary
cleanup.

================
DFHack utilities
================

DFHack utility functions are placed in the ``dfhack`` global tree.

Currently it defines the following features:

* ``dfhack.print(args...)``

  Output tab-separated args as standard lua print would do,
  but without a newline.

* ``print(args...)``, ``dfhack.println(args...)``

  A replacement of the standard library print function that
  works with DFHack output infrastructure.

* ``dfhack.printerr(args...)``

  Same as println; intended for errors. Uses red color and logs to stderr.log.

* ``dfhack.color([color])``

  Sets the current output color. If color is *nil* or *-1*, resets to default.

* ``dfhack.is_interactive()``

  Checks if the thread can access the interactive console and returns *true* or *false*.

* ``dfhack.lineedit([prompt[,history_filename]])``

  If the thread owns the interactive console, shows a prompt
  and returns the entered string. Otherwise returns *nil, error*.

* ``dfhack.interpreter([prompt[,env[,history_filename]]])``

  Starts an interactive lua interpreter, using the specified prompt
  string, global environment and command-line history file.

  If the interactive console is not accessible, returns *nil, error*.

* ``dfhack.pcall(f[,args...])``

  Invokes f via xpcall, using an error function that attaches
  a stack trace to the error. The same function is used by SafeCall
  in C++, and dfhack.safecall.

  The returned error is a table with separate ``message`` and
  ``stacktrace`` string fields; it implements ``__tostring``.

* ``safecall(f[,args...])``, ``dfhack.safecall(f[,args...])``

  Just like pcall, but also prints the error using printerr before
  returning. Intended as a convenience function.

* ``dfhack.with_suspend(f[,args...])``

  Calls ``f`` with arguments after grabbing the DF core suspend lock.
  Suspending is necessary for accessing a consistent state of DF memory.

  Returned values and errors are propagated through after releasing
  the lock. It is safe to nest suspends.

  Every thread is allowed only one suspend per DF frame, so it is best
  to group operations together in one big critical section. A plugin
  can choose to run all lua code inside a C++-side suspend lock.

Persistent configuration storage
================================

This api is intended for storing configuration options in the world itself.
It probably should be restricted to data that is world-dependent.

Entries are identified by a string ``key``, but it is also possible to manage
multiple entries with the same key; their identity is determined by ``entry_id``.
Every entry has a mutable string ``value``, and an array of 7 mutable ``ints``.

* ``dfhack.persistent.get(key)``, ``entry:get()``

  Retrieves a persistent config record with the given string key,
  or refreshes an already retrieved entry. If there are multiple
  entries with the same key, it is undefined which one is retrieved
  by the first version of the call.

  Returns entry, or *nil* if not found.

* ``dfhack.persistent.delete(key)``, ``entry:delete()``

  Removes an existing entry. Returns *true* if succeeded.

* ``dfhack.persistent.get_all(key[,match_prefix])``

  Retrieves all entries with the same key, or starting with key..'/'.
  Calling ``get_all('',true)`` will match all entries.

  If none found, returns nil; otherwise returns an array of entries.

* ``dfhack.persistent.save({key=str1, ...}[,new])``, ``entry:save([new])``

  Saves changes in an entry, or creates a new one. Passing true as
  new forces creation of a new entry even if one already exists;
  otherwise the existing one is simply updated.
  Returns *entry, did_create_new*

Since the data is hidden in data structures owned by the DF world,
and automatically stored in the save game, these save and retrieval
functions can just copy values in memory without doing any actual I/O.
However, currently every entry has a 180+-byte dead-weight overhead.
