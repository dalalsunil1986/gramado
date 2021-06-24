

#include <kernel.h>  


void I8042Controller_do_drain(void)
{
    //#define I8042_BUFFER 0x60
    //#define I8042_STATUS 0x64
    //#define I8042_BUFFER_FULL 0x01
    
    unsigned char status=0;
    
    for (;;) {
        status = in8(0x64);
        if (!(status & 0x01)){ return;}  //empty
        in8(0x60);
    }
}  

/*
 *********************
 * kbdc_wait:
 *     Espera por flag de autorização para ler ou escrever.
 */

// #??
// is this the besto option fo x86_64?
#define __local_out_any_b(p)  asm volatile ( "outb %%al,%0" : : "dN"((p)) : "eax" )

// Espera para ler ou para escrever!

#define __I8042_BUFFER_FULL  0x01

void kbdc_wait (unsigned char type)
{
    unsigned char Status=0;
    //unsigned char Type=0;   //#todo
    
    // 0 = READ
    if (type==0)
    {
        for (;;) 
        {
            Status = in8(0x64);
           
           // Sinalizado que o buffer ta cheio.
           // # Somente para mouse.
           // See: serenity os.
           if ( (Status & __I8042_BUFFER_FULL) != 0 )  
               return;

            // #todo: delay
            // __local_out_any_b (0x80);
            // wait_ns (400);

        };
    }


    // 1 = WRITE
    if (type==1)
    {
        for (;;)
        {
            if ( !(in8(0x64) & 2) )
                return;
            
            // #todo: delay
            // __local_out_any_b (0x80);
            // wait_ns (400);
        };
    };
}  


// =======================
// prepare ..

void prepare_for_input(void)
{
    // 0 = READ
    kbdc_wait(0);
}

void prepare_for_output(void)
{
    // 1 = WRITE
    kbdc_wait(1);
}

//====================================================================

//====================================================================


// =======================
// wait then ...

unsigned char wait_then_read (int port)
{
    prepare_for_input();
    return (unsigned char) in8 (port);
}

void wait_then_write ( int port, int data )
{
    prepare_for_output();
    out8 ( port, data );
}


// This is called by gdeshell.
int PS2_initialize(void)
{
    debug_print ("PS2_initialize: [TODO]\n");
	//ps2();
    return 0;
}


// this is called during the kernel initialization.
// Called by x64main in x64init.c
int PS2_early_initialization(void)
{
    debug_print ("PS2_early_initialization: [TODO]\n");
	//early_ps2_init();
    return 0;
}

int ps2_ioctl ( int fd, unsigned long request, unsigned long arg )
{
    debug_print("ps2_ioctl: [TODO]\n");
    return -1;
}


/*
 ***************
 * ps2:
 *     Inicializa o controlador ps2.
 * 
 *     Essa rotina de inicialização do controlador 
 * poderá ter seu próprio módulo.
 * 
 *     Inicializa a porta do teclado no controlador.
 *     Inicializa a porta do mouse no controlador.
 *     Obs: *importante: A ordem precisa ser respeitada.
 *     As vezes os dois não funcionam ao mesmo tempo se a 
 *     inicialização não for feita desse jeito. 
 */


// Essa é uma inicializaçao completa.
// See:
// https://wiki.osdev.org/%228042%22_PS/2_Controller

