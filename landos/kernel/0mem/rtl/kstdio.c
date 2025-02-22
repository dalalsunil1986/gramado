

#include <kernel.h>




/*
 *********************************************************
 * input:
 *     Coloca os caracteres digitados em um buffer, (string). 
 * Para depois comparar a string com outra string, que é um comando.
 * 
 *     Devemos nos certificar que input(.) não imprima nada.
 *
 * History:
 *     2015 - Created by Fred Nora.
 */




// #bugbug
// Acho que essa rotina em rin0 não é usada!
// Usamos apenas a que tem em ring3.

unsigned long input ( unsigned long ch )
{
    int i=0;

	// Converte 'unsigned long' em 'char'.
    char c = (char) ch;


    // Input mode.
    
    // #bugbug:
    // Estamos nos referindo ao tipo de linha, 
    // se é simples ou multiplas linhas.
    //  Mas temos outra flag relativa à imput mode 
    // que trata dos eventos.
    
    // See: ???
    // Onde estão as flags ???

    if ( g_inputmode == INPUT_MODE_LINE )
    {
        if (prompt_pos >= PROMPT_SIZE)
        {
            //printf ("kstdio-input: [FAIL] INPUT_MODE_LINE full buffer!\n");
            goto fail;
            // refresh_screen ();
            //return (unsigned long) 0; 
        }
    }

    //tem que ter o tamanho de um arquivo.
    if (g_inputmode == INPUT_MODE_MULTIPLE_LINES )
    {
        if (prompt_pos >= PROMPT_SIZE)
        {
            //printf("kstdio-input: [FAIL] INPUT_MODE_MULTIPLE_LINES full buffer\n");
            goto fail;
            //refresh_screen();
            //return (unsigned long) 0; 
        }
    }

    // Trata o caractere digitado. 

    switch (c){

	    //Enter.
		//+se for modo comando devemos finalizar com zero.
		//+se for modo texto, devemos apenas incluir os caracteres \r \n.
		//case 0x1C:
        case VK_RETURN:
            //modo linha 
            if (g_inputmode == INPUT_MODE_LINE )
            {
                prompt[prompt_pos] = (char )'\0'; //end of line.
			    //@todo: ?? ldiscCompare();
				//o compare está no aplicativo.
                for(i=0; i<PROMPT_MAX_DEFAULT;i++)
                {
                    prompt[i]     = (char) '\0';
                    prompt_out[i] = (char) '\0';
                    prompt_err[i] = (char) '\0';
                };
                prompt_pos = 0;
                goto input_done;
            }
            //modo multiplas linhas 
            if (g_inputmode == INPUT_MODE_MULTIPLE_LINES )
            {
                prompt[prompt_pos] = (char )'\r';  prompt_pos++;
                prompt[prompt_pos] = (char )'\n';  prompt_pos++;
            }
            break;

        // Backspace
        case 0x0E:

            if ( prompt_pos <= 0 )
            {
                prompt_pos = 0;
                prompt[prompt_pos] = (char ) '\0';
                break; 
            }

            // Apaga o anterior (no buffer).
            prompt_pos--;
            prompt[prompt_pos] = (char ) '\0';
            break;

        // ...

        // Para qualquer caractere que não sejam os especiais tratados acima.
        default:
            prompt[prompt_pos] = c;  
            prompt_pos++;
            break;
    };

input_more:
    return 0;
input_done:
    return VK_RETURN;
fail:
    refresh_screen();
    return (unsigned long) 0; 
}




/*
 *******************************************************
 * printchar:
 *     Coloca o caractere na string ou imprime.
 * Essa função chama uma rotina que deverá tratar o caractere e 
 * em seguida enviá-lo para a tela.
 * Essa rotina é chamada pelas funções: /print/printi/prints.
 */

void printchar (char **str, int c)
{
	// #importante
	// Se a string existe colocamos nela,
	// caso contrário imprimimos no backbuffer.
	// Vamos aproveitar esse momento para ativarmos a
	// pintura no caso dos caraters enviados para uma 
	// stream de output, como stdout.

	// Ativaremos a rotina de mostrar na tela só no momento em que 
	// encontramos um fim de linha.

    if (str)
    {
        if ( c == '\n' ){
            // tty->print_pending = 1;
        }

        **str = c;

        ++(*str);

    } else (void) putchar(c);
}



/*
 ********************************************************************
 * putchar:
 *     Put a char on the screen. (libC).
 *     Essa rotina chama uma rotina de tratamento de caractes, somente
 * depois é que o caractere será enviado para a tela.
 *     Essa rotina é chamada pelas funções: /printchar/input/.
 */

int putchar (int ch)
{ 
    // Para virtual consoles.
    // Em tty/console.c

    console_outbyte ( ch, fg_console );

    // Para pseudo terminal (pts).
    //Em tty/vt.c
    //vt_outbyte (ch,tty );

    return (int) ch;
}




