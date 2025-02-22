

#include <kernel.h>    



/*
 ***************************************************
 * get_cpu_intel_parameters:
 *     Pega os parâmetros da cpu x64 através da instrução cpuid.
 *
 * Vendor               = 0.
 * Apic ??              = 1.
 * Number of cores      = 4.
 * Name                 = 0x80000002,0x80000003,0x80000004. (Processor Brand String).
 * Max feature id       = 0x80000000.
 * L2 cache information = 0x80000006.(Extended L2 Cache Features)
 * core per die         = 0x80000008.
 */

// #todo:
// apic
// The CPUID.01h:EDX[bit 9] flag 
// specifies whether a CPU has a built-in local APIC. 


// According to the Intel CPUID application note, 
// we should first check the Vendor ID String for "GenuineIntel" 
// before taking out information, such as 
// the Processor Signature, Processor Feature Flags, etc. 

void get_cpu_intel_parameters (void)
{

    unsigned long eax=0; 
    unsigned long ebx=0; 
    unsigned long ecx=0;
    unsigned long edx=0;

    unsigned long name[32];

    int MASK_LSB_8 = 0xFF;


    debug_print ("get_cpu_intel_parameters: [FIXME]\n");


    //========================================
    // EAX=0: Highest Function Parameter and Manufacturer ID
    // #todo: eax
    //eax = Maximum meaningful value for the InfoType parameter. 
    // Vendor.
    //ebx = Identification String (part 1)
    //ecx = Identification String (part 3)
    //edx = Identification String (part 2)

    cpuid ( 0, eax, ebx, ecx, edx ); 
    name[0] = ebx;
    name[1] = edx;
    name[2] = ecx;
    name[3] = 0;

    processor->Vendor[0] = ebx;
    processor->Vendor[1] = edx;
    processor->Vendor[2] = ecx;
    processor->Vendor[3] = 0;

    hal_set_machine_type(Processor_NULL);
    
    // Intel?
    // #todo: definir Intel como 1.
    if ( ebx == CPUID_VENDOR_INTEL_1 && 
         edx == CPUID_VENDOR_INTEL_2 && 
         ecx == CPUID_VENDOR_INTEL_3 )
    {
        hal_set_machine_type(Processor_INTEL);
    }


    //========================================
    // EAX=1: Processor Info and Feature Bits

	//
	// Output para argumento 1. 
	//
	
    //eax:
    // 0- 3 Stepping ID
    // 4- 7 Model
    // 8-11 Family
    //12-13 Processor Type
    //14-15 Reserved
    //16-19 Extended model
    //20-27 Extended family
    //28-31 Reserved

    //ebx:
    // 0- 7 Brand Index
    // 8-15 CLFLUSH cache line size / 8
    //16-23 Reserved
    //24-31 APIC Physical ID

	//ecx:
	//0 SSE3 New Instructions
	//1-2 Reserved
	//3 MONITOR/MWAIT
	//4 CPL Qualified Debug Store
	//5-7 Reserved
	//8 Thermal Monitor 2
	//9 Reserved
	//10 L1 Context ID
	//11-31 Reserved
	
	//edx: 
	//0 FPU x87 FPU on Chip
	//1 VME Virtual-8086 Mode Enhancement
	//2 DE Debugging Extensions
	//3 PSE Page Size Extensions
	//4 TSC Time Stamp Counter
	//5 ~~> MSR RDMSR and WRMSR Support ***
	//6 PAE Physical Address Extensions
	//7 MCE Machine Check Exception
	//8 CX8 CMPXCHG8B Inst.
	//9 ~~~> APIC APIC on Chip ***
	//10 n/a Reserved
	//11 SEP SYSENTER and SYSEXIT
	//12 MTRR Memory Type Range Registers
	//13 PGE PTE Global Bit
	//14 MCA Machine Check Architecture
	//15 CMOV Conditional Move/Compare Instruction
	//16 PAT Page Attribute Table
	//17 PSE Page Size Extension
	//18 PSN Processor Serial Number
	//19 CLFSH CFLUSH Instruction
	//20 n/a Reserved
	//21 DS Debug Store
	//22 ACPI Thermal Monitor and Clock Ctrl
	//23 MMX MMX Technology
	//24 FXSR FXSAVE/FXRSTOR
	//25 SSE SSE Extensions
	//26 SSE2 SSE2 Extensions
	//27 SS Self Snoop
	//28 HTT Hyper-threading technology
	//29 TM Thermal Monitor
	//30 n/a Reserved
	//31 PBE Pend. Brk. En.
	
	

    cpuid ( 1, eax, ebx, ecx, edx );

	// eax:
	// Processor Version Information 
	processor->Stepping_ID        = (unsigned long)( eax        & 0xF);  //stepping
	processor->Model              = (unsigned long)((eax >> 4)  & 0xf);  //model
	processor->Family_ID          = (unsigned long)((eax >> 8)  & 0xf);  //family
	processor->Processor_Type     = (unsigned long)((eax >> 12) & 0x3);  //processor type
	processor->Extended_Model_ID  = (unsigned long)((eax >> 16) & 0xf);  //extended model
	processor->Extended_Family_ID = (unsigned long)((eax >> 20) & 0xff); //extended family
    // 31 ~ 28 Reserved??

	//ebx:
	// Additional Information 
	//processor->xx = (unsigned long)((ebx >> 9) & 0x0001);
    //... 



    // ecx e edx:
    // Feature Information 

	//edx:
	// The CPUID.01h:EDX[bit 9] flag 
	// specifies whether a CPU has a built-in local APIC. 
    processor->isApic = (unsigned long)((edx >> 9) & 0x0001);
	//processor->isApic = (unsigned long) edx;
	
	
	//The presence of MSRs on your processor is 
    //indicated by CPUID.01h:EDX[bit 5].
    //processor->hasMSR = (unsigned long)((edx >> 5) & 0x0001);
	//...
	
	// #todo:
	// Mostrar uma mensagem, se tem ou não apic;


    //========================================
    // EAX=2: Cache and TLB Descriptor information
    
    // ...
    
    //========================================
    // EAX=3: Processor Serial Number
    
	//Serial number ???
	//cpuid( 3, eax, ebx, ecx, edx );


    //========================================
    // EAX=4 and EAX=Bh: Intel thread/core and cache topology
    
	//Check number of cores.
	//cpuid( 4, eax, ebx, ecx, edx );



    //========================================
    // EAX=6: Thermal and power management
    

    //========================================
    // EAX=7, ECX=0: Extended Features
    // EAX=7, ECX=1: Extended Features
    

    // ==========================================================
    // hypervisor level 4000_0000h
    cpuid ( 0x40000000, eax, ebx, ecx, edx );
    processor->hvName[0] = ebx;
    processor->hvName[1] = ecx;
    processor->hvName[2] = edx;
    processor->hvName[3] = 0;



    //========================================
    // EAX=80000000h: Get Highest Extended Function Implemented

    // Max feature id.
    cpuid ( 0x80000000, eax, ebx, ecx, edx);
    name[0] = eax;  //Maximum meaningful value of InfoType for extended function CPUID information.
    name[1] = ebx;  //reserved
    name[2] = ecx;  //reserved
    name[3] = edx;  //reserved
    name[4] = 0; 
    processor->MaxFeatureId = (unsigned long)(eax & MASK_LSB_8);
    //printf("Max feature id ={%d}\n", (unsigned long) processor->MaxFeatureId);


    //========================================
    // EAX=80000001h: Extended Processor Info and Feature Bits
    
    //========================================
    // EAX=80000002h,80000003h,80000004h: Processor Brand String
    
    /*name part 1*/
    cpuid ( 0x80000002, eax, ebx, ecx, edx);
    name[0] = eax;  //Processor Brand String
    name[1] = ebx;  //Processor Brand String
    name[2] = ecx;  //Processor Brand String
    name[3] = edx;  //Processor Brand String
    name[4] = 0;
    //salva na estrutura.
    processor->BrandName[0] = eax;
    processor->BrandName[1] = ebx;
    processor->BrandName[2] = ecx;
    processor->BrandName[3] = edx;
	//printf("CPU={%s", &name[0]);
	//printf("%s",&name[0]);		

   /*name part 2*/
    cpuid ( 0x80000003, eax, ebx, ecx, edx);
    name[0] = eax;  //Processor Brand String, continued
    name[1] = ebx;  //Processor Brand String, continued
    name[2] = ecx;  //Processor Brand String, continued
    name[3] = edx;  //Processor Brand String, continued
    name[4] = 0;
	//salva na estrutura.	
    processor->BrandName[4] = eax;
    processor->BrandName[5] = ebx;
    processor->BrandName[6] = ecx;
    processor->BrandName[7] = edx;
	//printf("%s",&name[0]);

   /*name part 3*/
    cpuid ( 0x80000004, eax, ebx, ecx, edx);
    name[0] = eax; //Processor Brand String, continued
    name[1] = ebx; //Processor Brand String, continued
    name[2] = ecx; //Processor Brand String, continued
    name[3] = edx; //Processor Brand String, continued
    name[4] = 0;
	//salva na estrutura.	
    processor->BrandName[8]  = eax;
    processor->BrandName[9]  = ebx;
    processor->BrandName[10] = ecx;
    processor->BrandName[11] = edx;
	//printf("%s}\n",&name[0]);	


    //========================================
    // EAX=80000005h: L1 Cache and TLB Identifiers




    //========================================
    // EAX=80000006h: Extended L2 Cache Features
 
    // pegamos logo acima.
    if ( processor->MaxFeatureId < 6)
    {
        debug_print ("get_cpu_intel_parameters: Cache Extended Feature not supported\n");
        //printf("Cache Extended Feature not supported\n");
        //goto done;
        return;
    }

    /*
     * L2 cache information (Intel)
     *
     * EAX Reserved
     * EBX Reserved
     * ECX Bits:  
     *     Bits 0-7: Cache Line Size.
     *     Bits 12-15: L2 Associativity.
     *     Bits 16-31: Cache size in 1K units.   
     * EDX Reserved
     */

    cpuid ( 0x80000006, eax, ebx, ecx, edx );
    name[0] = eax;
    name[1] = ebx;
    name[2] = ecx;
    name[3] = edx;
    name[4] = 0;
    processor->L2LineSize      = (unsigned long)( ecx        & 0x00FF); //Bits 0-7: Cache Line Size.
    processor->L2Associativity = (unsigned long)((ecx >> 12) & 0x000F);	//Bits 12-15: L2 Associativity.
    processor->L2Cachesize     = (unsigned long)((ecx >> 16) & 0xFFFF); //Bits 16-31: Cache size in 1K units.
	//printf("L2LineSize={%d Byte}\n",(unsigned long) processor->L2LineSize);	
	//printf("L2Cachesize={%d KB}\n",(unsigned long) processor->L2Cachesize);


    //========================================
    // EAX=80000007h: Advanced Power Management Information



    //========================================
    //EAX=80000008h: 
    //Virtual and Physical address Sizes	
    //Returns largest virtual and physical address sizes.

    /*
     * Virtual and physical memory sizes.
     */

    cpuid ( 0x80000008, eax, ebx, ecx, edx );
    name[0] = eax;    //Virtual and physical memory sizes.
    name[1] = ebx;    //reserved
    name[2] = ecx;    //reserved
    name[3] = edx;    //reserved
    name[4] = 0;
    processor->Physical_Address_Size = (unsigned long) ( eax       & 0x00FF); //7-0
    processor->Virtual_Address_Size  = (unsigned long) ((eax >> 8) & 0x00FF); //15-8	
    //printf("Physical_Address_Size={%d}\n",(unsigned long) processor->Physical_Address_Size);
    //printf("Virtual_Address_Size={%d}\n", (unsigned long) processor->Virtual_Address_Size);

    //...
}


