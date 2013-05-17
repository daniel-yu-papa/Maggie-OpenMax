#ifndef __OMX_CLASS_MAGIC_H__
#define __OMX_CLASS_MAGIC_H__

#define CLASS(a) typedef struct a a; \
                 struct a {
                    
#define DERIVEDCLASS(child, parent) typedef struct child child; \
                                    struct child {

#define ENDCLASS(a) a##_FIELDS };

/*
* example: CLASS(root)
* CLASS(root)
*
* #define root_FIELDS \
* int a; \
* char b[10];
*
*ENDCLASS(root)
*/

/*
* example: DERIVEDCLASS(child, root)
* DERIVEDCLASS(child, root)
*
* #define child_FIELDS      \
* root_FIELDS     \
* int c_a; \
* char c_b[20];
*
* ENDCLASS(child)
*/
#endif