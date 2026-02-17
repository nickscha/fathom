# fathom
A C89 standard compliant, nostdlib (no C Standard Library) Fathom Engine.

<p align="center">
<a href="https://github.com/nickscha/fathom"><img src="assets/fathom.png" width="300"></a>
</p>

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

## Download the executable
<p align="center">
  <a href="https://github.com/nickscha/fathom/releases/latest/download/win32_fathom_x64.exe">
    <img src="https://img.shields.io/badge/Windows%20x64-green?style=flat-square" alt="Windows x64">
  </a>
  <a href="https://github.com/nickscha/fathom/releases/latest/download/win32_fathom_arm.exe">
    <img src="https://img.shields.io/badge/Windows%20ARM-green?style=flat-square" alt="Windows ARM">
  </a>
  <a href="https://github.com/nickscha/fathom/releases">
    <img src="https://img.shields.io/github/v/release/nickscha/fathom?style=flat-square&color=blue" alt="Latest Release">
  </a>
  <a href="https://github.com/nickscha/fathom/releases">
    <img src="https://img.shields.io/github/downloads/nickscha/fathom/total?style=flat-square&color=brightgreen" alt="Downloads">
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square" alt="License">
  </a>
  <img src="https://img.shields.io/badge/Standard-C89-orange?style=flat-square" alt="C Standard">
  <img src="https://img.shields.io/badge/nolib-nostdlib-lightgrey?style=flat-square" alt="nostdlib">
</p>

## Features
- **C89 compliant** — portable and legacy-friendly  
- **nostdlib** — no dependency on the C Standard Library
- **Zero dependencies** — own win32 layer implementation, own OpenGL context creation, ...
- **Strict compilation** — built with aggressive warnings & safety checks  

## Quick Start

### Download Windows Executable (Prebuilt)

Download the latest Windows executable for **X64** or **ARM** from the **Latest Release**:
- https://github.com/nickscha/fathom/releases/latest

### Building From Source

Clone the repository and build manually.
Either GCC or Clang is required to run the build script.

```bash
git clone https://github.com/nickscha/fathom.git
cd fathom
win32_fathom_build.bat
```

You can now run the `win32_fathom.exe` program.

### Running the program

> [!IMPORTANT]
> If you have donwloaded the release executeable depending on your platform use `win32_fathom_x64.exe` or `win32_fathom_arm.exe`

```bat
win32_fathom_x64.exe
```

## Roadmap

For the next release of **FATHOM** the following features are planend.


## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
