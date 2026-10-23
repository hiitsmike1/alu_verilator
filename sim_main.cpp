#include "Valu.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <string>

// Global ALU instance, VCD trace, and simulation time
Valu* top;
VerilatedVcdC* tfp;
vluint64_t main_time = 0;

// Helper function to test any implemented ALU operation
int test_op(int8_t a, int8_t b, uint8_t op, int expected, const std::string& desc, bool force_dump = false) {
    top->a = a;
    top->b = b;
    top->op = op;
    top->eval();
    if (force_dump || a % 16 == 0 && b % 16 == 0)  // dump only every 16th step to reduce vcd size
        tfp->dump(main_time++);
    else
        main_time++;

    int8_t y_val = (int8_t)top->y;

    if (y_val != expected) {
        std::cout << "ERROR: " << desc
                  << " = " << static_cast<int>(y_val)
                  << " expected " << expected << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    // Instantiate ALU and VCD tracing
    top = new Valu;
    Verilated::traceEverOn(true);
    tfp = new VerilatedVcdC;
    top->trace(tfp, 200);
    tfp->open("alu.vcd");

    int errors = 0;
    // Short demo sequence for the README waveform (always dumped)
    int8_t da = 25, db = 10;
    errors += test_op(da, db, 0, static_cast<int8_t>(da + db), "demo add", true);
    errors += test_op(da, db, 1, static_cast<int8_t>(da - db), "demo sub", true);
    errors += test_op(da, db, 2, da & db, "demo and", true);
    errors += test_op(da, db, 3, da | db, "demo or",  true);
    errors += test_op(da, db, 4, da ^ db, "demo xor", true);

    // Test all 8-bit values for ADD, SUB, AND, OR, XOR
    for (int i = -128; i <= 127; i++) {
        for (int j = -128; j <= 127; j++) {

            int8_t a = static_cast<int8_t>(i);
            int8_t b = static_cast<int8_t>(j);

            // Wrapping the expected result signed 8 bits. (-128 - 127)
            int expected_add = static_cast<int8_t>(a + b);
            int expected_sub = static_cast<int8_t>(a - b);
            int expected_and = static_cast<int8_t>(a & b);
            int expected_or  = static_cast<int8_t>(a | b);
            int expected_xor = static_cast<int8_t>(a ^ b);

            char desc[32];
            sprintf(desc, "%d + %d", i, j);
            errors += test_op(a, b, 0, expected_add, desc);

            sprintf(desc, "%d - %d", i, j);
            errors += test_op(a, b, 1, expected_sub, desc);

            sprintf(desc, "%d & %d", i, j);
            errors += test_op(a, b, 2, expected_and, desc);

            sprintf(desc, "%d | %d", i, j);
            errors += test_op(a, b, 3, expected_or, desc);

            sprintf(desc, "%d ^ %d", i, j);
            errors += test_op(a, b, 4, expected_xor, desc);
        }
    }

    // Wrap up
    tfp->close();
    top->final();
    delete top;

    if (errors == 0) {
        std::cout << "ALU passed all tests!" << std::endl;
    } else {
        std::cout << "Simulation finished with " << errors << " errors." << std::endl;
    }

    return errors ? 1 : 0;
}
