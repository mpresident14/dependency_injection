# A toy dependency injection framework for C++.

## Usage

Use the INJECT macro to declare a constructor to be used for injection. Parameters can be annotated with types using the ANNOTATED macro.

Example:

```
class MyType {
public:
  ANNOTATED(Type1, Type2)
  INJECT(MyType(Dep1 obj1, Dep2 obj2, Dep3 obj3)) {}
};
```

To bind objects, use one of the `bind*` functions. Retrieve a bound object  using `inject`.

## Requirements
* A class can only have one injection constructor.
* The injection constructor must be public.
* Classes with an injection constructor must be final and move-constructible (copy-constructible is not a requirement except for `bindObject`).
* Key and value types passed to `bind*` and `inject` must be in their decayed form (`inject` returns by value, see below for more info).


## Usage Best Practices
* To bind a singleton of type V, use `bindObject(shared_ptr<V>)` (if the singleton may differ based on some logic, use `bindSupplier(() -> shared_ptr<V>)`).
* To produce a new instance of V on every injection, use `bindSupplier(() -> unique_ptr<V>)` or `bindSupplier(() -> V)`.
* `bindObject(V)` avoids the overhead of allocating a smart pointer and may be more suitable for objects that are cheap to copy (e.g. primitives).
* To bind an interface Base to an implementation Child, use `bindClass<Base, Child>()`
* Since references cannot be bound, `inject<K>` always returns a non-reference. This means that non-const lvalue reference parameter in injection constructors are not supported. Also beware that const reference injection constructor parameters will reference temporary objects, so they are not safe to save as member variables. For this purpose, use a shared pointer instead.

