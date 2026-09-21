/*
 * cons_prod_util.c
 *
 *  Created on: 17/11/2016
 *      Author: csic
 */

#include "cons_prod_util.h"
#include "log.h"

cons_prod_t* cons_prod_array_p[MAX_CONS_PROD_STRUCTS];
static unsigned char gb_buffer_command[SIZE_BUFFER_COMMAND];

const u32 cons_buffers_start_addresses[MAX_CONS_PROD_STRUCTS]={HOST2UCI_BUF_START,UCI2HOST_BUF_START,IMAGE_BUF_START};
const u32 cons_buffers_sizes[MAX_CONS_PROD_STRUCTS]={HOST2UCI_BUF_SIZE,UCI2HOST_BUF_SIZE,IMAGE_BUF_SIZE};

//This function ensures the alignment of structures in complete cache lines.
int cons_prod_init_structs(void)
{
	int i;
	u8* temp;
	temp = (u8*)CONST_PROD_S_START;
	for(i=0;i<MAX_CONS_PROD_STRUCTS;i++)
	{
		cons_prod_array_p[i]=(cons_prod_t*)temp; //Esto lo debe hacer CADA procesador
		temp+=SIZE_PER_STRUCT_ALIGN;

		// A partir de aquí hay que hacer MUTEX e invalidar caché, ya que el otro procesador puede que ya haya iniciado su parte.
		mutex_utils_blocking_LOCK(MUTEX_START_NUMBER+i);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
		Xil_L1DCacheInvalidateRange(cons_prod_array_p[i],SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
		cons_prod_array_p[i]->mutex_ID=MUTEX_START_NUMBER+i;
		cons_prod_array_p[i]->signal_ID=SIC_START_NUMBER+i;
		cons_prod_array_p[i]->buffer_start_address=(u8*)cons_buffers_start_addresses[i];
		cons_prod_array_p[i]->buffer_size=cons_buffers_sizes[i];
		cons_prod_array_p[i]->write_position=0;
		cons_prod_array_p[i]->read_position=0;
		cons_prod_array_p[i]->int_threshold=0;
		Xil_L1DCacheFlushRange(cons_prod_array_p[i],SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
		mutex_utils_UNLOCK(MUTEX_START_NUMBER+i);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
	}
	return 0;
}

// To initiate consumer or producer, mutex and GIC systems should have been initiated already.
cons_prod_t* cons_init(u8 cons_prod_id,Xil_InterruptHandler cons_wake_up_interrupt)
{
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_array_p[cons_prod_id]->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_array_p[cons_prod_id],SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	if(cons_wake_up_interrupt!=NULL) cons_prod_array_p[cons_prod_id]->cons_wake_up_int_ena=1;
	else cons_prod_array_p[cons_prod_id]->cons_wake_up_int_ena=0;

	cons_prod_array_p[cons_prod_id]->cons_cpu_ID=THIS_CPU_GIC_MASK;

	cons_prod_array_p[cons_prod_id]->int_threshold=0;

	Xil_L1DCacheFlushRange(cons_prod_array_p[cons_prod_id],SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_array_p[cons_prod_id]->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	if(cons_wake_up_interrupt!=NULL)
	{
		gic_utils_register_interrupt(cons_prod_array_p[cons_prod_id]->signal_ID,cons_wake_up_interrupt);
		gic_utils_enable_interrupt(cons_prod_array_p[cons_prod_id]->signal_ID);
	}

	return cons_prod_array_p[cons_prod_id];
}

// To initiate consumer or producer, mutex and GIC systems should have been initiated already.
cons_prod_t* prod_init(u8 cons_prod_id)
{
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_array_p[cons_prod_id]->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_array_p[cons_prod_id],SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
if(0)// esto es de prueba
{
	cons_prod_array_p[cons_prod_id]->cons_wake_up_int_ena=1;
	cons_prod_array_p[cons_prod_id]->cons_cpu_ID=0x2;

	cons_prod_array_p[cons_prod_id]->int_threshold=1;// Interrumpir siempre que llega algún dato
}
	cons_prod_array_p[cons_prod_id]->prod_cpu_ID=THIS_CPU_GIC_MASK;
	cons_prod_array_p[cons_prod_id]->write_position=0;

	Xil_L1DCacheFlushRange(cons_prod_array_p[cons_prod_id],SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_array_p[cons_prod_id]->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	return cons_prod_array_p[cons_prod_id];
}

u32 cons_num_data_to_read(cons_prod_t* cons_prod_p)
{
	u32 data_available,read_pos,write_pos,size;
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
	size=cons_prod_p->buffer_size;
	read_pos=cons_prod_p->read_position;
	write_pos=cons_prod_p->write_position;

	if(read_pos>write_pos)
		data_available=size-read_pos+write_pos;
	else
		data_available=write_pos-read_pos;

	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	return data_available;
}

u32 prod_num_data_to_write(cons_prod_t* cons_prod_p, u32 *n_buffer_data)
{
	u32 room_available,read_pos,write_pos,size;
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores


	size=cons_prod_p->buffer_size;
	read_pos=cons_prod_p->read_position;
	write_pos=cons_prod_p->write_position;

	if(read_pos>write_pos)
		room_available = read_pos - write_pos - 1;
	else
		room_available = size - 1 - write_pos + read_pos;
   
   if (n_buffer_data != NULL) *n_buffer_data = size - room_available;

	Xil_L1DCacheFlushRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	return room_available;
}

void prod_refresh_write(cons_prod_t* cons_prod_p,u32 n_bytes)
{
	u32 read_pos,write_pos,size,data_available_prev_write,int_id,wake_up_ena,cons_cpu_id,threshold;
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	//First, I check if there is nothing to read, in order to wake up consumer when I write.
	write_pos=cons_prod_p->write_position;
	wake_up_ena=cons_prod_p->cons_wake_up_int_ena;
	size=cons_prod_p->buffer_size;

	if(wake_up_ena)
	{
		read_pos=cons_prod_p->read_position;
		int_id=cons_prod_p->signal_ID;
		cons_cpu_id=cons_prod_p->cons_cpu_ID;
		threshold=cons_prod_p->int_threshold;

		if(read_pos>write_pos)
			data_available_prev_write=size-read_pos+write_pos;
		else
			data_available_prev_write=write_pos-read_pos;
	}

	if(n_bytes<(size-write_pos))
		cons_prod_p->write_position+=n_bytes;
	else
		cons_prod_p->write_position=n_bytes-(size-cons_prod_p->write_position);

	Xil_L1DCacheFlushRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	if((data_available_prev_write==0 && wake_up_ena && threshold==0)||(data_available_prev_write+n_bytes)>threshold)
		gic_utils_software_interrupt(int_id,cons_cpu_id);		//CPU SIGNAL!
	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

}

void cons_refresh_read(cons_prod_t* cons_prod_p,u32 n_bytes)
{
	u32 read_pos,size;
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	read_pos=cons_prod_p->read_position;
	size=cons_prod_p->buffer_size;

	if(n_bytes<(size-read_pos))
		cons_prod_p->read_position+=n_bytes;
	else
		cons_prod_p->read_position=n_bytes-(size-cons_prod_p->read_position);

	Xil_L1DCacheFlushRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
}

void cons_set_int_threshold(cons_prod_t* cons_prod_p,u32 threshold)
{
	//LOCK STRUCT
	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	cons_prod_p->int_threshold=threshold;

	Xil_L1DCacheFlushRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Flush a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores
	// UNLOCK STRUCT
	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.
}

void cons_memcpy(cons_prod_t* cons_prod_p,u32 n_bytes,u8* destination_address)
{
	u8 *buf_start;
	u32 read_pos,size;

	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	read_pos=cons_prod_p->read_position;
	size=cons_prod_p->buffer_size;
	buf_start=cons_prod_p->buffer_start_address;

	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	if(n_bytes<(size-read_pos))
	{
		Xil_DCacheInvalidateRange(&buf_start[read_pos],n_bytes);
		memcpy(destination_address,&buf_start[read_pos],n_bytes); // En este caso, el contenido no está partido
	}
	else														  // En este caso sí, hay que coger hasta el final y lo que quede desde el principio
	{
		int data_to_the_end_of_buffer=size-read_pos;
		Xil_DCacheInvalidateRange(&buf_start[read_pos],data_to_the_end_of_buffer);
		memcpy(destination_address,&buf_start[read_pos],data_to_the_end_of_buffer);
		Xil_DCacheInvalidateRange(buf_start,n_bytes-data_to_the_end_of_buffer);
		memcpy(&destination_address[data_to_the_end_of_buffer],buf_start,n_bytes-data_to_the_end_of_buffer);
	}
}
u32 cons_memref(cons_prod_t* cons_prod_p,u32 n_bytes,u8** ref_address_p)
{
	u32 read_pos,size,ret_val;

	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	read_pos=cons_prod_p->read_position;
	size=cons_prod_p->buffer_size;
	*ref_address_p = &cons_prod_p->buffer_start_address[read_pos];

	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	if(n_bytes<(size-read_pos))
		ret_val=n_bytes; // En este caso, el contenido no está partido
	else
		ret_val=size-read_pos; // En este caso, el contenido está partido y se devuelve el número de bytes que no están partidos
	Xil_DCacheInvalidateRange(*ref_address_p,ret_val);
	return ret_val;
}
void prod_memcpy(cons_prod_t* cons_prod_p,u32 n_bytes,u8* source_address)
{
	u8 *buf_start;
	u32 write_pos,size;

	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	write_pos=cons_prod_p->write_position;
	size=cons_prod_p->buffer_size;
	buf_start=cons_prod_p->buffer_start_address;

	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.


	if(n_bytes<(size-write_pos))
	{
		memcpy(&buf_start[write_pos],source_address,n_bytes); // En este caso, el contenido no está partido
		Xil_DCacheFlushRange(&buf_start[write_pos],n_bytes);
		DMB();
		DSB();
	}
	else														  // En este caso sí, hay que coger hasta el final y lo que quede desde el principio
	{
		int data_to_the_end_of_buffer=size-write_pos;
		memcpy(&buf_start[write_pos],source_address,data_to_the_end_of_buffer);
		Xil_DCacheFlushRange(&buf_start[write_pos],data_to_the_end_of_buffer);
		DMB();
		DSB();
		memcpy(buf_start,&source_address[data_to_the_end_of_buffer],n_bytes-data_to_the_end_of_buffer);
		Xil_DCacheFlushRange(buf_start,n_bytes-data_to_the_end_of_buffer);
		DMB();
		DSB();
	}

}
u32 prod_memref(cons_prod_t* cons_prod_p,u32 n_bytes,u8** ref_address_p)
{
	u32 write_pos,size;

	mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	write_pos=cons_prod_p->write_position;
	size=cons_prod_p->buffer_size;

	*ref_address_p = &cons_prod_p->buffer_start_address[write_pos];

	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	if(n_bytes<(size-write_pos))
		return n_bytes; // En este caso, el contenido no está partido
	else
		return size-write_pos; // En este caso, el contenido está partido y se devuelve el número de bytes contiguos
}
u8 *cons_bufref(cons_prod_t* cons_prod_p, u32 n_bytes)
{
u8 *result = NULL;
u8 *buf_start;
u32 write_pos, read_pos, size, data_available;

	if (n_bytes > SIZE_BUFFER_COMMAND)
   {
      ELOG("Error, overflow swap receiver buffer", -1);
      return NULL;
   }
   mutex_utils_blocking_LOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

	Xil_L1DCacheInvalidateRange(cons_prod_p,SIZE_PER_STRUCT_ALIGN);	// Invalidamos  a la caché nivel 1, la nivel 2 no hace falta ya que es compartida entre los dos procesadores

	write_pos=cons_prod_p->write_position;
	size=cons_prod_p->buffer_size;
	buf_start=cons_prod_p->buffer_start_address;
	read_pos=cons_prod_p->read_position;
	write_pos=cons_prod_p->write_position;

	if (read_pos > write_pos)
		data_available = size - read_pos + write_pos;
	else
		data_available = write_pos - read_pos;

	mutex_utils_UNLOCK(cons_prod_p->mutex_ID);// mutex_ID no se debe cambiar, se puede acceder a él sin invalidar caché ni flush.

   if (n_bytes <= data_available)
   {
      if(n_bytes<(size-read_pos))
      {
         Xil_DCacheInvalidateRange(&buf_start[read_pos],n_bytes);
         memcpy(gb_buffer_command,&buf_start[read_pos],n_bytes); // En este caso, el contenido no está partido
      }
      else														  // En este caso sí, hay que coger hasta el final y lo que quede desde el principio
      {
         int data_to_the_end_of_buffer=size-read_pos;
         Xil_DCacheInvalidateRange(&buf_start[read_pos],data_to_the_end_of_buffer);
         memcpy(gb_buffer_command,&buf_start[read_pos],data_to_the_end_of_buffer);
         Xil_DCacheInvalidateRange(buf_start,n_bytes-data_to_the_end_of_buffer);
         memcpy(&gb_buffer_command[data_to_the_end_of_buffer],buf_start,n_bytes-data_to_the_end_of_buffer);
      }
      result = gb_buffer_command;
   }
   return result;
}
