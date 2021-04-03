Tested C++ feauture:

  - auto
  - const iterator
  - lambda calculas
  - new for loop (foreach)

In Visual Studio 2010, you can use:

  - auto
  - const iterator
  - lambda calculas

without any setup, see CMakeLists.txt for further information.

-------------------------------------------------------------

C++11 support can't be covered in a simple and elegant CMakeLists.txt,
you I won't continue test C++11 features here further. These links may
be useful:

  - <http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake>
  - <http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake/20165220>

My advice is:

  - don't use `for( int i, v )' feature
  - be conservative, use lambda, const iterator, auto is pretty much enough
  - for VS2010 project, check
    <https://github.com/district10/cmake-templates/tree/master/cpp11_vs2010>
  - for Linux GNUMake, this CMakeList.txt works fine    


# CPU Break mode.

When in break mode, the CPU will stop executing ARM instructions and instead will execute functions from the cmdline.


readw [0x08000000] # Reads a (w)ord from position 0x08000000 and yields it.
# Every function which yields a value, can have one last argument specifying where to store it, this is called a store argument.
# If no store argument is specified, the result will be printed to screen.

readw [$r0] [$r1] # Reads a (w)ord from register r0 into register r1.

# You can also read(b)ytes and read(h)alf words.

dumpb [0xFF00:0xFFFF] # Reads and yields a vector of (b)ytes, from 0xFF00 inclusive to 0xFFFF exclusive. If the begin and end addresses are not
# 3 aligned to 1 for dumpb, 2 for dumph or 4 for dumpw, the ends will be filled with garbage bytes.

dumpw [0xFF00:+4i] [$myVar] # Reads a vector of (w)ords, starting from 0xFF00 to offset of 4 instructions, to the user defined variable "myVar"
# Heres a list of all suffixes that can be used in this relative addressing. (N is any number or numbers from 0-9)
# Ni = Instruction size (2 in Thumb mode and 4 in ARM mode)
# Nw = N words (4)
# Nh = N half words (2)
# N = N bytes

dumpb \b1f_00_0a_e3 [$pc-2] # You can also dump raw bytes. Useful if you want to write several bytes at the same time.
# The function byteness does not need to match the array byteness. For example.

dumpb \we30a001f # Does the same thing as the function above. Useful for avoiding endianess mismatch errors.

ass $(ADD r0, r1, #128) [$pc-2] # Assembles the following instrucion and yields its bytes.
# Similar to dump, but compiling the input first. 
# Multiple instructions can be passed by appending (instr) after the first (you skip the $ at the start).
3 If you want to assemble a thumb function, use T() instead of $().
# The previous example is overriding whatever instruction would be executed next.

dass [$pc-2:+2i] # Disassembles the instructions inside the range.

dasst [] # Disassemble the current instruction forcing thumb mode

fillw \w00 [0x00:+10w] # Fills 10 words starting at position 0x00 with 0x00000000. This is similar to dump, Except that dump you go from range to value, where fill goes from value to range.

findw 0x10 [r0:+10] # This one is a pretty powerful one.
# Finds the word 0x00000010 inside the vector at r0 and yields two values! The first one is the absolute address where the value was found.
# The second value is the index relative to the start of the search. If you only want the second yield, you can pass _ for the first yield to ignore it.

findw $(B 0x128) [] # Search in the entire program the specified instruction.

findw $$(BL .*) [$ROM:+0xFFFF] # Search any branch with link instruction inside the range at the start of rom (0x08000000) plus 0xFFFF.

findw $$(ADD r0, r?, r1) [] # Search any add function which adds any register to r1 and store the result to r0.

# These findw functions with $$() essentially disassembles eaach instruction inside the range and compare with regex.

############## VIAJANDO AQUI AGORA

trigger break write [0x00:0xFF] # Registers a trigger to add a break point to any instruction which attempts to write inside range.
# Possible handlers are break, to stop before the instruction is executed, intercept, to enter instruction intercept mode, warn, to just print to screen.

execute $(SUB r0, r1, r2) # Execute this instruction.

execute $V(ADD $z, $x, $y) # Execute this instructions using these variables instead of registers

execute ${}(ADD {z}, {x}, {y}) # Execute with string interpolation.

compile [0x00:0xFF] # Compiles the ARM instructions inside range to x86 instructions.

compile [0x00:0xFF] hints @mode=thumb @at(0xFF)r0=128 
