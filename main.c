/*Copyright (C)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * francisco dot rodriguez at ingenieria dot unam dot edu
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#ifndef DBG_HELP
// Poner a cero para desactivar la ayuda de depuración
#define DBG_HELP 1
#endif

// NO TOCAR:
#if DBG_HELP > 0
#define DBG_PRINT( ... ) do{ fprintf( stderr, "DBG:" __VA_ARGS__ ); } while( 0 )
#else
#define DBG_PRINT( ... ) ;
#endif


// Es la función hash
static int h( int key, size_t m )
{
   return key % m;
}

// es la función de resolución de colisiones
static int probe( int key, size_t i )
{
   return i + 1;
}



/**
 * @brief Estado de la celda. Está codificado en el campo |id| de @see Entry_table
 */
enum
{
   EMPTY_CELL =   -1,
   DELETED_CELL = -2,
};


typedef struct
{
   int id;       ///< Entero positivo. El estado de la celda está codificado en la parte negativa
   float salary;
} Entry_table;

typedef struct
{
	Entry_table* table; ///< Es la tabla hash
	size_t  capacity;      ///< Es el número máximo de elementos que la tabla puede almacenar
	size_t  len;           ///< Es el número actual de elementos en la tabla
} Hash_table;

// for debugging purposes only!
static void print_hash_table( const Hash_table* ht )
{
   printf( "----------------------------------------\n" );
   printf( "HT.Capacity: %ld\n", ht->capacity );
   printf( "HT.Len: %ld\n", ht->len );
   printf( "HT.Table:\n" );
   for( size_t i = 0; i < ht->capacity; ++i )
   {
      printf( "[%02ld] (%d, %0.2f)\n", i, ht->table[ i ].id, ht->table[ i ].salary );
   }
   printf( "----------------------------------------\n\n" );
}



/**
 * @brief Crea una nueva tabla hash para la tupla (id, salary)
 *
 * @param capacity Número de celdas (slots) que tendrá la tabla. Es recomendable utilizar
 * un valor mayor al número de elementos que se quiere guardar y también que sea un
 * número primo.
 *
 * @return Una referencia a una tabla hash
 */
Hash_table* HT_New( size_t capacity )
{
   Hash_table* ht = ( Hash_table* )malloc( sizeof( Hash_table ) );
   if( NULL != ht )
   {
      ht->len = 0;
      ht->capacity = capacity;

      ht->table = ( Entry_table* ) malloc( capacity * sizeof( Entry_table ) );
      if( NULL != ht->table )
      {
         for( int i = 0; i < ht->capacity; ++i )
         {
            ht->table[ i ].id = EMPTY_CELL;
            // en esta aplicación los ID's son enteros positivos

            ht->table[ i ].salary = 0.0;
         }
      }
      else
      {
         free( ht );
         ht = NULL;
      }
   }

   return ht;
   // el cliente es responsable de verificar que efectivamente la tabla se creó
}

/**
 * @brief Destruye una tabla hash
 *
 * @param ht La dirección de una referencia a una tabla hash
 */
void HT_Delete( Hash_table** ht )
{
   assert( ht );

   free( (*ht)->table );
   free( *ht );
   *ht = NULL;
}

/**
 * @brief Inserta el campo |salary| en la tabla hash.
 *
 * @param ht Referencia a una tabla hash
 * @param id ID del empleado
 * @param salary Salario del empleado
 *
 * @return true si el elemento pudo ser insertado; false en caso contrario (elemento
 * duplicado, tabla llena, etc).
 */
