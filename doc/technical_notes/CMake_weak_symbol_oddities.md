# CMake Weak Symbol Oddities  
**Posted Aug 6, 2023**  
By Jeff Longo  
8 min read  

## CMake for STM32!

The Makefile that I had been using for a growing project had become unsustainable. As a result, I invested a good deal of time developing a [CMake framework for the STM32Cube libraries](https://github.com/jefflongo/cmake-stm32), to serve as a build system for STM32 microcontrollers. I did not anticipate how far down the CMake and GCC rabbit hole I’d have to go.

This all started when I had finished getting my existing codebase compiling with CMake. Expecting the code behavior to be unchanged, (after all, I hadn’t changed any of the source code) I booted up the code. What I found was my program stuck in an infinite assembly loop.

## Weak Symbols

GCC provides a way to create a weak symbol. That is to say that one can define a function as weak and later override it with a strong implementation. This is often used to provide a default function implementation that a user can later override with something custom. Typically, this is done by using the `__attribute__((weak))` GCC attribute.

```c
// strong.c
#include <stdio.h>

void foo(void) {
    printf("This is a strong function");
}
```

```c
// weak.c
#include <stdio.h>

void __attribute__((weak)) foo(void) {
    printf("This is a weak function");
}

int main(int argc, char* argv[]) {
    foo();
    return 0;
}
```

The output of this program is `This is a strong function`.

Assembly code can also define functions as weak. For example, STM32Cube defines the microcontroller’s interrupt handlers as weak in the assembly startup code:

```c
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak	NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

  .weak	HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

  .weak	MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

  .weak	BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

  ...
```

What does `Default_Handler` do you may ask?

```c
/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section	.text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
	b	Infinite_Loop
	.size	Default_Handler, .-Default_Handler
```

It puts you in an infinite loop. This explained what was going on, but why did my strong implementations of these handlers suddenly stop getting added to the build?

## Static Libraries and Linking

The answer has to do with libraries. In CMake, large projects are typically organized by creating directory structures containing libraries which can be transitively linked, allowing dependencies to propagate upward. This helps to avoid an enormous root `CMakeLists.txt` containing every dependency. In my project, I organized the structure as follows:

- Libraries are created for the STM32Cube libraries  
- A library is created for the BSP, which links in the STM32Cube libraries  
- The root CMake project defines the application and links in the BSP  

Given that I’m working with a microcontroller, a static library (which is CMake’s default for add_library) for the upper levels seemed like a reasonable choice. Here is where the issue lies. Let’s look at how the linker links a program:

- Linker arguments are evaluated left to right  
- Leftmost libraries are linked first, the order does matter!  
- The linker maintains a symbol table  
- A symbol is a function or a variable  
- The symbol table tracks what symbols have been seen so far that object files and libraries being linked export  
- The symbol table tracks undefined symbols that object files and libraries request to import  

When an object file is encountered:

- Its exported symbols are added to the symbol table  
- If any symbol with the same name already exists, a multiple definition error is thrown  
- Any symbols that are on the undefined list that were just added are removed from the undefined list  
- Any referenced symbols that the object imports are added to the undefined list if they aren’t already in the symbol table  

When a library is encountered (a static library is simply an uncompressed grouping of object files):

- If a symbol on the undefined list is exported by an object in the library, that object is linked as described above, otherwise the object is skipped  
- If any object is included in the link, the library is rescanned to check if any of the requested imports by the objects included in the link can be found in the library  

Once all objects are linked, the linker checks if the undefined list is empty. If it’s not, throw an undefined reference error.

>Recall that the linking order matters. If the linker decided it didn’t need an object from a library when it encountered that library, but needs that object when linking a future library, an error will occur. One that may not occur if the link order is switched.

>Another fine detail of the linking process is what happens when a weak symbol appears. A weak symbol does not appear on the undefined list. So if the linker skips over the object containing the weak symbol, it will not be included!

In summation, the cause of my issue was:

- The static library for the STM32Cube library exports the weak interrupt handlers  
- The static library for the BSP attempts to provide strong implementations for the handlers, but these implementations are ignored due to the linker already having these symbols in the symbol table  
- The executable is linked with the default handlers  

This behavior can be exemplified using our previous example code [above](#weak-symbols) and the following CMake script:

```cmake
cmake_minimum_required(VERSION 3.21)
project(weak-strong-demo)

add_library(strong STATIC strong.c)

add_executable(demo weak.c)
target_link_libraries(demo PRIVATE strong)
```

In this CMake snippet, we build `strong.c` as a static library and link it to the code containing the weak function. The output of this program is `This is a weak function`.

>The strong function is omitted from the build because the weak function is linked first and the strong function is in a static library.

So how do we fix this?

## Object Libraries

Fortunately, CMake has a feature called Object Libraries. [Object libraries](https://cmake.org/cmake/help/latest/command/add_library.html#object-libraries) have the nice organizational properties of CMake libraries, without having to actually compile the code into a library. If we build the same program but change the CMake script to use an object library:

```cmake
cmake_minimum_required(VERSION 3.21)
project(weak-strong-demo)

add_library(strong OBJECT strong.c)

add_executable(demo weak.c)
target_link_libraries(demo PRIVATE strong)
```

The output is now `This is a strong function`. Hooray! Unfortunately there is a new problem. Object libraries cannot be transitively linked. Take the following program:

```c
// a.c
int a(void) {
    return 0;
}
```

```c
// b.c
int b(void) {
    return 0;
}
int a(void);
```

```c
int main(int argc, char* argv[]) {
    return a();
}
```

And build the program with the following CMake script. This script creates object libraries for `a.c` and `b.c`, then links `A` to `B`. The user might expect `B` to inherit `a()` from `A`.

```cmake
cmake_minimum_required(VERSION 3.21)
project(object-library-transitive-demo)

add_library(A OBJECT a.c)
add_library(B OBJECT b.c)
target_link_libraries(B PUBLIC A)

add_executable(demo main.c)
target_link_libraries(demo PRIVATE B)
```

The result is `undefined reference to 'a'`. This limitation is outlined in the [CMake documentation](https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries) and is because object libraries do not have a link step, so no linking is done. Fortunately, there is a workaround, which is described in the following section of the documentation.

An [Interface Library](https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries) wrapper can be created. Interface libraries do not compile sources and do not produce output libraries, but can track dependencies and have properties.

>Interface libraries are typically used to model header-only libraries.

We can modify our CMake script to create a wrapper interface library around `A` which “links” to `A` and inherits its objects:

```cmake
add_library(A OBJECT a.c)
add_library(ifaceA INTERFACE)
target_link_libraries(ifaceA INTERFACE A $<TARGET_OBJECTS:A>)

add_library(B OBJECT b.c)
target_link_libraries(B PRIVATE ifaceA)

add_executable(demo main.c)
target_link_libraries(demo PRIVATE B)
```

>Since interface libraries do not produce outputs, linking A with target_link_libraries does not directly add the objects to its properties, only things such as the include paths and compile definitions. The objects themselves can be added to the interface libraries properties with the TARGET_OBJECTS generator.

We only need the wrapper around `A`, since its dependencies must be propagated. Because non-object libraries can be linked to object libraries and `B` is the last library in the dependency chain, `ifaceA` can simply be linked to `B`.

## Conclusion

With the understanding of how to create transitive object libraries, it is possible to build neat CMake project structures for embedded systems, without having to deal with the headache of correct link order or increasing binary size with `--whole-archive`.