void x64_init_fpu (void) 
{
    asm volatile ("fninit");
}


// x64 disable interrupts.
void x64_disable_interrupts (void)
{
    asm ("cli"); 
}

// x64 enable interrupts.
void x64_enable_interrupts (void)
{
    asm ("sti");
}


void x64_iretq (void)
{
    asm ("iretq");
}

void x64_iret (void)
{
    asm ("iret");
}

void x64_lret (void)
{
    asm ("lret");
}

void x64_cli_hlt(void)
{
    asm (" cli \n \t "); 
    asm (" hlt \n \t ");
}

void x64_stop_cpu (void)
{
    x64_cli_hlt();
}


// Enable cache.
// credits: barrelfish.
// #todo: Test it on my real machine.

void x64_enable_cache (void)
{
    /*
    uint32_t cr0 = 0; 
    unsigned long cr0 = 0; //use this one ??

    asm volatile ("mov %%cr0, %[cr0]" : [cr0] "=r" (cr0) );

    cr0 &= ~CPUX64_CR0_CD;
    cr0 &= ~CPUX64_CR0_NW;
    
    asm volatile ("mov %[cr0], %%cr0" :: [cr0] "r" (cr0) );
    */
}

// Called by init_architecture_dependent() on core/init.c

int x64_init_intel (void)
{
    debug_print ("[x64] x64_init_intel:");


    //x64fault_initialize();

    // #bugbug
    // Suspended.
    // We need to teste the effects os this routine in the real machine.
    // See: sysio/hal/arch/x86/x86.h
    // See: sysio/hal/arch/x86/x86.c
    
    //x64_enable_cache();

    // Get info.
    // See: x86.c
    get_cpu_intel_parameters();

    // ...

    return 0;
}










