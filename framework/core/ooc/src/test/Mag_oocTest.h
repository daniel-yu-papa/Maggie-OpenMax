#ifndef __OOC_TEST_H__
#define __OOC_TEST_H__

#include "Mag_ooc.h"

/*Parent class: Polygon*/
DeclareClass(Polygon, Base);

Virtuals(Polygon, Base) 
    int (*getEdges)(/*Polygon*/ void *thiz);
EndOfVirtuals;

ClassMembers(Polygon, Base, \
    int (*setColor)(Polygon thiz, 
                    int colour); \
    int (*getColor)(Polygon thiz); \
    void (*setOwner)(Polygon thiz, 
                     char *owner); \
    char *(*getOwner)(Polygon thiz); \
)
    char *mpOwner;
    int mEdgeNum;
    int mColor;
EndOfClassMembers;

/*Child class: Triangle*/
DeclareClass(Triangle, Polygon);

Virtuals(Triangle, Polygon) 
    int (*sellToCustomer)(Triangle thiz, int money);
    /*override the virtual function: int (*getEdges)(Polygon thiz);*/
EndOfVirtuals;

ClassMembers(Triangle, Polygon, \
    int (*bidBasePrice)(Triangle thiz, int price); \
)
    int mBasePrice;
    int mTotalPrice;
    int mEdgeNum;
EndOfClassMembers;   

/*Child-child class: TriangleInGold*/
DeclareClass(TriangleInGold, Triangle);

Virtuals(TriangleInGold, Triangle) 
    /*override the virtual function: int (*sellToCustomer)(Triangle thiz, int money);*/
EndOfVirtuals;

ClassMembers(TriangleInGold, Triangle, \
    int (*bidBasePrice)(TriangleInGold thiz, int price); \
)
    int mTotalPrice;
    int mBasePrice;
EndOfClassMembers;   


#endif