bool HT_Insert( Hash_table* ht, int id, float salary )
{
   assert( ht );
   assert( ht->len < ht->capacity );

   int pos;
   // índice que se actualizará en cada colisión

   int home = pos = h( id, ht->capacity );
   // calcula una hash key base

   DBG_PRINT( "HT_Insert: Calculé el valor hash: %d para la llave: %d\n", pos, id );
   // información de depuración

   int i = 0;

   // si el slot está desocupado, se salta el while; en caso contrario entra a buscar uno.
   // Asumimos que hay más slots que datos a guardar:
   while( ht->table[ pos ].id >= 0 )
   {
      // no aceptamos duplicados:
      if( ht->table[ pos ].id == id )
      {
         DBG_PRINT( "HT_Insert: Error: Llave duplicada\n" );
         return false;
      }

      pos = ( home + probe( id, i ) ) % ht->capacity;
      ++i;

      DBG_PRINT( "HT_Insert: Recalculé el valor hash: %d para la llave: %d\n", pos, id );
   }

   ht->table[ pos ].id = id;
   ht->table[ pos ].salary = salary;

   ++ht->len;

   return true;
}

/**
 * @brief Busca en la tabla hash el salario correspondiente a la llave ID del empleado.
 *
 * @param ht Referencia a una tabla hash.
 * @param id ID del empleado.
 * @param salary Argumento que guardará el salario del empleado (en caso de que éste exista en la tabla hash)
 *
 * @return true si el elemento fue encontrado; false en caso contrario.
 */
bool HT_Search( const Hash_table* ht, int id, float *salary )
{
   assert( ht );
   assert( ht->len > 0 );

   int home = h( id, ht->capacity );
   int pos = home;

   DBG_PRINT( "HT_Search: Calculé el valor hash: %d para la llave: %d\n", pos, id );

   bool found = false;
   if( ht->table[ pos ].id == id )
   {
      found = true;
   }
   else if( ht->table[ pos ].id == EMPTY_CELL )
   {
      found = false;
   }
   else
   {
      int i = 0;
      while( ht->table[ pos ].id != EMPTY_CELL && found == false )
      {
         pos = ( home + probe( id, i ) ) % ht->capacity;
         ++i;

         DBG_PRINT( "HT_Search: Recalculé el valor hash: %d para la llave: %d\n", pos, id );

         if( ht->table[ pos ].id == id )
         {
            found = true;
         }
      }
   }

   bool ret_val = false;
   if( found )
   {
      *salary = ht->table[ pos ].salary;
      ret_val = true;
   }
   return ret_val;
}

/**
 * @brief Elimina una entrada en la tabla hash.
 *
 * @param ht Referencia a una tabla hash.
 * @param id La llave de la entrada que se desea eliminar.
 *
 * @return   true si el elemento existía; false si el elemento no existe.
 */
bool HT_Remove( const Hash_table* ht, int id )
{
   return false;
}


#define HASH_TABLE_SIZE 10

int main()
{
   Hash_table* by_salary = HT_New( HASH_TABLE_SIZE );

   HT_Insert( by_salary, 1234, 13500.0 );
   HT_Insert( by_salary, 2345, 14650.0 );
   HT_Insert( by_salary, 9876, 16560.0 );
   HT_Insert( by_salary, 8765, 19876.0 );
   HT_Insert( by_salary, 7650, 11000.0 );
   HT_Insert( by_salary, 5665, 13500.0 );

   print_hash_table( by_salary );

   int id = 8765;
   float salary;
   bool ret_val = HT_Search( by_salary, id, &salary );
   if( ret_val )
   {
      printf( "El salario del empleado con ID=%d es: $%0.2f\n", id, salary );
   }
   else
   {
      printf( "El empleado con ID=%d no está en mis registros\n", id );
   }

   // también podemos preguntar únicamente por la existencia del empleado:
   id = 5000;
   printf( "El empleado con ID=%d [%s] está en mis registros\n", id,
         HT_Search( by_salary, id, &salary ) ? "SÍ" : "NO" );


// Activar cuando la función Remove() haya sido escrita
#if 0 
   if( HT_Remove( by_salary, 1234 ) )
   {
      printf( "Elemento eliminado\n" );
   }
   else
   {
      printf( "El elemento no pudo ser eliminado porque no existe\n" );
   }

   print_hash_table( by_salary );
#endif  


   HT_Delete( &by_salary );
}
