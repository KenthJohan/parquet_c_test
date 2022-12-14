# Parquete
https://parquet.apache.org/docs/file-format/metadata/

# Thrift
message               => protocol-id version-and-type seq-id method-name struct-encoding 
protocol-id           => 0x82
version-and-type      => (5-bit version identifier) (3-bit type identifier)
seq-id                => varint
method-name           => varint (N-byte string)
struct-encoding       => field_list stop
field_list            => field field_list | field
field                 => type-and-id value
type-and-id           => field-id-delta type-header | 0 type-header zigzag-varint
field-id-delta        => (4-bit offset from preceding field id, 1-15)
type-header           => boolean-true | boolean-false | 
                         byte-type-header | i16-type-header | i32-type-header | i64-type-header | 
                         double-type-header | string-type-header | binary-type-header | 
                         list-type-header | set-type-header | map-type-header | 
                         struct-type-header | 
                         extended-type-header
value                 => boolean-true | boolean-false | byte | i16 | i32 | i64 | double | string | binary | list | set | map | struct
stop                  => 0x0
boolean-true          => 0x1
boolean-false         => 0x2
byte-type-header      => 0x3
i16-type-header       => 0x4
i32-type-header       => 0x5
i64-type-header       => 0x6
double-type-header    => 0x7
binary-type-header    => 0x8
string-type-header    => binary-type-header
list-type-header      => 0x9
set-type-header       => 0xA
map-type-header       => 0xB
struct-type-header    => 0xC
extended-type-header  => 0xF
byte                  => (1-byte value)
i16                   => zigzag-varint
i32                   => zigzag-varint
i64                   => zigzag-varint
double                => (8-byte double)
binary                => varint(size) (bytes)
string                => (utf-8 encoded)binary
list                  => short-list | long-list
short-list            => (4-bit size 0-14) (element)type-header
long-list             => 0xf type-header varint(size - 15)
set                   => list
list-body             => value list-body | value
map                   => varint (key)type-header (value)type-header key-value-pair-list | 0
key-value-pair-list   => key-value-pair key-value-pair-list | key-value-pair
key-value-pair        => (key)value (value)value
