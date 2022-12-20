#pragma once
#include <stdint.h>
#include <stdio.h>
#include "thrift.h"


#define PARQUET_BOOLEAN 0
#define PARQUET_INT32 1
#define PARQUET_INT64 2
#define PARQUET_INT96 3
#define PARQUET_FLOAT 4
#define PARQUET_DOUBLE 5
#define PARQUET_BYTE_ARRAY 6
#define PARQUET_FIXED_LEN_BYTE_ARRAY 7

#define PARQUET_ID_TYPE 1
#define PARQUET_ID_NAME 4



/** a BYTE_ARRAY actually contains UTF8 encoded chars */
#define PARQUET_CONVERTET_TYPE_UTF8 0
/** a map is converted as an optional field containing a repeated key/value pair */
#define PARQUET_CONVERTET_TYPE_MAP 1
/** a key/value pair is converted into a group of two fields */
#define PARQUET_CONVERTET_TYPE_MAP_KEY_VALUE 2
/** a list is converted into an optional field containing a repeated field for its values */
#define PARQUET_CONVERTET_TYPE_LIST 3
/** an enum is converted into a binary field */
#define PARQUET_CONVERTET_TYPE_ENUM 4
/**
* A decimal value.
*
* This may be used to annotate binary or fixed primitive types. The
* underlying byte array stores the unscaled value encoded as two's
* complement using big-endian byte order (the most significant byte is the
* zeroth element). The value of the decimal is the value * 10^{-scale}.
*
* This must be accompanied by a (maximum) precision and a scale in the
* SchemaElement. The precision specifies the number of digits in the decimal
* and the scale stores the location of the decimal point. For example 1.23
* would have precision 3 (3 total digits) and scale 2 (the decimal point is
* 2 digits over).
*/
#define PARQUET_CONVERTET_TYPE_DECIMAL 5
/** A Date.Stored as days since Unix epoch, encoded as the INT32 physical type.*/
#define PARQUET_CONVERTET_TYPE_DATE 6
/** A time The total number of milliseconds since midnight.  The value is storedas an INT32 physical type.*/
#define PARQUET_CONVERTET_TYPE_TIME_MILLIS 7
/** A time. The total number of microseconds since midnight. The value is stored as an INT64 physical type. */
#define PARQUET_CONVERTET_TYPE_TIME_MICROS 8
/**
* A date/time combination
*
* Date and time recorded as milliseconds since the Unix epoch.  Recorded as
* a physical type of INT64.
*/
#define PARQUET_CONVERTET_TYPE_TIMESTAMP_MILLIS 9
/**
* A date/time combination
*
* Date and time recorded as microseconds since the Unix epoch.  The value is
* stored as an INT64 physical type.
*/
#define PARQUET_CONVERTET_TYPE_TIMESTAMP_MICROS 10
/**
* An unsigned integer value.
*
* The number describes the maximum number of meainful data bits in
* the stored value. 8, 16 and 32 bit values are stored using the
* INT32 physical type.  64 bit values are stored using the INT64
* physical type.
*
*/
#define PARQUET_CONVERTET_TYPE_UINT_8 11
#define PARQUET_CONVERTET_TYPE_UINT_16 12
#define PARQUET_CONVERTET_TYPE_UINT_32 13
#define PARQUET_CONVERTET_TYPE_UINT_64 14
/**
* A signed integer value.
*
* The number describes the maximum number of meainful data bits in
* the stored value. 8, 16 and 32 bit values are stored using the
* INT32 physical type.  64 bit values are stored using the INT64
* physical type.
*
*/
#define PARQUET_CONVERTET_TYPE_INT_8 15
#define PARQUET_CONVERTET_TYPE_INT_16 16
#define PARQUET_CONVERTET_TYPE_INT_32 17
#define PARQUET_CONVERTET_TYPE_INT_64 18

/** An embedded JSON document. A JSON document embedded within a single UTF8 column.*/
#define PARQUET_CONVERTET_TYPE_JSON 19

/** An embedded BSON document.A BSON document embedded within a single BINARY column.*/
#define PARQUET_CONVERTET_TYPE_BSON 20

/**
* An interval of time
*
* This type annotates data stored as a FIXED_LEN_BYTE_ARRAY of length 12
* This data is composed of three separate little endian unsigned
* integers.  Each stores a component of a duration of time.  The first
* integer identifies the number of months associated with the duration,
* the second identifies the number of days associated with the duration
* and the third identifies the number of milliseconds associated with
* the provided duration.  This duration of time is independent of any
* particular timezone or date.
*/
#define PARQUET_CONVERTET_TYPE_INTERVAL 21







// https://parquet.apache.org/docs/file-format/metadata/

typedef struct
{
	int32_t version; // id=1,
	ecs_vector_t *schema; // id=2, schema<parquet_schema_element_t>
	int64_t num_rows; // id=3
	ecs_vector_t *row_groups; // id=4, row_groups<parquet_rowgroup_t>
	ecs_vector_t *key_value_metadata; // id=5, key_value_metadata<parquet_rowgroup_t>
} parquet_filemetadata_t;

typedef struct
{
	int type;
	int32_t type_length;
	int32_t repetition_type;
	ecs_string_t name;
	int32_t num_children;
	int converted_type;
} parquet_schema_element_t;

typedef struct
{
	ecs_vector_t *columns;
	int64_t total_byte_size;
	int64_t num_rows;
} parquet_rowgroup_t;

typedef struct
{
	ecs_string_t file_path;
	int64_t file_offset;
} parquet_column_chunk_t;

typedef struct
{
	int type;
	ecs_vector_t *encodings; // encodings<int>
	ecs_vector_t *path_in_schema; // path_in_schema<ecs_string_t>
	int codec;
	int64_t num_values;
	int64_t total_uncompressed_size;
	int64_t total_compressed_size;
	ecs_vector_t *key_value_metadata; // key_value_metadata<parquet_rowgroup_t>
} parquet_column_meta_data_t;




typedef struct
{
	struct thrift_context footer;
	parquet_filemetadata_t meta;
} parquet_reader_t;












void parquet_read(parquet_reader_t *reader, char const * filename);
void parquet_testcase1();