void ps2(void)
{
    int keyboard_available = FALSE;
    int mouse_available    = FALSE;

    unsigned char configuration=0;
    int is_dual_channel=0;


    // #debug
    printf ("ps2: Initializing..\n");
    refresh_screen();

    // The main structure fisrt of all.
    // #todo: create ps_initialize_main_structure();
    PS2.used  = TRUE;
    PS2.magic = 1234;
    PS2.keyboard_initialized = FALSE;
    PS2.mouse_initialized    = FALSE;
    PS2.pooling = FALSE;


    // Zerando as estruturas que precisam serem reinicializadas.

    // keyboard
    PS2KeyboardDeviceObject    = NULL;
    PS2KeyboardDevice          = NULL;
    PS2KeyboardDeviceTTYDriver = NULL;
    PS2KeyboardDeviceTTY       = NULL;

    // mouse
    PS2MouseDeviceObject    = NULL;
    PS2MouseDevice          = NULL;
    PS2MouseDeviceTTYDriver = NULL;
    PS2MouseDeviceTTY       = NULL;


    // See the steps in:
    // https://wiki.osdev.org/%228042%22_PS/2_Controller

    // ======================================
    // Disable devices
    // Deactivate ports!
    wait_then_write (0x64,0xAD);  // Disable keyboard port.
    wait_then_write (0x64,0xA7);  // Disable mouse port. ignored if it doesn't exist
    // =================

    // Drain buffers
    I8042Controller_do_drain();

    //
    // configuration byte
    //

    // # todo
    // # Essa rotina desabilita as irqs
    // precisamos reabilita-las ao fim da inicializaçao
    wait_then_write (0x64,I8042_READ);    // I8042_READ = 0x20    
    configuration = wait_then_read(0x60);
    //wait_then_write (0x64,I8042_WRITE);   // I8042_WRITE = 0x60
    //configuration &= ~3; // Disable IRQs for all
    //wait_then_write (0x60,configuration);   

    // Check if ti is dual channel or not.
    // Checa um bit.
    // While you've got the Configuration Byte, 
    // test if bit 5 was set. If it was clear, 
    // then you know it can't be a "dual channel" PS/2 controller 
    // (because the second PS/2 port should be disabled). 

    is_dual_channel = (configuration & (1 << 5)) != 0;
    if (is_dual_channel == 1){ printf("Dual\n");   }
    if (is_dual_channel == 0){ printf("Single\n"); }


    // #define SELF_TEST_SUCCESS 0x55

    // Perform controller self-test
    // To test the PS/2 controller, send command 0xAA to it. 
    // Then wait for its response and check it replied with 0x55.
    
    wait_then_write(I8042_STATUS, 0xAA);
    
    if (wait_then_read(I8042_BUFFER) == 0x55){
        printf( "I8042: Self test OK\n");
        // Restore configuration in case the controller reset
        // At the very least, the Controller Configuration Byte 
        // should be restored for compatibility with such hardware.
        wait_then_write(I8042_STATUS, 0x60);
        wait_then_write(I8042_BUFFER, configuration);
     } else {
         panic ( "I8042: [FATAL.FIXME] Controller self test failed\n");
     };


    //
    // Step 8: Perform Interface Tests 
    //

    // This step tests the PS/2 ports. 
    // Use command 0xAB to test the first PS/2 port, 
    // then check the result. 
    // Then (if it's a "dual channel" controller) use command 0xA9 
    // to test the second PS/2 port, then check the result. 

    // Nesse momento podemos desistir se a porta que queremos não
    // está disponível. Ou prosseguirmos apenas com o que temos.

    // ==============================
    // Test ports and enable them if available

    // testar se o keyboard esta disponivel.
    wait_then_write(I8042_STATUS, 0xab); // test
    keyboard_available = (wait_then_read(I8042_BUFFER) == 0);

    // testar se o mouse esta disponivel.
    // pra isso precisamos ser dual channel.
    if (is_dual_channel == 1) {
        
        wait_then_write(I8042_STATUS, 0xa9); // test
        mouse_available = (wait_then_read(I8042_BUFFER) == 0);
    }


//#define STAT_OUTFULL         (1<<0)
//#define STAT_INFULL         (1<<1)
//#define STAT_SYSTEM_FLAG      (1<<2)
//#define STAT_FOR_CONTROLER   (1<<3)
//#define STAT_SPECIFIC      (1<<4)
//#define STAT_MOUSE_DATA      (1<<5)
//#define STAT_TIMEOUT_ERR      (1<<6)
//#define STAT_PARITY_ERR      (1<<7)

//#define CONFIG_PORT1_INT      (1<<0)
//#define CONFIG_PORT2_INT      (1<<1)
//#define CONFIG_SYSTEM_FLAG   (1<<2)
//#define CONFIG_ZERO1         (1<<3)
//#define CONFIG_PORT1_CLK_DS   (1<<4)
//#define CONFIG_PORT2_CLK_DS   (1<<5)
//#define CONFIG_PORT1_TRANSL   (1<<6)
//#define CONFIG_ZERO2         (1<<7)


    //
    // Step 9: Enable Devices 
    //

    // imprimir o resultado da disponibilidade.
    
    if (keyboard_available == 1){
        printf("----\n");
        printf("~ Keyboard available\n");
        ps2kbd_initialize_device();
        PS2.keyboard_initialized = TRUE;
        wait_then_write(I8042_STATUS, 0xae);  // enable keyboard port
        
        //ignoramos isso se é single channel
        // configuration |= 1;
        // configuration &= ~(1 << 4);
        // wait_then_write(I8042_STATUS, 0x60);
        // wait_then_write(I8042_BUFFER, configuration);
    }


    // If it is dual channel,
    // send a command 0xA8 to enable the second PS/2 port and 
    // read the Controller Configuration Byte again
    if (mouse_available == 1 ){
        printf("----\n");
        printf("~ Mouse available\n");
        
        // #todo
        ps2mouse_initialize_device();
        
        
        PS2.mouse_initialized = TRUE;
        wait_then_write(I8042_STATUS, 0xa8);  // enable mouse port

        //pega o config byte
        wait_then_write (0x64,I8042_READ);    // I8042_READ = 0x20    
        configuration = wait_then_read(0x60);

        // #todo
        // Now bit 5 of the Controller Configuration Byte should be clear - 
        // if it's set then you know it can't be a "dual channel" 
        // PS/2 controller (because the second PS/2 port should be enabled).
        
        configuration |= 2;
        configuration &= ~(1 << 5);
        wait_then_write(I8042_STATUS, 0x60);
        wait_then_write(I8042_BUFFER, configuration);
    
        // If it is a dual channel device, 
        // send a command 0xA7 to disable the second PS/2 port again. 
        // Lembre-se, vamos reabilitar no final.
        wait_then_write (0x64,0xA7);  // Disable mouse port. ignored if it doesn't exist
    }
    // ==============================

   //#todo
   // Enable IRQs for the ports that are usable
   if (keyboard_available || mouse_available) 
   {
        //pega o config byte
       //wait_then_write (0x64,I8042_READ);    // I8042_READ = 0x20    
       //configuration = wait_then_read(0x60);

       configuration &= ~0x30; // renable clocks
       wait_then_write(I8042_STATUS, 0x60);
       wait_then_write(I8042_BUFFER, configuration);
    }  


    //==========================

    // Wait for nothing!
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);

    // Done.

    //#debug
    printf ("ps2: done\n");
    refresh_screen();
}


