#ifndef calcH
#define calcH

// _____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================

void UpdateDataTypeRange(void);
unsigned int ceil_Log2 (unsigned int data);
unsigned int floor_Log2 (unsigned int data);
unsigned int Log2(unsigned int data);

int RoundSup (float f);

int RoundDown (float f);

int Round32(float data);

unsigned long Round32u(float data);

short int Round16(float data);

unsigned short Round16u(float data);

void convert_ulong_to_int(unsigned long int numero,int* izq_ret,int* cen_ret,int* der_ret);
void print_u64(unsigned long int numero);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern char MAX_CHAR;
extern char MIN_CHAR;
extern unsigned char MAX_UCHAR;

extern short int MAX_SHORT;
extern short int MIN_SHORT;
extern unsigned short MAX_USHORT;

extern int MAX_INT;
extern int MIN_INT;
extern unsigned int MAX_UINT;

extern long MAX_LONG;
extern long MIN_LONG;
extern unsigned long MAX_ULONG;

#endif
