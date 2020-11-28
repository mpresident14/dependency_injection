#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#include "src/injector/binding_map.hpp"
#include "src/injector/details.hpp"
#include "src/injector/typing.hpp"
#include "src/injector/util.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include <experimental/source_location>


#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#define ANNOTATED(...) using InjectAnnotations = std::tuple<__VA_ARGS__>;

namespace injector {

using namespace detail;
using location = std::experimental::source_location;

/**
 * @brief Associates the given type with type std::decay_t<Key>.
 * @details Key must be convertible to Value (as enforced by std::is_convertible). Injecting a type
 * of Key will call the corresponding injector for Value.
 */
template <typename Key, typename Value, typename Annotation = Unannotated>
requires ImplBindable<Key, Value> void bindToBase(const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(InjectFunctions<Key>(
          UniqueSupplier<Key>(injectImpl<std::unique_ptr<Value>, Annotation>),
          SharedSupplier<Key>(injectImpl<std::shared_ptr<Value>, Annotation>))),
      false,
      BindingType::IMPL,
      loc);
}

/**
 * @brief Associates the given value with type std::decay_t<Key>.
 *
 * @param val The value to be bound. Must be of type Value, shared_ptr<Value>, or unique_ptr<Value>,
 * where Value is convertible to Key
 *
 * @note Objects bound via this method may be copied upon injection.
 */
template <typename Key, typename Annotation = Unannotated, typename ValueHolder>
requires ValidKey<Key> void bindToObject(
    ValueHolder&& val, const location& loc = location::current()) {
  bindToSupplier<Key, Annotation>([val = std::forward<ValueHolder>(val)]() { return val; }, loc);
}

/**
 * @brief Convenience wrapper for bindToObject<Key, ValueHolder>
 * @details Given val of type Value, shared_ptr<Value>, or unique_ptr<Value>, Key is automatically
 * inferred as Value.
 *
 * @param val The value to be bound.
 */
template <typename Annotation = Unannotated, typename ValueHolder>
void bindToObject(ValueHolder&& val, const location& loc = location::current()) {
  return bindToObject<type_extractor_t<ValueHolder>, Annotation, ValueHolder>(
      std::forward<ValueHolder>(val), loc);
}

/**
 * @brief Associates the given supplier with type std::decay_t<Key>.
 *
 * @param supplier A function that takes in no arguments and returns Value, shared_ptr<Value>, or
 * unique_ptr<Value>, where Value is convertible Key. The return type of supplier must be
 * move-constructible.
 */
template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsUniqueSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(UniqueSupplier<Key>(std::forward<Supplier>(supplier))),
      std::is_const_v<Key>,
      BindingType::UNIQUE,
      loc);
}

template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsSharedSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(SharedSupplier<Key>(std::forward<Supplier>(supplier))),
      std::is_const_v<Key>,
      BindingType::SHARED,
      loc);
}

template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsNonPtrSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(NonPtrSupplier<Key>(std::forward<Supplier>(supplier))),
      false,
      BindingType::NON_PTR,
      loc);
}

/**
 * @brief Removes all bindings.
 * @details Mainly for use in unit tests.
 */
void clearBindings() { bindings.clearBindings(); }


/**
 * @brief Given KeyHolder of type Key, shared_ptr<Key>, or unique_ptr<Key>, retrieves the value
 * associated with type std::decay_t<Key>.
 *
 * @throw runtime_error if there is no binding associated with R
 */
template <typename KeyHolder, typename Annotation = Unannotated>
requires ValidKey<type_extractor_t<KeyHolder>> KeyHolder inject() {
  try {
    return injectImpl<std::decay_t<KeyHolder>, Annotation>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}


class BindingModule {
public:
  virtual void install() = 0;
};

}  // namespace injector

#endif  // INJECTOR_HPP
