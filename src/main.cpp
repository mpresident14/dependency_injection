#include "src/injector/injector.hpp"

#include <iostream>

using namespace std;

class Base {
public:
  int baseVal = 9;
};

class Derived : public Base {
public:
  Derived() { baseVal = 10; }
  INJECT(Derived(int n, char c)) {}
  Derived(const Derived&) = delete;
  Derived(Derived&&) = delete;
};

int main() {
  injector::bindToInstance(make_shared<Derived>());
  shared_ptr<Derived> d = injector::inject<shared_ptr<Derived>>();
  cout << d->baseVal << endl;

  injector::bindToProvider<Base>([]() { return make_unique<Base>(Derived()); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();
  cout << b->baseVal << endl;

  injector::bindToProvider<int>([]() { return make_unique<int>(123456); });
  shared_ptr<int> n = injector::inject<shared_ptr<int>>();
  cout << *n << endl;

  return 0;
}
