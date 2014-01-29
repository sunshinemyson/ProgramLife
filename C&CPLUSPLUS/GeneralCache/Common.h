#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifndef Cnt_Of_Array
#define Cnt_Of_Array( _Array ) ( sizeof( _Array )/sizeof( _Array[0] ) )
#endif // Cnt_Of_Array

#ifndef SINGLE_FLOW_BEGIN
#define SINGLE_FLOW_BEGIN do{
#endif // SINGLE_FLOW_BEGIN

#ifndef SINGLE_FLOW_END
#define SINGLE_FLOW_END }while(0);
#endif

#ifndef BREAK_IF_FALSE
#define BREAK_IF_FALSE(_C) if( !(_C) ) break
#endif // BREAK_IF_FALSE

#ifndef BREAK_IF_TRUE
#define BREAK_IF_TRUE(_C) if(_C) break
#endif // BREAK_IF_TRUE

#ifndef bool
typedef enum{
    false = 0,
    true = !false
}bool;
#endif // bool

#ifndef NULL
#define NULL ((void*)(0))
#endif // NULL

#endif // COMMON_H_INCLUDED
