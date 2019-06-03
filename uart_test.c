
/*
 * uart_test.c
 *
 * Created: 12/23/17 2:33:25 AM
 *  Author: Sivert
 */ 
void uart_test(){
		uart_init(BAUD_CALC(115200)); // 8n1 transmission is set as default
		stdout = &uart0_io; // attach uart stream to stdout & stdin
		stdin = &uart0_io; // uart0_in and uart0_out are only available if NO_USART_RX or NO_USART_TX is defined
		sei(); // enable interrupts, library wouldn't work without this
		uart_puts("hello from usart 0\r\n"); // write const string to usart buffer // C++ restriction, in C its the same as uart_putstr()
		// if you do not have enough SRAM memory space to keep all strings, try to use puts_P instead
		uart_puts_P("hello from flashed, usart\r\n"); // write string to usart buffer from flash memory // string is parsed by PSTR() macro
		uart_puts_p(foo_string);
		uart_puts_p(PSTR("we can also do like this\r\n"));
		
		printf("hello from printf\n");

		char buffer[25];
		uart_gets(buffer, 25); // read at most 24 bytes from buffer (CR,LF will not be cut)
		
		int a;
		
		uart_puts("gimmie a number: ");
		a = uart_getint();
		
		uart_puts("numba a: ");
		uart_putint(a);
		uart_puts("\r\n");
	
}