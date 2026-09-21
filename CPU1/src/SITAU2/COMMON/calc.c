// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "calc.h"
#include "xil_printf.h"
// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

char MAX_CHAR = 127;
char MIN_CHAR = -127 - 1;
unsigned char MAX_UCHAR = 255;

short int MAX_SHORT = 32767;
short int MIN_SHORT = -32767 - 1;
unsigned short MAX_USHORT = 65535;

int MAX_INT = 2147483647;
int MIN_INT = -2147483647 - 1;
unsigned int MAX_UINT = 4294967295;

long MAX_LONG = 2147483647;
long MIN_LONG = -2147483647 - 1;
unsigned long MAX_ULONG = 4294967295;

void convert_ulong_to_int(unsigned long int numero,int* izq_ret,int* cen_ret,int* der_ret)
{
	unsigned long int izquierda,centro,derecha,resto;
	resto    = numero;
	izquierda= (unsigned long int )(numero/1e18);
	resto    = resto-(izquierda*1e18);
	centro   = (unsigned long int )resto/1e9;
	resto    = resto-centro*1e9;
	derecha  = resto;
	*izq_ret=(int)izquierda;
	*cen_ret=(int)centro;
	*der_ret=(int)derecha;
}

void print_u64(unsigned long int numero)
{
	int izquierda,centro,derecha,resto;
	convert_ulong_to_int(numero,&izquierda,&centro,&derecha);
	xil_printf("%d%09d%09d",izquierda,centro,derecha);
}


// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float UnsignedMaxRange(int n_bytes)
{
   int i, n_bits;
   float result = 1;

   n_bits = n_bytes * 8;
   for (i = 0; i < n_bits; i++) result *= 2;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float SignedMinRange(int n_bytes)
{
   return UnsignedMaxRange(n_bytes) / -2;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float SignedMaxRange(int n_bytes)
{
   return UnsignedMaxRange(n_bytes) - 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void UpdateDataTypeRange(void)
{
   MAX_CHAR = (char)SignedMaxRange(sizeof(char));
   MIN_CHAR = (char)SignedMinRange(sizeof(char));
   MAX_UCHAR = (unsigned char)UnsignedMaxRange(sizeof(unsigned char));

   MAX_SHORT = (short int)SignedMaxRange(sizeof(short int));
   MIN_SHORT = (short int)SignedMinRange(sizeof(short int));
   MAX_USHORT = (unsigned short)UnsignedMaxRange(sizeof(unsigned short));

   MAX_INT = (int)SignedMaxRange(sizeof(int));
   MIN_INT = (int)SignedMinRange(sizeof(int));
   MAX_UINT = (unsigned int)UnsignedMaxRange(sizeof(unsigned int));

   MAX_LONG = (long)SignedMaxRange(sizeof(long));
   MIN_LONG = (long)SignedMinRange(sizeof(long));
   MAX_ULONG = (unsigned long)UnsignedMaxRange(sizeof(unsigned long));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned int Log2 (unsigned int data)//Creo que este log2 es incorrecto
{
unsigned int result = 0;

   while (data >>= 1) result++;
   result++;
   return result;
}
unsigned int floor_Log2 (unsigned int data)
{
unsigned int result = 0;

   while (data >>= 1) result++;

   return result;
}
unsigned int ceil_Log2 (unsigned int data)
{
unsigned int result = 0;

	result=floor_Log2(data);
	if ((data & (data-1)) == 0)  //Check if x is power of 2
		return result;
	else
		return result+1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
/*int RoundSup (float f)
{
int sgn = 1;
float aux;

	if (f < 0)
	{
		aux = f*(-1);
		if (aux-(int)aux > 0) return (int)(f-1);
		else return (int)f;
	}
	else
	{
		if (f-(int)f > 0)	return (int)(f+1)*sgn;
		else return (int)f*sgn;
   }
}*/
int RoundSup (float f)
{
   if (f-(int)f > 0)	return (int)(f+1);
	else return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int RoundDown (float f)
{
	if (f < 0)
	{
		if (f-(int)f > 0) return (int)f;
		else return (int)(f-1);
	}
	else return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int Round32 (float data)
{
int result, sgn = 1;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5) result = (int)(data + 1) * sgn;
	else result = (int)data * sgn;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long Round32u (float data)
{
	if (data < 0) return 0;
	if (data - (int)data > 0.5) return (unsigned long)(data + 1);
	else return (unsigned long)data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int Round16 (float data)
{
int data_int, sgn = 1;
short int result;
int min_value = MIN_SHORT;
int max_value = MAX_SHORT;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5 ) data_int = (int)(data + 1) * sgn;
	else data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (short int)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short Round16u (float data)
{
int data_int;
short int result;
int max_value = MAX_SHORT;

	if (data < 0) return 0;
	if (data - (int)data > 0.5) data_int = (int)(data + 1);
	else data_int = (int)data;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned short)data_int;
   return result;
}