/*
 ************************************************************
 * early_ps2_init:
 */

// Inicialização preliminar. Sem mouse.
// Ela existe porque a emulação de ps2 na máquina real 
// apresenta falhas.
// No gdeshell.bin incluiremos os comando "ps2-init"
// para obtermos a inicialização completa.

// #importante
// Nao chamamos a rotina de inicializaçao do mouse.
// mas poderiamos. O importante eh deixar a porta desabilitada
// ao final da rotina.
// Ao fim dessa rotina, reabilitamos apenas a porta de teclado.
// A porta de mouse permaneçe fechada.

//#todo

void early_ps2_init (void)
{
	// #debug
    printf ("early_ps2_init: Initializing..\n");
    refresh_screen();

    // The main structure fisrt of all.
    // #todo: create ps_initialize_main_structure();
    PS2.used  = TRUE;
    PS2.magic = 1234;
    PS2.keyboard_initialized = FALSE;
    PS2.mouse_initialized    = FALSE;
    PS2.pooling = FALSE;

    // Zerando estruturas que precisam serem reinicializadas.
    
    //keyboard
    PS2KeyboardDeviceObject = NULL;
    PS2KeyboardDevice = NULL;
    PS2KeyboardDeviceTTYDriver = NULL;
    PS2KeyboardDeviceTTY = NULL;
    
    //mouse
    PS2MouseDeviceObject = NULL;
    PS2MouseDevice = NULL;
    PS2MouseDeviceTTYDriver = NULL;
    PS2MouseDeviceTTY = NULL;


    // ======================================
    // Deactivate ports!
    wait_then_write (0x64,0xAD);  //Disable keyboard port.
    wait_then_write (0x64,0xA7);  //Disable mouse port.

    // Keyboard.
    
    // #debug: slow
    printf ("early_ps2_init: Initializing keyboard ..\n");
    refresh_screen();
    
    // #todo
    
    ps2kbd_initialize_device();
    
    PS2.keyboard_initialized = TRUE;

    // Mouse.
    //printf ("ps2: Initializing mouse ..\n");
    //refresh_screen();
    //ps2mouse_initialize_device ();
    PS2.mouse_initialized = FALSE;

    // ======================================
    // Reactivate ports!
    wait_then_write (0x64,0xAE);    // Reenable keyboard port.
    //wait_then_write (0x64,0xA8);  // do NOT reenable the mouse port.


    // We can call this one.
    // wait_then_write (0x64,0xA7);  //Disable mouse port.


    // Wait for nothing!
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);

    // Done.

    //#debug
    printf ("early_ps2_init: done\n");
    refresh_screen();
}