/*
 *======================================================================
 *  ==== Segue aqui o suporte a função 'printf' ====
 *
 * #obs:
 * Em user mode temos uma modelo mais tradicional de printf,
 * talvez seja bom implementa-lo aqui também.
 */

/*
 *****************************
 * prints:
 *     Rotina de suporta a printf. 
 */
 
// #bugbug
// We need to create a cleaner routine.
// This one is a mess.

int 
prints ( 
    char **out, 
    const char *string, 
    int width, 
    int pad )
{

    register int pc = 0, padchar = ' ';

    register int len = 0;
    register const char *ptr;

    if (width > 0) 
    {
        // ?? ugly
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }


    if ( !(pad & PAD_RIGHT) ) 
    {
        for ( ; width > 0; --width)
        {
            printchar (out,padchar);
            ++pc;
        };
    }


    for ( ; *string; ++string )
    {
        printchar (out, *string);
        ++pc;
    };


    for ( ; width > 0; --width )
    {
        printchar (out,padchar);
        ++pc;
    };


    return (int) pc;
}

/*
 ****************************************
 * printi:
 *     Rotina de suporta a printf.
 */

int 
printi (
    char **out, 
    int i, 
    int b, 
    int sg, 
    int width, 
    int pad, 
    int letbase )
{

    char print_buf[PRINT_BUF_LEN];
    
    register char *s;
    register int t, neg = 0, pc = 0;
    
    // loop
    register unsigned int u = i;


    if ( i == 0 ) 
    {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        
        return (int) prints (out, print_buf, width, pad);
    }


    if ( sg && b == 10 && i < 0 )
    {
        neg = 1;
        u = -i;
    }


    s = ( print_buf + ( PRINT_BUF_LEN -1 ) );

    *s = '\0';


    while (u)
    {
        t = (u % b);
        
        // ugly
        
        if ( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= b;
    };


    if (neg) 
    {
        if ( width && (pad & PAD_ZERO) ){

            printchar (out, '-');
            ++pc;
            --width;

        }else { *--s = '-'; };
    };


	// #obs: 
	// retorna pc + o retorno da função.
    
    // ugly shit
    
    return (int) pc + prints(out, s, width, pad);
}


/*
 ****************************************
 * print:
 *     Rotina de suporta a printf.
 */
 
// #bugbug
// E se essa rotina for chamada com o primeiro argumento nulo?
// vai escrever na IVT ?
// Vai chamar alguma rotina passando esse emsmo endereço de buffer.

//Atençao:
// print() nao analisa flags como runlevel ou kernel phase.

int print ( char **out, int *varg ){

    register int width = 0;
    register int pad = 0;
    register int pc = 0;
    register char *format = (char *) (*varg++);


    char scr[2];

    // ugly
    for ( ; *format != 0; ++format ) 
    {
        if ( *format == '%' ) 
        {
            ++format;

            width = pad = 0;

            if ( *format == '\0' ) { break; }
            if ( *format == '%' )  { goto ____out; }

            if ( *format == '-' )
            {
                ++format;
                pad = PAD_RIGHT;
            }

			while ( *format == '0' ) 
			{
				++format;
				pad |= PAD_ZERO;
			};
			
			for ( ; *format >= '0' && *format <= '9'; ++format )
			{
				width *= 10;
				width += *format - '0';
			};
			
			if ( *format == 's' ) 
			{
                // #bugbug: definition here.
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			
			if ( *format == 'd' ){
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			
			if ( *format == 'x' ){
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			
			if ( *format == 'X' ){
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			
			if ( *format == 'u' ){
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}

            if ( *format == 'c' ) 
            {
				/* char are converted to int then pushed on the stack */
				scr[0] = *varg++;
				scr[1] = '\0';
				
				// shit
				pc += prints (out, scr, width, pad);
                continue;
            }

        // Os caracteres normais diferentes de "%".
        }else{
            ____out:
                printchar (out, *format);
                ++pc;
        };
    };

    // Se o buffer existe, finaliza a string ?

    if (out){ **out = '\0'; }

    return (int) pc;
}


/*
 ***************************************************
 * printk:
 * 
 *     @field 2
 *     The printf function.
 *     Assuming sizeof(void *) == sizeof(int).
 *
 * Em user mode temos uma modelo mais tradicional de printf,
 * talvez seja bom implementa-lo aqui tambem.
 */

// #bugbug
// #todo:
// Devemos tentar usar o mesmo printf implementado na libc
// Essa aqui não está no padrão.

// #todo:
// Vamos substtuir essa função por uma de licensa bsd.
// Olhar na biblioteca.

// #suspensa
// Essa implementação foi feita para 32bit e não funciona
// por inteiro em long mode.
// Usaremos kinguio_printf por enquanto.
int printk_old ( const char *format, ... )
{
    register int *varg = (int *) (&format);

    // #bugbug:
    
    // Se print() está usando '0' como buffer,
    // então ele está sujando a IVT. 

    //Atençao:
    // print() nao analisa flags.

    return (int) print(0,varg);
}    

// print() is a helper function for this one.
/*
int vsprintf(char *string, const char *format, va_list ap);
int vsprintf(char *string, const char *format, va_list ap)
{
}
*/

// ===================================


#define HEX_LEN 8

void kinguio_i2hex(unsigned int val, char* dest, int len)
{

	char* cp;
	int i, x;
	unsigned n;
	
	if(val == 0) {
		cp = &dest[0];
		*cp++ = '0';
		*cp = '\0';
		return;
	}
	

	n = val;
	cp = &dest[len];
	while (cp > dest)
	{
		x = n & 0xF;
		n >>= 4;
		*--cp = x + ((x > (HEX_LEN+1)) ? 'A' - 10 : '0');
	}
    
	dest[len]='\0';

	cp = &dest[0];
	for(i=0; i < len;i++) {
	
		if(*cp == '0') {
			cp++;
		}
		else {
			strcpy(dest,cp);
			 break;
		}
			
	}

	cp = &dest[0];
	n = strlen(cp);
	memset(dest + n,0,8-n);
}

#define ____INT_MAX  2147483647
char *kinguio_itoa (int val, char *str) 
{
  	

	char* valuestring = (char*) str;
	int value = val;

	int min_flag;
  	char swap, *p;
 	min_flag = 0;

  	if (0 > value)
  	{
    		*valuestring++ = '-';
    		value = -____INT_MAX> value ? min_flag = ____INT_MAX : -value;
  	}

  	p = valuestring;

  	do
  	{
    		*p++ = (char)(value % 10) + '0';
    		value /= 10;
  	} while (value);

  	if (min_flag != 0)
  	{
    		++*valuestring;
  	}
  	*p-- = '\0';

  	while (p > valuestring)
  	{
    		swap = *valuestring;
    		*valuestring++ = *p;
    		*p-- = swap;
  	}

	return str;
}



static char *_vsputs_r(char *dest, char *src)
{
    unsigned char *usrc  = (unsigned char *) src;
    unsigned char *udest = (unsigned char *) dest;

    while ( *usrc ) 
    { 
        *udest++ = *usrc++; 
    };

    return (char * ) udest;
}


int kinguio_vsprintf(char * str,const char * fmt, va_list ap)
{

	char *str_tmp = str;

	char _c_r[] = "\0\0";

	int index = 0;
	unsigned char u;	
	int d;
	char c, *s;
	char buffer[256];

	while (fmt[index])
	{
		switch (fmt[index])
		{
		case '%':
			++index;
			switch (fmt[index])
			{
			
			case 'c':
				*_c_r = c = (char) va_arg (ap, int);
				str_tmp  = _vsputs_r(str_tmp,_c_r);
				break;

			case 's':
				s = va_arg (ap, char*);
				str_tmp  = _vsputs_r(str_tmp,s);
				break;

			case 'd':
			case 'i':
				d = va_arg (ap, int);
				kinguio_itoa (d,buffer);
				str_tmp  = _vsputs_r(str_tmp,buffer);
				break;

			case 'u':
				u = va_arg (ap, unsigned int);
				kinguio_itoa  (u,buffer);
				str_tmp  = _vsputs_r(str_tmp,buffer);
				break;

			case 'X':
			case 'x':
				d = va_arg (ap, int);
				kinguio_i2hex(d, buffer,8);
				str_tmp  = _vsputs_r(str_tmp,buffer);
				break;
			
			default:
				str_tmp  = _vsputs_r(str_tmp,"%%");
				break;
				
				
			}
			break;

		default:
			*_c_r = fmt[index]; //
			str_tmp  = _vsputs_r(str_tmp,_c_r);
			break;
		}
		++index;
	}
	
    return ((long)str_tmp - (long)str);
}


void kinguio_puts(const char* str)
{
    int i=0;

    if (!str)
        return;

    for (i=0; i <strlen(str); i++) 
        putchar(str[i]);
}



//  printf
// Credits: Nelson Cole. Project Sirius/Kinguio.
int kinguio_printf(const char *fmt, ...)
{
    int ret=0;

    char buf[256];
    memset(buf,0,256); 

    va_list ap;
    va_start (ap, fmt);

    ret = kinguio_vsprintf(buf, fmt, ap);

    va_end (ap);

    // Print
    kinguio_puts(buf);

    return (int) ret;
}


// ===================================



/*
 ******************
 * kputs: 
 *     provisório ...
 */

int kputs ( const char *str )
{

    // #todo
    // Validation
    
    //if ( (void*) str == NULL )
    //{
        //
    //}

    //return (int) printf ("%s",str);
    return (int) printk ("%s",str);
}


/*
 ***********************************************************
 * sprintf:
 *     @field 2
 *     int sprintf ( char * str, const char * format, ... );
 * 
 *     Composes a string with the same text that would be 
 * printed if format was used on printf, 
 * but instead of being printed, the content is stored 
 * as a C string in the buffer pointed by str.
 */

// #suspensa
// Essa implementação foi feita para 32bit e não funciona
// por inteiro em long mode.
int sprintf_old ( char *str, const char *format, ... )
{
    register int *varg = (int *) (&format);

    return (int) print (&str, varg);
}    


// sprintf
// Variable parameter form to achieve sprintf 
int mysprintf(char *buf, const char *fmt, ...)
{
    int i=0;

    va_list args;
    va_start(args, fmt);

     // Write the fmt format string to the buffer buf 
    i = kinguio_vsprintf(buf, fmt, args);

    // Release args 
    va_end(args);

    return (int) i;
}


int k_ungetc ( int c, file *f )
{
    if (c == EOF){ return (int) c; }

    if ( (void *) f == NULL ){ return EOF; }


	//@todo: flag oef.
	//stream->flags = (stream->flags & ~_IOEOF);

    f->_p--;
    f->_p[0] = (char) c;

    return (int) c;
}

long k_ftell (file *f)
{
    if ( (void *) f == NULL ){ return EOF; }

    return (long) (f->_p - f->_base);
}

// k_fileno: Return the fd.
int k_fileno ( file *f )
{
    if ( (void *) f == NULL ){ return EOF; }

    return (int) f->_file;  
}


int k_fgetc (file *f)
{
    int ch = 0;

    if ( (void *) f == NULL )
    {
        printf ("k_fgetc: f\n");
        goto fail;
        //refresh_screen();
        //return EOF;
    }

		 //(--(p)->_r < 0 ? __srget(p) : (int)(*(p)->_p++))
		
		//#fim.
		//cnt decrementou e chegou a zero.
		//Não há mais caracteres disponíveis entre 
		//stream->_ptr e o tamanho do buffer.
		
		/*
		if ( stream->_cnt <= 0 )
		{
			stream->_flags = (stream->_flags | _IOEOF); 
			stream->_cnt = 0;
			
		    //printf ("#debug: fgetc: $\n");
			
			//isso funciona, significa que a estrutura tem ponteiro e base validos.
			//printf("show fgetc:: %s @\n", stream->_base );
		    //refresh_screen();
			
			return EOF;
		};
		*/
		
		//#debug
		//nao podemos acessar um ponteiro nulo... no caso endereço.


    // if ( (void*) f->_p == NULL ){
    if ( f->_p == 0 ){
        printf ("k_fgetc: [FAIL] f->_p \n");
        goto fail;
        //refresh_screen();
        //return EOF;
    }else{
 
	// #obs: 
	// Tem que ter a opção de pegarmos usando o posicionamento
	// no buffer. O terminal gosta dessas coisas.

        //
        // Pega o char no posicionamento absoluto do arquivo
        //

        ch = (int) *f->_p;

        f->_p++;
        f->_cnt--;

        return (int) ch;
    };

    //fail

fail:

	//#debug
    printf ("k_fgetc: fail\n");
    refresh_screen();
 
    return EOF;
}


int k_feof ( file *f )
{
    int ch=0;
 
    if ( (void *) f == NULL ){
        return (int) (-1);
    } else {

        ch = k_fgetc(f);

        if ( ch == EOF ){
             return (int) 1;
        }else{
             return 0;
        };
    };

	//checar se o eof foi atingido.
	// return( (stream->_flag & _IOEOF) );

    return 0;
}


int k_ferror ( file *f )
{
    if ( (void *) f == NULL ){ return EOF; }

    return (int) ( ( f->_flags & _IOERR ) );
}


/*
 **************************************
 * k_fseek:
 *     offset argument is the position that you want to seek to,
 *     and whence is what that offset is relative to.
 */

int k_fseek ( file *f, long offset, int whence )
{
    if ( (void *) f == NULL )
    {
        debug_print ("k_fseek: f\n");
        goto fail;
    }

	// Checar limites do offset.

    switch (whence){

        case SEEK_SET:
		    //printf ("SEEK_SET\n");   
            f->_p = (f->_base + offset); 
            f->_r = offset;
            f->_w = offset;
            f->_cnt = (f->_lbfsize - offset);
			goto done;
			break;

        case SEEK_CUR:
		    //printf ("SEEK_CUR\n");
		    f->_p = (f->_p + offset);
            f->_r = (f->_p - f->_base);
            f->_w = (f->_p - f->_base);
		    f->_cnt = (f->_cnt - offset); 
		    goto done;
			break;

        case SEEK_END:
		    //printf ("SEEK_END stream->_lbfsize=%d \n",stream->_lbfsize);
		    f->_p = ((f->_base + f->_lbfsize) + offset); 
            f->_r = ( f->_lbfsize + offset );
            f->_w = ( f->_lbfsize + offset );
		    f->_cnt = 0;
		    goto done;
			break;

        default:
            //printf ("default:\n");
		    goto fail;
			break;
    };


fail:
	printf ("k_fseek fail\n");
	refresh_screen();
    return (int) (-1);

done:
    //refresh_screen();
    return 0;
}

// bsd stuff
/*
 * Various output routines call wsetup to be sure it is safe to write,
 * because either _flags does not include __SWR, or _buf is NULL.
 * _wsetup returns 0 if OK to write, nonzero otherwise.
 */
/*
int __swsetup(file *fp)
int __swsetup(file *fp)
{
    return -1;
}
*/


/*
 * Write the given character into the (probably full) buffer for
 * the given file.  Flush the buffer out if it is or becomes full,
 * or if c=='\n' and the file is line buffered.
 */

// #importante
// Isso é usado em __sputc no bsd.

/*
int __swbuf (int c, file *fp);
int __swbuf (int c, file *fp)
{
    return -1;
}
*/


int k_fputc ( int ch, file *f )
{
    if ( (void *) f == NULL ){
        return EOF;
    }else{

		// se tivermos um posicionamento válido de escrita no buffer ou
		// se a posição de escrita no buffer for maior que o limite
		// do buffer e o char for diferente de fim de linha
		// então usaremos ponteiro absoluto para escrever no arquivo.
		// Isso acontece poque desejamos continuar colocando coisa no arquivo
		// mesmo depois que o buffer se esgota.
		
		// caso contrário escreveremos no buffer.
			
			
		// Se ainda não esgotamos o buffer,
		// ou se esgotamos o buffer mas o caractere não é um 
		// caractere de fim de linha;

		// if ( stream->_w-- >= 0 || 
		//      ( stream->_w >= stream->_lbfsize && (char) ch != '\n' ) )

        // Buffer is not full.
        // Quanto falta para acabar.
        if ( f->_cnt > 0 && ch != '\n' )
        {    
             f->_cnt--;

             sprintf ( f->_p, "%c", ch);
             f->_p++;
            
             f->_w++; 
             
             return (int) ch;  
        }
        
        // se o buffer está cheio.
        
        /*
        //Now writing. 
        if (stream->flags & _IORW)
        {
            stream->flags &= ~_IOREAD;
            stream->flags |= _IOWRITE;
        }
        */        
        
        //File is not writable. 
        //if (!(stream->flags & _IOWRITE))
             //return (EOF);        
        
        
        // ...
    };

    //fail
    return EOF;
}


int k_fscanf (file *f, const char *format, ... )
{
    return -1;
}



/*
int vfprintf ( file *stream, const char *format, stdio_va_list argptr );
int vfprintf ( file *stream, const char *format, stdio_va_list argptr )
{
}
*/


/*
 * Writes format output of a stdarg argument list to a file.
 */

/* 
int vfprintf(file *stream, const char *format, va_list ap);
int vfprintf(file *stream, const char *format, va_list ap)
{
	int n;             // Characters written. 
	char buffer[1024]; // Buffer.             
	
	// Format string. 
	n = vsprintf(buffer, format, ap);
	
	// Write formated string to file. 
	if (n > 0)
		fputs(buffer, stream);
	
	return (n);
}
*/


void k_rewind ( file *f )
{
    //fseek (f, 0L, SEEK_SET);

    if ( (void *) f == NULL ){ return; }

    f->_p = f->_base;
}


int k_fclose (file *f)
{

    debug_print("k_fclose: [FIXME]\n");

    // fail
    if ( (void *) f == NULL ){
        debug_print("k_fclose: f\n");
        return EOF;
    }else{

        // #todo
        // Talvez possamos reaproveitar a estrutura
        // mudando o magic e não estragando a estrutura.

        f->used  = TRUE;
        f->magic = 1234;
        
        // #maybe
        // f->magic = 216;
        
        f->pid = (pid_t) 0;
        f->uid = (uid_t) 0;
        f->gid = (gid_t) 0;
        
        // #todo: sync 
        
        // #todo: free.
        // free(f->_base);
        
        f->_p = NULL;
        f->_cnt = 0;
        f->_base = NULL;
        f->_flags = 0;
        
        // #todo
        // Limpar a tabela de arquivos abertos do processo atual ??
        // Mas e se o arquivo pertence ao kernel ??
        f->_file = 0;
        
        f->_charbuf = 0;
        f->_lbfsize = 0;
        f->_tmpfname = NULL;
        
        // kill
        // free() ?
        
        // #maybe
        // Talvez vamos reaproveitar a estrutura.
        
        f = NULL;
    };

	//...

    return 0;
}

int k_fputs ( const char *str, file *f )
{
    int Size = 0;


    if ( (void *) f == NULL ){
        return (int) (-1);
    } else {
        Size = (int) strlen (str);

        if ( Size > f->_cnt ){ return (int) (-1); }

        f->_cnt = (int) (f->_cnt - Size);

        sprintf ( f->_p, str );

        f->_p = (f->_p + Size);

        return 0;
    };

    return (int) (-1);
}


/*
 *****************************
 * k_setbuf:
 * 
 */

// see: 
// https://linux.die.net/man/3/setvbuf

void k_setbuf (file *f, char *buf)
{
    if ( (void *) f == NULL ){
        debug_print("k_setbuf: f\n");
        return;
    }


    if ( (void *) buf == NULL ){
        debug_print("k_setbuf: buf\n");
        return;
    }


		//#todo
		//se o buffer é válido.
        //if (stream->_bf._base != NULL) 
        //{
            //if (stream->cnt > 0)
                //fflush (stream);
                
            //free (stream->buf);
        //}
        
        // Udate stream.
    f->_bf._base = buf;
        //stream->_lbfsize = size;        
        // ?? stream->bufmode = mode;

    f->_p = buf;
    // f->_p = f->_bf._base
    
    // ??stream->cnt = 0;
    
    
    // #todo
    // Setup all the buffer elements.
    
    // ...
    
}


/*
 * k_setbuffer:
 * 
 */

void k_setbuffer (file *f, char *buf, size_t size)
{
  // #todo
  // Check parameters.

    /*
    if ( (void *) f == NULL ){
        debug_print("k_setbuffer: f\n");
        return;
    }


    if ( (void *) buf == NULL ){
        debug_print("k_setbuffer: buf\n");
        return;
    }
    */


    if ( (void *) f == NULL )
    {
        // #todo
        // Maybe we need a message here.
        return;

    }else{

		//#todo
		//se o buffer é válido.
        //if (stream->_bf._base != NULL) 
        //{
            //if (stream->cnt > 0)
                //fflush (stream);
                
            //free (stream->buf);
        //}
        
        // Udate stream.
        f->_bf._base = buf;
        f->_lbfsize = size;        
        // ?? stream->bufmode = mode;

        f->_p = buf;
        // ??stream->cnt = 0;
        //...
    };
}


void k_setlinebuf (file *f)
{
    debug_print ("k_setlinebuf: [TODO] \n");
    
    if ( (void *) f == NULL )
    {
        return;
    }
}


int k_setvbuf (file *f, char *buf, int mode, size_t size)
{

    // #todo


    if ( (void *) f == NULL )
    {
        // MSG ?
        return -1;

    }else{

		//#todo
		//se o buffer é válido.
        //if (f->_bf._base != NULL) 
        //{
            //if (f->cnt > 0)
                //fflush (f);
                
            //free (f->buf);
        //}
        
        // Udate stream.
        f->_bf._base = buf;
        f->_lbfsize = size; 
        // ?? f->bufmode = mode;

        f->_p = buf;
        // ??f->cnt = 0;
        //...
    };


    return 0;
}


// Maybe we can do some operations in a regular file using ioctl.
int 
regularfile_ioctl ( 
    int fd, 
    unsigned long request, 
    unsigned long arg )
{
    debug_print ("regularfile_ioctl: [TODO]\n");
    return -1;
}


/*
 ******************************************
 * stdioInitialize:
 *     Inicializando stdio pertencente ao kernel base.
 *     Inicializa as estruturas do fluxo padrão.
 *     Quem chamou essa inicialização ?? Em que hora ??
 *
 * #bugbug: Pelo jeito somente depois dessa inicialização é que temos mensagens 
 * com printf decentes. Então a inicialização do kernel precisa disso.
 * >> precisamos antecipar essa inicilização. Mas ela precisa ser depois da
 * inicialização da paginação.
 */
 
// Estamos no kernel base em ring 0.
// Queremos que as streams sejam acessíveis para as rotinas
// da libc em ring3. Para a libc alterar os elementos
// da estrutura.
// #bugbug: Talvez seja possível criar essas estruturas
// em memória compartilhada, usado o alocaro apropriado.
// kmalloc com certeza e ring0.

// In this routine:
// + Initializing the structures for stdin, stdout and stderr

int stdioInitialize (void)
{
    kstdio_standard_streams_initialized =  FALSE;

    int Status = 0;
    int slot=-1;

    // register ?
    int i=0;

    file *tmp;
    struct inode_d *tmp_inode;    


    int cWidth  = get_char_width();
    int cHeight = get_char_height();

    if ( cWidth == 0 || cHeight == 0 ){
        x_panic ("kstdio-stdioInitialize: [FAIL] Char info\n");
    }

    // #
    // Ja temos suporte a print nesse momento por causa
    // das configurações de console. Mas nessa rotina 
    // refaremos as configurações de console.
    
    debug_print ("stdioInitialize: [TODO]\n");
    printf      ("stdioInitialize: [TODO]\n");


    // Ùltimo erro registrado.
    errno = 0;
    

    // Os buffers dos arquivos acima.
    // prompt[]
    // Esses prompts são usados como arquivos.
    // São buffers para as streams.
    // See: kstdio.h

    // Clean
    for ( i=0; i<PROMPT_SIZE; i++ )
    {
        prompt[i]     = (char) '\0';
        prompt_out[i] = (char) '\0';
        prompt_err[i] = (char) '\0';
    };
    prompt_pos = 0;



    //
    // file_table and inode_table
    //

    //=====================
    // file table
    for (i=0; i<NUMBER_OF_FILES; i++)
    {
        tmp = (void*) kmalloc (sizeof(file));
        if ((void*)tmp==NULL){
           x_panic("kstdio-stdioInitialize: tmp\n");
        }
        tmp->used  = TRUE;
        tmp->magic = 1234;
        tmp->____object = ObjectTypeFile; //Regular file
        tmp->_flags = 0; // (__SWR | __SRD); 
        tmp->fd_counter = 0;
        tmp->_tmpfname = NULL;
        //...
 
        //salva
        file_table[i] = (unsigned long) tmp; 
    };
    

    //===================================
    // inode table
    for (i=0; i<32; i++)
    {
        tmp_inode = (void*) kmalloc (sizeof(struct inode_d));
        if ((void*)tmp_inode==NULL){
            x_panic("kstdio-stdioInitialize: tmp_inode\n");
        }
        tmp_inode->used  = TRUE;
        tmp_inode->magic = 1234;
        tmp_inode->filestruct_counter = 0;
        tmp_inode->path[0] = 0;
        //...

        //salva
        inode_table[i] = (unsigned long) tmp_inode; 
    };


    // #bugbug
    // 0 - keyboard tty.
    // 1 - virtual console.
    // 2 - regular file.


    //===========================================
    // stdin
    // pega slot em file_table[] para stdin
    slot = get_free_slots_in_the_file_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: file slot");
    }
    stdin = file_table[slot];
    stdin->filetable_index = slot;
    // Configurando a estrutura de stdin. 
    stdin->_file = 0;  //fd
    stdin->____object = ObjectTypeTTY;  // TTY
    stdin->used  = TRUE;
    stdin->magic = 1234;
    stdin->sync.sender = -1;
    stdin->sync.receiver = -1;
    stdin->sync.can_read    = TRUE;
    stdin->sync.can_write   = FALSE;  //#this is a test
    stdin->sync.can_execute = FALSE;
    stdin->sync.can_accept  = FALSE;
    stdin->sync.can_connect = FALSE;
    stdin->_flags = (__SWR | __SRD); 
    stdin->_base     = &prompt[0];    //See: include/kernel/stdio.h
    stdin->_p        = &prompt[0];
    stdin->_bf._base = stdin->_base;
    stdin->_lbfsize = PROMPT_SIZE; //128; //#todo
    stdin->_r = 0;
    stdin->_w = 0;
    stdin->_cnt = PROMPT_SIZE;
    stdin->_tmpfname = "KSTDIN  TXT";
    stdin->fd_counter = 1;

    // inode support.
    // pega slot em inode_table[] 
    slot = get_free_slots_in_the_inode_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: [FAIL] stdin inode slot\n");
    }
    stdin->inode = inode_table[slot];
    stdin->inodetable_index = slot;
    if( (void*) stdin->inode == NULL ){
        x_panic("kstdio-stdioInitialize: [FAIL] stdin inode struct\n");
    }
    stdin->inode->filestruct_counter = 1; //inicialize
    memcpy( (void*) stdin->inode->path, (const void*) stdin->_tmpfname, sizeof( stdin->inode->path ) );
    // ... 


    //=============================================
    // stdout
    // pega slot em file_table[] para stdout
    slot = get_free_slots_in_the_file_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: slot\n");
    }
    stdout = file_table[slot];
    stdout->filetable_index = slot;
    // Configurando a estrutura de stdout.
    // This is a virtual console device. Used to output
    // directly into the virtual console.
    stdout->_file = 1;
    stdout->____object = ObjectTypeVirtualConsole;  // Virtual console.
    stdout->used  = TRUE;
    stdout->magic = 1234;
    stdout->sync.sender = -1;
    stdout->sync.receiver = -1;
    stdout->sync.can_read    = TRUE;
    stdout->sync.can_write   = TRUE;
    stdout->sync.can_execute = FALSE;
    stdout->sync.can_accept  = FALSE;
    stdout->sync.can_connect = FALSE;
    stdout->_flags = (__SWR | __SRD); 
    stdout->_base     = &prompt_out[0];  //See: include/kernel/stdio.h
    stdout->_p        = &prompt_out[0];
    stdout->_bf._base = stdout->_base;
    stdout->_lbfsize  = PROMPT_SIZE; //128; //#todo
    stdout->_r = 0;
    stdout->_w = 0;
    stdout->_cnt = PROMPT_SIZE;
    stdout->_tmpfname = "KSTDOUT DEV";
    stdout->fd_counter = 1;
    // ...
    // inode support.
    // pega slot em inode_table[] 
    slot = get_free_slots_in_the_inode_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: stdout inode slot\n");
    }
    stdout->inode = inode_table[slot];
    stdout->inodetable_index = slot;
    if( (void*) stdout->inode == NULL ){
        x_panic("kstdio-stdioInitialize: stdout inode struct\n");
    }
    stdout->inode->filestruct_counter = 1; //inicialize
    memcpy( 
        (void*)       stdout->inode->path, 
        (const void*) stdout->_tmpfname, 
        sizeof( stdout->inode->path ) );
    // ... 


    //=========================================
    // stderr
    // pega slot em file_table[] para stderr
    slot = get_free_slots_in_the_file_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: slot");
    }
    stderr = file_table[slot];
    stderr->filetable_index = slot;
    // Configurando a estrutura de stderr.
    stderr->_file = 2;
    stderr->____object = ObjectTypeFile;  // Regular file.
    stderr->used  = TRUE;
    stderr->magic = 1234;
    stderr->sync.sender = -1;
    stderr->sync.receiver = -1;
    stderr->sync.can_read    = TRUE;
    stderr->sync.can_write   = TRUE;
    stderr->sync.can_execute = FALSE;
    stderr->sync.can_accept  = FALSE;
    stderr->sync.can_connect = FALSE;
    stderr->_flags = (__SWR | __SRD); 
    stderr->_base     = &prompt_err[0];  //See: include/kernel/stdio.h
    stderr->_p        = &prompt_err[0];
    stderr->_bf._base = stderr->_base;
    stderr->_lbfsize  = PROMPT_SIZE; //128; //#todo
    stderr->_r = 0;
    stderr->_w = 0;
    stderr->_cnt = PROMPT_SIZE;
    stderr->_tmpfname = "KSTDERR TXT";
    stderr->fd_counter = 1;
    // ...
    // inode support.
    // pega slot em inode_table[] 
    slot = get_free_slots_in_the_inode_table();
    if(slot<0 || slot >=NUMBER_OF_FILES){
        x_panic("kstdio-stdioInitialize: stderr inode slot\n");
    }
    stderr->inode = inode_table[slot];
    stderr->inodetable_index = slot;
    if( (void*) stderr->inode == NULL ){
        x_panic("kstdio-stdioInitialize: stderr inode struct\n");
    }
    stderr->inode->filestruct_counter = 1; //inicialize
    //copy the name.
    memcpy ( 
        (void*)       stderr->inode->path, 
        (const void*) stderr->_tmpfname, 
        sizeof( stderr->inode->path ) );
    // ... 



    // ...

	// Flag para o tipo de input.
	// # Multiplas linhas.

    g_inputmode = INPUT_MODE_MULTIPLE_LINES;

    //
    // Virtual console.
    //
    
    // Configurando o cursor para todos os consoles.
    // Estamos fazendo isso pela segunda vez.
    // A primeira foi quando criamos os consoles.

    // See:
    // tty.h
    // console.h

    for (i=0; i<4; i++){

        CONSOLE_TTYS[i].cursor_x = 0;
        CONSOLE_TTYS[i].cursor_y = 0;
        CONSOLE_TTYS[i].cursor_left   = 0;
        CONSOLE_TTYS[i].cursor_top    = 0;
        CONSOLE_TTYS[i].cursor_right  = (SavedX/cWidth);
        CONSOLE_TTYS[i].cursor_bottom = (SavedY/cHeight);
        CONSOLE_TTYS[i].cursor_color  = COLOR_WHITE;
    
        CONSOLE_TTYS[i].initialized = TRUE;
    };

    // #bugbug
    // Estamos fazendo isso pela segunda vez.
    // A primeira foi em kernel_main.
    Background_initialize();

    // #bugbug
    // Testando o funcionamento das estruturas de console. tty.
    //set_up_cursor(0,1);
    //console_outbyte('x',fg_console);
    //refresh_screen();
    //while(1){}
    //Done !

    stdio_terminalmode_flag = TRUE;
    stdio_verbosemode_flag = TRUE;
    kstdio_standard_streams_initialized = TRUE;

    return 0;

fail:
    x_panic ("kstdio-stdioInitialize: fail\n");
}



