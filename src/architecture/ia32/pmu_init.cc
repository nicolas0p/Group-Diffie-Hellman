// EPOS IA32 PMU Mediator Initialization

#include <pmu.h>

__BEGIN_SYS

void PMU::init()
{
    db<Init, PMU>(TRC) << "PMU::init()" << endl;

    // Check if the CPU supports CPUID
    CPU::eflags(CPU::eflags() | CPU::FLAG_ID);
    if(!(CPU::eflags() & CPU::FLAG_ID)) {
        db<Init, PMU>(WRN) << "PMU::init: CPU does not support CPUID! PMU won't be available!" << endl;
        return;
    }

    // Enable rdpmc for any protection level
    CPU::cr4((CPU::cr4() | CPU::CR4_PSE));

    Reg32 eax, ebx, ecx = 0, edx;

    // Get CPUID max level (EAX) and vendor id (EBX+EDX+ECX)
    char vendor_id[13];
    vendor_id[12] = '\0';
    CPU::cpuid(0, &eax, (Reg32 *)&vendor_id[0], (Reg32 *)&vendor_id[8], (Reg32 *)&vendor_id[4]);
    db<Init, PMU>(INF) << "PMU::init:CPUID(0)={max=" << eax << ",vendor=" << vendor_id << "}" << endl;

    int cpu_family = 4;
    int cpu_model = 0;
    int cache_line_size = -1;
    if(eax != 0) {
        // Get version information and cache line organization
	CPU::cpuid(1, &eax, &ebx, &ecx, &edx);
	cpu_family = (eax >> 8) & 0xf;
	if(cpu_family == 0xf)
	    cpu_family += (eax >> 20) & 0xff;
        cpu_model = (eax >> 4) & 0xf;
	if((cpu_family == 0x6) || (cpu_family == 0xf))
	    cpu_model += ((eax >> 16) & 0xf) << 4;

	cache_line_size = ((ebx >> 8) & 0xff) * 8;
    }
    db<Init, PMU>(INF) << "PMU::init:CPUID(1)={family=" << cpu_family << ",model=" << cpu_model << ",cache line=" << cache_line_size << "}" << endl;

    CPU::cpuid(10, &eax, &ebx, &ecx, &edx);
    db<Init, PMU>(INF) << "PMU::init:CPUID(10)={eax=" << hex << eax << ",ebx=" << ebx << ",ecx=" << ecx << ",edx=" << edx << "}" << endl;
    int version = eax & 0xff;
    int counters = (eax >> 8) & 0xff;
    int cntval_bits = (eax >> 16) & 0xff;
//    int cntval_mask = (eax >> 24) & 0xff;
    int counters_fixed = 0;
    if(version > 1)
	counters_fixed = edx & 0xf;

    db<Init, PMU>(INF) << "PMU::init:CPUID(10)={ver=" << version << ",counters=" << counters << ",width=" << cntval_bits << ",fixed counters=" << counters_fixed << "}" << endl;
}

__END_SYS
