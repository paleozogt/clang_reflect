[![Build Status](https://dev.azure.com/paleozogt/clang_reflect/_apis/build/status/paleozogt.clang_reflect?branchName=master)](https://dev.azure.com/paleozogt/clang_reflect/_build/latest?definitionId=2&branchName=master)

# Clang Reflect

C++ lacks reflection, so its very hard to build things like inspection or serialization into classes.

Clang Reflect is a command-line tool with no runtime dependencies, built on top of [libclang](https://clang.llvm.org/docs/Tooling.html).

## Building

This project uses submodules, so make sure they're synced up:

```
git pull --recurse-submodules && git submodule sync
```

Then build with CMake+Ninja:

```
> mkdir .build
> cd .build
> cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_BUILD_TYPE=RELEASE
> ninja clang_reflect
```

To run tests:

```
> ninja check_clang_reflect
```

## Usage

The tool takes a list of headers to generate from, along with `-I` flags for include paths.

Here we generate the testing headers:

```
> ./clang_reflect -I../src -I../test ../test/foobase.hpp ../test/foobar.hpp 
> ls *.hpp
FoobarReflect.hpp  FoobaseReflect.hpp
```

Inspecting the generated `FoobarReflect.hpp` shows templated functions like this:

```
...

template<typename F>
void reflect(const Foobar &obj, F &f) {
    reflect(dynamic_cast<const example::foo::Foobase&>(obj), f);
    f("d", "std::string", obj.d);
    f("e", "uint32_t", obj.e);
    f("f", "float", obj.f);
}

...
```

With the hard part out of the way, we can build up functionality using regular C++ templating.

For example, we can print out the contents of a class easily by making a logging functor that we pass to `reflect`:

```
#include "foobar.hpp"
#include "FoobarReflect.hpp"

...

class FieldLogger {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &field) const {
        std::cout << name << " " << type << " \"" << field << "\"" << std::endl;
    }
};

...

Foobar foobar("blah", 1, 2, "boop", 3, 4);
reflect(foobar, FieldLogger());
```

This will output:

```
a std::string "blah"
b uint32_t "1"
c float "2"
d std::string "boop"
e uint32_t "3"
f float "4"
```

More examples of how this can be used are in the `test/` folder.

