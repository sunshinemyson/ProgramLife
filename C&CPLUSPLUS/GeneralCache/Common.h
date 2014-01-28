#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifndef Cnt_Of_Array
#define Cnt_Of_Array( _Array ) ( sizeof( _Array )/sizeof( _Array[0] ) )
#endif // Cnt_Of_Array

#ifndef SINGLE_FLOW_BEGIN
#define SINGLE_FLOW_BEGIN do{
#endif // SINGLE_FLOW_BEGIN

#ifndef SINGLE_FLOW_END
#define SINGLE_FLOW_END while(0);
#endif

#endif // COMMON_H_INCLUDED
