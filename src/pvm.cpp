#define PHOTON_IMPLEMENTATION
#include "PhotonVM.h"

namespace HostCalls
{
    /** Enumeration of all standard Host-Call function groups. */
	enum HC_GROUPS
	{
		HC_GROUP_CORE = 0 // Core group containing all basic calls.
	};

	/** Enumeration of all standard Host-Call function ids. */
	enum HC_FUNCTIONS
	{
		HC_FUNCTION_GET_VERSION,		///< Get the version number of the VM that the byte-code runs on.
		HC_FUNCTION_PRINT_VALUE,		///< Print the value to the standard output that is stored in the 'local' register as a signed integer.
        HC_FUNCTION_PRINT_CHARACTER,	///< Print the chacacter to the standard output that is stored in the 'local' register.
        // Only for testing and examples, some of these aren't even verry usefull.
        HC_FUNCTION_PRINT_VERSION,	    ///< Print the version info to the standard output.
        HC_FUNCTION_DUMP_REGISTERS,
    };

    HostCallback(getVersion)
    {
        Photon::RegisterType param = registers[Photon::Local];
		if(param == 3)
			registers[Photon::Local] = PHOTON_VM_VERSION_SUB_MINOR;
		else if(param == 2)
			registers[Photon::Local] = PHOTON_VM_VERSION_MINOR;
		else
			registers[Photon::Local] = PHOTON_VM_VERSION_MAJOR;
    }

    HostCallback(printValue)
    {
        fprintf(stdout, "%d", registers[Photon::Local]);
    }

    HostCallback(printChar)
    {
        putc(registers[Photon::Local], stdout);
    }

    HostCallback(printVersion)
    {
        printf("Photon Virtual-Machine V.%d.%d.%d\n", PHOTON_VM_VERSION_MAJOR, PHOTON_VM_VERSION_MINOR, PHOTON_VM_VERSION_SUB_MINOR);
    }

    HostCallback(dumpRegisters)
    {
        printf("Register Dump:\n");
        
        for(uint16_t i = 0, end = (Photon::RegisterCount - 1); i < end; i += 2)
            printf("\treg%.2u = %d\t| reg%.2u = %d\n", i , registers[i], i + 1, registers[i + 1]);
        printf("\treg12/local = %d\n", registers[Photon::Local]);
    }


    // Register all host calls.
    static void registerHostCalls(Photon::VirtualMachine* vm)
    {
        Photon::registerHostCall(vm, printVersion, HC_GROUP_CORE, HC_FUNCTION_PRINT_VERSION);
        Photon::registerHostCall(vm, getVersion, HC_GROUP_CORE, HC_FUNCTION_GET_VERSION);
        Photon::registerHostCall(vm, printValue, HC_GROUP_CORE, HC_FUNCTION_PRINT_VALUE);
        Photon::registerHostCall(vm, printChar, HC_GROUP_CORE, HC_FUNCTION_PRINT_CHARACTER);
        Photon::registerHostCall(vm, dumpRegisters, HC_GROUP_CORE, HC_FUNCTION_DUMP_REGISTERS);
    }
}

DebugCallback(myCallback)
{
    printf("Instruction op code: %d, reg0=%d\n", instruction->opCode, registers[Photon::Reg0]);
}

static void printEncodedByteCode(Photon::ByteCode* byteCode)
{
    if(byteCode)
    {
        printf("---------------------------------------\n");
        printf("Encoded byte-code: \n");
        for(uint32_t i = 0; i < byteCode->instructionCount; ++i)
        {
            printf("0x%.4hX\n", *(byteCode->instructions + i));
        }
        printf("Stats: %u total instructions, %zu bytes\n", byteCode->instructionCount, byteCode->instructionCount * sizeof(Photon::RawInstruction));
        printf("---------------------------------------\n");
    }
}


int main(int argc, char** argv)
{
    Photon::ByteCode byteCode;
    Photon::VirtualMachine vm;    

#if 0
    char* source = R"FOO(
        set reg0 3
        set reg1 7
        add reg12 reg0 reg1
        cpy reg6 reg1
        hcl 0 4
        halt 0
        )FOO";
#else
    char* source = R"Foo(
        # Mapping Table:
        # -----------------
        # N:        | reg0
        # Fib       | reg1
        # FibN-1    | reg2 tmp0
        # FibN-2    | reg3 tmp1
        # i         | reg4 tmp2 
        # blockSize | reg5 tmp3
        # local <= reg12
        # ------------------
        
        # Defines the iteration count of the algorithm.
        set reg0 18
        
        # Define the variables that are used to compute the sequence.
        set reg1 0
        set reg2 0
        set reg3 1
        
        
        # reg5 = instruction count of loop-block.
        set reg5 9
        # reg4 = start index of loop (i).
        set reg4 2
        
        # while(i < N) ...
        gre reg12 reg4 reg0
        mul reg12 reg12 reg5
        jmp reg12 0
        # BEGIN - While
            add reg1 reg2 reg3
            cpy reg2 reg3
            cpy reg3 reg1
            
            # Increment the loop counter 'i'.
            set reg12 1
            add reg4 reg4 reg12
            # jump back to the loop-head.
            set reg12 10
            inv reg12
            jmp reg12 0
        # END - While
        hcl 0 4
        halt 0
    )Foo";
#endif
    
    byteCode = Photon::compile(source, __FILE__);
    printEncodedByteCode(&byteCode);
    vm = Photon::createVirtualMachine(byteCode, Photon::VerbosityLevelAll);
    HostCalls::registerHostCalls(&vm);
    Photon::setDebugCallback(&vm, myCallback);

    // Note that when executing byte code, the VM will never assume that the byte code is correct.
    Photon::VMExitCode result = Photon::run(&vm);
    if(result != 0)
    {
        printf("VM Exited with code: %d\n", result);
    }

    Photon::releaseByteCode(&byteCode);

    return 0;
}